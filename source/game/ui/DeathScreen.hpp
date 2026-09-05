#ifndef GAME_UI_DEATHSCREEN_HPP
#define GAME_UI_DEATHSCREEN_HPP

#include <vector>

#include "game/core/GuiComponent.hpp"

class Button;
class Font;
class Minecraft;

class DeathScreen : public GuiComponent
{
public:
    virtual ~DeathScreen();

    virtual void init();
    virtual void keyPressed(wchar_t key, int action);
    virtual void buttonClicked(Button* button);
    virtual void render(int x, int y, float partialTick);
    virtual bool isPauseScreen();

private:
    Minecraft* mMinecraft;
    int mWidth;
    int mHeight;
    std::vector<Button*> mButtons;
    Font* mFont;
};

#endif
