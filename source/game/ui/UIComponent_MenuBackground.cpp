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

UIComponent_MenuBackground::~UIComponent_MenuBackground()
{
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

    int screenWidth;
    int screenHeight;
    gConsoleUIController.getRenderDimensions(C4JRender::eViewportType_FullScreen,
                                             screenWidth, screenHeight);

    if (viewport == C4JRender::eViewportType_BottomHalf ||
        viewport == C4JRender::eViewportType_BottomLeftQuarter)
        gConsoleUIController.setupRenderPosition(0, screenHeight / 2);
    else if (viewport == C4JRender::eViewportType_RightHalf)
        gConsoleUIController.setupRenderPosition(screenWidth / 2, 0);
    else if (viewport == C4JRender::eViewportType_BottomRightQuarter)
        gConsoleUIController.setupRenderPosition(screenWidth / 2, screenHeight / 2);
    else if (viewport == C4JRender::eViewportType_TopRightQuarter)
        gConsoleUIController.setupRenderPosition(0, 0);
    else if (viewport > C4JRender::eViewportType_BottomRightQuarter)
        gConsoleUIController.setupRenderPosition(0, 0);
    else
        gConsoleUIController.setupRenderPosition(viewport);

    if (viewport < C4JRender::eViewportType_LeftHalf)
    {
        gFui->render(m_movie, 0.0f, static_cast<float>(m_stageWidth / 2) - 176.0f,
                     static_cast<float>(screenWidth) - 176.0f,
                     static_cast<float>(height) - 176.0f);
        return;
    }
    if (viewport < C4JRender::eViewportType_TopLeftQuarter)
    {
        gFui->render(m_movie, 0.0f, 0.0f, static_cast<float>(width) - 176.0f,
                     static_cast<float>(screenHeight) - 176.0f);
        return;
    }
    if (viewport > C4JRender::eViewportType_BottomRightQuarter)
    {
        gFui->render(m_movie, 0.0f, 0.0f, static_cast<float>(width),
                     static_cast<float>(height));
        return;
    }

    gFui->render(m_movie, 0.0f, static_cast<float>(m_stageWidth / 2),
                 static_cast<float>(width), static_cast<float>(height));
}

int UIComponent_MenuBackground::getSceneType()
{
    return 72;
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
