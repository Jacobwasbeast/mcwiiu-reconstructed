#include "game/ui/UIComponent_Tooltips.hpp"

#include <cstring>

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/FJ_ToolTips.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CInput.hpp"

extern CProfile* gProfile;

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
    memset(m_touchState, 0, sizeof(m_touchState));
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
    UIScene::mapElementsAndNames();
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
    return 1;
}

std::wstring UIComponent_Tooltips::getMoviePath()
{
    unsigned int viewport = m_tooltipLayer->getViewport();
    m_splitTooltips = viewport > 0 && viewport <= 8;
    return m_splitTooltips ? std::wstring(L"ToolTipsSplit") : std::wstring(L"ToolTips");
}

double UIComponent_Tooltips::getSafeZoneHalfWidth()
{
    if (gRenderer->IsWidescreen())
        return UIScene::getSafeZoneHalfWidth();
    return (double)m_stageWidth * 0.075;
}

void UIComponent_Tooltips::updateSafeZone()
{
    switch (m_tooltipLayer->getViewport())
    {
    default:
    case 0:
        setSafeZone(getSafeZoneHalfHeight(), getSafeZoneHalfHeight(),
                   getSafeZoneHalfWidth(), getSafeZoneHalfWidth());
        break;
    case 1:
    case 5:
        setSafeZone(getSafeZoneHalfHeight(), 0.0, getSafeZoneHalfWidth(), 0.0);
        break;
    case 2:
    case 7:
        setSafeZone(0.0, getSafeZoneHalfHeight(), getSafeZoneHalfWidth(), 0.0);
        break;
    case 3:
        setSafeZone(0.0, getSafeZoneHalfHeight(), getSafeZoneHalfWidth(), 0.0);
        break;
    case 4:
        setSafeZone(0.0, getSafeZoneHalfHeight(), 0.0, getSafeZoneHalfWidth());
        break;
    case 6:
        setSafeZone(getSafeZoneHalfHeight(), 0.0, 0.0, getSafeZoneHalfWidth());
        break;
    case 8:
        setSafeZone(0.0, getSafeZoneHalfHeight(), 0.0, getSafeZoneHalfWidth());
        break;
    }
}

void UIComponent_Tooltips::_SetTooltip(unsigned int index, int type, int subtype,
                                       const UIString& text, bool enabled, bool force)
{
    _TooltipValues& value = m_tooltipValues[index];
    if (force || enabled || value.m_enabled)
    {
        int finalType = type;
        int finalSubtype = subtype;
        if (subtype >= 0)
        {
            finalType = subtype;
            finalSubtype = type;
        }
        value.m_enabled = enabled;
        value.m_text = text;
        UIString localText(text);
        static_cast<FJ_ToolTips*>(m_movie->getRootNode()->getFuiNode())
            ->SetToolTip((int)index, finalType, finalSubtype, localText.getString(), enabled);
    }
}

void UIComponent_Tooltips::_Relayout()
{
    FJ_ToolTips* toolTips = static_cast<FJ_ToolTips*>(m_movie->getRootNode()->getFuiNode());
    toolTips->UpdateLayout();
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIComponent_Tooltips::tick()
{
    UIScene::tick();

    int pad = gProfile->GetPrimaryPad();
    if (pad >= 0)
    {
        unsigned int level = GameSettings::GetGameSettings(pad, static_cast<eGameSetting>(0x10));
        if (level < 0x50)
        {
            if (gConsoleUIController.GetMenuDisplayed(m_playerId) != 0 && level < 0xf)
                level = 0xf;
            unsigned int timer = gApp.GetOpacityTimer(m_playerId);
            if (timer == 0)
                setOpacity(0.01f * (float)level);
            else if (timer < 10)
                setOpacity(0.01f * -((10.0f - (float)timer) * ((80.0f - (float)level) / 10.0f) - 80.0f));
            else
                setOpacity(0.8f);
        }
        else
        {
            setOpacity(0.01f * (float)level);
        }
    }

    bool changed = false;
    int i;
    for (i = 0; i < 16; ++i)
    {
        if (gConsoleUIController.IsReloadingSkin())
            continue;
        _TooltipValues& value = m_tooltipValues[i];
        if (value.m_enabled && value.m_text.needsUpdating())
        {
            changed = true;
            _SetTooltip((unsigned int)i, value.m_type, value.m_subtype, value.m_text,
                       value.m_enabled, true);
            value.m_text.setUpdated();
        }
    }
    if (changed)
        _Relayout();
}

void UIComponent_Tooltips::render(int width, int height, C4JRender::eViewportType viewport)
{
    if (g_pApp->GetPrimaryUserIndex() != -1)
    {
        if (gConsoleUIController.GetMenuDisplayed(m_playerId) == 0)
        {
            if (GameSettings::GetGameSettings(m_playerId, static_cast<eGameSetting>(0x11)) == 0)
                return;
            if (GameSettings::GetGameSettings(m_playerId, static_cast<eGameSetting>(0x17)) == 0)
                return;
        }
    }

    if (!m_splitTooltips)
    {
        UIScene::render(width, height, viewport);
        return;
    }

    int screenWidth = 0;
    int screenHeight = 0;
    gConsoleUIController.getRenderDimensions(C4JRender::eViewportType_FullScreen,
                                             screenWidth, screenHeight);

    switch (viewport)
    {
    case C4JRender::eViewportType_BottomHalf:
        gConsoleUIController.setupRenderPosition(0, static_cast<int>(screenHeight * 0.5f));
        break;
    case C4JRender::eViewportType_RightHalf:
    case C4JRender::eViewportType_TopRightQuarter:
        gConsoleUIController.setupRenderPosition(static_cast<int>(screenWidth * 0.5f), 0);
        break;
    case C4JRender::eViewportType_BottomLeftQuarter:
        gConsoleUIController.setupRenderPosition(0, static_cast<int>(screenHeight * 0.5f));
        break;
    case C4JRender::eViewportType_BottomRightQuarter:
        gConsoleUIController.setupRenderPosition(static_cast<int>(screenWidth * 0.5f),
                                                 static_cast<int>(screenHeight * 0.5f));
        break;
    default:
        gConsoleUIController.setupRenderPosition(0, 0);
        break;
    }

    if (viewport == C4JRender::eViewportType_FullScreen)
    {
        gFui->render(m_movie, 0.0f, 0.0f, static_cast<float>(width),
                     static_cast<float>(height));
    }
    else if (viewport < C4JRender::eViewportType_LeftHalf)
    {
        gFui->render(m_movie, 0.0f, static_cast<float>(m_stageWidth / 2),
                     static_cast<float>(screenWidth), static_cast<float>(height));
    }
    else if (viewport < C4JRender::eViewportType_TopLeftQuarter)
    {
        gFui->render(m_movie, 0.0f, 0.0f, static_cast<float>(width),
                     static_cast<float>(screenHeight));
    }
    else if (static_cast<int>(viewport) > C4JRender::eViewportType_BottomRightQuarter)
    {
        gFui->render(m_movie, 0.0f, 0.0f, static_cast<float>(width),
                     static_cast<float>(height));
    }
    else
    {
        gFui->render(m_movie, 0.0f, static_cast<float>(m_stageWidth / 2),
                     static_cast<float>(width), static_cast<float>(height));
    }
}

void UIComponent_Tooltips::updateViewportTouchOffset()
{
    unsigned int viewport = m_tooltipLayer == 0 ? 0 : m_tooltipLayer->getViewport();
    int screenWidth = 0;
    int screenHeight = 0;
    gConsoleUIController.getRenderDimensions(C4JRender::eViewportType_FullScreen,
                                             screenWidth, screenHeight);

    if (viewport < 5)
    {
        if (viewport == 1)
        {
            gConsoleUIController.updateViewportTouchOffset(
                0, -static_cast<int>(screenHeight * 0.5f));
            return;
        }
        if (viewport != 4)
        {
            gConsoleUIController.updateViewportTouchOffset(0, 0);
            return;
        }
    }
    else
    {
        if (viewport < 6)
        {
            gConsoleUIController.updateViewportTouchOffset(
                0, -static_cast<int>(screenHeight * 0.5f));
            return;
        }
        if (viewport == 6)
        {
            gConsoleUIController.updateViewportTouchOffset(
                static_cast<int>(screenWidth * 0.5f),
                -static_cast<int>(screenHeight * 0.5f));
            return;
        }
        if (viewport != 8)
        {
            gConsoleUIController.updateViewportTouchOffset(0, 0);
            return;
        }
    }

    gConsoleUIController.updateViewportTouchOffset(static_cast<int>(screenWidth * 0.5f), 0);
}

unsigned int UIComponent_Tooltips::_SetTooltip(unsigned int index, int value)
{
    _TooltipValues& slot = m_tooltipValues[index];
    if (value == slot.m_textId)
    {
        if (value == -1)
            return 0;
        if (slot.m_enabled)
            return 0;
    }

    int type = (int)index;
    if (g_pInput->IsCircleCrossSwapped())
    {
        if (index == 0)
            type = 1;
        else if (index == 1)
            type = 0;
    }

    slot.m_textId = value;
    slot.m_type = type;
    slot.m_subtype = -1;
    slot.m_dynamic = false;
    if (value == -1)
        _SetTooltip(index, type, -1, UIString(L""), false, false);
    else
        _SetTooltip(index, type, -1, UIString(value), true, false);
    return 1;
}

void UIComponent_Tooltips::SetTooltipText(unsigned int player, int index)
{
    if (_SetTooltip(player, index) != 0)
        _Relayout();
}

void UIComponent_Tooltips::SetEnableTooltips(bool enabled)
{
}

void UIComponent_Tooltips::ShowTooltip(unsigned int player, bool visible)
{
    if (visible != m_tooltipValues[player].m_enabled)
    {
        _SetTooltip(player, (int)player, -1, UIString(L""), visible, false);
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

unsigned int UIComponent_Tooltips::_SetTooltip(unsigned int index, int type, int subtype,
                                               int textId, bool& changed)
{
    if (type == -1 || textId == -1)
    {
        changed = false;
        return 0;
    }
    changed = true;
    unsigned int result = 0;
    _TooltipValues& value = m_tooltipValues[index];
    if (value.m_textId != textId || !value.m_enabled)
    {
        value.m_type = type;
        value.m_subtype = subtype;
        value.m_dynamic = true;
        value.m_textId = textId;
        result = 1;
        _SetTooltip(index, type, subtype, UIString(textId), true, false);
    }
    return result;
}

unsigned int UIComponent_Tooltips::HandleToolTipButton(unsigned int player,
                                                        EControllerActions action,
                                                        unsigned int& button, int type)
{
    unsigned int primary = GetToolTipButton(
        static_cast<unsigned int>(GameSettings::GetInputButton((int)player, false, action)));
    unsigned int secondary = GetToolTipButton(
        static_cast<unsigned int>(GameSettings::GetInputButton((int)player, true, action)));

    unsigned int result = 0;
    unsigned int index = button;
    bool changed = false;
    if (index < 16)
    {
        result = _SetTooltip(index, (int)primary, (int)secondary, type, changed);
        _TooltipValues& value = m_tooltipValues[index];
        if (value.m_dynamic && value.m_enabled)
            value.m_button = (int)action;
    }
    if (changed)
        button = index + 1;
    return result;
}

void UIComponent_Tooltips::UpdateControllerType(int controllerType)
{
    const int previousType = m_controllerType;
    bool notify = false;

    if (g_pInput->IsPadConnected(4) == 0)
    {
        if (g_pInput->IsPadClassicController(controllerType) == 0)
        {
            m_controllerType = 0;
            notify = previousType != 0;
        }
        else
        {
            m_controllerType = 2;
            notify = previousType != 2;
        }
    }
    else if (g_pInput->IsPadDRCController(controllerType) != 0 || controllerType == 4)
    {
        m_controllerType = 1;
        notify = previousType != 1;
    }
    else if (g_pInput->IsPadClassicController(controllerType) == 0)
    {
        m_controllerType = 0;
        notify = previousType != 0;
    }
    else
    {
        m_controllerType = 2;
        notify = previousType != 2;
    }

    if (notify)
    {
        static_cast<FJ_ToolTips*>(m_movie->getRootNode()->getFuiNode())
            ->UpdateControllerType(m_controllerType);
    }
}

void UIComponent_Tooltips::SetTooltips(int player, int* tooltips, bool visible)
{
    static const int actions[25] = { 0x20, 0x26, 0x25, 0x28, 0x21, 0x22, 0x27, 0x2e, 0x2f, 0x23,
                                     0x24, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x33, 0x32, 0x30, 0x31,
                                     0x34, 0x35, 0x38, 0x36, 0x37 };
    static const int valueIndex[25] = { 0, 6, 5, 8, 1, 2, 7, 14, 15, 3,
                                        4, 9, 10, 11, 12, 13, 19, 18, 16, 17,
                                        20, 21, 24, 22, 23 };
    unsigned int slot = 0;
    bool changed = false;
    int i;
    for (i = 0; i < 25; ++i)
    {
        if (HandleToolTipButton((unsigned int)player, (EControllerActions)actions[i], slot,
                                tooltips[valueIndex[i]]) != 0)
            changed = true;
    }
    for (; slot < 16; ++slot)
    {
        if (m_tooltipValues[slot].m_enabled)
        {
            changed = true;
            _SetTooltip(slot, -1, -1, UIString(L""), false, false);
        }
    }
    if (changed)
        _Relayout();
    if (player >= 0)
        UpdateControllerType(player);
}

void UIComponent_Tooltips::SetTooltips(int player, int first, int second, int third, int fourth,
                                       int fifth, int sixth, int seventh, int eighth, int ninth,
                                       int tenth, int eleventh, int twelfth, int thirteenth,
                                       int fourteenth, int fifteenth, int sixteenth, bool visible)
{
    bool changed = false;
    if (_SetTooltip(0, first) != 0) changed = true;
    if (_SetTooltip(1, second) != 0) changed = true;
    if (_SetTooltip(2, third) != 0) changed = true;
    if (_SetTooltip(3, fourth) != 0) changed = true;
    if (_SetTooltip(4, fifth) != 0) changed = true;
    if (_SetTooltip(5, sixth) != 0) changed = true;
    if (_SetTooltip(6, seventh) != 0) changed = true;
    if (_SetTooltip(7, eighth) != 0) changed = true;
    if (_SetTooltip(8, ninth) != 0) changed = true;
    if (_SetTooltip(9, tenth) != 0) changed = true;
    if (_SetTooltip(9, tenth) != 0) changed = true;
    if (_SetTooltip(10, eleventh) != 0) changed = true;
    if (_SetTooltip(11, twelfth) != 0) changed = true;
    if (_SetTooltip(12, thirteenth) != 0) changed = true;
    if (_SetTooltip(13, fourteenth) != 0) changed = true;
    if (_SetTooltip(14, fifteenth) != 0) changed = true;
    if (_SetTooltip(15, sixteenth) != 0) changed = true;
    if (changed)
        _Relayout();
    if (player >= 0)
        UpdateControllerType(player);
}

void UIComponent_Tooltips::EnableTooltip(unsigned int player, bool enabled)
{
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
    m_activeTooltip = -1;
    int i;
    for (i = 0; i < 16; ++i)
    {
        _TooltipValues& value = m_tooltipValues[i];
        _SetTooltip((unsigned int)i, value.m_type, value.m_subtype, UIString(value.m_textId),
                   value.m_enabled, true);
    }
    _Relayout();
}

void UIComponent_Tooltips::handleInput(int player, int action, bool repeat, bool released,
                                       bool pressed, bool& handled)
{
    UIScene::handleInput(player, action, repeat, released, pressed, handled);
}

void UIComponent_Tooltips::handleTouchInput(unsigned int playerId, int touchX, int touchY,
                                            int touchBox, bool repeat, bool released, bool pressed)
{
    if (gProfile->GetPrimaryPad() == -1)
        return;

    if (gConsoleUIController.GetMenuDisplayed(gProfile->GetPrimaryPad()) == 0)
    {
        if (GameSettings::GetGameSettings(gProfile->GetPrimaryPad(),
                                          static_cast<eGameSetting>(0x11)) == 0)
            return;
    }

    int player = (int)playerId;
    if (player == 4)
        player = 0;
    unsigned int settingsValue = GameSettings::GetGameSettings(player, static_cast<eGameSetting>(7));

    if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        const GameType* gameType = Minecraft::GetInstance()->mLocalPlayers[player]->GetGameType();
        if (gameType != GameType::NOT_SET && released && touchBox == 3)
            g_pInput->MapTouchInput(player, 8);
    }

    if (!pressed)
        return;

    EControllerActions dynamicAction = (EControllerActions)0;
    if (gConsoleUIController.IsToolTipDynamic((unsigned int)player, (unsigned int)touchBox,
                                              dynamicAction))
    {
        unsigned int primary =
            (unsigned int)GameSettings::GetInputButton(player, false, dynamicAction);
        unsigned int secondary =
            (unsigned int)GameSettings::GetInputButton(player, true, dynamicAction);
        g_pInput->MapTouchInput(player, primary | secondary);
        return;
    }

    switch (touchBox)
    {
    case 0:
        if (settingsValue != 2 || gConsoleUIController.GetMenuDisplayed(player))
        {
            if (g_pInput->IsCircleCrossSwapped())
                g_pInput->MapTouchInput(player, 2);
            else
                g_pInput->MapTouchInput(player, 1);
            return;
        }
        break;
    case 1:
        if (g_pInput->IsCircleCrossSwapped())
            g_pInput->MapTouchInput(player, 1);
        else
            g_pInput->MapTouchInput(player, 2);
        return;
    case 2:
        g_pInput->MapTouchInput(player, 4);
        return;
    case 3:
        g_pInput->MapTouchInput(player, 8);
        return;
    case 4:
        if (gConsoleUIController.GetMenuDisplayed(player))
        {
            g_pInput->MapTouchInput(player, 0x800000);
            return;
        }
        break;
    case 5:
        if (gConsoleUIController.GetMenuDisplayed(player))
        {
            g_pInput->MapTouchInput(player, 0x400000);
            return;
        }
        break;
    case 6:
        if (settingsValue != 1 || gConsoleUIController.GetMenuDisplayed(player))
        {
            g_pInput->MapTouchInput(player, 0x80);
            return;
        }
        break;
    case 7:
        if ((settingsValue != 1 && settingsValue != 2) ||
           gConsoleUIController.GetMenuDisplayed(player))
        {
            g_pInput->MapTouchInput(player, 0x40);
            return;
        }
        break;
    case 8:
        break;
    case 9:
        g_pInput->MapTouchInput(player, 0x100000);
        return;
    case 10:
        g_pInput->MapTouchInput(player, 0x20);
        return;
    }
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
