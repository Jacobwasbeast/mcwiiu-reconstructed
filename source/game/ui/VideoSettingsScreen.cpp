#include "game/ui/VideoSettingsScreen.hpp"

#include "game/core/Button.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Options.hpp"
#include "game/core/SlideButton.hpp"
#include "game/core/SmallButton.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"

VideoSettingsScreen::VideoSettingsScreen(Screen* parentScreen, Options* options)
    : Screen(), mParentScreen(parentScreen), mTitle(L"Video Settings"), mOptions(options)
{
}

VideoSettingsScreen::~VideoSettingsScreen()
{
}

void VideoSettingsScreen::init()
{
    Language* language = Language::getInstance();
    mTitle = language->getElement(L"options.videoTitle");

    const int optionIds[8] = { 6, 0, 7, 4, 2, 1, 8, 3 };
    int buttonIndex;
    for (buttonIndex = 0; buttonIndex < 8; ++buttonIndex)
    {
        Options::Option* option = Options::Option::getItem(optionIds[buttonIndex]);
        int x = mWidth / 2 - 155 + (buttonIndex % 2) * 160;
        int y = mHeight / 6 + (buttonIndex / 2) * 24;
        if (option->isProgress())
        {
            std::wstring message = mOptions->getMessage(option);
            int id = option->getId();
            double value = mOptions->getProgressValue(option);
            mButtons.push_back(new SlideButton(id, x, y, option, message,
                                               static_cast<float>(value)));
        }
        else
        {
            std::wstring message = mOptions->getMessage(option);
            int id = option->getId();
            mButtons.push_back(new SmallButton(id, x, y, option, message));
        }
    }

    mButtons.push_back(new Button(200, mWidth / 2 - 100,
                                  mHeight / 6 + 168,
                                  language->getElement(L"gui.done")));
}

void VideoSettingsScreen::buttonClicked(Button* button)
{
    if (!button->mActive)
    {
        return;
    }

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

    if (id == 200)
    {
        mMinecraft->mOptions->save();
        mMinecraft->setScreen(mParentScreen);
    }

    ScreenSizeCalculator screenSizeCalculator(mMinecraft->mOptions, -1);
    Screen::init(mMinecraft, static_cast<int>(screenSizeCalculator.getWidth()),
                 static_cast<int>(screenSizeCalculator.getHeight()));
}

void VideoSettingsScreen::render(int x, int y, float partialTick)
{
    renderBackground();
    drawCenteredString(mFont, mTitle, mWidth / 2, 20, 0xffffff);
    Screen::render(x, y, partialTick);
}
