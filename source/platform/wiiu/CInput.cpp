#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "game/core/GameSettings.hpp"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <new>

#include <coreinit/filesystem.h>
#include <coreinit/messagequeue.h>
#include <coreinit/systeminfo.h>
#include <coreinit/userconfig.h>
#include <nn/erreula.h>
#include <nn/swkbd.h>
#include <padscore/kpad.h>
#include <padscore/wpad.h>
#include <vpad/input.h>

extern "C" int SCIGetCafeLanguage(unsigned int* language);
int XEnableGuestSignin(int enable)
{
    (void)enable;
    return 0;
}

extern "C" int SCIGetPlatformRegion(unsigned int* region);
extern "C" int OSResumeThread(OSThread* thread);
extern "C" void KPADReset(void);

static void ConnectCallbackAdapter(WPADChan channel, WPADError result)
{
    CInput::ConnectCallback(static_cast<int>(channel), static_cast<int>(result));
}

extern "C" int _SCIReadSysConfig(const char* key, unsigned int type, unsigned int size, void* value);
int SCIGetSystemSettings(void* settings);

double multiBreak(double first, double second, double third)
{
    if (first < second)
    {
        return second;
    }
    if (static_cast<float>(first - third) < 0.0f)
    {
        third = first;
    }
    return third;
}

int VerifyLanguageRange(int* language)
{
    if (*language < 12 && static_cast<signed char>(*language) >= 0)
    {
        return 1;
    }
    *language = 0xff;
    return 0;
}

int VerifyCountryRange(unsigned int* country)
{
    int result = 1;
    if (*country < 0x100)
    {
        if (static_cast<int>(*country) < 0)
        {
            result = 0;
            *country = 0;
        }
    }
    else
    {
        result = 0;
        *country = 0xff;
    }
    return result;
}

extern "C" int SCIGetCafeLanguageCode(unsigned int* language)
{
    return _SCIReadSysConfig("cafe.language", 3, 4, language);
}

extern "C" int SCIGetCafeLanguage(unsigned int* language)
{
    int result = SCIGetCafeLanguageCode(language);
    VerifyLanguageRange(reinterpret_cast<int*>(language));
    if (result == 1)
    {
        if (*language == 0xff)
        {
            unsigned int platformRegion;
            result = SCIGetPlatformRegion(&platformRegion);
            if (result != 1)
            {
                *language = 1;
                return result;
            }
            if (platformRegion == 1)
            {
                *language = 0;
            }
            else if (platformRegion == 0x10)
            {
                *language = 6;
            }
            else if (platformRegion == 0x20)
            {
                *language = 7;
            }
            else if (platformRegion == 0x40)
            {
                *language = 0xb;
            }
            else
            {
                *language = 1;
            }
        }
        if (result == 1)
        {
            return 1;
        }
    }
    *language = 1;
    return result;
}

extern "C" int SCIGetPlatformRegion(unsigned int* region)
{
    unsigned char settings[0x48] = {};
    int result = SCIGetSystemSettings(settings);
    *region = settings[3];
    return result;
}

static KPADError s_connectionResults[4];
static VPADReadError s_vpadReadError;
static VPADStatus s_vpadStatus[16];
static KPADStatus s_kpadStatus[4][16];
static OSMessage s_fontMessage;
static OSMessage s_predictMessage;
static OSMessageQueue s_fontQueue;
static OSMessageQueue s_predictQueue;
static OSThread s_fontThread;
static OSThread s_predictThread;
static unsigned char s_fontStack[0x20000];
static unsigned char s_predictStack[0x20000];
static CInput* s_inputInstance = 0;
unsigned int g_inputPlatformFlag = 0;

static unsigned int s_xLanguage = 0xff;
static unsigned int s_xLocale = 0xff;

extern "C" int _SCIReadSysConfig(const char* key, unsigned int type, unsigned int size, void* value)
{
    UCHandle handle = UCOpen();
    if (handle < 0)
        return 0;

    UCSysConfig config;
    std::memset(&config, 0, sizeof(config));
    if (key != 0)
        std::strncpy(config.name, key, sizeof(config.name) - 1);
    config.access = 1911;
    config.dataType = static_cast<UCDataType>(type);
    config.dataSize = size;
    config.data = value;
    UCError error = UCReadSysConfig(handle, 1, &config);
    UCClose(handle);
    if (error == 0)
        return 1;
    if (error == -0x200009)
        return -4;
    if (error == -0x200015)
        return -3;
    return -1;
}

unsigned int XGetLanguage()
{
    unsigned int language = GameSettings::GetMinecraftLanguage(0);
    if (language != 0)
        return language;
    if (s_xLanguage == 0xff && SCIGetCafeLanguage(&s_xLanguage) != 1)
        s_xLanguage = 0xff;
    if (s_xLanguage == 0)
        return 2;
    if (s_xLanguage < 2)
        return 1;
    if (s_xLanguage < 0xc)
    {
        static const unsigned char languageMap[] = { 0, 0, 4, 3, 6, 5, 0x11, 7, 0x10, 9, 0xc, 8 };
        return languageMap[s_xLanguage];
    }
    return 1;
}

int XGetLocale()
{
    unsigned int minecraftLocale = GameSettings::GetMinecraftLocale(0);
    if (minecraftLocale != 0)
        return minecraftLocale;
    if (s_xLocale == 0xff && SCIGetCafeLanguage(&s_xLocale) != 1)
        s_xLocale = 0xff;
    if (s_xLocale > 0xb)
        return 0x24;
    static const unsigned char localeMap[] = { 0x14, 0x24, 0x16, 0x0d, 0x13, 0x05,
                                               0x25, 0x04, 0x04, 0x0d, 0x1b, 0x22 };
    return localeMap[s_xLocale];
}

static int GetStickValue(float value)
{
    if (value < -1.0f)
    {
        value = -1.0f;
    }
    else if (value > 1.0f)
    {
        value = 1.0f;
    }
    return static_cast<short>(value * 32767.0f);
}

static int ApplyDeadzone(int value, int deadzone)
{
    if (value > deadzone)
    {
        return value - deadzone;
    }
    if (value < -deadzone)
    {
        return value + deadzone;
    }
    return 0;
}

static float NormaliseStick(int value, float range)
{
    float result = static_cast<float>(value) / range;
    if (result > -0.15f && result < 0.15f)
    {
        return 0.0f;
    }
    return result;
}

static unsigned long long GetTimeValue(const CInput::TIME_VALUE& value)
{
    return static_cast<unsigned long long>(value.high) << 32 | value.low;
}

static void SetTimeValue(CInput::TIME_VALUE& value, unsigned long long time)
{
    value.high = static_cast<unsigned int>(time >> 32);
    value.low = static_cast<unsigned int>(time);
}

CInput* g_pInput = new (std::nothrow) CInput;

CInput::CInput()
{
    s_inputInstance = this;
    g_pInput = this;
    joypadMapsReady = 0;
    menuDisplayed[0] = 0;
    menuDisplayed[1] = 0;
    menuDisplayed[2] = 0;
    menuDisplayed[3] = 0;
    InitTime();
    debugSequence = 0;
    debugSequenceLength = 0;
    debugSequenceCallback = 0;
    inputTick = 0;
    initialRepeatDelay = 0.5f;
    repeatDelay = 0.8f;
    circleCrossSwapped = 0;
    for (int pad = 0; pad < 4; ++pad)
        padRemap[pad] = -1;
    KeyboardOnCreate();
    ErrorViewerOnCreate();
}
void CInput::InitTime()
{
    for (int pad = 0; pad < 5; ++pad)
    {
        idleStartTimes[pad].high = 0;
        idleStartTimes[pad].low = 0;
    }
    timeScale = 1.0f / static_cast<float>(OSGetSystemInfo()->busClockSpeed >> 2);
}
void CInput::ConnectCallback(int channel, int result)
{
    if (result != 0)
    {
        s_connectionResults[channel] = static_cast<KPADError>(0xFD);
    }
    else if (g_inputPlatformFlag != 0)
    {
        KPADEnableMpls(static_cast<KPADChan>(channel), static_cast<KPADMplsMode>(3));
    }
    else
    {
        WPADControlDpd(static_cast<WPADChan>(channel), WPAD_DPD_FMT_NONE, 0);
    }
}
void CInput::Initialise(int count, unsigned char gameMapCount, unsigned char alternateMapCount)
{
    VPADInit();
    WPADEnableURCC(1);
    KPADInit();
    KPADReset();
    std::memset(joypads, 0, sizeof(joypads));
    for (int channel = 0; channel < 4; ++channel)
    {
        KPADSetConnectCallback(static_cast<KPADChan>(channel), ConnectCallbackAdapter);
        s_connectionResults[channel] = static_cast<KPADError>(0xFD);
    }

    controllerCount = count;
    deadzone = 10000;
    maximumInput = 10000;
    movementRange = 0x7fff;
    movementSpan = 0x58ef;
    movementSpanFloat = 22767.0f;
    controllerOptions = 0;
    for (int pad = 0; pad < 5; ++pad)
    {
        joypads[pad].connected = 0;
        joypads[pad].sensitivity = 1.0f;
        joypads[pad].statusIndex = 0xff;
        joypads[pad].controllerId = -1;
        joypads[pad].lockedController = -1;
        joypads[pad].leftXSource = &joypads[pad].leftX;
        joypads[pad].leftYSource = &joypads[pad].leftY;
        joypads[pad].rightXSource = &joypads[pad].rightX;
        joypads[pad].rightYSource = &joypads[pad].rightY;
        joypads[pad].leftTriggerSource = &joypads[pad].hasTouch;
        joypads[pad].rightTriggerSource = &joypads[pad].touchPressed;
        if (pad == 0)
        {
            joypads[pad].statusStorage.vpad.statuses = new VPADStatus[count];
        }
        else
        {
            joypads[pad].statusStorage.kpad.statuses = new KPADStatus[count];
        }
        gameJoypadMaps[pad] = new unsigned int[gameMapCount];
        alternateJoypadMaps[pad] = new unsigned int[gameMapCount];
        for (unsigned int map = 0; map < gameMapCount; ++map)
        {
            gameJoypadMaps[pad][map] = 0;
        }
        for (unsigned int map = 0; map < gameMapCount; ++map)
        {
            alternateJoypadMaps[pad][map] = 0;
        }
    }
    controllerConfiguration[0] = static_cast<unsigned char>(count);
    controllerConfiguration[1] = gameMapCount;
    controllerConfiguration[2] = alternateMapCount;
    joypadMapsReady = 0;
    inputTick = 0;
    for (int pad = 0; pad < 4; ++pad)
    {
        padRemap[pad] = -1;
    }
    InitialiseErrorViewer();
    InitialiseKeyboard();
}
void CInput::ReadAllPads()
{
    VPADRead(static_cast<VPADChan>(0), s_vpadStatus, 16, &s_vpadReadError);
    if (s_vpadReadError == VPAD_READ_SUCCESS || s_vpadReadError == VPAD_READ_NO_SAMPLES)
    {
        VPADGetTPCalibratedPoint(static_cast<VPADChan>(0), &s_vpadStatus[0].tpNormal, &s_vpadStatus[0].tpNormal);
    }
    else
    {
        std::memset(s_vpadStatus, 0, sizeof(VPADStatus));
    }

    KPADError readError;
    for (int channel = 0; channel < 4; ++channel)
    {
        KPADReadEx(static_cast<KPADChan>(channel), s_kpadStatus[channel], 16, &readError);
    }

    JOYPAD& drc = joypads[0];
    drc.previousSampleIndex = drc.sampleIndex;
    drc.sampleIndex = 0;
    drc.statusIndex = static_cast<unsigned char>((drc.statusIndex + 1) % controllerConfiguration[0]);
    static_cast<VPADStatus*>(drc.statusStorage.vpad.statuses)[drc.statusIndex] = s_vpadStatus[0];
    bool drcConnected = s_vpadReadError == VPAD_READ_SUCCESS || s_vpadReadError == VPAD_READ_NO_SAMPLES;
    if (!drc.connected)
    {
        std::memset(&static_cast<VPADStatus*>(drc.statusStorage.vpad.statuses)[drc.statusIndex], 0, sizeof(VPADStatus));
    }
    drc.setConnected(drcConnected);

    bool preserveConnections = inputTick == 0;
    if (!preserveConnections)
    {
        --inputTick;
    }
    for (int channel = 0; channel < 4; ++channel)
    {
        JOYPAD& joypad = joypads[channel + 1];
        joypad.previousSampleIndex = joypad.sampleIndex;
        joypad.sampleIndex = 0;
        joypad.statusIndex = static_cast<unsigned char>((joypad.statusIndex + 1) % controllerConfiguration[0]);
        KPADStatus& status = static_cast<KPADStatus*>(joypad.statusStorage.kpad.statuses)[joypad.statusIndex];
        status = s_kpadStatus[channel][0];
        bool supported = status.extensionType == 2 || status.extensionType == 7 || status.extensionType == 0x1f;
        bool connected = supported && (status.error == 0 || status.error == -2);
        if (!preserveConnections && (status.extensionType == 0xff || (status.extensionType == 0x1f && status.error == -4)))
        {
            connected = true;
        }
        if (!connected)
        {
            std::memset(&status, 0, 0x3c);
        }
        joypad.setConnected(connected);
    }
}
void CInput::ClearJoypadValues(JOYPAD* joypad)
{
    joypad->fakeValue = 0;
    joypad->rawValue = 0;
    joypad->pressedValue = 0;
    joypad->releasedValue = 0;
    joypad->rawRightStickX = 0;
    joypad->rawRightStickY = 0;
    joypad->rawLeftStickX = 0;
    joypad->rawLeftStickY = 0;
    joypad->leftStickX = 0;
    joypad->leftStickY = 0;
    joypad->rightStickX = 0;
    joypad->rightStickY = 0;
    joypad->leftX = 0.0f;
    joypad->leftY = 0.0f;
    joypad->rightX = 0.0f;
    joypad->rightY = 0.0f;
    joypad->hasTouch = 0;
    joypad->touchPressed = 0;
}
void CInput::JoypadConnected(int pad)
{
    ClearJoypadValues(&joypads[pad]);
}
void CInput::JoypadRemoved(int pad)
{
    ClearJoypadValues(&joypads[pad]);
}
void CInput::SetDRCJoypadValue(JOYPAD* joypad, unsigned int sourceMask, unsigned int mappedMask)
{
    VPADStatus* status = static_cast<VPADStatus*>(joypad->statusStorage.vpad.statuses) + joypad->statusIndex;
    if ((status->hold & sourceMask) == sourceMask)
    {
        joypad->rawValue |= mappedMask;
    }
}
void CInput::SetJoypadValuesDRC(JOYPAD* joypad)
{
    VPADStatus* status = static_cast<VPADStatus*>(joypad->statusStorage.vpad.statuses) + joypad->statusIndex;
    static const unsigned int source[] = { 0x4000, 0x8000, 0x1000, 0x2000, 0x40, 0x80, 0x10, 0x20, 8, 4, 0x20000, 0x40000, 0x200, 0x100, 0x800, 0x400 };
    static const unsigned int destination[] = { 1, 2, 4, 8, 0x400000, 0x800000, 0x40, 0x80, 0x10, 0x20, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000 };
    for (int button = 0; button < 16; ++button)
    {
        SetDRCJoypadValue(joypad, source[button], destination[button]);
    }
    int leftX = GetStickValue(status->leftStick.x);
    int leftY = GetStickValue(status->leftStick.y);
    int rightX = GetStickValue(status->rightStick.x);
    int rightY = GetStickValue(status->rightStick.y);
    if (leftX > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x4000;
    if (leftX < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x8000;
    if (rightY < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x10000;
    if (rightY > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x20000;
    if (rightX > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x40000;
    if (rightX < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x80000;
    if (leftY < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x100000;
    if (leftY > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x200000;
    joypad->rawRightStickX = ApplyDeadzone(rightX, deadzone);
    joypad->rawRightStickY = ApplyDeadzone(rightY, deadzone);
    joypad->rawLeftStickX = ApplyDeadzone(leftX, deadzone);
    joypad->rawLeftStickY = ApplyDeadzone(leftY, deadzone);
    joypad->leftStickX = joypad->rawLeftStickX / static_cast<int>(movementSpan);
    joypad->leftStickY = joypad->rawLeftStickY / static_cast<int>(movementSpan);
    joypad->rightStickX = joypad->rawRightStickX / static_cast<int>(movementSpan);
    joypad->rightStickY = joypad->rawRightStickY / static_cast<int>(movementSpan);
    joypad->leftX = NormaliseStick(joypad->rawRightStickX, movementSpanFloat);
    joypad->leftY = NormaliseStick(joypad->rawRightStickY, movementSpanFloat);
    joypad->rightX = NormaliseStick(joypad->rawLeftStickX, movementSpanFloat);
    joypad->rightY = NormaliseStick(joypad->rawLeftStickY, movementSpanFloat);
}
void CInput::GetStartTime(int pad, int type)
{
    BUTTON_TIME& time = buttonTimes[pad][type];
    unsigned long long now = OSGetTime();
    SetTimeValue(time.lastUpdate, now);
    time.elapsed.high = 0;
    time.elapsed.low = 0;
    time.heldTime = 0.0f;
    time.repeatTime = -1.0f;
    time.repeated = 0;
    SetTimeValue(idleStartTimes[pad], now);
}
void CInput::UpdateTime(int pad, int type)
{
    BUTTON_TIME& time = buttonTimes[pad][type];
    unsigned long long now = OSGetTime();
    unsigned long long elapsed = now - GetTimeValue(time.lastUpdate);
    SetTimeValue(time.lastUpdate, now);
    SetTimeValue(time.elapsed, GetTimeValue(time.elapsed) + elapsed);
    time.heldTime += static_cast<float>(elapsed) * timeScale;
    SetTimeValue(idleStartTimes[pad], now);
}
void CInput::UpdateDRCPad()
{
    JOYPAD& joypad = joypads[0];
    unsigned int previous = joypad.rawValue;
    controllerCount = 0;
    joypad.rawValue = 0;
    joypad.leftStickX = 0;
    joypad.leftStickY = 0;
    joypad.rightStickX = 0;
    joypad.rightStickY = 0;
    joypad.hasTouch = 0;
    joypad.touchPressed = 0;
    if (!joypad.sampleIndex)
    {
        if (joypad.newlyConnected)
        {
            JoypadConnected(0);
        }
        if (joypad.disconnected)
        {
            JoypadRemoved(0);
        }
        if (!joypad.connected)
        {
            if (joypad.disconnected)
            {
                ClearJoypadValues(&joypad);
            }
            return;
        }

        controllerCount = 1;
        SetJoypadValuesDRC(&joypad);
        joypad.rawValue |= joypad.fakeValue;
        joypad.fakeValue = 0;
        joypad.previousValue = previous;
        joypad.pressedValue = joypad.rawValue & ~previous;
        joypad.releasedValue = previous & ~joypad.rawValue;
        for (int button = 0; button < 24; ++button)
        {
            unsigned int mask = 1u << button;
            if (joypad.pressedValue & mask)
            {
                GetStartTime(0, button);
            }
            else if (joypad.rawValue & mask)
            {
                UpdateTime(0, button);
            }
        }
    }
}
void CInput::SetClassicJoypadValue(JOYPAD* joypad, unsigned int sourceMask, unsigned int mappedMask)
{
    KPADStatus* status = static_cast<KPADStatus*>(joypad->statusStorage.kpad.statuses) + joypad->statusIndex;
    if ((status->classic.hold & sourceMask) == sourceMask)
    {
        joypad->rawValue |= mappedMask;
    }
}
void CInput::SetFreestyleJoypadValue(JOYPAD* joypad, unsigned int sourceMask, unsigned int mappedMask)
{
    KPADStatus* status = static_cast<KPADStatus*>(joypad->statusStorage.kpad.statuses) + joypad->statusIndex;
    if ((status->hold & sourceMask) == sourceMask)
    {
        joypad->rawValue |= mappedMask;
    }
}
void CInput::SetURCCJoypadValue(JOYPAD* joypad, unsigned int sourceMask, unsigned int mappedMask)
{
    KPADStatus* status = static_cast<KPADStatus*>(joypad->statusStorage.kpad.statuses) + joypad->statusIndex;
    if ((status->pro.hold & sourceMask) == sourceMask)
    {
        joypad->rawValue |= mappedMask;
    }
}
void CInput::SetJoypadValues(JOYPAD* joypad)
{
    KPADStatus* status = static_cast<KPADStatus*>(joypad->statusStorage.kpad.statuses) + joypad->statusIndex;
    joypad->leftX = joypad->leftY = joypad->rightX = joypad->rightY = 0.0f;
    if (status->extensionType == 2 || status->extensionType == 7)
    {
        static const unsigned int source[] = { 0x10, 0x40, 0x20, 8, 4, 0x80, 0x200, 0x2000, 0x400, 0x1000, 1, 0x4000, 2, 0x8000 };
        static const unsigned int destination[] = { 2, 1, 4, 8, 0x400000, 0x800000, 0x40, 0x80, 0x10, 0x20, 0x400, 0x800, 0x1000, 0x2000 };
        for (int button = 0; button < 14; ++button)
        {
            SetClassicJoypadValue(joypad, source[button], destination[button]);
        }
        joypad->leftX = status->classic.leftStick.x;
        joypad->leftY = status->classic.leftStick.y;
        joypad->rightX = status->classic.rightStick.x;
        joypad->rightY = status->classic.rightStick.y;
        joypad->hasTouch = static_cast<unsigned char>(status->classic.leftTrigger * 255.0f);
        joypad->touchPressed = static_cast<unsigned char>(status->classic.rightTrigger * 255.0f);
    }
    else if (status->extensionType == 1)
    {
        static const unsigned int source[] = { 0x800, 0x400, 0x200, 0x100, 0x2000, 0x4000, 0x10, 0x1000, 8, 4, 1, 2 };
        static const unsigned int destination[] = { 2, 1, 4, 8, 0x400000, 0x800000, 0x10, 0x20, 0x400, 0x800, 0x1000, 0x2000 };
        for (int button = 0; button < 12; ++button)
        {
            SetFreestyleJoypadValue(joypad, source[button], destination[button]);
        }
        joypad->leftX = status->nunchuk.stick.x;
        joypad->leftY = status->nunchuk.stick.y;
    }
    else if (status->extensionType == 0x1f)
    {
        static const unsigned int source[] = { 0x10, 0x40, 0x20, 8, 4, 0x80, 0x200, 0x2000, 0x400, 0x1000, 0x10000, 0x20000, 1, 0x4000, 2, 0x8000 };
        static const unsigned int destination[] = { 2, 1, 4, 8, 0x400000, 0x800000, 0x40, 0x80, 0x10, 0x20, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000 };
        for (int button = 0; button < 16; ++button)
        {
            SetURCCJoypadValue(joypad, source[button], destination[button]);
        }
        joypad->leftX = status->pro.leftStick.x;
        joypad->leftY = status->pro.leftStick.y;
        joypad->rightX = status->pro.rightStick.x;
        joypad->rightY = status->pro.rightStick.y;
    }
    int leftX = GetStickValue(joypad->leftX);
    int leftY = GetStickValue(joypad->leftY);
    int rightX = GetStickValue(joypad->rightX);
    int rightY = GetStickValue(joypad->rightY);
    if (leftX > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x4000;
    if (leftX < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x8000;
    if (rightY < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x10000;
    if (rightY > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x20000;
    if (rightX > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x40000;
    if (rightX < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x80000;
    if (leftY < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x100000;
    if (leftY > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x200000;
    joypad->rawRightStickX = ApplyDeadzone(rightX, deadzone);
    joypad->rawRightStickY = ApplyDeadzone(rightY, deadzone);
    joypad->rawLeftStickX = ApplyDeadzone(leftX, deadzone);
    joypad->rawLeftStickY = ApplyDeadzone(leftY, deadzone);
    joypad->leftStickX = joypad->rawLeftStickX / static_cast<int>(movementSpan);
    joypad->leftStickY = joypad->rawLeftStickY / static_cast<int>(movementSpan);
    joypad->rightStickX = joypad->rawRightStickX / static_cast<int>(movementSpan);
    joypad->rightStickY = joypad->rawRightStickY / static_cast<int>(movementSpan);
    joypad->leftX = NormaliseStick(joypad->rawRightStickX, movementSpanFloat);
    joypad->leftY = NormaliseStick(joypad->rawRightStickY, movementSpanFloat);
    joypad->rightX = NormaliseStick(joypad->rawLeftStickX, movementSpanFloat);
    joypad->rightY = NormaliseStick(joypad->rawLeftStickY, movementSpanFloat);
}
void CInput::UpdateJoypads()
{
    int connected = 0;
    for (int pad = 1; pad < 5; ++pad)
    {
        JOYPAD& joypad = joypads[pad];
        unsigned int previous = joypad.rawValue;
        joypad.previousValue = previous;
        joypad.rawValue = joypad.pendingValue;
        joypad.pendingValue = 0;
        if (joypad.fakeTimer)
        {
            --joypad.fakeTimer;
        }
        joypad.hasTouch = 0;
        joypad.touchPressed = 0;
        joypad.leftStickX = 0;
        joypad.leftStickY = 0;
        joypad.rightStickX = 0;
        joypad.rightStickY = 0;
        if (joypad.sampleIndex)
        {
            ClearJoypadValues(&joypad);
            if (joypad.connected)
            {
                ++connected;
            }
            continue;
        }
        if (joypad.newlyConnected)
        {
            JoypadConnected(pad);
        }
        if (joypad.disconnected)
        {
            JoypadRemoved(pad);
        }
        if (joypad.connected)
        {
            SetJoypadValues(&joypad);
            joypad.rawValue |= joypad.fakeValue;
            joypad.fakeValue = 0;
            if (!joypad.previousSampleIndex)
            {
                joypad.pressedValue = joypad.rawValue & ~previous;
                joypad.releasedValue = previous & ~joypad.rawValue;
            }
            else
            {
                joypad.pressedValue = 0;
                joypad.releasedValue = 0;
            }
            for (int button = 0; button < 24; ++button)
            {
                unsigned int mask = 1u << button;
                if (joypad.pressedValue & mask)
                {
                    GetStartTime(pad, button);
                }
                else if (joypad.rawValue & mask)
                {
                    UpdateTime(pad, button);
                }
            }
        }
        else if (joypad.disconnected)
        {
            ClearJoypadValues(&joypad);
        }
        if (joypad.connected)
        {
            ++connected;
        }
    }
    controllerCount += connected;
}
void CInput::Tick()
{
    ReadAllPads();
    TickErrorViewer();
    TickKeyboard();
    UpdateDRCPad();
    UpdateJoypads();
}
void CInput::SetDeadzoneAndMovementRange(unsigned int value, unsigned int maximum, unsigned int range)
{
    deadzone = value;
    maximumInput = maximum;
    movementRange = range;
    movementSpan = range - value;
    movementSpanFloat = static_cast<float>(movementSpan);
}
void CInput::SetGameControllerTypeJoypadMaps(int controllerType, unsigned char map, unsigned int value)
{
    unsigned int** maps = controllerType == 0 ? gameJoypadMaps : alternateJoypadMaps;
    for (int pad = 0; pad < 5; ++pad)
    {
        maps[pad][map] = value;
    }
}
bool CInput::RemapIpad(int& pad)
{
    if (pad > 3)
    {
        if (pad > 8)
        {
            return false;
        }
        pad -= 4;
        return true;
    }

    int mappedPad = padRemap[pad];
    if (mappedPad < 0)
    {
        return false;
    }
    pad = mappedPad;
    return true;
}
void CInput::SetGameJoypadMaps(int pad, unsigned char map, unsigned int value)
{
    RemapIpad(pad);
    gameJoypadMaps[pad][map] = value;
    alternateJoypadMaps[pad][map] = value;
}
unsigned int CInput::GetGameJoypadMaps(int pad, unsigned char map)
{
    int originalPad = pad;
    if (!RemapIpad(pad))
    {
        return 0;
    }
    return IsPadClassicController(originalPad) ? alternateJoypadMaps[pad][map] : gameJoypadMaps[pad][map];
}
void CInput::SetJoypadMapArraySetupComplete()
{
    joypadMapsReady = 1;
}
void CInput::SetJoypadSensitivity(int pad, float sensitivity)
{
    if (RemapIpad(pad))
    {
        joypads[pad].sensitivity = sensitivity;
    }
}
unsigned int CInput::only_one_bit_set(unsigned int value)
{
    return value != 0 && (value & (value - 1)) == 0;
}
unsigned int CInput::allowInput(int pad, unsigned char map, unsigned int value)
{
    int mappedPad = pad;
    if (!RemapIpad(mappedPad))
    {
        return 0;
    }
    unsigned int mapping = GetGameJoypadMaps(pad, map);
    if (value == 0 || value != mapping)
    {
        return 0;
    }
    if (only_one_bit_set(mapping))
    {
        for (unsigned char other = controllerConfiguration[2] + 1; other < controllerConfiguration[1]; ++other)
        {
            unsigned int otherMapping = GetGameJoypadMaps(pad, other);
            if (!only_one_bit_set(otherMapping) && (mapping & otherMapping) &&
                (joypads[mappedPad].rawValue & mapping & otherMapping) &&
                (joypads[mappedPad].rawValue & otherMapping) == otherMapping)
            {
                return 0;
            }
        }
    }
    return 1;
}
unsigned int CInput::GetValue(int pad, unsigned char map, bool repeated)
{
    bool menuOpen = pad < 4 && menuDisplayed[pad];
    int mappedPad = pad;
    if (!RemapIpad(mappedPad) || (menuOpen && map > controllerConfiguration[2]))
    {
        return 0;
    }
    unsigned int value = joypads[mappedPad].rawValue & GetGameJoypadMaps(pad, map);
    if (map > controllerConfiguration[2] && !allowInput(pad, map, value))
    {
        return 0;
    }
    if (!repeated || value == 0)
    {
        return value;
    }
    int button = 0;
    unsigned int buttons = value;
    while ((buttons & 1) == 0)
    {
        ++button;
        buttons >>= 1;
    }
    BUTTON_TIME& time = buttonTimes[mappedPad][button];
    BUTTON_TIME* repeatTime = &time;
    unsigned int remainingButtons = buttons >> 1;
    if (remainingButtons != 0)
    {
        int secondButton = button + 1;
        while ((remainingButtons & 1) == 0)
        {
            ++secondButton;
            remainingButtons >>= 1;
        }
        BUTTON_TIME& secondTime = buttonTimes[mappedPad][secondButton];
        if (time.heldTime <= secondTime.heldTime)
        {
            if (secondTime.heldTime - time.heldTime < 0.001f)
            {
                secondTime.repeatTime = secondTime.heldTime + 0.002f;
            }
        }
        else if (time.heldTime - secondTime.heldTime < 0.001f)
        {
            time.repeatTime = time.heldTime + 0.002f;
            repeatTime = &secondTime;
        }
        else
        {
            repeatTime = &secondTime;
        }
    }
    if (repeatTime->heldTime >= initialRepeatDelay)
    {
        if (repeatTime->repeatTime == -1.0f)
        {
            repeatTime->repeatTime = initialRepeatDelay;
            return value;
        }
        if (repeatTime->heldTime - repeatTime->repeatTime > repeatDelay)
        {
            repeatTime->repeatTime += repeatDelay;
            return value;
        }
    }
    else if (!repeatTime->repeated)
    {
        repeatTime->repeated = 1;
        return value;
    }
    return 0;
}
void CInput::SetFakePadValue(int pad, unsigned char map)
{
    bool menuOpen = pad < 4 && menuDisplayed[pad];
    int mappedPad = pad;
    if (RemapIpad(mappedPad) && (!menuOpen || map <= controllerConfiguration[2]))
    {
        joypads[mappedPad].fakeValue |= GetGameJoypadMaps(pad, map);
        joypads[mappedPad].fakeTimer = 10;
    }
}
bool CInput::ButtonPressed(int pad, unsigned char map, unsigned int mask)
{
    int mappedPad = pad;
    if (!RemapIpad(mappedPad))
    {
        return false;
    }
    if (map == 0xff)
    {
        return joypads[mappedPad].pressedValue != 0;
    }
    unsigned int mapping = GetGameJoypadMaps(pad, map);
    if (map <= controllerConfiguration[2])
    {
        return (joypads[mappedPad].pressedValue & mapping) != 0;
    }
    if (pad < 4 && menuDisplayed[pad])
    {
        return false;
    }
    unsigned int value = joypads[mappedPad].pressedValue & mapping;
    if (!only_one_bit_set(mapping) && (joypads[mappedPad].rawValue & mask) != 0)
    {
        value |= mask;
    }
    return value == mapping && allowInput(pad, map, value);
}
bool CInput::ButtonReleased(int pad, unsigned char map)
{
    int mappedPad = pad;
    return RemapIpad(mappedPad) && (!(pad < 4 && menuDisplayed[pad]) || map <= controllerConfiguration[2]) &&
           (joypads[mappedPad].releasedValue & GetGameJoypadMaps(pad, map)) != 0;
}
bool CInput::ButtonDown(int pad, unsigned char map)
{
    int mappedPad = pad;
    if (!RemapIpad(mappedPad))
    {
        return false;
    }
    if (map == 0xff)
    {
        return joypads[mappedPad].rawValue != 0;
    }
    if (pad < 4 && menuDisplayed[pad] && map > controllerConfiguration[2])
    {
        return false;
    }
    unsigned int mapping = GetGameJoypadMaps(pad, map);
    unsigned int value = joypads[mappedPad].rawValue & mapping;
    if (map <= controllerConfiguration[2])
    {
        return value != 0;
    }
    return value == mapping && allowInput(pad, map, value);
}
void CInput::SetJoypadStickAxisMap(int pad, unsigned int destination, unsigned int source)
{
    if (!RemapIpad(pad))
    {
        return;
    }
    float* sourceValue = 0;
    if (source == 0)
        sourceValue = &joypads[pad].rightX;
    else if (source == 1)
        sourceValue = &joypads[pad].rightY;
    else if (source == 2)
        sourceValue = &joypads[pad].leftX;
    else if (source == 3)
        sourceValue = &joypads[pad].leftY;
    if (destination == 0)
        joypads[pad].leftXSource = sourceValue;
    else if (destination == 1)
        joypads[pad].leftYSource = sourceValue;
    else if (destination == 2)
        joypads[pad].rightXSource = sourceValue;
    else if (destination == 3)
        joypads[pad].rightYSource = sourceValue;
}
void CInput::SetJoypadStickTriggerMap(int pad, unsigned int destination, unsigned int source)
{
    if (!RemapIpad(pad))
    {
        return;
    }
    unsigned char* sourceValue = 0;
    if (source == 0)
        sourceValue = &joypads[pad].hasTouch;
    else if (source == 1)
        sourceValue = &joypads[pad].touchPressed;
    if (destination == 0)
        joypads[pad].leftTriggerSource = sourceValue;
    else if (destination == 1)
        joypads[pad].rightTriggerSource = sourceValue;
}
void CInput::SetKeyRepeatRate(float initialDelay, float delay)
{
    initialRepeatDelay = initialDelay;
    repeatDelay = delay;
}
void CInput::SetDebugSequence(const char* sequence, InputCallback callback, void* context)
{
    debugSequenceLength = std::strlen(sequence);
    debugSequence = new char[debugSequenceLength + 1];
    std::strcpy(debugSequence, sequence);
    debugSequenceCallback = callback;
    debugSequenceContext = context;
}
double CInput::GetIdleSeconds(int pad)
{
    return static_cast<double>(OSGetTime() - GetTimeValue(idleStartTimes[pad])) * timeScale;
}
unsigned char CInput::IsPadConnected(int pad)
{
    int mappedPad = pad;
    if (RemapIpad(mappedPad))
    {
        return joypads[mappedPad].connected;
    }
    return pad < static_cast<int>(controllerCount);
}
void CInput::SetCircleCrossSwapped(bool value)
{
    circleCrossSwapped = value;
}
unsigned char CInput::IsCircleCrossSwapped()
{
    return circleCrossSwapped;
}
void CInput::MapTouchInput(int pad, unsigned int mask)
{
    joypads[pad].fakeValue = joypads[pad].fakeValue & 0xff3fc300u | mask;
}
double CInput::GetJoypadStick_LX(int pad, bool checkMenu)
{
    bool menuShown = false;
    if (pad < 4)
    {
        menuShown = menuDisplayed[pad];
    }
    int mappedPad = pad;
    if (RemapIpad(mappedPad) && (!checkMenu || !menuShown))
    {
        return *joypads[mappedPad].leftXSource * joypads[mappedPad].sensitivity;
    }
    return 0.0;
}
double CInput::GetJoypadStick_LY(int pad, bool checkMenu)
{
    bool menuShown = false;
    if (pad < 4)
    {
        menuShown = menuDisplayed[pad];
    }
    int mappedPad = pad;
    if (RemapIpad(mappedPad) && (!checkMenu || !menuShown))
    {
        return *joypads[mappedPad].leftYSource * joypads[mappedPad].sensitivity;
    }
    return 0.0;
}
double CInput::GetJoypadStick_RX(int pad, bool checkMenu)
{
    bool menuShown = false;
    if (pad < 4)
    {
        menuShown = menuDisplayed[pad];
    }
    int mappedPad = pad;
    if (RemapIpad(mappedPad) && (!checkMenu || !menuShown))
    {
        return *joypads[mappedPad].rightXSource * joypads[mappedPad].sensitivity;
    }
    return 0.0;
}
double CInput::GetJoypadStick_RY(int pad, bool checkMenu)
{
    bool menuShown = false;
    if (pad < 4)
    {
        menuShown = menuDisplayed[pad];
    }
    int mappedPad = pad;
    if (RemapIpad(mappedPad) && (!checkMenu || !menuShown))
    {
        return *joypads[mappedPad].rightYSource * joypads[mappedPad].sensitivity;
    }
    return 0.0;
}
unsigned char CInput::GetJoypadLTrigger(int pad, bool raw)
{
    if (pad < 4 && raw && menuDisplayed[pad])
    {
        return 0;
    }
    if (RemapIpad(pad) && joypads[pad].leftTriggerSource != 0)
    {
        return *joypads[pad].leftTriggerSource;
    }
    return 0;
}
unsigned char CInput::GetJoypadRTrigger(int pad, bool raw)
{
    if (pad < 4 && raw && menuDisplayed[pad])
    {
        return 0;
    }
    if (RemapIpad(pad) && joypads[pad].rightTriggerSource != 0)
    {
        return *joypads[pad].rightTriggerSource;
    }
    return 0;
}
unsigned short* CInput::GetTouchPadData(int pad, C_4JInput::ETouchType type, bool previous)
{
    if (!RemapIpad(pad) || pad != 0 || !joypads[pad].connected || (previous && joypads[pad].sampleIndex))
    {
        return 0;
    }
    if (type == C_4JInput::ETouchType_0)
    {
        return reinterpret_cast<unsigned short*>(&static_cast<VPADStatus*>(joypads[pad].statusStorage.vpad.statuses)[joypads[pad].statusIndex].tpNormal);
    }
    if (type == C_4JInput::ETouchType_1)
    {
        return reinterpret_cast<unsigned short*>(&static_cast<VPADStatus*>(joypads[pad].statusStorage.vpad.statuses)[joypads[pad].statusIndex].tpFiltered1);
    }
    if (type == C_4JInput::ETouchType_2)
    {
        return reinterpret_cast<unsigned short*>(&static_cast<VPADStatus*>(joypads[pad].statusStorage.vpad.statuses)[joypads[pad].statusIndex].tpFiltered2);
    }
    return 0;
}
void CInput::SetMenuDisplayed(int pad, bool displayed)
{
    menuDisplayed[pad] = displayed;
}
void CInput::SetLocalMultiplayer(bool value)
{
    localMultiplayer = value;
}
unsigned int CInput::IsLocalMultiplayerAvailable()
{
    return 1;
}
unsigned int CInput::UsingRemoteVita()
{
    return 0;
}
unsigned int CInput::VerifyStrings(wchar_t** strings, int (*callback)(void*, _STRING_VERIFY_RESPONSE*), void* context)
{
    return 1;
}
unsigned int CInput::LockPad(int pad, int controllerId)
{
    if (!RemapIpad(pad))
    {
        return 0;
    }
    int previousPad = padRemap[controllerId];
    if (previousPad != pad)
    {
        if (previousPad >= 0)
        {
            joypads[previousPad].lockedController = -1;
        }
        int previousController = joypads[pad].lockedController;
        if (previousController >= 0)
        {
            padRemap[previousController] = -1;
        }
        padRemap[controllerId] = pad;
        joypads[pad].lockedController = controllerId;
        if (previousPad >= 0)
        {
            for (int controller = 0; controller < 4; ++controller)
            {
                if (padRemap[controller] == -1)
                {
                    padRemap[controller] = previousPad;
                    joypads[previousPad].lockedController = controller;
                    break;
                }
            }
        }
    }
    return 1;
}
int CInput::UnlockPad(int pad)
{
    if (pad < 4)
    {
        int mappedPad = padRemap[pad];
        if (mappedPad >= 0)
        {
            joypads[mappedPad].lockedController = -1;
            padRemap[pad] = -1;
            unsigned int previousButtons = joypads[mappedPad].rawValue;
            ClearJoypadValues(&joypads[mappedPad]);
            joypads[mappedPad].rawValue = previousButtons;
            return mappedPad + 4;
        }
    }
    return -1;
}
unsigned int CInput::IsPadLocked(int pad)
{
    if (RemapIpad(pad))
    {
        return joypads[pad].lockedController >= 0;
    }
    return 0;
}
int CInput::GetControllerID(int pad)
{
    return RemapIpad(pad) ? pad + 4 : -1;
}
unsigned char CInput::IsPadDRCController(int pad)
{
    return RemapIpad(pad) && pad == 0 && joypads[0].connected;
}
unsigned int CInput::IsPadClassicController(int pad)
{
    if (!RemapIpad(pad) || pad == 0)
    {
        return 0;
    }
    KPADStatus* status = static_cast<KPADStatus*>(joypads[pad].statusStorage.kpad.statuses) + joypads[pad].statusIndex;
    return status->extensionType == 2 || status->extensionType == 7;
}
unsigned char CInput::IsDRCConnected()
{
    return joypads[0].connected;
}
void CInput::AcquiredForeground()
{
    inputTick = 20;
}
bool CInput::IsSet(int pad, unsigned char map)
{
    int mappedPad = pad;
    if (!RemapIpad(mappedPad))
    {
        return false;
    }
    if (pad < 4 && menuDisplayed[pad] && map > controllerConfiguration[2])
    {
        return false;
    }
    return (joypads[mappedPad].rawValue & GetGameJoypadMaps(pad, map)) != 0;
}
double CInput::GetJoypadStick_Menu_LX(int pad)
{
    int mappedPad = pad;
    return RemapIpad(mappedPad) ? joypads[mappedPad].rightX : 0.0;
}
double CInput::GetJoypadStick_Menu_LY(int pad)
{
    int mappedPad = pad;
    return RemapIpad(mappedPad) ? joypads[mappedPad].rightY : 0.0;
}
double CInput::GetJoypadStick_Menu_RX(int pad)
{
    int mappedPad = pad;
    return RemapIpad(mappedPad) ? joypads[mappedPad].leftX : 0.0;
}
double CInput::GetJoypadStick_Menu_RY(int pad)
{
    int mappedPad = pad;
    return RemapIpad(mappedPad) ? joypads[mappedPad].leftY : 0.0;
}
unsigned char CInput::GetJoypadLTrigger_Menu(int pad)
{
    int mappedPad = pad;
    return RemapIpad(mappedPad) ? joypads[mappedPad].hasTouch : 0;
}
unsigned char CInput::GetJoypadRTrigger_Menu(int pad)
{
    int mappedPad = pad;
    return RemapIpad(mappedPad) ? joypads[mappedPad].touchPressed : 0;
}
int CInput::GetVPadStatus()
{
    if (joypads[0].sampleIndex == 0)
    {
        joypads[0].sampleIndex = 1;
        return (int)(static_cast<VPADStatus*>(joypads[0].statusStorage.vpad.statuses) + joypads[0].statusIndex);
    }
    return 0;
}
int CInput::GetKPadStatus(int pad)
{
    if (joypads[pad].sampleIndex == 0)
    {
        joypads[pad].sampleIndex = 1;
        return (int)(static_cast<KPADStatus*>(joypads[pad].statusStorage.kpad.statuses) + joypads[pad].statusIndex);
    }
    return 0;
}
void CInput::CancelQueuedVerifyStrings(int (*callback)(void*, _STRING_VERIFY_RESPONSE*), void* context)
{
}
void CInput::CancelAllVerifyInProgress()
{
}
unsigned char CInput::IsShowingOnscreenKeyboard()
{
    return keyboardVisible;
}
void CInput::SetKeyboardMode(eKeyboardProcessState value)
{
    keyboardState = value;
}
unsigned int CInput::RequestKeyboard(const wchar_t* title, const wchar_t* initialText, unsigned int pad, unsigned int maximumLength, KeyboardCallback callback, void* context, C_4JInput::EKeyboardMode mode)
{
    if (keyboardState != eKeyboardProcessState_0)
    {
        s_inputInstance->DebugPrintf("CInput::RequestKeyboard: Ignored a keyboard request while keyboard is active");
        return 1;
    }

    SetKeyboardMode(eKeyboardProcessState_1);
    const wchar_t* diagnosticText = initialText == 0 ? L"" : initialText;
    s_inputInstance->DebugPrintf("CInput::RequestKeyboard: Opening keyboard : maxChars - %d init s", maximumLength, diagnosticText);
    keyboardCallback = callback;
    keyboardMaximumLength = maximumLength;
    keyboardContext = context;
    keyboardPad = pad;
    keyboardMode = mode;
    s_inputInstance->RemapIpad(keyboardPad);
    std::memset(keyboardText, 0, 0x200);
    std::memcpy(keyboardText, initialText, maximumLength << 1);
    return 0;
}
void CInput::DestroyKeyboard()
{
    if (keyboardState == eKeyboardProcessState_2)
    {
        destroyKeyboardRequested = 1;
    }
}
void CInput::GetText(wchar_t* value, int length)
{
    std::wcsncpy(value, keyboardText, length);
}
void CInput::ProcFont(int value, void* context)
{
    for (;;)
    {
        OSMessage message;
        OSReceiveMessage(&s_fontQueue, &message, OS_MESSAGE_FLAGS_BLOCKING);
        nn::swkbd::CalcSubThreadFont();
    }
}
void CInput::ProcPredict(int value, void* context)
{
    for (;;)
    {
        OSMessage message;
        OSReceiveMessage(&s_predictQueue, &message, OS_MESSAGE_FLAGS_BLOCKING);
        nn::swkbd::CalcSubThreadPredict();
    }
}

static int ProcFontThreadEntry(int value, void* context)
{
    CInput::ProcFont(value, context);
    return 0;
}

static int ProcPredictThreadEntry(int value, void* context)
{
    CInput::ProcPredict(value, context);
    return 0;
}

void CInput::KeyboardOnCreate()
{
    keyboardState = eKeyboardProcessState_0;
    destroyKeyboardRequested = 0;
    OSInitMessageQueue(&s_fontQueue, &s_fontMessage, 1);
    OSInitMessageQueue(&s_predictQueue, &s_predictMessage, 1);
    C4JThreadImpl::OSCreateThread4J(&s_fontThread, ProcFontThreadEntry, 0, 0, s_fontStack + sizeof(s_fontStack), sizeof(s_fontStack), 0x11, 0);
    OSResumeThread(&s_fontThread);
    C4JThreadImpl::OSCreateThread4J(&s_predictThread, ProcPredictThreadEntry, 0, 0, s_predictStack + sizeof(s_predictStack), sizeof(s_predictStack), 0x12, 0);
    OSResumeThread(&s_predictThread);
}
unsigned int CInput::GetKeyboardControllerType()
{
    static const unsigned char controllerTypes[5] = { 4, 0, 1, 2, 3 };
    if (keyboardPad < 5)
    {
        return controllerTypes[keyboardPad];
    }
    return keyboardPad;
}
unsigned int CInput::GetKeyboardMode()
{
    unsigned int mode = static_cast<unsigned int>(keyboardMode);
    if (mode == 1 || mode == 6 || mode == 7)
    {
        keyboardControllerType = 0x7ffff;
        return 1;
    }
    keyboardControllerType = 0x7ffff;
    if (mode != 3 && mode != 5)
    {
        return 0;
    }
    if (keyboardLanguage == 1 || keyboardLanguage == 0x10 || keyboardLanguage == 0x20)
    {
        keyboardControllerType = 2;
    }
    else if (keyboardLanguage == 2)
    {
        if (keyboardRegion == 2)
        {
            keyboardControllerType = 4;
        }
        else if (keyboardRegion == 5)
        {
            keyboardControllerType = 8;
        }
        else
        {
            keyboardControllerType = 2;
        }
    }
    else if (keyboardLanguage == 4)
    {
        static const unsigned short accessFlags[] = { 0x20, 0x20, 0x40, 0x80, 0x100, 0x200 };
        keyboardControllerType = keyboardRegion < 6 ? accessFlags[keyboardRegion] : keyboardRegion == 8 ? 0x400 : keyboardRegion == 9 ? 0x800 : 0x20;
    }
    return 0;
}
unsigned char CInput::GetLanguageType()
{
    static const unsigned char regionLanguages[] = { 0, 1, 1, 6, 7, 11 };
    static const unsigned char languages[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
    if (keyboardRegion < 12)
    {
        return languages[keyboardRegion];
    }
    if (keyboardLanguage < 6)
    {
        return regionLanguages[keyboardLanguage];
    }
    return 1;
}
void CInput::TickKeyboard()
{
    if (keyboardState == eKeyboardProcessState_1)
    {
        nn::swkbd::AppearArg appearArg;
        appearArg.keyboardArg.configArg.controllerType = static_cast<nn::swkbd::ControllerType>(GetKeyboardControllerType());
        appearArg.keyboardArg.configArg.keyboardMode = static_cast<nn::swkbd::KeyboardMode>(GetKeyboardMode());
        appearArg.keyboardArg.configArg.accessFlags = keyboardControllerType;
        appearArg.keyboardArg.configArg.languageType = static_cast<nn::swkbd::LanguageType>(GetLanguageType());
        appearArg.inputFormArg.type = static_cast<nn::swkbd::InputFormType>(1);
        appearArg.inputFormArg.initialText = reinterpret_cast<const char16_t*>(keyboardText);
        appearArg.inputFormArg.unk_0x04 = -1;
        appearArg.inputFormArg.hintText = 0;
        appearArg.inputFormArg.maxTextLength = keyboardMaximumLength;
        appearArg.inputFormArg.passwordMode = static_cast<nn::swkbd::PasswordMode>(0);
        nn::swkbd::AppearInputForm(appearArg);
        SetKeyboardMode(eKeyboardProcessState_2);
        return;
    }

    if (keyboardState == eKeyboardProcessState_2)
    {
        keyboardVisible = 1;
        nn::swkbd::ControllerInfo controllerInfo;
        std::memset(&controllerInfo, 0, sizeof(controllerInfo));
        if (keyboardPad == 0)
        {
            controllerInfo.vpad = (VPADStatus*)s_inputInstance->GetVPadStatus();
        }
        else if (keyboardPad < 5)
        {
            controllerInfo.kpad[keyboardPad - 1] = (KPADStatus*)s_inputInstance->GetKPadStatus(keyboardPad);
        }
        nn::swkbd::Calc(controllerInfo);
        OSMessage fontMessage;
        OSMessage predictMessage;
        if (nn::swkbd::IsNeedCalcSubThreadFont())
        {
            OSSendMessage(&s_fontQueue, &fontMessage, OS_MESSAGE_FLAGS_NONE);
        }
        if (nn::swkbd::IsNeedCalcSubThreadPredict())
        {
            OSSendMessage(&s_predictQueue, &predictMessage, OS_MESSAGE_FLAGS_NONE);
        }
        if (nn::swkbd::IsDecideOkButton(0))
        {
            std::wcsncpy(keyboardText, (const wchar_t*)nn::swkbd::GetInputFormString(), keyboardMaximumLength);
            keyboardText[keyboardMaximumLength] = 0;
            if (keyboardCallback)
            {
            keyboardCallback(keyboardContext, true);
            }
            nn::swkbd::DisappearInputForm();
        }
        else if (nn::swkbd::IsDecideCancelButton(0) || destroyKeyboardRequested)
        {
            if (keyboardCallback)
            {
                keyboardCallback(keyboardContext, false);
            }
            nn::swkbd::DisappearInputForm();
            destroyKeyboardRequested = 0;
        }
        if (static_cast<int>(nn::swkbd::GetStateKeyboard()) == 0)
        {
            SetKeyboardMode(eKeyboardProcessState_0);
        }
    }
}
void CInput::InitialiseKeyboard()
{
    keyboardFileClient = static_cast<FSClient*>(std::malloc(sizeof(FSClient)));
    FSAddClient(keyboardFileClient, FS_ERROR_FLAG_NONE);
    nn::swkbd::CreateArg createArg;
    keyboardErrorViewerWorkMemory = new unsigned char[nn::swkbd::GetWorkMemorySize(0)];
    createArg.workMemory = keyboardErrorViewerWorkMemory;
    createArg.fsClient = keyboardFileClient;
    unsigned int platformRegion = 4;
    if (SCIGetPlatformRegion(&platformRegion) == 1)
    {
        if (platformRegion == 1)
            keyboardLanguage = 0;
        else if (platformRegion == 2)
            keyboardLanguage = 1;
        else if (platformRegion == 0x10)
            keyboardLanguage = 3;
        else if (platformRegion == 0x20)
            keyboardLanguage = 4;
        else if (platformRegion == 0x40)
            keyboardLanguage = 5;
        else
            keyboardLanguage = 2;
    }
    createArg.regionType = static_cast<nn::swkbd::RegionType>(keyboardLanguage);
    if (SCIGetCafeLanguage(&keyboardRegion) != 1)
    {
        keyboardRegion = 0xff;
    }
    nn::swkbd::Create(createArg);
}
unsigned char CInput::IsShowingErrorViewer()
{
    return errorViewerVisible;
}
void CInput::SetErrorViewerMode(eErrorViewerProcessState value)
{
    errorViewerState = value;
}
void CInput::RequestErrorViewer(unsigned int value, unsigned int type, ErrorViewerCallback callback, void* context)
{
    if (errorViewerState == ERROR_VIEWER_NONE)
    {
        SetErrorViewerMode(ERROR_VIEWER_REQUESTED);
        errorViewerController = value;
        errorViewerContext = context;
        errorViewerMessage = 0;
        errorViewerCode = type;
        errorViewerTitle = 0;
        errorViewerCallback = callback;
    }
}
void CInput::RequestErrorViewer(unsigned int value, wchar_t* title, wchar_t* message, ErrorViewerCallback callback, void* context)
{
    if (errorViewerState == ERROR_VIEWER_NONE)
    {
        SetErrorViewerMode(ERROR_VIEWER_REQUESTED);
        errorViewerContext = context;
        errorViewerController = value;
        errorViewerMessage = message;
        errorViewerCallback = callback;
        errorViewerTitle = title;
    }
}
void CInput::DestroyErrorViewer()
{
    if (errorViewerState == ERROR_VIEWER_ACTIVE)
    {
        SetErrorViewerMode(ERROR_VIEWER_DESTROYED);
    }
}
void CInput::ErrorViewerOnCreate()
{
    errorViewerVisible = 1;
    errorViewerState = ERROR_VIEWER_NONE;
    errorViewerMessage = 0;
    errorViewerController = 0;
    errorViewerTitle = 0;
}
void CInput::InitialiseErrorViewer()
{
    errorViewerFileClient = static_cast<FSClient*>(std::malloc(sizeof(FSClient)));
    FSAddClient(errorViewerFileClient, FS_ERROR_FLAG_NONE);
    nn::erreula::CreateArg createArg;
    keyboardErrorViewerWorkMemory = std::malloc(nn::erreula::GetWorkMemorySize());
    createArg.workMemory = keyboardErrorViewerWorkMemory;
    createArg.fsClient = errorViewerFileClient;
    unsigned int platformRegion = 4;
    if (SCIGetPlatformRegion(&platformRegion) == 1)
    {
        createArg.region = static_cast<nn::erreula::RegionType>(platformRegion == 4 ? 2 : platformRegion == 2 ? 1 : 0);
    }
    unsigned int language = 1;
    if (SCIGetCafeLanguage(&language) == 1 && language < 12)
    {
        createArg.language = static_cast<nn::erreula::LangType>(language);
    }
    nn::erreula::Create(createArg);
}
unsigned int CInput::GetErrorViewerControllerType()
{
    static const unsigned char controllerTypes[5] = { 0, 0, 1, 2, 3 };
    return errorViewerController < 5 ? controllerTypes[errorViewerController] : errorViewerController;
}
void CInput::TickErrorViewer()
{
    if (errorViewerState == ERROR_VIEWER_REQUESTED)
    {
        nn::erreula::AppearArg appearArg;
        appearArg.errorArg.controllerType = static_cast<nn::erreula::ControllerType>(GetErrorViewerControllerType());
        appearArg.errorArg.renderTarget = static_cast<nn::erreula::RenderTarget>(2);
        if (errorViewerTitle == 0)
        {
            appearArg.errorArg.errorCode = errorViewerCode;
        }
        else
        {
            appearArg.errorArg.errorType = static_cast<nn::erreula::ErrorType>(2);
            appearArg.errorArg.errorTitle = reinterpret_cast<const char16_t*>(errorViewerTitle);
            appearArg.errorArg.errorMessage = reinterpret_cast<const char16_t*>(errorViewerMessage);
        }
        nn::erreula::AppearErrorViewer(appearArg);
        SetErrorViewerMode(ERROR_VIEWER_ACTIVE);
    }
    else if (errorViewerState == ERROR_VIEWER_ACTIVE)
    {
        nn::erreula::ControllerInfo controllerInfo;
        std::memset(&controllerInfo, 0, sizeof(controllerInfo));
        errorViewerVisible = 1;
        controllerInfo.vpad = (VPADStatus*)s_inputInstance->GetVPadStatus();
        for (int pad = 1; pad < 5; ++pad)
        {
            controllerInfo.kpad[pad - 1] = (KPADStatus*)s_inputInstance->GetKPadStatus(pad);
        }
        nn::erreula::Calc(controllerInfo);
        if (nn::erreula::IsDecideSelectButtonError() && static_cast<int>(nn::erreula::GetStateErrorViewer()) == 2)
        {
            nn::erreula::DisappearErrorViewer();
        }
        if (static_cast<int>(nn::erreula::GetStateErrorViewer()) != 0)
        {
            return;
        }

        unsigned int result = static_cast<unsigned int>(nn::erreula::GetResultType());
        if (result < 2)
        {
            SetErrorViewerMode(ERROR_VIEWER_NONE);
            if (errorViewerCallback == 0)
            {
                return;
            }
            errorViewerCallback(errorViewerContext);
            return;
        }
        if (result == 2)
        {
            nn::erreula::AppearArg appearArg;
            appearArg.errorArg.controllerType = static_cast<nn::erreula::ControllerType>(GetErrorViewerControllerType());
            appearArg.errorArg.renderTarget = static_cast<nn::erreula::RenderTarget>(2);
            appearArg.errorArg.errorCode = nn::erreula::GetResultCode();
            nn::erreula::AppearErrorViewer(appearArg);
            return;
        }
        if (result != 3)
        {
            return;
        }
        SetErrorViewerMode(ERROR_VIEWER_NONE);
        int jumpResult = nn::erreula::Jump("my app", 6);
        std::printf("nn::erreula::Jump returned %d\n", jumpResult);
        return;
    }
    else if (errorViewerState == ERROR_VIEWER_DESTROYED)
    {
        nn::erreula::DisappearErrorViewer();
        SetErrorViewerMode(ERROR_VIEWER_DISAPPEARING);
    }
    else if (errorViewerState == ERROR_VIEWER_DISAPPEARING && static_cast<int>(nn::erreula::GetStateErrorViewer()) == 0)
    {
        SetErrorViewerMode(ERROR_VIEWER_NONE);
    }
    nn::erreula::ControllerInfo controllerInfo;
    std::memset(&controllerInfo, 0, sizeof(controllerInfo));
    nn::erreula::Calc(controllerInfo);
}
void CInput::DebugPrintf(const char*, ...)
{
}

void CInput::JOYPAD::setConnected(bool connected)
{
    if (this->connected != connected)
    {
        this->connected = connected;
        newlyConnected = connected;
        disconnected = connected ^ 1;
        return;
    }

    disconnected = 0;
    newlyConnected = 0;
}
