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

extern OSFastMutex gMainMutex;
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
    if (s_altMainExit == 0)
    {
        do
        {
            gRenderer->InitialiseContext();
            OSReceiveMessage(&gMainMessageQueue, &gMainMessageBuffer, OS_MESSAGE_FLAGS_BLOCKING);
            while (s_altMainOKToRun == 0)
            {
                Sleep(10);
            }

            EnterCriticalSection(&gMainMutex);
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
                    LeaveCriticalSection(&gMainMutex);
                    ShutdownAndExit();
                    LeaveCriticalSection(&gMainMutex);
                    break;
                }
                if (processResult == 2)
                {
                    ReleaseCallback();
                }

                gRenderer->StartFrame();
                gApp.UpdateTime();

                PIXBeginNamedEvent(0.0f, "Input manager tick");
                g_pInput->Tick();
                PIXEndNamedEvent();

                PIXBeginNamedEvent(0.0f, "Render manager tick");
                gRenderer->Tick();
                PIXEndNamedEvent();

                PIXBeginNamedEvent(0.0f, "Network manager do work #1");
                CGameNetworkManager::sInstance.DoWork();
                PIXEndNamedEvent();

                gRenderer->ResetRenderState();
                const float clearColour[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
                gRenderer->SetClearColour(clearColour);
                gRenderer->Clear(3);
                gRenderer->StartDRCFrame(false);
                gRenderer->Clear(3);

                PIXBeginNamedEvent(0.0f, "Frame present");
                gRenderer->Present();
                PIXEndNamedEvent();

                gRenderer->Set_matrixDirty();
                Vec3::resetPool();
            }

            LeaveCriticalSection(&gMainMutex);
        }
        while (s_altMainExit == 0);
    }

    return 0;
}
