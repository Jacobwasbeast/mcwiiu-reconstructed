#include "game/ui/UIComponent_DebugUIMarketingGuide.hpp"

UIComponent_DebugUIMarketingGuide::UIComponent_DebugUIMarketingGuide(int type, void*, UILayer* layer)
    : UIScene(type, layer)
{
    initialiseMovie();
}

UIComponent_DebugUIMarketingGuide::~UIComponent_DebugUIMarketingGuide()
{
}

std::wstring UIComponent_DebugUIMarketingGuide::getMoviePath()
{
    return std::wstring(L"DebugUIMarketingGuide");
}

int UIComponent_DebugUIMarketingGuide::getSceneType()
{
    return 69;
}

bool UIComponent_DebugUIMarketingGuide::stealsFocus()
{
    return false;
}

bool UIComponent_DebugUIMarketingGuide::hasFocus(int)
{
    return false;
}

bool UIComponent_DebugUIMarketingGuide::hidesLowerScenes()
{
    return false;
}
