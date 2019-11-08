#ifndef VRFONT_H_
#define VRFONT_H_

#include <FTGL/ftgl.h>
#include <string>


class VRFontHandler
	{
		public:
			enum TextAlignment
			{
				CENTER,
				LEFT,
				RIGHT
			};

			virtual ~VRFontHandler();
			static VRFontHandler* getInstance();

			FTFont* getFont();

			void renderTextBox(std::string text, double x, double y, double z, double width, double height, TextAlignment alignment = CENTER, bool rotateY = false);
			void renderMultiLineTextBox(std::vector<std::string> btext, double x, double y, double z, double width, double height, TextAlignment alignment = CENTER, bool rotateY = false);

		private:
			VRFontHandler();
			static VRFontHandler* instance;

			FTFont* font;
			double m_fontMinMax[2];
	};

#endif /* VRFONT_H_ */

