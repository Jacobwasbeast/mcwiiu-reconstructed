#include "game/core/Options.hpp"

#include "game/client/audio/SoundEngine.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferedReader.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/FileInputStream.hpp"
#include "game/core/FileOutputStream.hpp"
#include "game/core/I18n.hpp"
#include "game/core/InputStreamReader.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"

template<typename T> std::wstring _toString(T value);
template<typename T> T _fromString(const std::wstring& value);

namespace
{
std::wstring renderDistanceNames[4] = {
    L"options_renderDistance_far",
    L"options_renderDistance_normal",
    L"options_renderDistance_short",
    L"options_renderDistance_tiny"
};

std::wstring difficultyNames[4] = {
    L"options_difficulty_peaceful",
    L"options_difficulty_easy",
    L"options_difficulty_normal",
    L"options_difficulty_hard"
};

std::wstring guiScaleNames[4] = {
    L"options_guiScale_auto",
    L"options_guiScale_small",
    L"options_guiScale_normal",
    L"options_guiScale_large"
};

std::wstring framerateNames[3] = {
    L"performance_max",
    L"performance_balanced",
    L"performance_powersaver"
};

std::wstring particleNames[3] = {
    L"options_particles_all",
    L"options_particles_decreased",
    L"options_particles_minimal"
};

Options::Option items[18] = {
    Options::Option(L"options_music", true, false),
    Options::Option(L"options_sound", true, false),
    Options::Option(L"options_invertMouse", false, true),
    Options::Option(L"options_sensitivity", true, false),
    Options::Option(L"options_renderDistance", false, false),
    Options::Option(L"options_viewBobbing", false, true),
    Options::Option(L"options_anaglyph", false, true),
    Options::Option(L"options_advancedOpengl", false, true),
    Options::Option(L"options_framerateLimit", false, false),
    Options::Option(L"options_difficulty", false, false),
    Options::Option(L"options_graphics", false, false),
    Options::Option(L"options_ao", false, true),
    Options::Option(L"options_guiScale", false, false),
    Options::Option(L"options_fov", true, false),
    Options::Option(L"options_gamma", true, false),
    Options::Option(L"options_renderClouds", false, true),
    Options::Option(L"options_particles", false, false),
    Options::Option(L"options_autoJump", false, true)
};
}

Options::Option::Option(const std::wstring& captionId, bool progress, bool boolean)
    : mProgress(progress), mBoolean(boolean)
{
    mCaptionId.assign(captionId, 0, 0xffffffff);
}

Options::Option::~Option()
{
}

std::wstring Options::Option::getCaptionId() const
{
    return mCaptionId;
}

bool Options::Option::isProgress() const
{
    return mProgress;
}

bool Options::Option::isBoolean() const
{
    return mBoolean;
}

Options::Option* Options::Option::getItem(int id)
{
    return &::items[id];
}

int Options::Option::getId() const
{
    return static_cast<int>(this - ::items);
}

Options::Options(Minecraft* minecraft, File directory)
{
    init();
    mMinecraft = minecraft;
    mOptionsFile = File(directory, L"options.txt");
}

void Options::init()
{
    mMusicVolume = 1.0f;
    mSoundVolume = 1.0f;
    mSensitivity = 0.5f;
    mInvertYMouse = false;
    mViewDistance = 0;
    mViewBobbing = true;
    mAnaglyph3d = false;
    mAdvancedOpengl = false;
    mFpsLimit = 2;
    mFancyGraphics = true;
    mAmbientOcclusion = true;
    mClouds = true;
    mAutoJump = true;
    mSkin = L"Default";

    mKeyForward = new KeyMapping(L"key_forward", 22);
    mKeyLeft = new KeyMapping(L"key_left", 0);
    mKeyBack = new KeyMapping(L"key_back", 18);
    mKeyRight = new KeyMapping(L"key_right", 3);
    mKeyJump = new KeyMapping(L"key_jump", 26);
    mKeyInventory = new KeyMapping(L"key_inventory", 4);
    mKeyDrop = new KeyMapping(L"key_drop", 16);
    mKeyChat = new KeyMapping(L"key_chat", 19);
    mKeySneak = new KeyMapping(L"key_sneak", 27);
    mKeyAttack = new KeyMapping(L"key_attack", -100);
    mKeyUse = new KeyMapping(L"key_use", -99);
    mKeyPlayerList = new KeyMapping(L"key_playerlist", 34);
    mKeyPickItem = new KeyMapping(L"key_pickItem", -98);
    mKeyFog = new KeyMapping(L"key_fog", 5);

    mKeyMappings[0] = mKeyForward;
    mKeyMappings[1] = mKeyLeft;
    mKeyMappings[2] = mKeyBack;
    mKeyMappings[3] = mKeyRight;
    mKeyMappings[4] = mKeyJump;
    mKeyMappings[5] = mKeyInventory;
    mKeyMappings[6] = mKeyDrop;
    mKeyMappings[7] = mKeyChat;
    mKeyMappings[8] = mKeySneak;
    mKeyMappings[9] = mKeyAttack;
    mKeyMappings[10] = mKeyUse;
    mKeyMappings[11] = mKeyPlayerList;
    mKeyMappings[12] = mKeyPickItem;
    mKeyMappings[13] = mKeyFog;

    mMinecraft = 0;
    mDifficulty = 2;
    mUnknownFlag1 = false;
    mUnknownValue1 = 0;
    mUnknownValue2 = 0;
    mUnknownFlag2 = false;
    mUnknownValue3 = 0;
    mLastServer = L"";
    mUnknownFlag3 = false;
    mUnknownFlag4 = false;
    mUnknownFlag5 = false;
    mUnknownScale1 = 1.0f;
    mUnknownScale2 = 1.0f;
    mGuiScale = 0;
    mParticles = 0;
    mFov = 0.0f;
    mGamma = 0.0f;
    mUnknownFlag6 = true;
}

std::wstring Options::getKeyDescription(int index)
{
    return Language::getInstance()->getElement(mKeyMappings[index]->mName);
}

std::wstring Options::getKeyMessage(int index)
{
    int key = mKeyMappings[index]->mKey;
    if (key < 0)
        return I18n::get(L"key_mouseButton", key + 101);
    return L"KEYNAME";
}

void Options::save()
{
    FileOutputStream fileOutput(mOptionsFile);
    DataOutputStream output(&fileOutput);

    output.writeChars(L"music:" + _toString<float>(mMusicVolume) + L"\n");
    output.writeChars(L"sound:" + _toString<float>(mSoundVolume) + L"\n");
    output.writeChars(L"invertYMouse:" + _toString<bool>(mInvertYMouse) + L"\n");
    output.writeChars(L"mouseSensitivity:" + _toString<float>(mSensitivity) + L"\n");
    output.writeChars(L"fov:" + _toString<float>(mFov) + L"\n");
    output.writeChars(L"gamma:" + _toString<float>(mGamma) + L"\n");
    output.writeChars(L"viewDistance:" + _toString<int>(mViewDistance) + L"\n");
    output.writeChars(L"guiScale:" + _toString<int>(mGuiScale) + L"\n");
    output.writeChars(L"particles:" + _toString<int>(mParticles) + L"\n");
    output.writeChars(L"bobView:" + _toString<bool>(mViewBobbing) + L"\n");
    output.writeChars(L"anaglyph3d:" + _toString<bool>(mAnaglyph3d) + L"\n");
    output.writeChars(L"advancedOpengl:" + _toString<bool>(mAdvancedOpengl) + L"\n");
    output.writeChars(L"fpsLimit:" + _toString<int>(mFpsLimit) + L"\n");
    output.writeChars(L"difficulty:" + _toString<int>(mDifficulty) + L"\n");
    output.writeChars(L"fancyGraphics:" + _toString<bool>(mFancyGraphics) + L"\n");
    output.writeChars(L"ao:" + _toString<bool>(mAmbientOcclusion) + L"\n");
    output.writeChars(L"clouds:" + _toString<bool>(mClouds) + L"\n");
    output.writeChars(L"skin:" + mSkin + L"\n");
    output.writeChars(L"lastServer:" + mLastServer + L"\n");

    int i;
    for (i = 0; i < 14; ++i)
        output.writeChars(L"key_" + mKeyMappings[i]->mName + L":" + _toString<int>(mKeyMappings[i]->mKey) + L"\n");

    output.close();
}

void Options::setKey(int index, int key)
{
    mKeyMappings[index]->mKey = key;
}

void Options::set(const Option* option, float value)
{
    if (option == Option::getItem(0))
    {
        mMusicVolume = value;
        mMinecraft->mSoundEngine->updateMusicVolume(value);
    }
    if (option == Option::getItem(1))
    {
        mSoundVolume = value;
        mMinecraft->mSoundEngine->updateSoundEffectVolume(value);
    }
    if (option == Option::getItem(3)) mSensitivity = value;
    if (option == Option::getItem(13)) mFov = value;
    if (option == Option::getItem(14)) mGamma = value;
}

void Options::toggle(const Option* option, int amount)
{
    if (option == Option::getItem(2)) mInvertYMouse = !mInvertYMouse;
    if (option == Option::getItem(4)) mViewDistance = (mViewDistance + amount) & 3;
    if (option == Option::getItem(12)) mGuiScale = (mGuiScale + amount) & 3;
    if (option == Option::getItem(16)) mParticles = (mParticles + amount) % 3;
    if (option == Option::getItem(5)) mViewBobbing = amount != 0;
    if (option == Option::getItem(15)) mClouds = !mClouds;
    if (option == Option::getItem(7))
    {
        mAdvancedOpengl = !mAdvancedOpengl;
        mMinecraft->mLevelRenderer->allChanged();
    }
    if (option == Option::getItem(6))
    {
        mAnaglyph3d = !mAnaglyph3d;
        mMinecraft->mTextures->reloadAll();
    }
    if (option == Option::getItem(8)) mFpsLimit = (mFpsLimit + amount + 3) % 3;
    if (option == Option::getItem(9)) mDifficulty = amount & 3;
    if (option == Option::getItem(10))
    {
        mFancyGraphics = !mFancyGraphics;
        mMinecraft->mLevelRenderer->allChanged();
    }
    if (option == Option::getItem(11))
    {
        mAmbientOcclusion = !mAmbientOcclusion;
        mMinecraft->mLevelRenderer->allChanged();
    }
    if (option == Option::getItem(17)) mAutoJump = !mAutoJump;
}

double Options::getProgressValue(const Option* option)
{
    if (option == Option::getItem(13)) return mFov;
    if (option == Option::getItem(14)) return mGamma;
    if (option == Option::getItem(0)) return mMusicVolume;
    if (option == Option::getItem(1)) return mSoundVolume;
    if (option == Option::getItem(3)) return mSensitivity;
    return 0.0;
}

bool Options::getBooleanValue(const Option* option)
{
    if (option == Option::getItem(2)) return mInvertYMouse;
    if (option == Option::getItem(5)) return mViewBobbing;
    if (option == Option::getItem(6)) return mAnaglyph3d;
    if (option == Option::getItem(7)) return mAdvancedOpengl;
    if (option == Option::getItem(11)) return mAmbientOcclusion;
    if (option == Option::getItem(15)) return mClouds;
    if (option == Option::getItem(17)) return mAutoJump;
    return false;
}

std::wstring Options::getMessage(const Option* option)
{
    Language* language = Language::getInstance();
    std::wstring caption = language->getElement(option->getCaptionId()) + L": ";

    if (!option->isProgress())
    {
        if (option->isBoolean())
            return caption + language->getElement(getBooleanValue(option) ? L"options_on" : L"options_off");
        if (option == Option::getItem(4)) return caption + language->getElement(renderDistanceNames[mViewDistance]);
        if (option == Option::getItem(9)) return caption + language->getElement(difficultyNames[mDifficulty]);
        if (option == Option::getItem(12)) return caption + language->getElement(guiScaleNames[mGuiScale]);
        if (option == Option::getItem(16)) return caption + language->getElement(particleNames[mParticles]);
        if (option == Option::getItem(8)) return caption + I18n::get(framerateNames[mFpsLimit]);
        if (option == Option::getItem(10))
            return caption + language->getElement(mFancyGraphics ? L"options_graphics_fancy" : L"options_graphics_fast");
        return caption;
    }

    double value = getProgressValue(option);
    if (option == Option::getItem(3))
    {
        if (value == 0.0) return caption + language->getElement(L"options_sensitivity_min");
        if (value == 1.0) return caption + language->getElement(L"options_sensitivity_max");
        return caption + _toString<int>(static_cast<int>(value * 200.0)) + L"%";
    }
    if (option == Option::getItem(13))
    {
        if (value == 0.0) return caption + language->getElement(L"options_fov_min");
        if (value == 1.0) return caption + language->getElement(L"options_fov_max");
        return caption + _toString<int>(static_cast<int>(value * 40.0 + 70.0));
    }
    if (option != Option::getItem(14))
    {
        if (value == 0.0) return caption + language->getElement(L"options_off");
        return caption + _toString<int>(static_cast<int>(value * 100.0)) + L"%";
    }
    if (value == 0.0) return caption + language->getElement(L"options_gamma_min");
    if (value == 1.0) return caption + language->getElement(L"options_gamma_max");
    return caption + L"+" + _toString<int>(static_cast<int>(value * 100.0)) + L"%";
}

double Options::readFloat(std::wstring value)
{
    if (value == L"true") return 1.0;
    if (value == L"false") return 0.0;
    return _fromString<float>(value);
}

void Options::load()
{
    if (!mOptionsFile.exists())
        return;

    FileInputStream* fileInput = new FileInputStream(mOptionsFile);
    InputStreamReader* input = new InputStreamReader(fileInput);
    BufferedReader* reader = new BufferedReader(input);
    std::wstring line;

    while ((line = reader->readLine()) != L"")
    {
        std::wstring name;
        std::wstring value;
        std::wstring::size_type separator = line.find(L':');
        if (separator == std::wstring::npos)
        {
            name = line;
            value = L"";
        }
        else
        {
            name = line.substr(0, separator);
            value = line.substr(separator + 1);
        }

        if (name == L"music") mMusicVolume = static_cast<float>(readFloat(value));
        if (name == L"sound") mSoundVolume = static_cast<float>(readFloat(value));
        if (name == L"mouseSensitivity") mSensitivity = static_cast<float>(readFloat(value));
        if (name == L"fov") mFov = static_cast<float>(readFloat(value));
        if (name == L"gamma") mGamma = static_cast<float>(readFloat(value));
        if (name == L"invertYMouse") mInvertYMouse = value == L"true";
        if (name == L"viewDistance") mViewDistance = _fromString<int>(value);
        if (name == L"guiScale") mGuiScale = _fromString<int>(value);
        if (name == L"particles") mParticles = _fromString<int>(value);
        if (name == L"bobView") mViewBobbing = value == L"true";
        if (name == L"anaglyph3d") mAnaglyph3d = value == L"true";
        if (name == L"advancedOpengl") mAdvancedOpengl = value == L"true";
        if (name == L"fpsLimit") mFpsLimit = _fromString<int>(value);
        if (name == L"difficulty") mDifficulty = _fromString<int>(value);
        if (name == L"fancyGraphics") mFancyGraphics = value == L"true";
        if (name == L"ao") mAmbientOcclusion = value == L"true";
        if (name == L"clouds") mClouds = value == L"true";
        if (name == L"skin") mSkin.assign(value, 0, 0xffffffff);
        if (name == L"lastServer") mLastServer.assign(value, 0, 0xffffffff);

        int i;
        for (i = 0; i < 14; ++i)
        {
            if (name == L"key_" + mKeyMappings[i]->mName)
                mKeyMappings[i]->mKey = _fromString<int>(value);
        }
    }

    reader->close();
}
