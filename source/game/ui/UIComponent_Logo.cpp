#include "game/ui/UIComponent_Logo.hpp"
#include "game/ui/FJ_ComponentLogo.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"

UIComponent_Logo::UIComponent_Logo(int sceneType, void* callback, UILayer* layer) : UIScene(sceneType, layer)
{
    m_itemRenderer = callback;
    initialiseMovie();
    setLogoLanguage(0);
    m_visible = true;
}
UIComponent_Logo::~UIComponent_Logo() {}
void UIComponent_Logo::setLogoLanguage(int language)
{
    if (m_movie != 0 && m_movie->getRootNode() != 0)
    {
        FJ_ComponentLogo* logo = (FJ_ComponentLogo*)m_movie->getRootNode();
        logo->SetLogoLanguage(language);
    }
}
std::wstring UIComponent_Logo::getMoviePath()
{
    if (m_layer != 0 && m_layer->getViewport() > 0 && m_layer->getViewport() <= 8)
        return L"ComponentLogoSplit";
    return L"ComponentLogo";
}
void UIComponent_Logo::handleReload() { setLogoLanguage(0); }
int UIComponent_Logo::getSceneType() { return 67; }
bool UIComponent_Logo::stealsFocus() { return false; }
bool UIComponent_Logo::hasFocus(int controller) { return false; }
bool UIComponent_Logo::hidesLowerScenes() { return false; }
