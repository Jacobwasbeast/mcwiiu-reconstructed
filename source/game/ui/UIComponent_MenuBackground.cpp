#include "game/ui/UIComponent_MenuBackground.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fui.hpp"

UIComponent_MenuBackground::UIComponent_MenuBackground(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer), m_splitScreen(false)
{
    initialiseMovie();
    m_movieLoaded = true;
}

std::wstring UIComponent_MenuBackground::getMoviePath()
{
    unsigned int viewport = m_layer->getViewport();
    if (viewport == C4JRender::eViewportType_FullScreen ||
        viewport > C4JRender::eViewportType_BottomRightQuarter)
    {
        m_splitScreen = false;
    }
    else
    {
        m_splitScreen = true;
    }
    return L"MenuBackground";
}

void UIComponent_MenuBackground::render(int width, int height,
                                        C4JRender::eViewportType viewport)
{
    if (!m_splitScreen)
    {
        UIScene::render(width, height, viewport);
        return;
    }

    switch (viewport)
    {
    case C4JRender::eViewportType_BottomHalf:
    case C4JRender::eViewportType_BottomLeftQuarter:
        gConsoleUIController.setupRenderPosition(0, static_cast<int>(gConsoleUIController.getScreenHeight() * 0.5f));
        break;
    case C4JRender::eViewportType_RightHalf:
    case C4JRender::eViewportType_TopRightQuarter:
        gConsoleUIController.setupRenderPosition(static_cast<int>(gConsoleUIController.getScreenWidth() * 0.5f), 0);
        break;
    case C4JRender::eViewportType_BottomRightQuarter:
        gConsoleUIController.setupRenderPosition(static_cast<int>(gConsoleUIController.getScreenWidth() * 0.5f),
                                                 static_cast<int>(gConsoleUIController.getScreenHeight() * 0.5f));
        break;
    default:
        gConsoleUIController.setupRenderPosition(0, 0);
        break;
    }

    switch (viewport)
    {
    case C4JRender::eViewportType_TopHalf:
    case C4JRender::eViewportType_BottomHalf:
        gFui->render(m_movie, 0.0f, m_stageHeight / 2, static_cast<int>(gConsoleUIController.getScreenWidth()), height);
        break;
    case C4JRender::eViewportType_LeftHalf:
    case C4JRender::eViewportType_RightHalf:
        gFui->render(m_movie, 0.0f, 0, width, static_cast<int>(gConsoleUIController.getScreenHeight()));
        break;
    case C4JRender::eViewportType_TopLeftQuarter:
    case C4JRender::eViewportType_TopRightQuarter:
    case C4JRender::eViewportType_BottomLeftQuarter:
    case C4JRender::eViewportType_BottomRightQuarter:
        gFui->render(m_movie, 0.0f, m_stageHeight / 2, width, height);
        break;
    default:
        gFui->render(m_movie, 0.0f, 0, width, height);
        break;
    }
}

int UIComponent_MenuBackground::getSceneType()
{
    return EUIScene_MenuBackground;
}

bool UIComponent_MenuBackground::stealsFocus()
{
    return false;
}

bool UIComponent_MenuBackground::hasFocus(int)
{
    return false;
}

bool UIComponent_MenuBackground::hidesLowerScenes()
{
    return false;
}
