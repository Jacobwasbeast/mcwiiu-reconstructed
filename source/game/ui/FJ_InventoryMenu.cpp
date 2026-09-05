#include "game/ui/FJ_InventoryMenu.hpp"

#include <cstdio>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_MobEffect.hpp"
#include "game/util/Math.hpp"

FJ_InventoryMenu::FJ_InventoryMenu(fuiRenderNode* renderNode)
    : FJ_AbstractContainerDocument(renderNode), m_inventoryHidden(0)
{
}

FJ_InventoryMenu::~FJ_InventoryMenu()
{
}

FJ_AbstractContainerDocument* FJ_InventoryMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_InventoryMenu(renderNode);
}

unsigned int FJ_InventoryMenu::getType()
{
    return 12;
}

void FJ_InventoryMenu::InitEffectsGroup()
{
    char name[64];
    int row;
    int column;
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            std::sprintf(name, "EffectsGroup_Effect_%d", row * 9 + column + 1);
            m_effectNodes[row * 9 + column] = getChildByName(std::string(name));
            FJ_MobEffect* effect = m_effectNodes[row * 9 + column]->asFJ_MobEffect();
            effect->SetupEffectType(row * 9 + column + 2);
            m_effectNodes[row * 9 + column]->setVisible(false);
            m_effectFlags[row * 9 + column] = 0;
        }
    }
    std::sprintf(name, "EffectsGroup_Effect_%d", 27);
    m_effectNodes[26] = getChildByName(std::string(name));
    FJ_MobEffect* effect = m_effectNodes[26]->asFJ_MobEffect();
    effect->SetupEffectType(31);
    m_effectNodes[26]->setVisible(false);
    m_effectFlags[26] = 0;
    m_inventoryHidden = 0;
    m_effectStartY = static_cast<int>(m_effectNodes[0]->getY());
    m_effectEndY = static_cast<int>(m_effectNodes[2]->getY());
    m_effectBottomY = static_cast<int>(m_effectNodes[6]->getY());
}

void FJ_InventoryMenu::init(FJ_Event* event)
{
    FJ_AbstractContainerDocument::init(event);
    InitEffectsGroup();
    FJ_FuiNode* mainPanel = getChildByName(std::string("MainPanel"));
    FJ_FuiNode* equipmentHolder = mainPanel->getChildByName(std::string("EquipmentHolder"));
    FJ_FuiNode* slotHead = equipmentHolder->getChildByName(std::string("Slot_Head"));
    FJ_FuiNode* slotBody = equipmentHolder->getChildByName(std::string("Slot_Body"));
    FJ_FuiNode* slotLegs = equipmentHolder->getChildByName(std::string("Slot_Legs"));
    FJ_FuiNode* slotFeet = equipmentHolder->getChildByName(std::string("Slot_Feet"));
    FJ_FuiNode* iconHead = mainPanel->getChildByName(std::string("Icon_Head"));
    FJ_FuiNode* iconBody = mainPanel->getChildByName(std::string("Icon_Body"));
    FJ_FuiNode* iconLegs = mainPanel->getChildByName(std::string("Icon_Legs"));
    FJ_FuiNode* iconFeet = mainPanel->getChildByName(std::string("Icon_Feet"));
    m_armorSlots[0] = iconHead->asMovieClip();
    m_armorSlots[1] = iconBody->asMovieClip();
    m_armorSlots[2] = iconLegs->asMovieClip();
    m_armorSlots[3] = iconFeet->asMovieClip();
    slotHead->asMovieClip();
    slotBody->asMovieClip();
    slotLegs->asMovieClip();
    slotFeet->asMovieClip();
}

void FJ_InventoryMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    const double stageHeight = getStage()->getStageHeight();
    if (stageHeight != 360.0 && stageHeight != 540.0)
    {
        return;
    }
    setX(static_cast<float>(-right - 176.0));
    setY(static_cast<float>(-top - 176.0));
}

void FJ_InventoryMenu::RepositionEffects()
{
    int activeCount = 0;
    int row;
    int column;
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            if (m_effectFlags[row * 9 + column] != 0)
            {
                ++activeCount;
            }
        }
    }
    int spacing = m_inventoryHidden ? 4 : 7;
    if (activeCount > spacing)
    {
        spacing = Math::round(static_cast<float>((m_effectStartY - m_effectBottomY) / static_cast<float>(activeCount - 1)));
    }
    int y = m_effectStartY;
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            const int index = row * 9 + column;
            if (m_effectFlags[index] != 0)
            {
                m_effectNodes[index]->setY(static_cast<float>(y));
                y -= spacing;
            }
        }
    }
}

void FJ_InventoryMenu::HideInventory()
{
    FJ_FuiNode* mainPanel = getChildByName(std::string("MainPanel"));
    FJ_FuiNode* backgroundPanel = mainPanel->getChildByName(std::string("BackgroundPanel"));
    FJ_FuiNode* background = backgroundPanel->getChildAt(0);
    const int oldHeight = static_cast<int>(background->getHeight());
    FJ_AbstractContainerDocument::HideInventory();
    background = backgroundPanel->getChildAt(0);
    const int newHeight = static_cast<int>(background->getHeight());
    m_inventoryHidden = 1;
    m_effectStartY += newHeight - oldHeight;
    m_effectEndY += newHeight - oldHeight;
    RepositionEffects();
}

void FJ_InventoryMenu::SetClassicCrafting(bool classicCrafting)
{
    if (!classicCrafting)
    {
        FJ_FuiNode* armorList = getChildByName(std::string("MainPanel.armorList"))->asMovieClip();
        FJ_FuiNode* equipmentHolder = getChildByName(std::string("EquipmentHolder"))->asMovieClip();
        FJ_FuiNode* mainPanel = getChildByName(std::string("MainPanel.iggy_player"))->asMovieClip();
        mainPanel->getChildByName(std::string("craftingLabel"))->setVisible(false);
        mainPanel->removeChild(mainPanel->getChildByName(std::string("craftingList")), true);
        mainPanel->removeChild(mainPanel->getChildByName(std::string("Result")), true);
        mainPanel->removeChild(mainPanel->getChildByName(std::string("CraftingArrow")), true);
        const int armorX = static_cast<int>(armorList->getX());
        const int width = Math::round((armorList->getWidth() + equipmentHolder->getX() - (armorList->getX() + armorList->getWidth()) + equipmentHolder->getWidth()) * 0.5f);
        const int backgroundWidth = Math::round(getChildByName(std::string("MainPanel.BackgroundPanel"))->getWidth() * 0.5f);
        const float offset = static_cast<float>(backgroundWidth - armorX - width - 176);
        armorList->incX(offset);
        getChildByName(std::string("BackgroundPanel"))->incX(offset);
        equipmentHolder->incX(offset);
        getChildByName(std::string("PlayerBackground"))->incX(offset);
        getChildByName(std::string("ShieldSlot"))->incX(offset);
        getChildByName(std::string("shieldList"))->incX(offset);
    }
}

void FJ_InventoryMenu::AddEffect(int effect, const std::wstring& name, int duration)
{
    static const int effectSlots[] = {-1, 12, 9, 2};
    if (effect != 0)
    {
        const int slot = effectSlots[effect];
        FJ_MobEffect* mobEffect = m_effectNodes[slot]->asFJ_MobEffect();
        mobEffect->Init(name, duration);
        m_effectNodes[slot]->setVisible(true);
        m_effectFlags[slot] = 1;
        m_effectState[slot] = 0;
        RepositionEffects();
    }
}

unsigned int FJ_InventoryMenu::UpdateEffects(int count, int* effects)
{
    static const int effectSlots[] = {-1, 12, 9, 2};
    unsigned int changed = 0;
    int i;
    if (count >= 2)
    {
        for (i = 0; i < count; i += 2)
        {
            const int slot = effectSlots[effects[i]];
            m_effectNodes[slot]->asFJ_MobEffect()->SetDuration(effects[i + 1]);
            m_effectState[slot] = 1;
            changed = 1;
        }
    }
    for (i = 0; i < 27; ++i)
    {
        if (m_effectFlags[i] != 0 && m_effectState[i] == 0)
        {
            FJ_MobEffect* effect = m_effectNodes[i]->asFJ_MobEffect();
            const unsigned int timeLeft = effect->getTimeLeft();
            if (timeLeft != 0)
            {
                effect->SetDuration(static_cast<int>(timeLeft) - 1);
                changed = 1;
            }
            else
            {
                m_effectNodes[i]->setVisible(false);
                m_effectFlags[i] = 0;
                RepositionEffects();
                changed = 1;
            }
        }
        m_effectState[i] = 0;
    }
    return changed;
}

void FJ_InventoryMenu::ShowArmorSlot(int slot, bool visible)
{
    m_armorSlots[slot]->setVisible(visible);
}
