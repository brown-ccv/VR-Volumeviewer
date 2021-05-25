#ifndef FONTHANDLER_H_
#define FONTHANDLER_H_

#include <FTGL/ftgl.h>
#include <string>

enum TextAlignment
{
  CENTER,
  LEFT,
  RIGHT
};

class FontHandler
{
public:


  virtual ~FontHandler();
  static FontHandler* getInstance();

  FTFont* getFont();

  void renderTextBox(std::string text, double x, double y, double z, double width, double height, TextAlignment alignment = CENTER, bool rotateY = false);
  void renderMultiLineTextBox(std::vector<std::string> btext, double x, double y, double z, double width, double height, TextAlignment alignment = CENTER, bool rotateY = false);
  void renderTextBox2D(std::string text, double x, double y, double width, double height, TextAlignment alignment = CENTER, bool rotateY = false);
  void renderMultiLineTextBox2D(std::vector<std::string> btext, double x, double y, double width, double height, TextAlignment alignment = CENTER, bool rotateY = false);
  void drawClock(time_t time);

private:
  FontHandler();
  static FontHandler* instance;

  FTFont* font;
  double m_fontMinMax[2];
};

#endif /* VRFONT_H_ */

