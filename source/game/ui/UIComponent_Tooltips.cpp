#include "game/ui/UIComponent_Tooltips.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/FJ_ToolTips.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"

UIComponent_Tooltips::_TooltipValues::_TooltipValues()
    : m_enabled(false), m_textId(-1), m_text(), m_type(-1), m_subtype(-1),
      m_dynamic(false), m_button(0x20)
{
}

UIComponent_Tooltips::_TooltipValues::~_TooltipValues()
{
}

UIComponent_Tooltips::UIComponent_Tooltips(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer), m_splitTooltips(false), m_tooltipsEnabled(true),
      m_controllerType(0), m_tooltipLayer(layer), m_safeZoneWidth(0),
      m_safeZoneHeight(0), m_touchVisible(false), m_activeTooltip(-1)
{
    int i;
    for (i = 0; i < 16; ++i)
        m_touchControls[i].init(i);
    m_touchState[0] = 0;
    initialiseMovie();
}

UIComponent_Tooltips::~UIComponent_Tooltips()
{
}

void UIComponent_Tooltips::overrideSFX(int first, int second, bool enabled)
{
    if (first >= 0 && first < 5 && second >= 0 && second < 23)
        m_touchState[first * 23 + second] = enabled ? 1 : 0;
}

unsigned int UIComponent_Tooltips::mapElementsAndNames()
{
    unsigned int result = UIScene::mapElementsAndNames();
    if (m_movie != 0)
    {
        fuiRenderNode* root = m_movie->getRootNode();
        int i;
        for (i = 0; i < 16; ++i)
        {
            std::string name("Touch_");
            char digit[4];
            digit[0] = (char)('0' + i / 10);
            digit[1] = (char)('0' + i % 10);
            digit[2] = 0;
            if (i < 10)
            {
                digit[0] = (char)('0' + i);
                digit[1] = 0;
            }
            name += digit;
            m_touchControls[i].setupControl(this, root, name);
            m_controls.push_back(&m_touchControls[i]);
        }
    }
    return result;
}

std::wstring UIComponent_Tooltips::getMoviePath()
{
    unsigned int viewport = m_tooltipLayer == 0 ? 0 : m_tooltipLayer->getViewport();
    m_splitTooltips = viewport > 0 && viewport <= 8;
    return m_splitTooltips ? std::wstring(L"ToolTipsSplit") : std::wstring(L"ToolTips");
}

double UIComponent_Tooltips::getSafeZoneHalfWidth()
{
    return (double)m_safeZoneWidth * 0.075;
}

void UIComponent_Tooltips::updateSafeZone()
{
    unsigned int viewport = m_tooltipLayer == 0 ? 0 : m_tooltipLayer->getViewport();
    m_safeZoneWidth = viewport == 0 ? 0 : 854;
    m_safeZoneHeight = viewport == 0 ? 0 : 480;
    setSafeZone(0.0, 0.0, getSafeZoneHalfWidth(), 1000.0);
}

void UIComponent_Tooltips::_SetTooltip(int index, int type, int subtype,
                                       unsigned int button, const UIString& text,
                                       bool enabled)
{
    if (index < 0 || index >= 16)
        return;
    _TooltipValues& value = m_tooltipValues[index];
    value.m_type = type;
    value.m_subtype = subtype;
    value.m_button = (int)button;
    value.m_text = text;
    value.m_enabled = enabled;
}

void UIComponent_Tooltips::_Relayout()
{
    if (m_movie != 0)
        m_movie->getRootNode();
}

void UIComponent_Tooltips::tick()
{
    UIScene::tick();
    int i;
    bool changed = false;
    for (i = 0; i < 16; ++i)
    {
        if (m_tooltipValues[i].m_enabled && m_tooltipValues[i].m_text.needsUpdating())
        {
            m_tooltipValues[i].m_text.setUpdated();
            changed = true;
        }
    }
    if (changed)
        _Relayout();
}

void UIComponent_Tooltips::render(int width, int height, C4JRender::eViewportType viewport)
{
    if (m_tooltipsEnabled)
        UIScene::render(width, height, viewport);
}

void UIComponent_Tooltips::updateViewportTouchOffset()
{
    if (m_tooltipLayer != 0)
    {
        m_tooltipLayer->getViewport();
    }
}

unsigned int UIComponent_Tooltips::_SetTooltip(unsigned int player, int index)
{
    if (index < 0 || index >= 16)
        return 0;
    _TooltipValues& value = m_tooltipValues[index];
    if (value.m_textId == index && !value.m_enabled)
        return 0;
    value.m_textId = index;
    _SetTooltip(index, index, -1, value.m_button, UIString(index), true);
    return 1;
}

void UIComponent_Tooltips::SetTooltipText(unsigned int player, int index)
{
    if (_SetTooltip(player, index) != 0)
        _Relayout();
}

void UIComponent_Tooltips::SetEnableTooltips(bool enabled)
{
    m_tooltipsEnabled = enabled;
}

void UIComponent_Tooltips::ShowTooltip(unsigned int player, bool visible)
{
    int index = (int)player;
    if (index >= 0 && index < 16)
    {
        m_tooltipValues[index].m_enabled = visible;
        _Relayout();
    }
}

unsigned int UIComponent_Tooltips::GetToolTipButton(unsigned int button)
{
    switch (button)
    {
    case 1: return 0; case 2: return 1; case 4: return 2; case 8: return 3;
    case 0x80: return 6; case 0x40: return 7; case 0x100: return 9; case 0x200: return 8;
    case 0x400: return 13; case 0x800: return 14; case 0x1000: return 11;
    case 0x2000: return 12; case 0x400000: return 5; case 0x800000: return 4;
    default: return 0xffffffff;
    }
}

unsigned int UIComponent_Tooltips::_SetTooltip(unsigned int player, int type, int subtype,
                                               int textId, bool& changed)
{
    changed = false;
    if (type == -1 || textId == -1)
        return 0;
    if (player >= 16)
        return 0;
    _TooltipValues& value = m_tooltipValues[player];
    changed = true;
    if (value.m_textId != textId || !value.m_enabled)
    {
        value.m_type = type;
        value.m_subtype = subtype;
        value.m_textId = textId;
        value.m_enabled = true;
        _SetTooltip((int)player, type, subtype, value.m_button, UIString(textId), true);
        return 1;
    }
    return 0;
}

unsigned int UIComponent_Tooltips::HandleToolTipButton(unsigned int player,
                                                        EControllerActions action,
                                                        unsigned int& button, int type)
{
    button = GetToolTipButton(player);
    bool changed;
    return _SetTooltip(player, type, (int)action, (int)button, changed);
}

void UIComponent_Tooltips::UpdateControllerType(int controllerType)
{
    m_controllerType = controllerType;
}

void UIComponent_Tooltips::SetTooltips(int player, int* tooltips, bool visible)
{
    int i;
    if (tooltips == 0)
        return;
    for (i = 0; i < 16; ++i)
        _SetTooltip((unsigned int)i, player, 0, tooltips[i], visible);
    _Relayout();
}

void UIComponent_Tooltips::SetTooltips(int player, int first, int second, int third, int fourth,
                                       int fifth, int sixth, int seventh, int eighth, int ninth,
                                       int tenth, int eleventh, int twelfth, int thirteenth,
                                       int fourteenth, int fifteenth, int sixteenth, bool visible)
{
    int values[16] = { first, second, third, fourth, fifth, sixth, seventh, eighth,
                       ninth, tenth, eleventh, twelfth, thirteenth, fourteenth,
                       fifteenth, sixteenth };
    SetTooltips(player, values, visible);
}

void UIComponent_Tooltips::EnableTooltip(unsigned int player, bool enabled)
{
    if (player < 16)
        m_tooltipValues[player].m_enabled = enabled;
}

bool UIComponent_Tooltips::IsToolTipDynamic(unsigned int player, EControllerActions& action)
{
    if (player >= 16)
        return false;
    action = (EControllerActions)m_tooltipValues[player].m_subtype;
    return m_tooltipValues[player].m_dynamic;
}

void UIComponent_Tooltips::handleReload()
{
    UIScene::handleReload();
    _Relayout();
}

void UIComponent_Tooltips::handleInput(int player, int action, bool repeat, bool released,
                                       bool pressed, bool& handled)
{
    UIScene::handleInput(player, action, repeat, released, pressed, handled);
}

void UIComponent_Tooltips::handleTouchInput(unsigned int player, int action, int x, int y,
                                            bool repeat, bool released, bool pressed)
{
    if (player < 16 && pressed)
        m_activeTooltip = (int)player;
    UIScene::handleTouchInput(player, action, x, y, repeat, released, pressed);
}

int UIComponent_Tooltips::getSceneType()
{
    return 0x46;
}

bool UIComponent_Tooltips::stealsFocus()
{
    return false;
}

bool UIComponent_Tooltips::hasFocus(int sceneType)
{
    return false;
}

bool UIComponent_Tooltips::hidesLowerScenes()
{
    return false;
}
