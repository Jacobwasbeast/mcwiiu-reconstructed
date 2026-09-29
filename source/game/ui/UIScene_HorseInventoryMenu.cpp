#include "game/ui/UIScene_HorseInventoryMenu.hpp"

#include <cstring>
#include "game/core/EntityType.hpp"
#include "game/core/StringIds.hpp"

#include "game/entity/mob/AbstractChestedHorse.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Inventory.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/HorseInventoryMenu.hpp"
#include "game/ui/FJ_HorseInventoryMenu.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/ui/UILayer.hpp"

extern CMinecraftApp* g_pApp;

namespace
{
struct HorseMenuArgs
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Container> container;
    boost::shared_ptr<AbstractHorse> horse;
    int player;
    bool splitscreen;
};
}

UIScene_HorseInventoryMenu::UIScene_HorseInventoryMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_container(),
      m_playerInventory(), m_horse(), m_saddleSlot(), m_armorSlot(), m_donkeyInventoryList(),
      m_horseLabel(), m_horseControl()
{
    initialiseMovie();

    HorseMenuArgs* args = static_cast<HorseMenuArgs*>(data);
    m_playerId = args->player;
    m_playerInventory = boost::dynamic_pointer_cast<Container>(args->inventory);
    m_container = args->container;
    m_horse = args->horse;
    const std::wstring containerName = m_container->getName();
    if (containerName.empty())
    {
        if (m_horse->GetType() == EntityType::LLAMA)
            m_horseLabel.init(UIString(L10N::GetString(IDS_LLAMA)));
        else if (m_horse->GetType() == EntityType::DONKEY)
            m_horseLabel.init(UIString(L10N::GetString(IDS_DONKEY)));
        else if (m_horse->GetType() == EntityType::MULE)
            m_horseLabel.init(UIString(L10N::GetString(IDS_MULE)));
        else if (m_horse->GetType() == EntityType::SKELETON_HORSE)
            m_horseLabel.init(UIString(L10N::GetString(IDS_SKELETON_HORSE)));
        else
            m_horseLabel.init(UIString(L10N::GetString(0x2568bd7a)));
    }
    else
    {
        m_horseLabel.init(UIString(containerName));
    }
    boost::shared_ptr<Player> player = Minecraft::GetInstance()->mLocalPlayers[m_playerId];
    m_menu = new HorseInventoryMenu(
        m_playerInventory, m_container, m_horse, player);

    if (Minecraft::GetInstance()->mGameModes[m_playerId] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[m_playerId]->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_HorseMenu, this);
        }
    }
    setUp(false);
    delete args;
}

UIScene_HorseInventoryMenu::~UIScene_HorseInventoryMenu()
{
}

bool UIScene_HorseInventoryMenu::IsSectionSlotList(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section >= IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar &&
        section <= IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory)
        return true;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseChest)
        return m_horse && boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->hasChest();
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor)
        return m_horse && m_horse->wearsArmor();
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle)
        return m_horse && m_horse->isSaddled();
    return false;
}

unsigned int UIScene_HorseInventoryMenu::IsVisible(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar ||
        section == IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory)
        return 1;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseChest)
        return (m_horse->GetType() & EntityType::ABSTRACT_CHESTED_HORSE) == EntityType::ABSTRACT_CHESTED_HORSE &&
            boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->hasChest() ? 1U : 0U;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor)
        return m_horse->wearsArmor() ? 1U : 0U;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle)
        return m_horse->canBeSaddled() ? 1U : 0U;
    return 0;
}

unsigned int UIScene_HorseInventoryMenu::getSectionColumns(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar ||
        section == IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory)
        return 9;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseChest)
        return m_horse ? static_cast<unsigned int>(boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->getInventoryColumns()) : 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor ||
        section == IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle)
        return 1;
    return 0;
}

unsigned int UIScene_HorseInventoryMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar)
        return 1;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory)
        return GetInventoryRows();
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseChest)
        return 3;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor ||
        section == IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle)
        return 1;
    return 0;
}

unsigned int UIScene_HorseInventoryMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    const IUIScene_AbstractContainerMenu::ESceneSection hotbar =
        IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar;
    const IUIScene_AbstractContainerMenu::ESceneSection inventory =
        IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory;
    const IUIScene_AbstractContainerMenu::ESceneSection chest =
        IUIScene_AbstractContainerMenu::ESceneSection_HorseChest;
    const IUIScene_AbstractContainerMenu::ESceneSection armor =
        IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor;
    const IUIScene_AbstractContainerMenu::ESceneSection saddle =
        IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle;
    if (section == hotbar && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        updateSlotPosition(section, chest, state, x, y, 3, 0);
        return chest;
    }
    if (section == hotbar && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        const IUIScene_AbstractContainerMenu::ESceneSection target =
            m_smallInventory ? chest : inventory;
        updateSlotPosition(section, target, state, x, y, 0, 0);
        return target;
    }
    if (section == inventory && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        updateSlotPosition(section, hotbar, state, x, y, 0, 0);
        return hotbar;
    }
    if (section == inventory && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        updateSlotPosition(section, chest, state, x, y, 3, 0);
        return chest;
    }
    if (section == chest && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        updateSlotPosition(section, hotbar, state, x, y, -3, 0);
        return hotbar;
    }
    if (section == chest && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        const IUIScene_AbstractContainerMenu::ESceneSection target =
            m_smallInventory ? hotbar : inventory;
        updateSlotPosition(section, target, state, x, y, -3, 0);
        return target;
    }
    if (section == armor && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        updateSlotPosition(section, chest, state, x, y, -1, 0);
        return chest;
    }
    if (section == saddle && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        updateSlotPosition(section, chest, state, x, y, -1, 0);
        return chest;
    }
    updateSlotPosition(section, section, state, x, y, 0, 0);
    return section;
}

void UIScene_HorseInventoryMenu::GetPositionOfSection(
    IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    if (!position)
        return;
    UIControl* control = 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar) control = &m_hotbarList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory) control = &m_inventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseChest) control = &m_donkeyInventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor) control = &m_armorSlot;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle) control = &m_saddleSlot;
    if (control)
    {
        position->x = static_cast<float>(control->getXPos());
        position->y = static_cast<float>(control->getYPos());
    }
}

void UIScene_HorseInventoryMenu::GetItemScreenData(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
    _UIVec2D* position, _UIVec2D* size)
{
    UIControl* control = 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar) control = &m_hotbarList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory) control = &m_inventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseChest) control = &m_donkeyInventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor) control = &m_armorSlot;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle) control = &m_saddleSlot;
    const unsigned int columns = getSectionColumns(section);
    const unsigned int rows = getSectionRows(section);
    if (control && position && size && columns && rows)
    {
        size->x = static_cast<float>(control->getWidth() / static_cast<float>(columns));
        size->y = static_cast<float>(control->getHeight() / static_cast<float>(rows));
        position->x = static_cast<float>(slot % static_cast<int>(columns)) * size->x;
        position->y = static_cast<float>(slot / static_cast<int>(columns)) * size->y;
    }
}

void UIScene_HorseInventoryMenu::handleSectionClick(
    IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_HorseInventoryMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar)
        return m_horse ? m_horse->getInventorySize() + 27 : 27;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory)
        return m_horse ? m_horse->getInventorySize() : 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseChest)
        return 2;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor)
        return 1;
    return 0;
}

void UIScene_HorseInventoryMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    const int columns = getSectionColumns(section);
    UIControl_SlotList* slotList = NULL;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar:
        slotList = &m_hotbarList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory:
        slotList = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseChest:
        if ((m_horse->GetType() & EntityType::ABSTRACT_CHESTED_HORSE) == EntityType::ABSTRACT_CHESTED_HORSE &&
            boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->hasChest())
            slotList = &m_donkeyInventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor:
        if (!m_horse->wearsArmor())
            return;
        slotList = &m_armorSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle:
        if (!m_horse->canBeSaddled())
            return;
        slotList = &m_saddleSlot;
        break;
    default:
        return;
    }
    if (slotList != NULL && wrap)
        slotList->setHighlightSlot(direction * columns + slot);
}

void UIScene_HorseInventoryMenu::SetInventoryType(int type)
{
    fuiRenderNode* root = m_movie->getRootNode();
    FJ_HorseInventoryMenu* document = static_cast<FJ_HorseInventoryMenu*>(root->getFuiNode());
    if (document)
        document->SetInventoryType(type);
}

void UIScene_HorseInventoryMenu::SetHasInventory(bool hasInventory)
{
    fuiRenderNode* root = m_movie->getRootNode();
    FJ_HorseInventoryMenu* document = static_cast<FJ_HorseInventoryMenu*>(root->getFuiNode());
    if (document)
        document->SetHasInventory(hasInventory);
}

void UIScene_HorseInventoryMenu::SetHasSaddle(bool hasSaddle)
{
    fuiRenderNode* root = m_movie->getRootNode();
    FJ_HorseInventoryMenu* document = static_cast<FJ_HorseInventoryMenu*>(root->getFuiNode());
    if (document)
        document->SetCanHaveSaddle(hasSaddle);
}

void UIScene_HorseInventoryMenu::setUp(bool enabled)
{
    const bool hasChest = (m_horse->GetType() & EntityType::ABSTRACT_CHESTED_HORSE) == EntityType::ABSTRACT_CHESTED_HORSE &&
        boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->hasChest();
    const int sectionCount = hasChest
        ? boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->getInventoryColumns() * 3 + 2
        : 2;
    Initialize(enabled, m_playerId,
                                 m_menu, true, sectionCount,
                                 IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar,
                                 IUIScene_AbstractContainerMenu::ESceneSection_HorseMax, false);
    if (!m_horse->wearsArmor())
    {
        SetInventoryType(1);
    }
    else
    {
        int type = 0;
        if (m_horse->GetType() == EntityType::LLAMA)
            type = 2;
        SetInventoryType(type);
        m_armorSlot.addSlots(1, 1);
    }
    const bool hasInventory = (m_horse->GetType() & EntityType::ABSTRACT_CHESTED_HORSE) == EntityType::ABSTRACT_CHESTED_HORSE &&
        boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->hasChest();
    if (hasInventory)
    {
        SetHasInventory(true);
        const int columns = boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->getInventoryColumns();
        m_donkeyInventoryList.setWidth(m_donkeyInventoryList.getWidth() * (static_cast<float>(columns) / 5.0f));
        m_donkeyInventoryList.addSlots(2, columns * 3);
    }
    else
    {
        SetHasInventory(false);
    }
    SetHasSaddle(m_horse->canBeSaddled());
    if (m_horse->canBeSaddled())
        m_saddleSlot.addSlots(0, 1);
}

unsigned int UIScene_HorseInventoryMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_saddleSlot.setupControl(this, panel, std::string("SlotSaddle"));
    m_controls.push_back(&m_saddleSlot);
    m_armorSlot.setupControl(this, panel, std::string("SlotArmor"));
    m_controls.push_back(&m_armorSlot);
    m_donkeyInventoryList.setupControl(this, panel, std::string("DonkeyInventoryList"));
    m_controls.push_back(&m_donkeyInventoryList);
    m_horseLabel.setupControl(this, panel, std::string("horseinventoryText"));
    m_controls.push_back(&m_horseLabel);
    m_horseControl.setupControl(this, panel, std::string("iggy_horse"));
    m_controls.push_back(&m_horseControl);
    return 1;
}

std::wstring UIScene_HorseInventoryMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"HorseInventoryMenu") :
        std::wstring(L"HorseInventoryMenuSplit");
}

void UIScene_HorseInventoryMenu::handleReload()
{
    setUp(true);
}

UIControl* UIScene_HorseInventoryMenu::getSection(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar)
        return &m_hotbarList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory)
        return &m_inventoryList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseChest)
        return (m_horse->GetType() & EntityType::ABSTRACT_CHESTED_HORSE) == EntityType::ABSTRACT_CHESTED_HORSE &&
            boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->hasChest()
            ? &m_donkeyInventoryList : 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor)
        return m_horse->wearsArmor() ? &m_armorSlot : 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle)
        return m_horse->canBeSaddled() ? &m_saddleSlot : 0;
    return 0;
}

void UIScene_HorseInventoryMenu::customDraw(const char* name, fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mLocalPlayers[m_playerId] == NULL || minecraft->mGameModes[m_playerId] == NULL)
        return;
    if (std::strcmp(name, "horse") == 0)
    {
        m_horseControl.render(rect);
        return;
    }
    UIScene_AbstractContainerMenu::customDraw(name, rect);
}

int UIScene_HorseInventoryMenu::getSceneType()
{
    return EUIScene_HorseInventoryMenu;
}
