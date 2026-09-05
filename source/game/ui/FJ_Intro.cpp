#include "game/ui/FJ_Intro.hpp"
#include "game/ui/UIScene.hpp"

#include <string>
#include <vector>

namespace
{
    std::vector<std::string> g_introLabels[11];
    bool g_introLabelsPending = true;
}

FJ_Intro::FJ_Intro(fuiRenderNode* renderNode)
    : FJ_Document(renderNode), m_platform(0), m_animationStep(0), m_alternate(false)
{
    if (g_introLabelsPending)
    {
        g_introLabels[0].push_back("ESRB");
        g_introLabels[0].push_back("Mojang");
        g_introLabels[0].push_back("4J");

        g_introLabels[1].push_back("ESRB");
        g_introLabels[1].push_back("XboxLive");
        g_introLabels[1].push_back("Microsoft");
        g_introLabels[1].push_back("Mojang");
        g_introLabels[1].push_back("4J");

        g_introLabels[2].push_back("ESRB");
        g_introLabels[2].push_back("Microsoft");
        g_introLabels[2].push_back("Mojang");
        g_introLabels[2].push_back("4J");

        g_introLabels[3].push_back("ESRB");
        g_introLabels[3].push_back("Mojang");
        g_introLabels[3].push_back("4J");

        g_introLabels[4].push_back("ESRB");
        g_introLabels[4].push_back("Mojang");
        g_introLabels[4].push_back("4J");

        g_introLabels[5].push_back("ESRB");
        g_introLabels[5].push_back("Mojang");
        g_introLabels[5].push_back("4J");

        g_introLabels[6].push_back("ESRB");
        g_introLabels[6].push_back("Mojang");
        g_introLabels[6].push_back("4J");

        g_introLabels[7].push_back("ESRB");
        g_introLabels[7].push_back("Mojang");
        g_introLabels[7].push_back("4J");

        g_introLabels[8].push_back("ESRB");
        g_introLabels[8].push_back("Chinese_Health_Warning");
        g_introLabels[8].push_back("Mojang");
        g_introLabels[8].push_back("4J");
        g_introLabels[8].push_back("Chinese_Logo_XboxOne");

        g_introLabels[9].push_back("ESRB");
        g_introLabels[9].push_back("Chinese_Health_Warning");
        g_introLabels[9].push_back("Mojang");
        g_introLabels[9].push_back("4J");
        g_introLabels[9].push_back("Chinese_Logo_PS4");

        g_introLabels[10].push_back("ESRB");
        g_introLabels[10].push_back("Chinese_Health_Warning");
        g_introLabels[10].push_back("Mojang");
        g_introLabels[10].push_back("4J");
        g_introLabels[10].push_back("Chinese_Logo_PSVita");

        g_introLabelsPending = false;
    }
}

FJ_Intro::~FJ_Intro()
{
}

FJ_Document* FJ_Intro::Create(fuiRenderNode* renderNode)
{
    return new FJ_Intro(renderNode);
}

unsigned int FJ_Intro::getType()
{
    return 7;
}

void FJ_Intro::handleAnimationStep(FJ_Event*)
{
    ++m_animationStep;
    if (!g_introLabels[m_platform].empty() &&
        m_animationStep < static_cast<int>(g_introLabels[m_platform].size()))
    {
        gotoAndPlay(g_introLabels[m_platform][m_animationStep]);
    }
    else
    {
        m_pCallbackScene->handleAnimationEnd();
    }
}

void FJ_Intro::_handleAnimationStep(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Intro*>(node)->handleAnimationStep(event);
}

void FJ_Intro::init(FJ_Event* event)
{
    FJ_Document::init(event);
    addEventListener(eFJEventType_AnimationStep, this, &_handleAnimationStep, false);
}

void FJ_Intro::SetIntroPlatform(int platform, bool alternate, bool mappedPlatform)
{
    m_alternate = alternate;
    if (mappedPlatform)
    {
        if (platform == 2)
        {
            platform = 8;
        }
        else if (platform == 4)
        {
            platform = 9;
        }
        else if (platform == 5)
        {
            platform = 10;
        }
    }
    m_platform = platform;
    gotoAndPlay(g_introLabels[m_platform][m_alternate]);
}
