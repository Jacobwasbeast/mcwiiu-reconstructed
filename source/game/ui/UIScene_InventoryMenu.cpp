#include "game/ui/UIScene_InventoryMenu.hpp"
#include "game/core/StringIds.hpp"
#include <cstring>

#include <cstdio>
#include <vector>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/L10N.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/core/Slot.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/FJ_InventoryMenu.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/ui/UILayer.hpp"

extern CMinecraftApp* g_pApp;

namespace
{
struct InventoryMenuSceneData
{
    boost::shared_ptr<LocalPlayer> localPlayer;
    bool creative;
    int player;
    bool splitscreen;
};
}

UIScene_InventoryMenu::UIScene_InventoryMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_effectDurations(0),
      m_armorList(), m_craftingList(), m_resultList(), m_shieldList(), m_playerControl(), m_craftingLabel(),
      m_classicCrafting(false)
{
    InventoryMenuSceneData* sceneData = static_cast<InventoryMenuSceneData*>(data);
    initialiseMovie();
    m_effectDurations = new int[MobEffect::GetEffectCount() + 1];
    m_classicCrafting = GameSettings::GetGameSettings(m_playerId, eGameSetting_ClassicCrafting) != 0 &&
        g_pApp->GetGameHostOption(eGameHostOption_MiniGame) == 0;
    SetClassicCrafting(m_classicCrafting);

    MultiPlayerGameMode* gameMode = Minecraft::GetInstance()->mGameModes[sceneData->player];
    if (gameMode != 0)
    {
        Tutorial* tutorial = gameMode->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_InventoryMenu, this);
        }
    }

    sceneData->localPlayer->awardStat(GenericStats::openInventory(), GenericStats::param_openInventory());
    Initialize(false, sceneData->player, sceneData->localPlayer->mInventoryMenu,
                                 false, 9,
                                 IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar,
                                 m_classicCrafting ? IUIScene_AbstractContainerMenu::ESceneSection_InventoryMax : IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting,
                                 sceneData->creative);
    m_armorList.addSlots(5, 4);
    m_shieldList.addSlots(45, 1);
    if (m_classicCrafting)
    {
        m_craftingList.addSlots(1, 4);
        m_resultList.addSlots(0, 1);
        m_craftingLabel.setLabel(UIString(IDS_CONTROLS_CRAFTING), false, false);
    }
    delete sceneData;

    for (unsigned int i = 0; i <= MobEffect::GetEffectCount(); ++i)
        m_effectDurations[i] = 0;
    updateEffectsDisplay();
    addTimer(10, 1000);
}

UIScene_InventoryMenu::~UIScene_InventoryMenu()
{
    delete[] m_effectDurations;
}

unsigned int UIScene_InventoryMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryInventory:
        return 9;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryArmor:
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryShield:
        return 1;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting:
        return m_classicCrafting ? 2 : 1;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult:
        return 1;
    default:
        return 0;
    }
}

unsigned int UIScene_InventoryMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar:
        return 1;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryInventory:
        return GetInventoryRows();
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryArmor:
        return 4;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryShield:
        return 1;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting:
        return m_classicCrafting ? 2 : 1;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult:
        return 1;
    default:
        return 0;
    }
}

unsigned int UIScene_InventoryMenu::GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section, IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    typedef IUIScene_AbstractContainerMenu::ESceneSection ESceneSection;
    const ESceneSection hotbar = ESceneSection_InventoryHotbar;
    const ESceneSection inventory = ESceneSection_InventoryInventory;
    const ESceneSection armor = ESceneSection_InventoryArmor;
    const ESceneSection shield = ESceneSection_InventoryShield;
    const ESceneSection crafting = ESceneSection_InventoryCrafting;
    const ESceneSection result = ESceneSection_InventoryResult;
    const ESceneSection below = m_smallInventory ? hotbar : inventory;

    ESceneSection target = section;
    int xOffset = 0;
    int yOffset = 0;
    bool choosePlayerColumn = false;

    if (section == hotbar)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            choosePlayerColumn = true;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (!m_smallInventory)
                target = inventory;
            else
                choosePlayerColumn = true;
        }
    }
    else if (section == inventory)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = hotbar;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            choosePlayerColumn = true;
    }
    else if (section == armor)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = below;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = hotbar;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = m_classicCrafting && *y <= 2 ? result : shield;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            if (m_classicCrafting && *y < 3)
            {
                target = crafting;
                xOffset = 1;
                yOffset = 1;
            }
            else
            {
                target = shield;
            }
        }
    }
    else if (section == shield)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = below;
            xOffset = m_classicCrafting ? -5 : -8;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = m_classicCrafting ? crafting : hotbar;
            xOffset = m_classicCrafting ? -5 : -8;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left ||
                 state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            target = armor;
            yOffset = 3;
        }
    }
    else if (section == crafting && m_classicCrafting)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = below;
            xOffset = -5;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = hotbar;
            xOffset = -5;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
        {
            target = armor;
            yOffset = -1;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            target = result;
        }
    }
    else if (section == result && m_classicCrafting)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = below;
            xOffset = -8;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = hotbar;
            xOffset = -8;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
        {
            *x = static_cast<int>(getSectionColumns(crafting));
            target = crafting;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            target = armor;
        }
    }

    if (choosePlayerColumn)
    {
        if (m_classicCrafting)
        {
            if (*x < 3)
                target = armor;
            else if (*x > 4 && *x < 7)
            {
                target = crafting;
                xOffset = 5;
            }
            else if (*x > 4)
                target = result;
            else
                target = shield;
        }
        else
        {
            target = *x < 5 ? armor : shield;
        }
    }

    updateSlotPosition(section, target, state, x, y, xOffset, yOffset);
    return static_cast<unsigned int>(target);
}

void UIScene_InventoryMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    UIControl* control = NULL;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar:
        control = &m_hotbarList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryInventory:
        control = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryArmor:
        control = &m_armorList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting:
        control = m_classicCrafting ? static_cast<UIControl*>(&m_craftingList) : &m_shieldList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult:
        control = m_classicCrafting ? static_cast<UIControl*>(&m_resultList) : &m_shieldList;
        break;
    default:
        if (section == IUIScene_AbstractContainerMenu::ESceneSection_InventoryShield || section > IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult)
            control = &m_shieldList;
        break;
    }
    if (control == NULL)
        return;
    position->x = static_cast<float>(control->getXPos());
    position->y = static_cast<float>(control->getYPos());
}

void UIScene_InventoryMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int itemIndex, _UIVec2D* position, _UIVec2D* size)
{
    UIControl* control = NULL;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar:
        control = &m_hotbarList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryInventory:
        control = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryArmor:
        control = &m_armorList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting:
        control = m_classicCrafting ? static_cast<UIControl*>(&m_craftingList) : &m_shieldList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult:
        control = m_classicCrafting ? static_cast<UIControl*>(&m_resultList) : &m_shieldList;
        break;
    default:
        if (section == IUIScene_AbstractContainerMenu::ESceneSection_InventoryShield)
            control = &m_shieldList;
        break;
    }
    if (control == NULL)
        return;
    int rows = static_cast<int>(getSectionRows(section));
    int columns = static_cast<int>(getSectionColumns(section));
    float itemWidth = static_cast<float>(control->getWidth() / static_cast<float>(columns));
    float itemHeight = static_cast<float>(control->getHeight() / static_cast<float>(rows));
    int row = itemIndex / columns;
    int column = itemIndex - row * columns;
    size->x = itemWidth;
    size->y = itemHeight;
    position->x = static_cast<float>(column) * itemWidth;
    position->y = static_cast<float>(row) * itemHeight;
}

void UIScene_InventoryMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_InventoryMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar) return 36;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_InventoryInventory) return 9;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_InventoryArmor) return 5;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_InventoryShield) return 45;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting) return m_smallInventory ? 1 : 9;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult) return m_smallInventory ? 0 : 5;
    return 0;
}

void UIScene_InventoryMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    if (!wrap) return;
    UIControl_SlotList* list = 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_InventoryArmor) list = &m_armorList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting) list = &m_craftingList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult) list = &m_resultList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_InventoryShield || section > IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult) list = &m_shieldList;
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
    boost::shared_ptr<Player> player = minecraft->mLocalPlayers[m_playerId];
    if (!player) return;
    std::vector<MobEffectInstance*>* effects = player->getActiveEffects();
    if (effects == 0) return;

    FJ_InventoryMenu* document = static_cast<FJ_InventoryMenu*>(m_movie->getRootNode()->getFuiNode());
    int* values = new int[effects->size() * 2];
    int count = 0;
    std::vector<MobEffectInstance*>::iterator effect;
    for (effect = effects->begin(); effect != effects->end(); ++effect)
    {
        MobEffectInstance* instance = *effect;
        MobEffect* type = instance->getEffect();
        const int id = MobEffect::getId(type);
        const int duration = instance->getDuration();
        if (m_effectDurations[id] <= duration)
        {
            std::wstring displayName = L10N::GetString(instance->getDescriptionId());
            const int amplifier = instance->getAmplifier();
            if (amplifier > 0)
            {
                std::wstring suffix;
                switch (amplifier)
                {
                case 1: suffix = std::wstring(L" ") + L10N::GetString(0xe0296084); break;
                case 2: suffix = std::wstring(L" ") + L10N::GetString(0x5230cfbf); break;
                case 3: suffix = std::wstring(L" ") + L10N::GetString(0x2c2e5556); break;
                case 4: suffix = std::wstring(L" ") + L10N::GetString(0x6e21f149); break;
                case 5: suffix = std::wstring(L" ") + L10N::GetString(0x481f76e0); break;
                default: suffix = L10N::GetString(0x62bdaed); break;
                }
                displayName += suffix;
            }
            document->AddEffect(type->hasIcon() ? type->getIcon() : 0, displayName, duration / 20);
        }
        if (type->hasIcon())
        {
            values[count++] = type->getIcon();
            values[count++] = duration / 20;
        }
        m_effectDurations[id] = duration;
    }
    document->UpdateEffects(count, values);
    delete[] values;
}

unsigned int UIScene_InventoryMenu::getSubSceneType() const
{
    return m_classicCrafting;
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
    Initialize(true, m_playerId, m_menu, false, 9,
        IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar,
        getSubSceneType() ? IUIScene_AbstractContainerMenu::ESceneSection_InventoryMax : IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting,
        m_navigateBack != 0);
    m_armorList.addSlots(5, 4); m_shieldList.addSlots(45, 1);
    if (getSubSceneType()) { m_craftingList.addSlots(1, 4); m_resultList.addSlots(0, 1); }
    for (unsigned int i = 0; i <= MobEffect::GetEffectCount(); ++i) m_effectDurations[i] = 0;
}

UIControl* UIScene_InventoryMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar:
        return &m_hotbarList;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryInventory:
        return &m_inventoryList;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryArmor:
        return &m_armorList;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryShield:
        return &m_shieldList;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting:
        return m_classicCrafting ? &m_craftingList : &m_shieldList;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult:
        return m_classicCrafting ? &m_resultList : &m_shieldList;
    default:
        return NULL;
    }
}

void UIScene_InventoryMenu::customDraw(const char* name, fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mLocalPlayers[m_playerId] == NULL || minecraft->mGameModes[m_playerId] == NULL)
        return;

    if (std::strcmp(name, "player") == 0)
    {
        m_playerControl.render(rect);
        return;
    }

    UIScene_AbstractContainerMenu::customDraw(name, rect);

    int slotId = -1;
    std::sscanf(name, "slot_%d", &slotId);
    if (slotId >= 5 && slotId < 9)
    {
        bool empty = m_menu->getSlot(slotId)->getItem()->isEmpty();
        static_cast<FJ_InventoryMenu*>(m_movie->getRootNode()->getFuiNode())->ShowArmorSlot(slotId - 5, empty);
    }
}

void UIScene_InventoryMenu::handleTimerComplete(int timerId)
{
    if (timerId == 10) updateEffectsDisplay();
}

int UIScene_InventoryMenu::getSceneType()
{
    return EUIScene_InventoryMenu;
}
