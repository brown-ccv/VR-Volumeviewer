#include "../include/vrapp/VolumeVisualizationApp.h"

int main(int argc, char **argv) {
    
   if (argc > 1) {
		if (strcmp(argv[1], "-t") == 0) {
		
        std::cout << "VR-Volumeviewer exists" << std::endl;
        return 0;

		}
	}


    // argc, argv need to be passed to the app so that MinVR can parse command
    // line args to see which config files to load.
    
    int num_parameters = 4;
    char * arguments[] = { argv[0],"-c","desktop_observer.minvr", "use2DUI"};

	VolumeVisualizationApp app(num_parameters, arguments);
 

    // Does not return until the program shuts down.
    app.run();

    return 0;
}