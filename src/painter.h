#ifndef PAINTER_H
#define PAINTER_H

#include "TGUI/TGUI.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Graphics/Text.hpp"
#include "rendernode.h"

class Painter
{
public:
    Painter(RenderNode *node);
    static void paintNode(RenderNode *node, sf::RenderWindow *window);

private:
    static std::string parseTextToLines(std::string textToParse, sf::RenderWindow *window);
    static void drawPicture(tgui::Window *window);

};

#endif // PAINTER_H
