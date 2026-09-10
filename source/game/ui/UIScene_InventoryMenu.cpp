#include "game/ui/UIScene_InventoryMenu.hpp"

#include <cstdio>
#include <vector>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/L10N.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/world/generation/Player.hpp"
#include "game/ui/FJ_InventoryMenu.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern CMinecraftApp* g_pApp;

UIScene_InventoryMenu::UIScene_InventoryMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_sceneInterface(0), m_effectDurations(0),
      m_armorList(), m_craftingList(), m_resultList(), m_shieldList(), m_playerControl(), m_craftingLabel()
{
    m_sceneInterface = static_cast<IUIScene_AbstractContainerMenu*>(static_cast<UIScene_AbstractContainerMenu*>(this));
    initialiseMovie();
    m_effectDurations = new int[MobEffect::GetEffectCount() + 1];
    const bool classicCrafting = GameSettings::GetGameSettings(m_playerId, static_cast<eGameSetting>(0x20)) != 0 &&
        g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1f)) == 0;
    m_sceneInterface->m_inventorySmall = classicCrafting;
    SetClassicCrafting(classicCrafting);
    handleReload();
    updateEffectsDisplay();
    addTimer(10, 1000);
    (void)data;
}

UIScene_InventoryMenu::~UIScene_InventoryMenu()
{
    delete[] m_effectDurations;
}

unsigned int UIScene_InventoryMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    const unsigned int value = static_cast<unsigned int>(section);
    if (value < 10) return 0;
    if (value == 10) return 9;
    if (value == 11) return m_sceneInterface->GetInventoryRows();
    if (value == 12 || value == 13) return 1;
    if (value == 14) return m_sceneInterface->m_inventorySmall ? 2 : 1;
    return 1;
}

unsigned int UIScene_InventoryMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    const unsigned int value = static_cast<unsigned int>(section);
    if (value < 10) return 0;
    if (value == 10) return 1;
    if (value < 12) return m_sceneInterface->GetInventoryRows();
    if (value == 12) return 4;
    if (value == 14 && m_sceneInterface->m_inventorySmall) return 2;
    return 1;
}

unsigned int UIScene_InventoryMenu::GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section, IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    const unsigned int value = static_cast<unsigned int>(section);
    const IUIScene_AbstractContainerMenu::ESceneSection inventory = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(10);
    const IUIScene_AbstractContainerMenu::ESceneSection crafting = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(12);
    const IUIScene_AbstractContainerMenu::ESceneSection armor = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(13);
    const IUIScene_AbstractContainerMenu::ESceneSection result = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(14);
    if (value == 10 && state == IUIScene_AbstractContainerMenu::ETapState_Down) { m_sceneInterface->updateSlotPosition(section, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(11), state, x, y, 0, 0); return 11; }
    if (value == 10 && state == IUIScene_AbstractContainerMenu::ETapState_Up) { m_sceneInterface->updateSlotPosition(section, crafting, state, x, y, 0, 0); return crafting; }
    if (value == 12 && state == IUIScene_AbstractContainerMenu::ETapState_Down) { m_sceneInterface->updateSlotPosition(section, inventory, state, x, y, 0, 0); return inventory; }
    if (value == 12 && state == IUIScene_AbstractContainerMenu::ETapState_Up) { m_sceneInterface->updateSlotPosition(section, armor, state, x, y, 0, 0); return armor; }
    if (value == 13 && state == IUIScene_AbstractContainerMenu::ETapState_Down) { m_sceneInterface->updateSlotPosition(section, result, state, x, y, 1, 1); return result; }
    if (value == 14 && state == IUIScene_AbstractContainerMenu::ETapState_Down) { m_sceneInterface->updateSlotPosition(section, inventory, state, x, y, -5, 0); return inventory; }
    if (value == 14 && state == IUIScene_AbstractContainerMenu::ETapState_Up) { m_sceneInterface->updateSlotPosition(section, crafting, state, x, y, 0, 0); return crafting; }
    m_sceneInterface->updateSlotPosition(section, section, state, x, y, 0, 0);
    return section;
}

void UIScene_InventoryMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    if (position == 0) return;
    UIControl* control = 0;
    const unsigned int value = static_cast<unsigned int>(section);
    if (value == 10) control = &m_hotbarList;
    else if (value == 11) control = &m_inventoryList;
    else if (value == 12) control = &m_armorList;
    else if (value == 14) control = &m_craftingList;
    else if (value == 15) control = &m_resultList;
    else if (value == 13 || value > 15) control = &m_shieldList;
    if (control == 0) return;
    position->x = static_cast<float>(control->getXPos());
    position->y = static_cast<float>(control->getYPos());
}

void UIScene_InventoryMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, _UIVec2D* position, _UIVec2D* size)
{
    if (position == 0 || size == 0) return;
    const unsigned int columns = getSectionColumns(section);
    const unsigned int rows = getSectionRows(section);
    if (columns == 0 || rows == 0) return;
    UIControl* control = 0;
    const unsigned int value = static_cast<unsigned int>(section);
    if (value == 10) control = &m_hotbarList;
    else if (value == 11) control = &m_inventoryList;
    else if (value == 12) control = &m_armorList;
    else if (value == 14) control = &m_craftingList;
    else if (value == 15) control = &m_resultList;
    else if (value == 13 || value > 15) control = &m_shieldList;
    if (control == 0) return;
    position->x = static_cast<float>(slot % static_cast<int>(columns));
    position->y = static_cast<float>(slot / static_cast<int>(columns));
    size->x = static_cast<float>(control->getWidth() / columns);
    size->y = static_cast<float>(control->getHeight() / rows);
}

void UIScene_InventoryMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_InventoryMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    const unsigned int value = static_cast<unsigned int>(section);
    if (value == 10) return 36;
    if (value == 11) return 9;
    if (value == 12) return 5;
    if (value == 13) return 45;
    if (value == 14) return m_sceneInterface->m_inventorySmall ? 1 : 9;
    if (value == 15) return m_sceneInterface->m_inventorySmall ? 0 : 5;
    return 0;
}

void UIScene_InventoryMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    if (!wrap) return;
    const unsigned int value = static_cast<unsigned int>(section);
    UIControl_SlotList* list = 0;
    if (value == 12) list = &m_armorList;
    else if (value == 14) list = &m_craftingList;
    else if (value == 15) list = &m_resultList;
    else if (value == 13 || value > 15) list = &m_shieldList;
    if (list != 0) list->setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
}

void UIScene_InventoryMenu::SetClassicCrafting(bool classicCrafting)
{
    FJ_InventoryMenu* document = static_cast<FJ_InventoryMenu*>(m_movie->getRootNode()->getFuiNode());
    document->SetClassicCrafting(classicCrafting);
    if (!classicCrafting) { m_armorList.UpdateControl(); m_playerControl.UpdateControl(); m_shieldList.UpdateControl(); }
}

void UIScene_InventoryMenu::updateEffectsDisplay()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft == 0) return;
    boost::shared_ptr<Player> player = minecraft->GetPlayerByPlayerIndex(m_playerId);
    if (!player) return;
    std::vector<MobEffectInstance*>* effects = player->getActiveEffects();
    if (effects == 0) return;
    int values[54];
    int count = 0;
    std::vector<MobEffectInstance*>::iterator effect;
    FJ_InventoryMenu* document = static_cast<FJ_InventoryMenu*>(m_movie->getRootNode()->getFuiNode());
    for (effect = effects->begin(); effect != effects->end(); ++effect)
    {
        MobEffectInstance* instance = *effect;
        MobEffect* type = instance->getEffect();
        const int id = MobEffect::getId(type);
        const int duration = instance->getDuration();
        if (id >= 0 && id <= static_cast<int>(MobEffect::GetEffectCount()) && m_effectDurations[id] <= duration)
        {
            m_effectDurations[id] = duration;
            if (type->hasIcon())
            {
                std::wstring name = L10N::GetString(instance->getDescriptionId());
                if (instance->getAmplifier() > 0) name += L" ";
                if (count < 54) { values[count++] = type->getIcon(); values[count++] = duration / 20; }
                document->AddEffect(type->getIcon(), name, duration / 20);
            }
        }
    }
    document->UpdateEffects(count, values);
}

unsigned int UIScene_InventoryMenu::getSubSceneType() const
{
    return m_sceneInterface->m_inventorySmall;
}

unsigned int UIScene_InventoryMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_armorList.setupControl(this, panel, std::string("armorList")); m_controls.push_back(&m_armorList);
    m_craftingList.setupControl(this, panel, std::string("craftingList")); m_controls.push_back(&m_craftingList);
    m_resultList.setupControl(this, panel, std::string("Result")); m_controls.push_back(&m_resultList);
    m_playerControl.setupControl(this, panel, std::string("iggy_player")); m_controls.push_back(&m_playerControl);
    m_craftingLabel.setupControl(this, panel, std::string("craftingLabel")); m_controls.push_back(&m_craftingLabel);
    m_shieldList.setupControl(this, panel, std::string("shieldList")); m_controls.push_back(&m_shieldList);
    return 1;
}

std::wstring UIScene_InventoryMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"InventoryMenu") : std::wstring(L"InventoryMenuSplit");
}

void UIScene_InventoryMenu::handleReload()
{
    SetClassicCrafting(getSubSceneType() != 0);
    m_sceneInterface->Initialize(true, m_sceneInterface->m_containerId, m_sceneInterface->m_menu, false, m_playerId,
        static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(10), static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(getSubSceneType() ? 16 : 14), m_sceneInterface->m_inventorySmall != 0);
    m_armorList.addSlots(5, 4); m_shieldList.addSlots(45, 1);
    if (getSubSceneType()) { m_craftingList.addSlots(1, 4); m_resultList.addSlots(0, 1); }
    for (unsigned int i = 0; i <= MobEffect::GetEffectCount(); ++i) m_effectDurations[i] = 0;
}

UIControl* UIScene_InventoryMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    const unsigned int value = static_cast<unsigned int>(section);
    if (value == 10) return &m_hotbarList;
    if (value == 11) return &m_inventoryList;
    if (value == 12) return &m_armorList;
    if (value == 14) return &m_craftingList;
    if (value == 15) return &m_resultList;
    if (value == 13 || value > 15) return &m_shieldList;
    return 0;
}

void UIScene_InventoryMenu::customDraw(const char* name, fuiRect* rect)
{
    if (Minecraft::GetInstance() != 0 && std::string(name) == std::string("px____0")) { m_playerControl.render(rect); return; }
    UIScene_AbstractContainerMenu::customDraw(name, rect);
}

void UIScene_InventoryMenu::handleTimerComplete(int timerId)
{
    if (timerId == 10) updateEffectsDisplay();
}

int UIScene_InventoryMenu::getSceneType()
{
    return 11;
}
