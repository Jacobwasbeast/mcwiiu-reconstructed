#include "game/ui/UIComponent_Panorama.hpp"

#include <string>

#include "game/client/render/C4JRender.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/FJ_Panorama.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"

UIComponent_Panorama::UIComponent_Panorama(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer)
{
    initialiseMovie();
    m_panoramaDay = true;
    m_currentFrame = 0;
    m_movieLoaded = true;
    while (!m_hasTicked)
        tick();
}

UIComponent_Panorama::~UIComponent_Panorama()
{
}

void UIComponent_Panorama::setPanorama(bool day)
{
    if (day != m_panoramaDay) {
        m_panoramaDay = day;
        fuiRenderNode* root = m_movie->getRootNode();
        static_cast<FJ_Panorama*>(root->getFuiNode())->ShowPanoramaDay(day);
    }
}

void UIComponent_Panorama::tick()
{
    if (!hasMovie())
        return;

    Minecraft* minecraft = Minecraft::GetInstance();
    EnterCriticalSection(&minecraft->mLevelLock);
    if (minecraft->mLevel != NULL && minecraft->mLevel->getLevelData() != NULL) {
        long long dayTime = minecraft->mLevel->getLevelData()->getDayTime();
        if (dayTime % 24000 > 14000) {
            setPanorama(false);
            LeaveCriticalSection(&minecraft->mLevelLock);
            m_currentFrame = static_cast<FJ_Panorama*>(m_movie->getRootNode()->getFuiNode())->GetCurrentFrame();
            UIScene::tick();
            return;
        }
    }
    setPanorama(true);
    LeaveCriticalSection(&minecraft->mLevelLock);
    m_currentFrame = static_cast<FJ_Panorama*>(m_movie->getRootNode()->getFuiNode())->GetCurrentFrame();
    UIScene::tick();
}

std::wstring UIComponent_Panorama::getMoviePath()
{
    C4JRender::eViewportType viewport = m_layer->getViewport();
    if (viewport == C4JRender::eViewportType_FullScreen ||
        viewport > C4JRender::eViewportType_BottomRightQuarter) {
        m_splitScreen = false;
        return L"Panorama";
    }
    m_splitScreen = true;
    return L"PanoramaSplit";
}

void UIComponent_Panorama::handleReload()
{
    static_cast<FJ_Panorama*>(m_movie->getRootNode()->getFuiNode())->SetCurrentFrame(m_currentFrame);
}

void UIComponent_Panorama::render(int width, int height, C4JRender::eViewportType viewport)
{
    if (!m_splitScreen || viewport < C4JRender::eViewportType_TopHalf ||
        viewport > C4JRender::eViewportType_RightHalf) {
        UIScene::render(width, height, viewport);
        return;
    }

    int screenWidth = 0;
    int screenHeight = 0;
    gConsoleUIController.getRenderDimensions(C4JRender::eViewportType_FullScreen,
                                             screenWidth, screenHeight);
    if (viewport == C4JRender::eViewportType_BottomHalf)
        gConsoleUIController.setupRenderPosition(0, screenHeight / 2);
    else if (viewport == C4JRender::eViewportType_RightHalf)
        gConsoleUIController.setupRenderPosition(screenWidth / 2, 0);
    else
        gConsoleUIController.setupRenderPosition(0, 0);

    if (viewport == C4JRender::eViewportType_RightHalf ||
        viewport == C4JRender::eViewportType_LeftHalf) {
        gFui->render(m_movie, 854.0f, 854.0f, static_cast<float>(width - 176),
                     static_cast<float>(screenHeight - 176));
    } else {
        gFui->render(m_movie, 854.0f, 854.0f, static_cast<float>(screenWidth),
                     static_cast<float>(screenHeight), 0.5f, 0.5f);
    }
}

int UIComponent_Panorama::getSceneType()
{
    return EUIScene_Panorama;
}

bool UIComponent_Panorama::stealsFocus()
{
    return false;
}

bool UIComponent_Panorama::hasFocus(int)
{
    return false;
}
