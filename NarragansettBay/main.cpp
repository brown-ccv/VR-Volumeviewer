
// OpenGL platform-specific headers
#if defined(WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <gl/GLU.h>
#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#endif


#define GLOBAL_SCALE 0.001
#define XY_SCALE GLOBAL_SCALE 
#define DEPTH_SCALE XY_SCALE
#define DEPTH_MULTIPLIER 100
#define THICKNESS XY_SCALE *30
#define MOVE_SCALE XY_SCALE  * 250


// MinVR header
#include <api/MinVR.h>
#include "main/VREventInternal.h"
#include "tinyxml2.h"
#include "VRFontHandler.h"
using namespace MinVR;

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// Just included for some simple Matrix math used below
// This is not required for use of MinVR in general
#include <math/VRMath.h>

#include <GL/gle.h>

struct pt {
	float vertex[3];
	float color[3];
	float value;
};
struct triangle {
	int idx[3];
};
std::vector<std::string> values_legend = {
	"Temperature (degrees C)",
	"Salinity (parts per thousand)",
	"Density (kg / m ^ 3)",
	"Chl fluorescence (relative counts)",
	"Backscattering (relative counts)",
	"Beam attenuation (m^-1)",
	"Sunlight (W / m ^ 2)"
};

//std::vector<int> date [5];
std::vector<string> marker;
std::vector<double> marker_idx;
std::vector<double> min_max[2];
bool min_max_set = false;
bool has_mesh = false;
std::vector< std::vector <pt> >points[2];
std::vector< std::vector <pt> >points_mesh;
std::vector< std::vector <triangle> >triangles_mesh;

double pts[300000][3];
float col[300000][3];
float col_alpha[300000][4];
int currentScale = 0;
bool measuring = false;
bool play = false;
unsigned int currentFrame;




/** MyVRApp is a subclass of VRApp and overrides two key methods: 1. onVREvent(..)
    and 2. onVRRenderGraphics(..).  This is all that is needed to create a
    simple graphics-based VR application and run it on any display configured
    for use with MinVR.
 */
class MyVRApp : public VRApp {
public:
	MyVRApp(int argc, char** argv) : VRApp(argc, argv), isInitialised(false),movement_y(0.0), movement_x(0.0), currentList(0){
		computeCenter();
    }

    virtual ~MyVRApp() {}

	bool startsWith(std::string string1, std::string string2)
	{
		if (strlen(string1.c_str()) < strlen(string2.c_str())) return false;

		return !strncmp(string1.c_str(), string2.c_str(), strlen(string2.c_str()));
	}
	
	template <typename T> int sgn(T val) {
		return (T(0) < val) - (val < T(0));
	}

	// Callback for event handling, inherited from VRApp
	virtual void onVREvent(const VREvent &event) {
		if (event.getName() == "HTC_HMD_1"){
			if (event.getInternal()->getDataIndex()->exists("/HTC_HMD_1/State/Pose")){
				headpose = event.getInternal()->getDataIndex()->getValue("/HTC_HMD_1/State/Pose");
			}
		}
		if (event.getName() == "HTC_Controller_Right_AButton_Pressed"){
			currentList++;
			if (currentList >= list[0].size())currentList = 0;
		}

		if (event.getName() == "HTC_Controller_Right_Axis1Button_Pressed"){
			measuring = true;
		}

		if (event.getName() == "HTC_Controller_Right_Axis1Button_Released"){
			measuring = false;
		}

		//if (event.getName() == "HTC_Controller_Right_GripButton_Pressed"){
		if (event.getName() == "HTC_Controller_Right_ApplicationMenuButton_Pressed"){
			currentScale++;
			if (currentScale > 1)currentScale = 0;
		}

		if (event.getName() == "HTC_Controller_Right_Axis0Button_Pressed"){
			play = !play;
		}


		if (event.getName() == "HTC_Controller_Right")
		{
			if (event.getInternal()->getDataIndex()->exists("/HTC_Controller_Right/Pose")){
				controllerpose = event.getDataAsFloatArray("Pose");
			}
			//if (event.getInternal()->getDataIndex()->exists("/HTC_Controller_1/State/Axis0Button_Pressed") &&
			//	(int)event.getInternal()->getDataIndex()->getValue("/HTC_Controller_1/State/Axis0Button_Pressed"))
			{
				movement_x = event.getInternal()->getDataIndex()->getValue("/HTC_Controller_Right/State/Axis0/XPos");
				movement_y = event.getInternal()->getDataIndex()->getValue("/HTC_Controller_Right/State/Axis0/YPos");
			}
			//else
			//{
			//	movement_y = 0;
			//	movement_x = 0;
			//}
		}

		if (event.getName() == "KbdEsc_Down") {
            shutdown();
            return;
		}
        
	}

	void computeClosestPoint()
	{
		closestPoint = -1;
		VRPoint3 pos_tmp = roompose.inverse() * controllerpose * VRPoint3(0, 0, 0);
		VRVector3 pos = VRVector3(pos_tmp.x, pos_tmp.y, pos_tmp.z);
		VRVector3 pos2 = pos + roompose.inverse() * controllerpose * VRVector3(0, 0, -5);
		double dist = 100;
		double d = (pos2 - pos).length();
		float dist2 = 30;
		for (int i = 0; i < points[currentScale][0].size(); i++)
		{
			VRVector3 pt_vector3 = VRVector3(points[currentScale][0][i].vertex);
			double dist_tmp = (pt_vector3 - pos).cross(pt_vector3 - pos2).length() / d;

			if (dist_tmp < dist)
			{
				VRPoint3 pt_point3 = VRPoint3(pt_vector3.x, pt_vector3.y, pt_vector3.z);
				double dist2_tmp = -(controllerpose.inverse() * roompose * pt_point3).z;
				if (dist2_tmp >0){
					dist = dist_tmp;
					dist2 = dist2_tmp;
					closestPoint = i;
				}
			}
		}

		//std::cerr << closestPoint << " " << dist << std::endl;
	}

	// Callback for rendering, inherited from VRRenderHandler
	

	

	

	void drawTriangles(int id)
	{
		float min = 100000000;
		float max = -100000000;

		for (int i = 0; i < points_mesh[id].size(); i++)
		{
			if (points_mesh[id][i].value != -9999999 && min > points_mesh[id][i].value) min = points_mesh[id][i].value;
			if (points_mesh[id][i].value != -9999999 && max < points_mesh[id][i].value) max = points_mesh[id][i].value;
		}

		float a;
		for (int i = 0; i < points_mesh[id].size(); i++)
		{
			/*if (id == 0)
				colormap_jet(points_mesh[id][i].value, min_max[0][4], min_max[1][4], r, g, b);
			else if (id == 1)
				colormap_jet(points_mesh[id][i].value, min_max[0][3], min_max[1][3], r, g, b);
			else if (id == 2)
				colormap_jet(points_mesh[id][i].value, min_max[0][0], min_max[0][1], r, g, b);
			*/
			colormap_jet(points_mesh[id][i].value, min, max, points_mesh[id][i].color[0], points_mesh[id][i].color[1], points_mesh[id][i].color[2],a);
		}

		glBegin(GL_TRIANGLES);
		for (int i = 0; i < triangles_mesh[id].size(); i++)
		{
			glColor4f(points_mesh[id][triangles_mesh[id][i].idx[0]].color[0], points_mesh[id][triangles_mesh[id][i].idx[0]].color[1], points_mesh[id][triangles_mesh[id][i].idx[0]].color[2],0.7);
			glVertex3d(points_mesh[id][triangles_mesh[id][i].idx[0]].vertex[0], points_mesh[id][triangles_mesh[id][i].idx[0]].vertex[1], points_mesh[id][triangles_mesh[id][i].idx[0]].vertex[2]);
			glColor4f(points_mesh[id][triangles_mesh[id][i].idx[1]].color[0], points_mesh[id][triangles_mesh[id][i].idx[1]].color[1], points_mesh[id][triangles_mesh[id][i].idx[1]].color[2], 0.7);
			glVertex3d(points_mesh[id][triangles_mesh[id][i].idx[1]].vertex[0], points_mesh[id][triangles_mesh[id][i].idx[1]].vertex[1], points_mesh[id][triangles_mesh[id][i].idx[1]].vertex[2]);
			glColor4f(points_mesh[id][triangles_mesh[id][i].idx[2]].color[0], points_mesh[id][triangles_mesh[id][i].idx[2]].color[1], points_mesh[id][triangles_mesh[id][i].idx[2]].color[2], 0.7);
			glVertex3d(points_mesh[id][triangles_mesh[id][i].idx[2]].vertex[0], points_mesh[id][triangles_mesh[id][i].idx[2]].vertex[1], points_mesh[id][triangles_mesh[id][i].idx[2]].vertex[2]);

		}
		glEnd();
	}

	

	

	

protected:
	
	bool isInitialised;
	std::vector <unsigned int> list[2];
	std::vector <unsigned int> list_thin[2];
	std::vector <unsigned int> list_mesh;
	VRMatrix4 controllerpose;
	VRMatrix4 roompose;
	VRMatrix4 headpose;
	unsigned int currentList;
	float movement_y, movement_x;
	int closestPoint;
};

void loadTriangles(std::string file, int id)
{
	if (id >= triangles_mesh.size()){
		std::vector <triangle> tri;
		triangles_mesh.push_back(tri);
	}

	std::ifstream fin(file.c_str());
	std::istringstream in;
	std::string line;
	int count = 0;
	bool good = true;
	while (!safeGetline(fin, line).eof() && good)
	{
		in.clear();
		in.str(line);
		std::vector<int> tmp;
		for (int value; in >> value; comma(in))
		{
			tmp.push_back(value);
		}

		
		triangle p;
		p.idx[0] = tmp[0];
		p.idx[1] = tmp[1];
		p.idx[2] = tmp[2];

		triangles_mesh[id].push_back(p);

		line.clear();
		tmp.clear();
		count++;		
	}

	fin.close();
}


