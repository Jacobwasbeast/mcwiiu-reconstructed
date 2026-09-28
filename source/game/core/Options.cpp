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
    L"options.renderDistance.far",
    L"options.renderDistance.normal",
    L"options.renderDistance.short",
    L"options.renderDistance.tiny"
};

std::wstring difficultyNames[4] = {
    L"options.difficulty.peaceful",
    L"options.difficulty.easy",
    L"options.difficulty.normal",
    L"options.difficulty.hard"
};

std::wstring guiScaleNames[4] = {
    L"options.guiScale.auto",
    L"options.guiScale.small",
    L"options.guiScale.normal",
    L"options.guiScale.large"
};

std::wstring framerateNames[3] = {
    L"performance.max",
    L"performance.balanced",
    L"performance.powersaver"
};

std::wstring particleNames[3] = {
    L"options.particles.all",
    L"options.particles.decreased",
    L"options.particles.minimal"
};

Options::Option items[18] = {
    Options::Option(L"options.music", true, false),
    Options::Option(L"options.sound", true, false),
    Options::Option(L"options_invertMouse", false, true),
    Options::Option(L"options.sensitivity", true, false),
    Options::Option(L"options.renderDistance", false, false),
    Options::Option(L"options.viewBobbing", false, true),
    Options::Option(L"options.anaglyph", false, true),
    Options::Option(L"options.advancedOpengl", false, true),
    Options::Option(L"options.framerateLimit", false, false),
    Options::Option(L"options.difficulty", false, false),
    Options::Option(L"options.graphics", false, false),
    Options::Option(L"options.ao", false, true),
    Options::Option(L"options.guiScale", false, false),
    Options::Option(L"options.fov", true, false),
    Options::Option(L"options.gamma", true, false),
    Options::Option(L"options.renderClouds", false, true),
    Options::Option(L"options.particles", false, false),
    Options::Option(L"options.autoJump", false, true)
};
}

const Options::Option* Options::Option::music = &::items[0];
const Options::Option* Options::Option::sound = &::items[1];
const Options::Option* Options::Option::invertMouse = &::items[2];
const Options::Option* Options::Option::sensitivity = &::items[3];
const Options::Option* Options::Option::renderDistance = &::items[4];
const Options::Option* Options::Option::viewBobbing = &::items[5];
const Options::Option* Options::Option::anaglyph = &::items[6];
const Options::Option* Options::Option::advancedOpengl = &::items[7];
const Options::Option* Options::Option::framerateLimit = &::items[8];
const Options::Option* Options::Option::difficulty = &::items[9];
const Options::Option* Options::Option::graphics = &::items[10];
const Options::Option* Options::Option::ambientOcclusion = &::items[11];
const Options::Option* Options::Option::guiScale = &::items[12];
const Options::Option* Options::Option::fov = &::items[13];
const Options::Option* Options::Option::gamma = &::items[14];
const Options::Option* Options::Option::renderClouds = &::items[15];
const Options::Option* Options::Option::particles = &::items[16];
const Options::Option* Options::Option::autoJump = &::items[17];

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

    mKeyForward = new KeyMapping(L"key.forward", 22);
    mKeyLeft = new KeyMapping(L"key.left", 0);
    mKeyBack = new KeyMapping(L"key.back", 18);
    mKeyRight = new KeyMapping(L"key.right", 3);
    mKeyJump = new KeyMapping(L"key.jump", 26);
    mKeyInventory = new KeyMapping(L"key.inventory", 4);
    mKeyDrop = new KeyMapping(L"key.drop", 16);
    mKeyChat = new KeyMapping(L"key.chat", 19);
    mKeySneak = new KeyMapping(L"key.sneak", 27);
    mKeyAttack = new KeyMapping(L"key.attack", -100);
    mKeyUse = new KeyMapping(L"key.use", -99);
    mKeyPlayerList = new KeyMapping(L"key.playerlist", 34);
    mKeyPickItem = new KeyMapping(L"key.pickItem", -98);
    mKeyFog = new KeyMapping(L"key.fog", 5);

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
    mHideGui = false;
    mThirdPersonView = 0;
    mRenderDebug = 0;
    mShowInfoBars = false;
    mShowChunkInfoMap = 0;
    mLastServer = L"";
    mScrollHotbar = false;
    mUnknownFlag4 = false;
    mUnknownFlag5 = false;
    mHotbarScroll = 1.0f;
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
        return I18n::get(L"key.mouseButton", key + 101);
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
    save();
}

void Options::set(const Option* option, float value)
{
    if (option == Option::music)
    {
        mMusicVolume = value;
        mMinecraft->mSoundEngine->updateMusicVolume(value);
    }
    if (option == Option::sound)
    {
        mSoundVolume = value;
        mMinecraft->mSoundEngine->updateSoundEffectVolume(value);
    }
    if (option == Option::sensitivity) mSensitivity = value;
    if (option == Option::fov) mFov = value;
    if (option == Option::gamma) mGamma = value;
}

void Options::toggle(const Option* option, int amount)
{
    if (option == Option::invertMouse) mInvertYMouse = !mInvertYMouse;
    if (option == Option::renderDistance) mViewDistance = (mViewDistance + amount) & 3;
    if (option == Option::guiScale) mGuiScale = (mGuiScale + amount) & 3;
    if (option == Option::particles) mParticles = (mParticles + amount) % 3;
    if (option == Option::viewBobbing) mViewBobbing = amount != 0;
    if (option == Option::renderClouds) mClouds = !mClouds;
    if (option == Option::advancedOpengl)
    {
        mAdvancedOpengl = !mAdvancedOpengl;
        mMinecraft->mLevelRenderer->allChanged();
    }
    if (option == Option::anaglyph)
    {
        mAnaglyph3d = !mAnaglyph3d;
        mMinecraft->mTextures->reloadAll();
    }
    if (option == Option::framerateLimit) mFpsLimit = (mFpsLimit + amount + 3) % 3;
    if (option == Option::difficulty) mDifficulty = amount & 3;
    if (option == Option::graphics)
    {
        mFancyGraphics = !mFancyGraphics;
        mMinecraft->mLevelRenderer->allChanged();
    }
    if (option == Option::ambientOcclusion)
    {
        mAmbientOcclusion = !mAmbientOcclusion;
        mMinecraft->mLevelRenderer->allChanged();
    }
    if (option == Option::autoJump) mAutoJump = !mAutoJump;
}

float Options::getProgressValue(const Option* option)
{
    if (option == Option::fov) return mFov;
    if (option == Option::gamma) return mGamma;
    if (option == Option::music) return mMusicVolume;
    if (option == Option::sound) return mSoundVolume;
    if (option == Option::sensitivity) return mSensitivity;
    return 0.0f;
}

bool Options::getBooleanValue(const Option* option)
{
    if (option == Option::invertMouse) return mInvertYMouse;
    if (option == Option::viewBobbing) return mViewBobbing;
    if (option == Option::anaglyph) return mAnaglyph3d;
    if (option == Option::advancedOpengl) return mAdvancedOpengl;
    if (option == Option::ambientOcclusion) return mAmbientOcclusion;
    if (option == Option::renderClouds) return mClouds;
    if (option == Option::autoJump) return mAutoJump;
    return false;
}

std::wstring Options::getMessage(const Option* option)
{
    Language* language = Language::getInstance();
    std::wstring caption = language->getElement(option->getCaptionId()) + L": ";

    if (!option->isProgress())
    {
        if (option->isBoolean())
            return caption + language->getElement(getBooleanValue(option) ? L"options.on" : L"options.off");
        if (option == Option::renderDistance) return caption + language->getElement(renderDistanceNames[mViewDistance]);
        if (option == Option::difficulty) return caption + language->getElement(difficultyNames[mDifficulty]);
        if (option == Option::guiScale) return caption + language->getElement(guiScaleNames[mGuiScale]);
        if (option == Option::particles) return caption + language->getElement(particleNames[mParticles]);
        if (option == Option::framerateLimit) return caption + I18n::get(framerateNames[mFpsLimit]);
        if (option == Option::graphics)
            return caption + language->getElement(mFancyGraphics ? L"options.graphics.fancy" : L"options.graphics.fast");
        return caption;
    }

    float value = getProgressValue(option);
    if (option == Option::sensitivity)
    {
        if (value == 0.0f) return caption + language->getElement(L"options.sensitivity.min");
        if (value == 1.0f) return caption + language->getElement(L"options.sensitivity.max");
        return caption + _toString<int>(static_cast<int>(value * 200.0f)) + L"%";
    }
    if (option == Option::fov)
    {
        if (value == 0.0f) return caption + language->getElement(L"options.fov.min");
        if (value == 1.0f) return caption + language->getElement(L"options.fov.max");
        return caption + _toString<int>(static_cast<int>(value * 40.0f + 70.0f));
    }
    if (option != Option::gamma)
    {
        if (value == 0.0f) return caption + language->getElement(L"options.off");
        return caption + _toString<int>(static_cast<int>(value * 100.0f)) + L"%";
    }
    if (value == 0.0f) return caption + language->getElement(L"options.gamma.min");
    if (value == 1.0f) return caption + language->getElement(L"options.gamma.max");
    return caption + L"+" + _toString<int>(static_cast<int>(value * 100.0f)) + L"%";
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
