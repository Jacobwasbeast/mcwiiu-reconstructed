#include "game/ui/TitleScreen.hpp"

#include <string>

#include "game/core/Button.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/JoinMultiplayerScreen.hpp"
#include "game/ui/OptionsScreen.hpp"
#include "game/ui/SelectWorldScreen.hpp"

Screen* createOptionsScreen(TitleScreen* parent, void* options)
{
    return new OptionsScreen(parent, static_cast<Options*>(options));
}

Screen* createSelectWorldScreen(TitleScreen* parent)
{
    return new SelectWorldScreen(parent);
}

TitleScreen::TitleScreen()
    : mLogoWidth(256.0f), mTitle(), mMultiplayerButton(NULL)
{
    mTitle.assign(L"missingno");
    mTitle.assign(L"");
}

TitleScreen::~TitleScreen()
{
}

void TitleScreen::tick()
{
}

void TitleScreen::keyPressed(wchar_t key, int action)
{
}

void TitleScreen::init()
{
    Language* language = Language::getInstance();
    const int centerX = mWidth / 2;
    const int quarterHeight = mHeight / 4;

    mButtons.push_back(new Button(1, centerX - 100, quarterHeight + 48,
                                  language->getElement(std::wstring(L"menu.singleplayer"))));

    mMultiplayerButton = new Button(2, centerX - 100, quarterHeight + 72,
                                    language->getElement(std::wstring(L"menu.multiplayer")));
    mButtons.push_back(mMultiplayerButton);

    mButtons.push_back(new Button(3, centerX - 100, quarterHeight + 96,
                                  language->getElement(std::wstring(L"menu.mods"))));

    if (mMinecraft->mDefaultHeight == 0)
    {
        mButtons.push_back(new Button(0, centerX - 100, quarterHeight + 132,
                                      98, 20,
                                      language->getElement(std::wstring(L"menu.options"))));
        mButtons.push_back(new Button(4, centerX + 2, quarterHeight + 132,
                                      98, 20,
                                      language->getElement(std::wstring(L"menu.quit"))));
    }
    else
    {
        mButtons.push_back(new Button(0, centerX - 100, quarterHeight + 120,
                                      language->getElement(std::wstring(L"menu.options"))));
    }

    if (mMinecraft->mDefaultHeight == 0 && mMultiplayerButton != NULL)
    {
        mMultiplayerButton->mActive = false;
    }
}

void TitleScreen::buttonClicked(Button* button)
{
    if (button->mId == 0)
    {
        mMinecraft->setScreen(createOptionsScreen(this, mMinecraft->mOptions));
    }

    if (button->mId == 1)
    {
        mMinecraft->setScreen(createSelectWorldScreen(this));
    }

    if (button->mId == 2)
    {
        mMinecraft->setScreen(new JoinMultiplayerScreen(this));
    }

    if (button->mId == 4)
    {
        mMinecraft->stop();
    }
}

void TitleScreen::render(int x, int y, float partialTick)
{
}
