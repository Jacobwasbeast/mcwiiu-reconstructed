#include "game/ui/UIControl_EnchantmentButton.hpp"

#include <sstream>

#include "game/client/render/Font.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/item/Enchantment.hpp"
#include "game/ui/EnchantmentMenu.hpp"
#include "game/ui/FJ_EnchantingButton.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UIScene_EnchantingMenu.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

std::wstring _toString(int value);

extern CMinecraftApp g_MinecraftApp;
static UIControl_EnchantmentButton::EnchantmentNames g_enchantmentNames;

UIControl_EnchantmentButton::EnchantmentNames::EnchantmentNames()
{
    std::wstring source(L"the elder scrolls klaatu berata ");
    std::wistringstream stream(source);
    std::wstring name;
    while (stream >> name)
    {
        m_names.push_back(name);
    }
}

UIControl_EnchantmentButton::EnchantmentNames::~EnchantmentNames()
{
}

std::wstring UIControl_EnchantmentButton::EnchantmentNames::getRandomName()
{
    const int count = nextInt(2) + 3;
    std::wstring result;
    int i;
    for (i = 0; i < count; ++i)
    {
        if (i > 0)
        {
            result.append(L" ");
        }
        result.append(m_names[nextInt((int)m_names.size())]);
    }
    return result;
}

UIControl_EnchantmentButton::UIControl_EnchantmentButton()
    : UIControl_Button(), m_state(0), m_level(0), m_index(0), m_name(L""),
      m_focus(false),
      m_colour((int)(g_MinecraftApp.GetHTMLColour(eMinecraftColour_Color_EnchantText) |
                     0xff000000)),
      m_focusColour((int)(g_MinecraftApp.GetHTMLColour(eMinecraftColour_Color_EnchantTextFocus) |
                          0xff000000)),
      m_inactiveColour((int)(g_MinecraftApp.GetHTMLColour(
                                 eMinecraftColour_Color_EnchantTextDisabled) |
                             0xff000000))
{
}

UIControl_EnchantmentButton::~UIControl_EnchantmentButton()
{
}

bool UIControl_EnchantmentButton::setupControl(UIScene* scene, fuiRenderNode* node,
                                                const std::string& name)
{
    m_controlType = 6;
    m_controlId = 1;
    return UIControl_Button::setupControl(scene, node, name);
}

void UIControl_EnchantmentButton::updateState()
{
    UIScene_EnchantingMenu* scene = static_cast<UIScene_EnchantingMenu*>(m_scene);
    EnchantmentMenu* menu = scene->getMenu();
    const int level = menu->mCosts[m_index];
    const int goldCount = menu->getGoldCount();
    Enchantment* enchantment = Enchantment::byId(menu->mEnchantmentIds[m_index]);
    Minecraft* minecraft = Minecraft::GetInstance();
    int state;
    if (enchantment == NULL || goldCount < m_index + 1)
    {
        state = minecraft->mLocalPlayers[scene->getPad()]->mAbilities.mInstabuild
                    ? (m_focus ? 2 : 1)
                    : 0;
    }
    else if (minecraft->mLocalPlayers[scene->getPad()]->mExperienceLevel < level)
    {
        state = minecraft->mLocalPlayers[scene->getPad()]->mAbilities.mInstabuild
                    ? (m_focus ? 2 : 1)
                    : 0;
    }
    else
    {
        state = m_focus ? 2 : 1;
    }
    if (level != m_level)
    {
        setLabel(UIString(_toString(level)), false, false);
        m_level = level;
        m_name = g_enchantmentNames.getRandomName();
    }
    if (enchantment == NULL || level == 0)
    {
        setLabel(UIString(L""), false, false);
        state = 0;
    }
    if (state != m_state)
    {
        FJ_EnchantingButton* button = static_cast<FJ_EnchantingButton*>(
            m_fuiRenderNode->getFuiNode()->asFJ_Base());
        button->ChangeState(state);
        m_state = state;
    }
}

void UIControl_EnchantmentButton::tick()
{
    updateState();
    UIControl_Button::tick();
}

void UIControl_EnchantmentButton::init(int index)
{
    m_index = index;
}

void UIControl_EnchantmentButton::ReInit()
{
    UIControl_Button::ReInit();
    m_focus = false;
    m_state = 0;
    m_level = 0;
    updateState();
}

void UIControl_EnchantmentButton::render(fuiRect* rect)
{
    UIScene_EnchantingMenu* scene = static_cast<UIScene_EnchantingMenu*>(m_scene);
    EnchantmentMenu* menu = scene->getMenu();
    float scale;
    if (scene->getPad() == 0)
    {
        scale = g_MinecraftApp.GetLocalPlayerCount() < 2 ? 3.5F : 3.0F;
    }
    else
    {
        scale = g_MinecraftApp.GetLocalPlayerCount() < 2 ? 2.5F : 2.0F;
    }

    GlStateManager::scalef((rect->xMax - rect->xMin) / m_width,
                           (rect->yMax - rect->yMin) / m_height, 1.0F);
    GlStateManager::scalef(scale, scale, scale);
    const int level = menu->mCosts[m_index];
    const int goldCount = menu->getGoldCount();
    Enchantment* enchantment = Enchantment::byId(menu->mEnchantmentIds[m_index]);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    if (enchantment != NULL && level != 0)
    {
        GlStateManager::enableAlphaTest();
        GlStateManager::alphaFunc(4, 0.1F);
        Minecraft* minecraft = Minecraft::GetInstance();
        std::wstring levelText = _toString(level);
        int colour = m_inactiveColour;
        if (goldCount >= m_index + 1 &&
            minecraft->mLocalPlayers[scene->getPad()]->mExperienceLevel >= level)
        {
            colour = m_focus ? m_focusColour : m_colour;
        }
        else if (minecraft->mLocalPlayers[scene->getPad()]->mAbilities.mInstabuild)
        {
            colour = m_focus ? m_focusColour : m_colour;
        }
        Minecraft::GetInstance()->mFont->drawWordWrap(
            m_name, 0, 0, (int)(m_width / scale),
            colour);
        GlStateManager::disableAlphaTest();
    }
    GlStateManager::disableRescaleNormal();
}

void UIControl_EnchantmentButton::SetEnchantmentLevel(int level)
{
    FJ_EnchantingButton* button = static_cast<FJ_EnchantingButton*>(
        m_fuiRenderNode->getFuiNode()->asFJ_Base());
    button->SetEnchantmentLevel(level);
}

bool UIControl_EnchantmentButton::IsInactive()
{
    return m_state == 0;
}

void UIControl_EnchantmentButton::setFocus(bool focus)
{
    m_focus = focus;
    updateState();
}
