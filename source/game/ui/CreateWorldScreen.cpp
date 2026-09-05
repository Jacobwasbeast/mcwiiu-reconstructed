#include "game/ui/CreateWorldScreen.hpp"

#include <string>

#include "game/core/Button.hpp"
#include "game/core/EditBox.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/LevelStorageSource.hpp"
#include "game/util/Random.hpp"

std::wstring trimString(const std::wstring& value);

template<typename T>
T _fromString(const std::wstring& value);

extern int gCreateWorldScreenTickCount;

CreateWorldScreen::CreateWorldScreen(Screen* parentScreen)
    : Screen(), mParentScreen(parentScreen), mNameEdit(0), mSeedEdit(0), mResultFolder(),
      mCreatingWorld(false)
{
}

CreateWorldScreen::~CreateWorldScreen()
{
}

void CreateWorldScreen::tick()
{
    mNameEdit->tick();
    mSeedEdit->tick();
    bool invokeButton = gCreateWorldScreenTickCount == 100;
    ++gCreateWorldScreenTickCount;
    if (invokeButton)
    {
        buttonClicked(mButtons[0]);
    }
}

std::wstring CreateWorldScreen::findAvailableFolderName(LevelStorageSource*,
                                                          const std::wstring& folderName)
{
    return folderName;
}

void CreateWorldScreen::updateResultFolder()
{
    std::wstring name = trimString(mNameEdit->getValue());
    mResultFolder = name;

    const wchar_t invalidCharacters[] = L"/\n\r\t\f<>|\":";
    int pass;
    for (pass = 0; pass < 3; ++pass)
    {
        const wchar_t* character = invalidCharacters + pass * 5;
        int index;
        for (index = 0; index < 5; ++index)
        {
            wchar_t invalid = character[index];
            if (invalid == 0)
            {
                continue;
            }
            std::wstring::size_type position;
            while ((position = mResultFolder.find(&invalid, 0, 1)) != std::wstring::npos)
            {
                mResultFolder[position] = L'_';
            }
        }
    }

    if (mResultFolder.empty())
    {
        mResultFolder = L"World";
    }
    mResultFolder = findAvailableFolderName(mMinecraft->getLevelSource(), mResultFolder);
}

void CreateWorldScreen::init()
{
    Language* language = Language::getInstance();
    mButtons.push_back(new Button(0, mWidth / 2 - 100, mHeight / 4 + 108,
                                  language->getElement(L"selectWorld.create")));
    mButtons.push_back(new Button(1, mWidth / 2 - 100, mHeight / 4 + 132,
                                  language->getElement(L"gui.cancel")));

    mNameEdit = new EditBox(this, mFont, mWidth / 2 - 100, 60, 200, 20,
                            language->getElement(L"testWorld"));
    mNameEdit->focus(true);
    mNameEdit->setMaxLength(32);
    mSeedEdit = new EditBox(this, mFont, mWidth / 2 - 100, 116, 200, 20, L"");
    updateResultFolder();
}

void CreateWorldScreen::removed()
{
}

void CreateWorldScreen::buttonClicked(Button* button)
{
    if (button->mActive)
    {
        if (button->mId == 1)
        {
            mMinecraft->setScreen(mParentScreen);
            return;
        }
        if (button->mId == 0)
        {
            mMinecraft->setScreen(0);
            if (!mCreatingWorld)
            {
                mCreatingWorld = true;
                Random random;
                random.nextLong();
                std::wstring seedText = mSeedEdit->getValue();
                if (!seedText.empty())
                {
                    _fromString<long long>(seedText);
                }
            }
        }
    }
}

void CreateWorldScreen::keyPressed(wchar_t key, int action)
{
    mNameEdit->keyPressed(key, action);
    if (action == 13)
    {
        tabPressed();
    }
    mButtons[0]->mActive = !mNameEdit->getValue().empty();
    updateResultFolder();
}

void CreateWorldScreen::mouseClicked(int x, int y, int button)
{
    Screen::mouseClicked(x, y, button);
    mNameEdit->mouseClicked(button, x, y);
    mSeedEdit->mouseClicked(button, x, y);
}

void CreateWorldScreen::render(int x, int y, float partialTick)
{
    Language* language = Language::getInstance();
    renderBackground();
    drawCenteredString(mFont, language->getElement(L"selectWorld.create"), mWidth / 2,
                       mHeight / 4 - 40, 0xffffff);
    drawString(mFont, language->getElement(L"selectWorld.enterName"), mWidth / 2 - 100,
               47, 0xa0a0a0);
    drawString(mFont, language->getElement(L"selectWorld.resultFolder") + L" " + mResultFolder,
               mWidth / 2 - 100, 85, 0xa0a0a0);
    drawString(mFont, language->getElement(L"selectWorld.enterSeed"), mWidth / 2 - 100,
               104, 0xa0a0a0);
    drawString(mFont, language->getElement(L"selectWorld.seedInfo"), mWidth / 2 - 100,
               140, 0xa0a0a0);
    mNameEdit->render();
    mSeedEdit->render();
    Screen::render(x, y, partialTick);
}

void CreateWorldScreen::tabPressed()
{
    mNameEdit->focus(false);
    mSeedEdit->focus(true);
}
