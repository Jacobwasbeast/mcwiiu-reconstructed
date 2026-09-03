#ifndef GAME_CORE_CINPUT_HPP
#define GAME_CORE_CINPUT_HPP

class CInput
{
public:
    CInput();
    class JOYPAD
    {
    public:
        unsigned char state[5];
        void setConnected(bool connected);
    };
    unsigned char storage[5352];
    void InitTime();
    static unsigned long long ConnectCallback(int value, int type);
    void Initialise(int value, unsigned char type, unsigned char flags);
    void ReadAllPads();
    void ClearJoypadValues(JOYPAD* value);
    void JoypadConnected(int value);
    void JoypadRemoved(int value);
    void SetDRCJoypadValue(JOYPAD* value, unsigned int type, unsigned int flags);
    unsigned long long SetJoypadValuesDRC(JOYPAD* value);
    void GetStartTime(int value, int type);
    void UpdateTime(int value, int type);
    unsigned int UpdateDRCPad();
    void SetClassicJoypadValue(JOYPAD* value, unsigned int type, unsigned int flags);
    void SetFreestyleJoypadValue(JOYPAD* value, unsigned int type, unsigned int flags);
    void SetURCCJoypadValue(JOYPAD* value, unsigned int type, unsigned int flags);
    unsigned long long SetJoypadValues(JOYPAD* value);
    unsigned int UpdateJoypads();
    unsigned long long Tick();
    void SetDeadzoneAndMovementRange(unsigned int value, unsigned int type, unsigned int flags);
    void SetGameControllerTypeJoypadMaps(int value, unsigned char type, unsigned int flags);
    unsigned int RemapIpad(int* value);
    unsigned long long SetGameJoypadMaps(int value, unsigned char type, unsigned int flags);
    unsigned int GetGameJoypadMaps(int value, unsigned char type);
    void SetJoypadMapArraySetupComplete();
    unsigned long long SetJoypadSensitivity(int value, float type);
    unsigned int only_one_bit_set(unsigned int value);
    unsigned int allowInput(int value, unsigned char type, unsigned int flags);
    unsigned int GetValue(int value, unsigned char type, bool flags);
    unsigned long long SetFakePadValue(int value, unsigned char type);
    bool ButtonPressed(int value, unsigned char type, unsigned int flags);
    bool ButtonReleased(int value, unsigned char type);
    bool ButtonDown(int value, unsigned char type);
    unsigned long long SetJoypadStickAxisMap(int value, unsigned int type, unsigned int flags, unsigned int count, unsigned int callback, int context);
    unsigned long long SetJoypadStickTriggerMap(int value, int type, int flags, unsigned int count, unsigned int callback, int context);
    void SetKeyRepeatRate(float value, float type);
    unsigned long long SetDebugSequence(unsigned int value, unsigned int type, unsigned int flags);
    double GetIdleSeconds(int value);
    unsigned char IsPadConnected(int value);
    void SetCircleCrossSwapped(bool value);
    unsigned char IsCircleCrossSwapped();
    void MapTouchInput(int value, unsigned int type);
    double GetJoypadStick_LX(int value, bool type);
    double GetJoypadStick_LY(int value, bool type);
    double GetJoypadStick_RX(int value, bool type);
    double GetJoypadStick_RY(int value, bool type);
    unsigned char GetJoypadLTrigger(int value, bool type);
    unsigned char GetJoypadRTrigger(int value, bool type);
    int GetTouchPadData(int value, int type, bool flags);
    void SetMenuDisplayed(int value, bool type);
    void SetLocalMultiplayer(bool value);
    unsigned int IsLocalMultiplayerAvailable();
    unsigned int UsingRemoteVita();
    unsigned int VerifyStrings();
    unsigned int LockPad(int value, int type);
    int UnlockPad(int value);
    unsigned int IsPadLocked(int value);
    int GetControllerID(int value);
    unsigned char IsPadDRCController(int value);
    unsigned int IsPadClassicController(int value);
    unsigned char IsDRCConnected();
    void AcquiredForeground();
    bool IsSet(int value, unsigned char type);
    double GetJoypadStick_Menu_LX(int value);
    double GetJoypadStick_Menu_LY(int value);
    double GetJoypadStick_Menu_RX(int value);
    double GetJoypadStick_Menu_RY(int value);
    unsigned char GetJoypadLTrigger_Menu(int value);
    unsigned char GetJoypadRTrigger_Menu(int value);
    int GetVPadStatus();
    int GetKPadStatus(int value);
    void CancelQueuedVerifyStrings();
    void CancelAllVerifyInProgress();
    unsigned char IsShowingOnscreenKeyboard();
    void SetKeyboardMode(unsigned int value);
    unsigned int RequestKeyboard(unsigned int value, unsigned int type, unsigned int flags, int count, unsigned int callback, unsigned int context, unsigned int mode);
    void DestroyKeyboard();
    void GetText(wchar_t* value, int type);
    static void ProcFont(int value, unsigned int type);
    static void ProcPredict(int value, unsigned int type);
    void KeyboardOnCreate();
    unsigned int GetKeyboardControllerType();
    unsigned int GetKeyboardMode();
    unsigned char GetLanguageType();
    void TickKeyboard();
    void InitialiseKeyboard();
    unsigned char IsShowingErrorViewer();
    void SetErrorViewerMode(unsigned int value);
    unsigned long long RequestErrorViewer(unsigned int value, unsigned int type, unsigned int flags, unsigned int count);
    unsigned long long RequestErrorViewer(unsigned int value, unsigned int type, unsigned int flags, unsigned int count, unsigned int callback);
    void DestroyErrorViewer();
    void ErrorViewerOnCreate();
    void InitialiseErrorViewer();
    unsigned int GetErrorViewerControllerType();
    void TickErrorViewer();
    ~CInput();
    void SetSigninJoypadMask(unsigned int);
    static const wchar_t* getDeviceTypeName(unsigned int);
    int GetControllerType(int);
    void FinaliseKeyboard();
    void FinaliseErrorViewer();
    void DebugPrintf(const char*, ...);
};

extern CInput* g_pInput;

#endif

