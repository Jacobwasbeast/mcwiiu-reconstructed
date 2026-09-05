#ifndef GAME_UI_DISCONNECTEDSCREEN_HPP
#define GAME_UI_DISCONNECTEDSCREEN_HPP

#include <string>
#include <vector>

class Button;
class Font;
class Minecraft;

class DisconnectedScreen
{
public:
    DisconnectedScreen(const std::wstring& title, std::wstring message, void* data, ...);
    virtual ~DisconnectedScreen();

    virtual void tick();
    virtual void keyPressed(char key, int action);
    virtual void init();
    virtual void buttonClicked(Button* button);
    virtual void render(int x, int y, float partialTick);

private:
    Minecraft* mMinecraft;
    int mWidth;
    int mHeight;
    std::vector<Button*> mButtons;
    Font* mFont;
    std::wstring mTitle;
    std::wstring mMessage;
};

#endif
