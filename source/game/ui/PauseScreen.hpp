#ifndef GAME_UI_PAUSESCREEN_HPP
#define GAME_UI_PAUSESCREEN_HPP

#include <vector>

class Button;
class Font;
class Minecraft;

class PauseScreen
{
public:
    PauseScreen();
    virtual ~PauseScreen();

    virtual void render(int x, int y, float partialTick);
    virtual void buttonClicked(Button button);
    virtual void tick();
    void init();

private:
    Minecraft* mMinecraft;
    int mWidth;
    int mHeight;
    std::vector<Button*> mButtons;
    Font* mFont;
    int mButtonListState;
    int mTickCount;
    int mUnknown;
};

#endif
