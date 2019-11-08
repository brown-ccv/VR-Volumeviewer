#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "VRFontHandler.h"
#include <iostream>

#define TEXTBORDER 0.003

VRFontHandler* VRFontHandler::instance = NULL;

VRFontHandler::VRFontHandler() 
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

VRFontHandler::~VRFontHandler()
{
	instance = NULL;
	delete font;
}

VRFontHandler* VRFontHandler::getInstance()
{
	if (!instance)
	{
		instance = new VRFontHandler();
	}
	return instance;
}

FTFont* VRFontHandler::getFont()
{
	return font;
}

void VRFontHandler::renderTextBox(std::string text, double x, double y, double z, double width, double height, TextAlignment alignment, bool rotateY)
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
	font->Render(text.c_str());
	glPopMatrix();
}

void VRFontHandler::renderMultiLineTextBox(std::vector<std::string> text, double x, double y, double z, double width, double height, TextAlignment alignment, bool rotateY)
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
		font->Render(text[i].c_str());
		glPopMatrix();
	}
}
