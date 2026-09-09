#include "game/ui/SelectWorldScreen.hpp"

#include <cstdio>
#include <string>

#include "game/core/Button.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/LevelStorageSource.hpp"
#include "game/ui/ConfirmScreen.hpp"
#include "game/ui/CreateWorldScreen.hpp"
#include "game/ui/RenameWorldScreen.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/world/LevelSummary.hpp"

std::wstring _toString(int value);
std::wstring _toString(float value);

struct FILETIME
{
    unsigned long low;
    unsigned long high;
};

struct SYSTEMTIME
{
    unsigned short year;
    unsigned short month;
    unsigned short dayOfWeek;
    unsigned short day;
    unsigned short hour;
    unsigned short minute;
    unsigned short second;
    unsigned short milliseconds;
};

void FileTimeToSystemTime(const FILETIME* fileTime, SYSTEMTIME* systemTime);

SelectWorldScreen::SelectWorldScreen(Screen* parent)
    : Screen(), mParent(parent), mTitle(L"Select_world"), mSelecting(false), mSelectedWorld(0),
      mLevelList(NULL), mWorldSelectionList(NULL), mWorldName(), mConversion(), mConfirmingDelete(false),
      mRenameButton(NULL), mSelectButton(NULL), mDeleteButton(NULL)
{
}

SelectWorldScreen::~SelectWorldScreen()
{
}

void SelectWorldScreen::loadLevelList()
{
    mLevelList = mMinecraft->getLevelSource()->getLevelList();
    mSelectedWorld = -1;
}

void SelectWorldScreen::init()
{
    Language* language = Language::getInstance();
    mTitle = language->getElement(std::wstring(L"selectWorld_title"));
    mWorldName = language->getElement(std::wstring(L"selectWorld_world"));
    mConversion = language->getElement(std::wstring(L"selectWorld_conversion"));
    loadLevelList();
    mWorldSelectionList = new WorldSelectionList(this);
    mWorldSelectionList->init(&mButtons, 4, 5);
    postInit();
}

std::wstring SelectWorldScreen::getWorldId(int index)
{
    return mLevelList->at(index)->getLevelId();
}

std::wstring SelectWorldScreen::getWorldName(int index)
{
    std::wstring name = mLevelList->at(index)->getLevelName();
    if (name.empty())
        name = Language::getInstance()->getElement(std::wstring(L"selectWorld_world")) + L" " + _toString(index + 1);
    return name;
}

void SelectWorldScreen::postInit()
{
    Language* language = Language::getInstance();
    mSelectButton = new Button(1, mWidth / 2 - 154, mHeight - 52, 150, 20,
                               language->getElement(std::wstring(L"selectWorld_select")));
    mButtons.push_back(mSelectButton);
    mRenameButton = new Button(6, mWidth / 2 - 154, mHeight - 28, 70, 20,
                               language->getElement(std::wstring(L"selectWorld_rename")));
    mButtons.push_back(mRenameButton);
    mDeleteButton = new Button(2, mWidth / 2 - 74, mHeight - 28, 70, 20,
                               language->getElement(std::wstring(L"selectWorld_delete")));
    mButtons.push_back(mDeleteButton);
    mButtons.push_back(new Button(3, mWidth / 2 + 4, mHeight - 52, 150, 20,
                                  language->getElement(std::wstring(L"selectWorld_create"))));
    mButtons.push_back(new Button(0, mWidth / 2 + 4, mHeight - 28, 150, 20,
                                  language->getElement(std::wstring(L"gui_cancel"))));
    mSelectButton->mActive = false;
    mRenameButton->mActive = false;
    mDeleteButton->mActive = false;
}

void SelectWorldScreen::worldSelected(int index)
{
    mMinecraft->setScreen(NULL);
    if (!mSelecting)
    {
        mSelecting = true;
        mMinecraft->mWidth = 0;
    }
}

void SelectWorldScreen::buttonClicked(Button* button)
{
    if (!button->mActive)
        return;

    if (button->mId == 2)
    {
        std::wstring name = getWorldName(mSelectedWorld);
        if (name != mWorldName)
        {
            mConfirmingDelete = true;
            Language* language = Language::getInstance();
            std::wstring message = language->getElement(std::wstring(L"selectWorld_deleteQuestion"));
            std::wstring warning = std::wstring(L"'") + name + L"'";
            std::wstring title = language->getElement(std::wstring(L"selectWorld_deleteWarning"));
            std::wstring text = warning + L" " + title;
            ConfirmScreen* confirm = new ConfirmScreen(this, message, text,
                language->getElement(std::wstring(L"selectWorld_deleteButton")),
                language->getElement(std::wstring(L"gui_cancel")), mSelectedWorld);
            mMinecraft->setScreen(confirm);
        }
    }
    else if (button->mId == 1)
    {
        worldSelected(mSelectedWorld);
    }
    else if (button->mId == 3)
    {
        if (mMinecraft->isClientSide())
            mMinecraft->setScreen(new CreateWorldScreen(this));
        else
        {
            mMinecraft->setScreen(NULL);
            if (!mSelecting)
                mSelecting = true;
        }
    }
    else if (button->mId == 6)
    {
        mMinecraft->setScreen(new RenameWorldScreen(this, getWorldId(mSelectedWorld)));
    }
    else if (button->mId == 0)
    {
        mMinecraft->setScreen(mParent);
    }
    else
    {
        mWorldSelectionList->buttonClicked(button);
    }
}

void SelectWorldScreen::confirmResult(bool result, int id)
{
    if (mConfirmingDelete)
    {
        mConfirmingDelete = false;
        if (result)
        {
            mMinecraft->getLevelSource()->deleteLevel(getWorldId(id));
            loadLevelList();
        }
        mMinecraft->setScreen(this);
    }
}

void SelectWorldScreen::render(int x, int y, float partialTick)
{
    drawCenteredString(mFont, mTitle, mWidth / 2, 20, 0xffffff);
    Screen::render(x, y, partialTick);
    static int counter = 0;
    ++counter;
    if (counter > 99)
    {
        if (mLevelList != NULL && !mLevelList->empty())
        {
            if (mLevelList->size() > 1 && mDeleteButton->mActive)
            {
                mSelectedWorld = 1;
                counter = 0;
                buttonClicked(mSelectButton);
                return;
            }
            if (mSelectButton->mActive)
            {
                mSelectedWorld = 0;
                buttonClicked(mSelectButton);
                return;
            }
            counter = 0;
            mSelectButton->mActive = true;
            mRenameButton->mActive = true;
            mDeleteButton->mActive = true;
            return;
        }
        mMinecraft->setScreen(new CreateWorldScreen(this));
    }
}

SelectWorldScreen::WorldSelectionList::WorldSelectionList(SelectWorldScreen* screen)
    : ScrolledSelectionList(screen->mMinecraft, screen->mWidth, screen->mHeight,
                            screen->mHeight - 64, 32, 36), mScreen(screen)
{
}

int SelectWorldScreen::WorldSelectionList::getNumberOfItems()
{
    return mScreen->mLevelList == NULL ? 0 : (int)mScreen->mLevelList->size();
}

void SelectWorldScreen::WorldSelectionList::selectItem(int index, bool doubleClick)
{
    bool valid = false;
    mScreen->mSelectedWorld = index;
    if (index >= 0 && index < getNumberOfItems())
        valid = true;
    mScreen->mSelectButton->mActive = valid;
    mScreen->mRenameButton->mActive = valid;
    mScreen->mDeleteButton->mActive = valid;
    if (doubleClick && valid)
        mScreen->worldSelected(index);
}

bool SelectWorldScreen::WorldSelectionList::isSelectedItem(int index)
{
    return index == mScreen->mSelectedWorld;
}

int SelectWorldScreen::WorldSelectionList::getMaxPosition()
{
    return getNumberOfItems() * 36;
}

void SelectWorldScreen::WorldSelectionList::renderBackground()
{
    mScreen->renderBackground();
}

void SelectWorldScreen::WorldSelectionList::renderItem(int index, int x, int y, int height, Tesselator*)
{
    LevelSummary* summary = mScreen->mLevelList->at(index);
    std::wstring name = summary->getLevelName();
    if (name.empty())
        name = mScreen->mWorldName + L" " + _toString(index + 1);

    FILETIME fileTime;
    SYSTEMTIME systemTime;
    long long lastPlayed = summary->getLastPlayed() * 10000;
    fileTime.low = (unsigned long)lastPlayed;
    fileTime.high = (unsigned long)(lastPlayed >> 32);
    FileTimeToSystemTime(&fileTime, &systemTime);
    wchar_t date[20];
    swprintf(date, 20, L"%d/%d/%d %02d:%02d", systemTime.month, systemTime.day,
             systemTime.year, systemTime.hour, systemTime.minute);

    std::wstring id = summary->getLevelId() + L" (" + std::wstring(date) + L")";
    long long size = summary->getSizeOnDisk() / 1024;
    size = size * 100 / 1024;
    std::wstring details = id + L", " + _toString((float)size / 100.0f) + L"MB";
    if (summary->isRequiresConversion())
        details += L" " + mScreen->mConversion;

    mScreen->drawString(mScreen->mFont, name, x + 2, y + 1, 0xffffff);
    mScreen->drawString(mScreen->mFont, id, x + 2, y + 12, 0x808080);
    mScreen->drawString(mScreen->mFont, details, x + 2, y + 22, 0x808080);
}
