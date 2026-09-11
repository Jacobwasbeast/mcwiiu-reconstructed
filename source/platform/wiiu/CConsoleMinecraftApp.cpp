#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/util/Vec3.hpp"
#include "platform/wiiu/CInput.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/PIXNamedEvents.hpp"

#include <coreinit/messagequeue.h>
#include <coreinit/thread.h>

extern "C" int ProcUIProcessMessages(int count);
extern "C" void ShutdownAndExit();
extern "C" void ReleaseCallback();
extern void InitializeCriticalSection(OSFastMutex*);
extern void EnterCriticalSection(OSFastMutex*);
extern void LeaveCriticalSection(OSFastMutex*);
void Sleep(unsigned int milliseconds);

extern CInput* g_pInput;
extern Renderer* gRenderer;
extern CConsoleMinecraftApp gApp;
extern unsigned char s_altMainOKToRun;
extern unsigned char s_altMainPending;
extern unsigned char s_altMainExit;
extern OSMessageQueue gMainMessageQueue;
extern OSMessage gMainMessageBuffer;

static OSFastMutex s_altMainMutex;
static unsigned char s_altMainStopRequested;

void altMain()
{
    s_altMainPending = 1;
    OSSendMessage(&gMainMessageQueue, &gMainMessageBuffer, OS_MESSAGE_FLAGS_NONE);
}

void altMainStop()
{
    s_altMainStopRequested = 0;
}

void waitForAltMainThreadToComplete()
{
    while (s_altMainStopRequested != 0)
        Sleep(10);
}

int altMainThread(int, void*)
{
    InitializeCriticalSection(&s_altMainMutex);
    if (s_altMainExit == 0)
    {
        do
        {
            OSReceiveMessage(&gMainMessageQueue, &gMainMessageBuffer, OS_MESSAGE_FLAGS_BLOCKING);
            while (s_altMainOKToRun == 0)
            {
                Sleep(10);
            }

            EnterCriticalSection(&s_altMainMutex);
            s_altMainStopRequested = 1;
            Sleep(200);
            s_altMainPending = 0;
            s_altMainOKToRun = 0;

            while (s_altMainStopRequested != 0)
            {
                int processResult = ProcUIProcessMessages(1);
                if (processResult == 3)
                {
                    s_altMainExit = 1;
                    LeaveCriticalSection(&s_altMainMutex);
                    ShutdownAndExit();
                    LeaveCriticalSection(&s_altMainMutex);
                    break;
                }
                if (processResult == 2)
                {
                    ReleaseCallback();
                }

                g_pInput->Tick();
                gApp.UpdateTime();

                PIXBeginNamedEvent(0.0f, "Input manager tick");
                PIXEndNamedEvent();

                PIXBeginNamedEvent(0.0f, "Render manager tick");
                gRenderer->Tick();
                PIXEndNamedEvent();

                PIXBeginNamedEvent(0.0f, "Network manager do work #1");
                CGameNetworkManager::sInstance.DoWork();
                PIXEndNamedEvent();

                PIXBeginNamedEvent(0.0f, "Frame present");
                gRenderer->Present();
                PIXEndNamedEvent();

                Vec3::resetPool();
            }

            LeaveCriticalSection(&s_altMainMutex);
        }
        while (s_altMainExit == 0);
    }

    return 0;
}
