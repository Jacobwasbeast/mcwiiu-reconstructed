#include "game/ui/UIComponent_Logo.hpp"
#include "game/ui/FJ_ComponentLogo.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIComponent_Logo::UIComponent_Logo(int sceneType, void* callback, UILayer* layer) : UIScene(sceneType, layer)
{
    initialiseMovie();
    setLogoLanguage(0);
    m_movieLoaded = true;
}
void UIComponent_Logo::setLogoLanguage(int language)
{
    FJ_ComponentLogo* logo = static_cast<FJ_ComponentLogo*>(m_movie->getRootNode()->getFuiNode());
    logo->SetLogoLanguage(language);
}
std::wstring UIComponent_Logo::getMoviePath()
{
    unsigned int viewport = m_layer->getViewport();
    if (viewport == 0 || viewport > 8)
        return L"ComponentLogo";
    return L"ComponentLogoSplit";
}
void UIComponent_Logo::handleReload() { setLogoLanguage(0); }
int UIComponent_Logo::getSceneType() { return EUIScene_Logo; }
bool UIComponent_Logo::stealsFocus() { return false; }
bool UIComponent_Logo::hasFocus(int controller) { return false; }
bool UIComponent_Logo::hidesLowerScenes() { return false; }
