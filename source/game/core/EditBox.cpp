#include "game/core/EditBox.hpp"

#include "game/client/render/Font.hpp"
#include "game/core/GuiComponent.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/ui/Screen.hpp"

EditBox::EditBox(Screen* screen, Font* font, int x, int y, int width, int height,
                 const std::wstring& value)
{
    mX = x;
    mFont = font;
    mTickCount = 0;
    mY = y;
    mWidth = width;
    mHeight = height;
    mScreen = screen;
    mMaxLength = 0;
    setValue(value);
}

void EditBox::tick()
{
    ++mTickCount;
}

void EditBox::setValue(const std::wstring& value)
{
    mValue.assign(value);
}

void EditBox::setMaxLength(int maxLength)
{
    mMaxLength = maxLength;
}

std::wstring EditBox::getValue()
{
    return mValue;
}

void EditBox::keyPressed(wchar_t key, int action)
{
    if (!mActive || !mFocused)
    {
        return;
    }

    if (key == 9)
    {
        mScreen->tabPressed();
    }

    if (action == 29 && !mValue.empty())
    {
        mValue = mValue.substr(0, mValue.size() - 1);
    }

    if (ALLOWED_CHARACTERS.find(&key, 0, 1) != std::wstring::npos
        && (mValue.size() < static_cast<std::wstring::size_type>(mMaxLength) || mMaxLength == 0))
    {
        mValue += key;
    }
}

void EditBox::focus(bool focused)
{
    if (focused && !mFocused)
    {
        mTickCount = 0;
    }
    mFocused = focused;
}

void EditBox::mouseClicked(int button, int x, int y)
{
    bool focused = mActive && mX <= x && x < mX + mWidth
        && mY <= y && y < mY + mHeight;
    if (focused && !mFocused)
    {
        mTickCount = 0;
    }
    mFocused = focused;
}

void EditBox::render()
{
    fill(mX - 1, mY - 1, mX + mWidth + 1, mY + mHeight + 1, 0xffa0a0a0);
    fill(mX, mY, mX + mWidth, mY + mHeight, 0xff000000);

    int textY = mY + (mHeight - 8) / 2;
    if (!mActive)
    {
        drawString(mFont, mValue, mX + 4, textY, 0x707070);
    }
    else
    {
        const wchar_t* cursor = L"";
        if (mFocused && (mTickCount / 6 & 1) == 0)
        {
            cursor = L"_";
        }
        std::wstring displayedValue = mValue + cursor;
        drawString(mFont, displayedValue, mX + 4, textY, 0xe0e0e0);
    }
}
