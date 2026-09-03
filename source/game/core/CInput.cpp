#include "game/core/CInput.hpp"

CInput* g_pInput = 0;

CInput::CInput(){}
CInput::~CInput(){}
void CInput::InitTime() {}
unsigned long long CInput::ConnectCallback(int value, int type) { return 0; }
void CInput::Initialise(int value, unsigned char type, unsigned char flags) {}
void CInput::ReadAllPads() {}
void CInput::ClearJoypadValues(JOYPAD* value) {}
void CInput::JoypadConnected(int value) {}
void CInput::JoypadRemoved(int value) {}
void CInput::SetDRCJoypadValue(JOYPAD* value, unsigned int type, unsigned int flags) {}
unsigned long long CInput::SetJoypadValuesDRC(JOYPAD* value) { return 0; }
void CInput::GetStartTime(int value, int type) {}
void CInput::UpdateTime(int value, int type) {}
unsigned int CInput::UpdateDRCPad() { return 0; }
void CInput::SetClassicJoypadValue(JOYPAD* value, unsigned int type, unsigned int flags) {}
void CInput::SetFreestyleJoypadValue(JOYPAD* value, unsigned int type, unsigned int flags) {}
void CInput::SetURCCJoypadValue(JOYPAD* value, unsigned int type, unsigned int flags) {}
unsigned long long CInput::SetJoypadValues(JOYPAD* value) { return 0; }
unsigned int CInput::UpdateJoypads() { return 0; }
unsigned long long CInput::Tick() { return 0; }
void CInput::SetDeadzoneAndMovementRange(unsigned int value, unsigned int type, unsigned int flags) {}
void CInput::SetGameControllerTypeJoypadMaps(int value, unsigned char type, unsigned int flags) {}
unsigned int CInput::RemapIpad(int* value) { return 0; }
unsigned long long CInput::SetGameJoypadMaps(int value, unsigned char type, unsigned int flags) { return 0; }
unsigned int CInput::GetGameJoypadMaps(int value, unsigned char type) { return 0; }
void CInput::SetJoypadMapArraySetupComplete() {}
unsigned long long CInput::SetJoypadSensitivity(int value, float type) { return 0; }
unsigned int CInput::only_one_bit_set(unsigned int value) { return 0; }
unsigned int CInput::allowInput(int value, unsigned char type, unsigned int flags) { return 0; }
unsigned int CInput::GetValue(int value, unsigned char type, bool flags) { return 0; }
unsigned long long CInput::SetFakePadValue(int value, unsigned char type) { return 0; }
bool CInput::ButtonPressed(int value, unsigned char type, unsigned int flags) { return false; }
bool CInput::ButtonReleased(int value, unsigned char type) { return false; }
bool CInput::ButtonDown(int value, unsigned char type) { return false; }
unsigned long long CInput::SetJoypadStickAxisMap(int value, unsigned int type, unsigned int flags, unsigned int count, unsigned int callback, int context) { return 0; }
unsigned long long CInput::SetJoypadStickTriggerMap(int value, int type, int flags, unsigned int count, unsigned int callback, int context) { return 0; }
void CInput::SetKeyRepeatRate(float value, float type) {}
unsigned long long CInput::SetDebugSequence(unsigned int value, unsigned int type, unsigned int flags) { return 0; }
double CInput::GetIdleSeconds(int value) { return 0; }
unsigned char CInput::IsPadConnected(int value) { return 0; }
void CInput::SetCircleCrossSwapped(bool value) {}
unsigned char CInput::IsCircleCrossSwapped() { return 0; }
void CInput::MapTouchInput(int value, unsigned int type) {}
double CInput::GetJoypadStick_LX(int value, bool type) { return 0; }
double CInput::GetJoypadStick_LY(int value, bool type) { return 0; }
double CInput::GetJoypadStick_RX(int value, bool type) { return 0; }
double CInput::GetJoypadStick_RY(int value, bool type) { return 0; }
unsigned char CInput::GetJoypadLTrigger(int value, bool type) { return 0; }
unsigned char CInput::GetJoypadRTrigger(int value, bool type) { return 0; }
int CInput::GetTouchPadData(int value, int type, bool flags) { return 0; }
void CInput::SetMenuDisplayed(int value, bool type) {}
void CInput::SetLocalMultiplayer(bool value) {}
unsigned int CInput::IsLocalMultiplayerAvailable() { return 0; }
unsigned int CInput::UsingRemoteVita() { return 0; }
unsigned int CInput::VerifyStrings() { return 0; }
unsigned int CInput::LockPad(int value, int type) { return 0; }
int CInput::UnlockPad(int value) { return 0; }
unsigned int CInput::IsPadLocked(int value) { return 0; }
int CInput::GetControllerID(int value) { return 0; }
unsigned char CInput::IsPadDRCController(int value) { return 0; }
unsigned int CInput::IsPadClassicController(int value) { return 0; }
unsigned char CInput::IsDRCConnected() { return 0; }
void CInput::AcquiredForeground() {}
bool CInput::IsSet(int value, unsigned char type) { return false; }
double CInput::GetJoypadStick_Menu_LX(int value) { return 0; }
double CInput::GetJoypadStick_Menu_LY(int value) { return 0; }
double CInput::GetJoypadStick_Menu_RX(int value) { return 0; }
double CInput::GetJoypadStick_Menu_RY(int value) { return 0; }
unsigned char CInput::GetJoypadLTrigger_Menu(int value) { return 0; }
unsigned char CInput::GetJoypadRTrigger_Menu(int value) { return 0; }
int CInput::GetVPadStatus() { return 0; }
int CInput::GetKPadStatus(int value) { return 0; }
void CInput::CancelQueuedVerifyStrings() {}
void CInput::CancelAllVerifyInProgress() {}
unsigned char CInput::IsShowingOnscreenKeyboard() { return 0; }
void CInput::SetKeyboardMode(unsigned int value) {}
unsigned int CInput::RequestKeyboard(unsigned int value, unsigned int type, unsigned int flags, int count, unsigned int callback, unsigned int context, unsigned int mode) { return 0; }
void CInput::DestroyKeyboard() {}
void CInput::GetText(wchar_t* value, int type) {}
void CInput::ProcFont(int value, unsigned int type) {}
void CInput::ProcPredict(int value, unsigned int type) {}
void CInput::KeyboardOnCreate() {}
unsigned int CInput::GetKeyboardControllerType() { return 0; }
unsigned int CInput::GetKeyboardMode() { return 0; }
unsigned char CInput::GetLanguageType() { return 0; }
void CInput::TickKeyboard() {}
void CInput::InitialiseKeyboard() {}
unsigned char CInput::IsShowingErrorViewer() { return 0; }
void CInput::SetErrorViewerMode(unsigned int value) {}
unsigned long long CInput::RequestErrorViewer(unsigned int value, unsigned int type, unsigned int flags, unsigned int count) { return 0; }
unsigned long long CInput::RequestErrorViewer(unsigned int value, unsigned int type, unsigned int flags, unsigned int count, unsigned int callback) { return 0; }
void CInput::DestroyErrorViewer() {}
void CInput::ErrorViewerOnCreate() {}
void CInput::InitialiseErrorViewer() {}
unsigned int CInput::GetErrorViewerControllerType() { return 0; }
void CInput::TickErrorViewer() {}
void CInput::SetSigninJoypadMask(unsigned int) {}
const wchar_t* CInput::getDeviceTypeName(unsigned int) { return L""; }
int CInput::GetControllerType(int) { return 0; }
void CInput::FinaliseKeyboard()
{
    if (IsShowingOnscreenKeyboard())
    {
        DestroyKeyboard();
    }
    else
    {
        SetKeyboardMode(0);
    }
}
void CInput::FinaliseErrorViewer()
{
    if (IsShowingErrorViewer())
    {
        DestroyErrorViewer();
    }
    else
    {
        SetErrorViewerMode(0);
    }
}
void CInput::DebugPrintf(const char*, ...) {}
void CInput::JOYPAD::setConnected(bool connected){ state[0] = connected ? 1 : 0; }
