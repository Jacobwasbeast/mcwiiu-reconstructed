#ifndef GAME_CORE_OPTIONS_HPP
#define GAME_CORE_OPTIONS_HPP

#include <string>

#include "game/core/File.hpp"
#include "game/core/KeyMapping.hpp"

class Minecraft;

class Options
{
public:
    class Option
    {
    public:
        Option(const std::wstring& captionId, bool progress, bool boolean);
        ~Option();

        std::wstring getCaptionId() const;
        bool isProgress() const;
        bool isBoolean() const;
        static Option* getItem(int id);
        int getId() const;

    private:
        bool mProgress;
        bool mBoolean;
        std::wstring mCaptionId;
    };

    Options(Minecraft* minecraft, File directory);
    Options();

    void init();
    std::wstring getKeyDescription(int index);
    std::wstring getKeyMessage(int index);
    void save();
    void setKey(int index, int key);
    void set(const Option* option, float value);
    void toggle(const Option* option, int amount);
    double getProgressValue(const Option* option);
    bool getBooleanValue(const Option* option);
    std::wstring getMessage(const Option* option);
    double readFloat(std::wstring value);
    void load();
    bool isCloudsOn();

private:
    float mMusicVolume;
    float mSoundVolume;
    float mSensitivity;
    bool mInvertYMouse;

public:
    int mViewDistance;

private:
    bool mViewBobbing;
    bool mAnaglyph3d;
    bool mAdvancedOpengl;
    int mFpsLimit;
    bool mFancyGraphics;
    bool mAmbientOcclusion;
    bool mClouds;
    bool mAutoJump;
    std::wstring mSkin;
    KeyMapping* mKeyForward;
    KeyMapping* mKeyLeft;
    KeyMapping* mKeyBack;
    KeyMapping* mKeyRight;
    KeyMapping* mKeyJump;
    KeyMapping* mKeyInventory;
    KeyMapping* mKeyDrop;
    KeyMapping* mKeyChat;
    KeyMapping* mKeySneak;
    KeyMapping* mKeyAttack;
    KeyMapping* mKeyUse;
    KeyMapping* mKeyPlayerList;
    KeyMapping* mKeyPickItem;
    KeyMapping* mKeyFog;
    KeyMapping* mKeyMappings[14];
    Minecraft* mMinecraft;
    File mOptionsFile;
    int mDifficulty;
    bool mUnknownFlag1;
    int mUnknownValue1;
    int mUnknownValue2;
    bool mUnknownFlag2;
    int mUnknownValue3;
    std::wstring mLastServer;
    bool mUnknownFlag3;
    bool mUnknownFlag4;
    bool mUnknownFlag5;
    float mUnknownScale1;
    float mUnknownScale2;
    int mGuiScale;
    int mParticles;
    float mFov;
    float mGamma;
    bool mUnknownFlag6;
};

#endif
