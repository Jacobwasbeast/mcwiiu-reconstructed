#include "game/ui/OptionsScreen.hpp"

#include <string>

#include "game/core/Button.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Options.hpp"
#include "game/core/SmallButton.hpp"

class SlideButton : public Button
{
public:
    SlideButton(int id, int x, int y, const Options::Option* option,
                const std::wstring& message, float value);
};

class VideoSettingsScreen : public Screen
{
public:
    VideoSettingsScreen(Screen* parentScreen, Options* options);
};

class ControlsScreen : public Screen
{
public:
    ControlsScreen(Screen* parentScreen, Options* options);
};

OptionsScreen::OptionsScreen(Screen* parentScreen, Options* options)
    : Screen(), mParentScreen(parentScreen), mTitle(L"Options"), mOptions(options)
{
}

OptionsScreen::~OptionsScreen()
{
}

void OptionsScreen::init()
{
    Language* language = Language::getInstance();
    int index;
    int buttonIndex = 0;
    const int optionCount = 5;
    const int optionX = mWidth / 2 - 155;

    for (index = 0; index < optionCount; ++index)
    {
        Options::Option* option = Options::Option::getItem(index);
        int x = optionX + (buttonIndex % 2) * 160;
        int y = mHeight / 6 + (buttonIndex / 2) * 24;
        if (option->isProgress())
        {
            std::wstring message = mOptions->getMessage(option);
            int id = option->getId();
            double value = mOptions->getProgressValue(option);
            mButtons.push_back(new SlideButton(id, x, y, option, message, (float)value));
        }
        else
        {
            std::wstring message = mOptions->getMessage(option);
            int id = option->getId();
            mButtons.push_back(new SmallButton(id, x, y, option, message));
        }
        ++buttonIndex;
    }

    mButtons.push_back(new Button(101, mWidth / 2 - 100, mHeight / 6 + 108,
                                  language->getElement(L"options_video")));
    mButtons.push_back(new Button(100, mWidth / 2 - 100, mHeight / 6 + 132,
                                  language->getElement(L"options_controls")));
    mButtons.push_back(new Button(200, mWidth / 2 - 100, mHeight / 6 + 168,
                                  language->getElement(L"gui_done")));
}

void OptionsScreen::buttonClicked(Button* button)
{
    if (!button->mActive)
        return;

    int id = button->mId;
    if (id < 100)
    {
        SmallButton* smallButton = dynamic_cast<SmallButton*>(button);
        if (smallButton != NULL)
        {
            mOptions->toggle(smallButton->getOption(), 1);
            button->mMessage = mOptions->getMessage(Options::Option::getItem(id));
        }
        id = button->mId;
    }

    if (id == 101)
    {
        mMinecraft->mOptions->save();
        mMinecraft->setScreen(new VideoSettingsScreen(this, mOptions));
    }
    if (id == 100)
    {
        mMinecraft->mOptions->save();
        mMinecraft->setScreen(new ControlsScreen(this, mOptions));
    }
    if (id == 200)
    {
        mMinecraft->mOptions->save();
        mMinecraft->setScreen(mParentScreen);
    }
}

void OptionsScreen::render(int x, int y, float partialTick)
{
    renderBackground();
    drawCenteredString(mFont, mTitle, mWidth / 2, 20, 0xffffff);
    Screen::render(x, y, partialTick);
}
