#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "FontHandler.h"
#include <iostream>
#include <time.h>
#include <iomanip>
#include <sstream>
#include <cmath>

#define TEXTBORDER 0.003

FontHandler* FontHandler::instance = NULL;

FontHandler::FontHandler()
{
	font= new FTGLPolygonFont("calibri.ttf");

	if (font->Error()){
		std::cerr << "Font load error" << std::endl;
	}
	font->FaceSize(10);

	float ll[3], ur[3];
	std::string test = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	font->BBox(test.c_str(), ll[0], ll[1], ll[2], ur[0], ur[1], ur[2]);
	m_fontMinMax[0] = ll[1];
	m_fontMinMax[1] = ur[1];
}

FontHandler::~FontHandler()
{
	instance = NULL;
	delete font;
}

FontHandler* FontHandler::getInstance()
{
	if (!instance)
	{
		instance = new FontHandler();
	}
	return instance;
}

FTFont* FontHandler::getFont()
{
	return font;
}

void FontHandler::renderTextBox(std::string text, double x, double y, double z, double width, double height, TextAlignment alignment, bool rotateY)
{
	float ll[3], ur[3], fontWidth, fontHeight;
	float scale = 10000.0;
	font->BBox(text.c_str(), ll[0], ll[1], ll[2], ur[0], ur[1], ur[2]);
	fontWidth = (ur[0] - ll[0]) * scale;
	fontHeight = (ur[1] - ll[1]) * scale;

	if (fontWidth > (width - 2.0*TEXTBORDER) || fontHeight > (height - 2.0*TEXTBORDER))
	{
		float scale_x = (width - 2.0*TEXTBORDER) / fontWidth * scale;
		float scale_y = (height - 2.0*TEXTBORDER) / fontHeight * scale;

		scale = (scale_x < scale_y) ? scale_x : scale_y;
		scale = scale;
		fontWidth = (ur[0] - ll[0]) * scale;
		fontHeight = (ur[1] - ll[1]) * scale;
	}

	double off_x = (width  - fontWidth) / 2.0f;
	double off_y = (height - fontHeight) / 2.0f;  //Bounding box isn't centered, so we need to add fudge factor

	if (alignment == TextAlignment::LEFT) off_x = TEXTBORDER;
	if (alignment == TextAlignment::RIGHT) off_x = width - TEXTBORDER - fontWidth;

	off_x -= ll[0] * scale;
	off_y -= ll[1] * scale;

	glPushMatrix();
	glTranslatef(x + off_x, y + off_y, z);
	glScalef(scale, scale, scale);
	if (rotateY)glRotatef(180, 0, 1, 0);
	glEnable(GL_POLYGON_SMOOTH);
	font->Render(text.c_str());
	glDisable(GL_POLYGON_SMOOTH);
	glPopMatrix();
}

void FontHandler::renderMultiLineTextBox(std::vector<std::string> text, double x, double y, double z, double width, double height, TextAlignment alignment, bool rotateY)
{
	if (text.size() == 0)
		return;

	float ll[3], ur[3], fontWidth, fontHeight;
	float scale = 10000.0;
	double textheight = height / text.size();

	for (std::vector <std::string>::const_iterator it = text.begin(); it != text.end(); ++it){
		font->BBox(it->c_str(), ll[0], ll[1], ll[2], ur[0], ur[1], ur[2]);
		fontWidth = (ur[0] - ll[0]) * scale;
		
		if (fontWidth > (width - 2.0*TEXTBORDER))
		{
			scale = (width - 2.0*TEXTBORDER) / fontWidth * scale;
		}
	}
	
	fontHeight = (m_fontMinMax[1] - m_fontMinMax[0]) * scale;
	if (fontHeight > (textheight - 2.0*TEXTBORDER))
	{
		scale = (textheight - 2.0*TEXTBORDER) / fontHeight * scale;
		fontHeight = (m_fontMinMax[1] - m_fontMinMax[0]) * scale;
	}

	double off_y = (textheight - fontHeight) / 2.0f - m_fontMinMax[0] * scale;  //Bounding box isn't centered, so we need to add fudge factor
	
	for (int i = 0; i < text.size(); i++){
		font->BBox(text[i].c_str(), ll[0], ll[1], ll[2], ur[0], ur[1], ur[2]);
		fontWidth = (ur[0] - ll[0]) * scale;
		double off_x = (width  - fontWidth) / 2.0f;
		
		if (alignment == TextAlignment::LEFT) off_x = TEXTBORDER;
		if (alignment == TextAlignment::RIGHT) off_x = width - TEXTBORDER - fontWidth;
		off_x -= ll[0] * scale;
		
		glPushMatrix();
		glTranslatef(x + off_x, y + off_y + (text.size() - i - 1) * textheight, z);
		glScalef(scale, scale, scale);
		if (rotateY)glRotatef(180, 0, 1, 0);
		glEnable(GL_POLYGON_SMOOTH);
		font->Render(text[i].c_str());
		glDisable(GL_POLYGON_SMOOTH);
		glPopMatrix();
	}
}

void FontHandler::renderTextBox2D(std::string text, double x, double y, double width, double height, TextAlignment alignment, bool rotateY) {
	GLint viewport[4];
	GLfloat projection[16];
	GLfloat modelview[16];

	glGetIntegerv(GL_VIEWPORT, &viewport[0]);
	glGetFloatv(GL_PROJECTION_MATRIX, &projection[0]);
	glGetFloatv(GL_MODELVIEW_MATRIX, &modelview[0]);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, viewport[2], 0, viewport[3], -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	renderTextBox(text,x, y, 0, width, height, alignment, rotateY);
	glEnable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projection);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelview);
}


void FontHandler::renderMultiLineTextBox2D(std::vector<std::string> btext, double x, double y, double width, double height, TextAlignment alignment, bool rotateY) {
	GLint viewport[4];
	GLfloat projection[16];
	GLfloat modelview[16];

	glGetIntegerv(GL_VIEWPORT, &viewport[0]);
	glGetFloatv(GL_PROJECTION_MATRIX, &projection[0]);
	glGetFloatv(GL_MODELVIEW_MATRIX, &modelview[0]);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, viewport[2], 0, viewport[3], -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderMultiLineTextBox(btext, x, y, 0, width, height, alignment, rotateY);
	glEnable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projection);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelview);
}

std::string months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };


void drawAnalogClock(int x_pos, int y_pos, int radius, tm * time) {
	GLint viewport[4];
	GLfloat projection[16];
	GLfloat modelview[16];

	glGetIntegerv(GL_VIEWPORT, &viewport[0]);
	glGetFloatv(GL_PROJECTION_MATRIX, &projection[0]);
	glGetFloatv(GL_MODELVIEW_MATRIX, &modelview[0]);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, viewport[2], 0, viewport[3], -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	//draw Circle
	const GLfloat c = 3.14169f / 180.0f;
	GLint i;
	glBegin(GL_LINE_LOOP);
	for (i = 0; i <= 360; i += 2)
	{
		float a = i * c;
		glVertex2f(x_pos + sin(a) * radius, y_pos + cos(a) * radius);
	}
	glEnd();
	
	glPushMatrix();
		glTranslatef(x_pos,y_pos,0.0f);
		//minute
		glPushMatrix();
			glRotatef(time->tm_min * 6, 0.0f, 0.0f, -1.0f);
			glBegin(GL_LINES);
			//glColor3f(1.0f, 0.0f, 0.0f); // vermelho
			glVertex2i(0, 0);
			glVertex2i(0, radius * 0.8);
			glEnd();
		glPopMatrix();

		//hour
		glPushMatrix();
		glRotatef((time->tm_hour % 12) * 30 + time->tm_min * 0.5, 0.0f, 0.0f, -1.0f);
			glBegin(GL_LINES);
			//glColor3f(1.0f, 0.0f, 0.0f); // vermelho
			glVertex2i(0, 0);
			glVertex2i(0, radius * 0.5);
			glEnd();
		glPopMatrix();
	glPopMatrix();

	glEnable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projection);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelview);
}

void FontHandler::drawClock(time_t time) {
	tm * time_info = localtime(&time);

	bool pm = time_info->tm_hour >= 12; 
	int hour_12 = (time_info->tm_hour >= 13) ? time_info->tm_hour - 12 : time_info->tm_hour;

	std::stringstream ss_time;
	ss_time << std::setw(2) << std::setfill('0') << hour_12 << ":"; 
	ss_time << std::setw(2) << std::setfill('0') << time_info->tm_min << " ";
	ss_time << (pm ? "PM" : "AM");

	std::stringstream ss_day;
	ss_day << months[time_info->tm_mon] << " ";
	ss_day << std::setw(2) << std::setfill('0') << time_info->tm_mday;
	ss_day << ", " << time_info->tm_year + 1900;

	renderMultiLineTextBox2D({ ss_time.str(), ss_day.str() }, 750, 90, 200, 50);
	drawAnalogClock(850, 200, 50, time_info);
}

