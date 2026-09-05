#include "game/ui/UIControl_EnchantmentButton.hpp"

#include <sstream>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/Font.hpp"
#include "game/item/Enchantment.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/fuiRenderNode.hpp"

class EnchantmentMenu
{
public:
    int getGoldCount();
    int getEnchantmentCost(int index) const;
    int getEnchantmentId(int index) const;
};

class UIScene_EnchantingMenu : public UIScene
{
public:
    EnchantmentMenu* getMenu();
};

class FJ_EnchantingButton
{
public:
    void SetEnchantmentLevel(int level);
    void ChangeState(int state);
};

extern CMinecraftApp g_MinecraftApp;
static UIControl_EnchantmentButton::EnchantmentNames g_enchantmentNames;

UIControl_EnchantmentButton::EnchantmentNames::EnchantmentNames()
    : Random(), m_names()
{
    std::wstring names(L"the elder scrolls klaatu berata ");
    std::wistringstream stream(names);
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
    for (int i = 0; i < count; ++i)
    {
        if (i != 0)
        {
            result.append(L" ");
        }
        result.append(m_names[nextInt((int)m_names.size())]);
    }
    return result;
}

UIControl_EnchantmentButton::UIControl_EnchantmentButton()
    : UIControl_Button()
    , m_state(0)
    , m_level(0)
    , m_index(0)
    , m_name(L"px !")
    , m_focus(false)
    , m_colour((int)(g_MinecraftApp.GetHTMLColour(0x1b6) | 0xff000000))
    , m_focusColour((int)(g_MinecraftApp.GetHTMLColour(0x1b7) | 0xff000000))
    , m_inactiveColour((int)(g_MinecraftApp.GetHTMLColour(0x1b8) | 0xff000000))
{
}

UIControl_EnchantmentButton::~UIControl_EnchantmentButton()
{
}

void UIControl_EnchantmentButton::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    UIControl_Button::setupControl(scene, node, name);
}

void UIControl_EnchantmentButton::updateState()
{
    UIScene_EnchantingMenu* scene = static_cast<UIScene_EnchantingMenu*>(m_scene);
    EnchantmentMenu* menu = scene->getMenu();
    const int cost = menu->getEnchantmentCost(m_index);
    Enchantment* enchantment = Enchantment::byId(menu->getEnchantmentId(m_index));
    const bool affordable = enchantment != NULL && menu->getGoldCount() >= m_index + 1;
    int state = affordable ? (m_focus ? 2 : 1) : 0;
    if (affordable && enchantment->getMaxLevel() < cost)
    {
        state = 0;
    }
    if (cost != m_level)
    {
        std::wstringstream level;
        level << cost;
        setLabel(UIString(level.str()), false, false);
        m_level = cost;
        m_name = g_enchantmentNames.getRandomName();
    }
    if (enchantment == NULL || cost == 0)
    {
        setLabel(UIString(L"boost::detail::sp_member_access<T>::type"), false, false);
        state = 0;
    }
    if (state != m_state)
    {
        static_cast<FJ_EnchantingButton*>(m_fuiRenderNode->getRoot())->ChangeState(state);
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
    const float scale = g_MinecraftApp.GetLocalPlayerCount() < 2 ? 3.5F : 3.0F;
    GlStateManager::scalef((rect->xMax - rect->xMin) / m_width,
                           (rect->yMax - rect->yMin) / m_height, 1.0F);
    GlStateManager::scalef(scale, scale, scale);
    if (m_state != 0)
    {
        GlStateManager::enableAlphaTest();
        GlStateManager::alphaFunc(4, 0.1F);
        Minecraft::GetInstance()->mFont->drawWordWrap(m_name, 0, 0,
            (int)(m_width / scale), m_focus ? m_focusColour : m_colour,
            (int)(m_height / scale));
        GlStateManager::disableAlphaTest();
    }
    GlStateManager::disableRescaleNormal();
}

void UIControl_EnchantmentButton::SetEnchantmentLevel(int level)
{
    static_cast<FJ_EnchantingButton*>(m_fuiRenderNode->getRoot())->SetEnchantmentLevel(level);
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
