#include "game/ui/JoinMultiplayerScreen.hpp"

#include <cwchar>
#include <string>
#include <vector>

#include "game/core/Button.hpp"
#define private public
#include "game/core/EditBox.hpp"
#undef private
#include "game/core/GuiComponent.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#define private public
#include "game/core/Options.hpp"
#undef private

template<typename T>
T _fromString(const std::wstring& value);

std::wstring trimString(const std::wstring& value);
void replaceAll(std::wstring& result, const std::wstring& source,
                const std::wstring& search, const std::wstring& replacement);
std::vector<std::wstring> stringSplit(const std::wstring& text, wchar_t separator);

JoinMultiplayerScreen::JoinMultiplayerScreen(Screen* parentScreen)
    : Screen(), mParentScreen(parentScreen), mEditBox(NULL)
{
}

JoinMultiplayerScreen::~JoinMultiplayerScreen()
{
}

void JoinMultiplayerScreen::tick()
{
    mEditBox->tick();
}

void JoinMultiplayerScreen::init()
{
    Language* language = Language::getInstance();
    mButtons.push_back(new Button(0, mWidth / 2 - 100, mHeight / 4 + 108,
                                  language->getElement(L"multiplayer_connect")));
    mButtons.push_back(new Button(1, mWidth / 2 - 100, mHeight / 4 + 132,
                                  language->getElement(L"gui_cancel")));

    std::wstring value;
    replaceAll(value, mMinecraft->mOptions->mLastServer, L"_", L":");
    mButtons[0]->mActive = !value.empty();
    mEditBox = new EditBox(this, mFont, mWidth / 2 - 100, mHeight / 4 + 58,
                           200, 20, value);
    mEditBox->mActive = true;
    mEditBox->setMaxLength(0x80);
}

void JoinMultiplayerScreen::removed()
{
}

void JoinMultiplayerScreen::buttonClicked(Button* button)
{
    if (!button->mActive)
        return;

    if (button->mId == 1)
    {
        mMinecraft->setScreen(mParentScreen);
    }

    if (button->mId == 0)
    {
        std::wstring value = trimString(mEditBox->getValue());
        std::wstring savedValue;
        replaceAll(savedValue, value, L":", L"_");
        mMinecraft->mOptions->mLastServer = savedValue;
        mMinecraft->mOptions->save();

        std::vector<std::wstring> parts = stringSplit(value, L',');
        if (!parts.empty() && !parts[0].empty() && parts[0][0] == L'[')
        {
            std::wstring::size_type close = value.find(L"]");
            if (close != std::wstring::npos)
            {
                std::wstring address = value.substr(1, close - 1);
                std::wstring port = trimString(value.substr(close + 1));
                if (!port.empty() && port[0] == L':')
                {
                    port = port.substr(1);
                    parts.clear();
                    parts.push_back(address);
                    parts.push_back(port);
                }
                else
                {
                    parts.clear();
                    parts.push_back(address);
                }
            }
        }

        if (parts.size() > 2)
            parts.push_back(value);
    }
}

void JoinMultiplayerScreen::parseInt(const std::wstring& value, int radix)
{
    _fromString<int>(value);
}

void JoinMultiplayerScreen::keyPressed(wchar_t key, int action)
{
    mEditBox->keyPressed(key, action);

    if (action == 13)
        buttonClicked(mButtons[0]);

    mButtons[0]->mActive = !mEditBox->getValue().empty();
}

void JoinMultiplayerScreen::mouseClicked(int x, int y, int button)
{
    Screen::mouseClicked(x, y, button);
    mEditBox->mouseClicked(x, y, button);
}

void JoinMultiplayerScreen::render(int x, int y, float partialTick)
{
    Language* language = Language::getInstance();
    renderBackground();
    drawCenteredString(mFont, language->getElement(L"multiplayer_title"),
                       mWidth / 2, mHeight / 4 - 40, 0xffffff);
    drawString(mFont, language->getElement(L"multiplayer_info1"),
               mWidth / 2 - 140, mHeight / 4, 0xa0a0a0);
    drawString(mFont, language->getElement(L"multiplayer_info2"),
               mWidth / 2 - 140, mHeight / 4 + 9, 0xa0a0a0);
    drawString(mFont, language->getElement(L"multiplayer_ipinfo"),
               mWidth / 2 - 140, mHeight / 4 + 36, 0xa0a0a0);
    mEditBox->render();
    Screen::render(x, y, partialTick);
}
