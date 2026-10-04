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
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
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
    if (minecraft->mLevel != NULL && minecraft->mLevel->mDimension->getType() == DimensionType::OVERWORLD) {
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
        static_cast<unsigned int>(viewport) > C4JRender::eViewportType_BottomRightQuarter) {
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
    bool isHalf = viewport >= C4JRender::eViewportType_TopHalf && viewport <= C4JRender::eViewportType_RightHalf;
    if (!m_splitScreen || !isHalf) {
        UIScene::render(width, height, viewport);
        return;
    }

    if (viewport == C4JRender::eViewportType_BottomHalf)
        gConsoleUIController.setupRenderPosition(0, static_cast<int>(gConsoleUIController.getScreenHeight() * 0.5f));
    else if (viewport == C4JRender::eViewportType_RightHalf)
        gConsoleUIController.setupRenderPosition(static_cast<int>(gConsoleUIController.getScreenWidth() * 0.5f), 0);
    else
        gConsoleUIController.setupRenderPosition(0, 0);

    if (viewport == C4JRender::eViewportType_RightHalf ||
        viewport == C4JRender::eViewportType_LeftHalf) {
        gFui->render(m_movie, 0.0f, 0.0f, static_cast<float>(width),
                     static_cast<float>(static_cast<int>(gConsoleUIController.getScreenHeight())));
    } else {
        gFui->render(m_movie, 0.0f, 0.0f, gConsoleUIController.getScreenWidth(),
                     gConsoleUIController.getScreenHeight(), 0.5f, 0.5f);
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
