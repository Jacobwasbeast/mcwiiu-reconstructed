#include "game/ui/UIScene_HorseInventoryMenu.hpp"

#include <cstring>
#include "game/core/EntityType.hpp"
#include "game/core/StringIds.hpp"

#include "game/entity/mob/AbstractChestedHorse.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
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

namespace
{
struct HorseMenuArgs
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Container> container;
    boost::shared_ptr<AbstractHorse> horse;
};
}

UIScene_HorseInventoryMenu::UIScene_HorseInventoryMenu(int player, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(player, layer), m_playerInventory(), m_container(), m_horse(),
      m_saddleSlot(), m_armorSlot(), m_donkeyInventoryList(), m_horseLabel(), m_horseControl()
{
    initialiseMovie();

    HorseMenuArgs* args = static_cast<HorseMenuArgs*>(data);
    m_horse = args->horse;
    if (!args->container->getName().empty())
    {
        m_horseLabel.init(UIString(args->container->getName()));
    }
    else
    {
        const bool llama = m_horse->GetType() == EntityType::LLAMA;
        if (llama)
            m_horseLabel.init(UIString(L10N::GetString(IDS_LLAMA)));
        else
        {
            const bool donkey = m_horse->GetType() == EntityType::DONKEY;
            if (donkey)
                m_horseLabel.init(UIString(L10N::GetString(IDS_DONKEY)));
            else
            {
                const bool mule = m_horse->GetType() == EntityType::MULE;
                if (mule)
                    m_horseLabel.init(UIString(L10N::GetString(IDS_MULE)));
                else
                {
                    const bool skeleton = m_horse->GetType() == EntityType::SKELETON_HORSE;
                    if (skeleton)
                        m_horseLabel.init(UIString(L10N::GetString(IDS_SKELETON_HORSE)));
                    else
                        m_horseLabel.init(UIString(L10N::GetString(IDS_CONTAINER_ANIMAL)));
                }
            }
        }
    }
    m_playerInventory = args->inventory;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[player] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[player]->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_HorseMenu, this);
        }
    }
    m_playerId = player;
    m_menu = new HorseInventoryMenu(boost::dynamic_pointer_cast<Container>(args->inventory),
                                    args->container, args->horse,
                                    minecraft->mLocalPlayers[m_playerId]);
    setUp(false);
    delete args;
    m_ignoreInput = false;
}

UIScene_HorseInventoryMenu::~UIScene_HorseInventoryMenu()
{
}

bool UIScene_HorseInventoryMenu::IsSectionSlotList(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseChest:
    {
        const bool slotList = (m_horse->GetType() & EntityType::ABSTRACT_CHESTED_HORSE) == EntityType::ABSTRACT_CHESTED_HORSE &&
            boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->hasChest();
        return slotList;
    }
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor:
        return m_horse->wearsArmor();
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle:
        return m_horse->canBeSaddled();
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory:
        return true;
    default:
        return false;
    }
}

unsigned int UIScene_HorseInventoryMenu::IsVisible(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseChest:
    {
        const bool visible = (m_horse->GetType() & EntityType::ABSTRACT_CHESTED_HORSE) == EntityType::ABSTRACT_CHESTED_HORSE &&
            boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->hasChest();
        return visible;
    }
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor:
        return m_horse->wearsArmor();
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle:
        return m_horse->canBeSaddled();
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory:
        return 1;
    default:
        return 0;
    }
}

unsigned int UIScene_HorseInventoryMenu::getSectionColumns(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    unsigned int columns = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor:
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle:
        columns = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseChest:
        columns = boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->getInventoryColumns();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory:
        columns = 9;
        break;
    default:
        break;
    }
    return columns;
}

unsigned int UIScene_HorseInventoryMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    unsigned int rows = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor:
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle:
        rows = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseChest:
        rows = 3;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory:
        rows = GetInventoryRows();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar:
        rows = 1;
        break;
    default:
        break;
    }
    return rows;
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
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar:
        position->x = static_cast<float>(m_hotbarList.getXPos());
        position->y = static_cast<float>(m_hotbarList.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory:
        position->x = static_cast<float>(m_inventoryList.getXPos());
        position->y = static_cast<float>(m_inventoryList.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseChest:
        position->x = static_cast<float>(m_donkeyInventoryList.getXPos());
        position->y = static_cast<float>(m_donkeyInventoryList.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor:
        position->x = static_cast<float>(m_armorSlot.getXPos());
        position->y = static_cast<float>(m_armorSlot.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle:
        position->x = static_cast<float>(m_saddleSlot.getXPos());
        position->y = static_cast<float>(m_saddleSlot.getYPos());
        break;
    default:
        break;
    }
}

void UIScene_HorseInventoryMenu::GetItemScreenData(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
    _UIVec2D* position, _UIVec2D* size)
{
    _UIVec2D controlSize;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle:
        controlSize.x = m_saddleSlot.getWidth();
        controlSize.y = m_saddleSlot.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor:
        controlSize.x = m_armorSlot.getWidth();
        controlSize.y = m_armorSlot.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseChest:
        controlSize.x = m_donkeyInventoryList.getWidth();
        controlSize.y = m_donkeyInventoryList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory:
        controlSize.x = m_inventoryList.getWidth();
        controlSize.y = m_inventoryList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar:
        controlSize.x = m_hotbarList.getWidth();
        controlSize.y = m_hotbarList.getHeight();
        break;
    default:
        break;
    }
    if (IsSectionSlotList(section))
    {
        const int rows = getSectionRows(section);
        const int columns = getSectionColumns(section);
        size->y = controlSize.y / rows;
        size->x = controlSize.x / columns;
        position->x = (slot % columns) * size->x;
        position->y = (slot / columns) * size->y;
    }
    else
    {
        GetPositionOfSection(section, position);
        size->x = controlSize.x;
        size->y = controlSize.y;
    }
}

void UIScene_HorseInventoryMenu::handleSectionClick(
    IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_HorseInventoryMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor:
        return 1;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseChest:
        return 2;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory:
        return m_horse->getInventorySize();
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar:
        return m_horse->getInventorySize() + 27;
    default:
        return 0;
    }
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
    document->SetInventoryType(type);
}

void UIScene_HorseInventoryMenu::SetHasInventory(bool hasInventory)
{
    fuiRenderNode* root = m_movie->getRootNode();
    FJ_HorseInventoryMenu* document = static_cast<FJ_HorseInventoryMenu*>(root->getFuiNode());
    document->SetHasInventory(hasInventory);
}

void UIScene_HorseInventoryMenu::SetHasSaddle(bool hasSaddle)
{
    fuiRenderNode* root = m_movie->getRootNode();
    FJ_HorseInventoryMenu* document = static_cast<FJ_HorseInventoryMenu*>(root->getFuiNode());
    document->SetCanHaveSaddle(hasSaddle);
}

void UIScene_HorseInventoryMenu::setUp(bool enabled)
{
    int sectionCount = 2;
    if ((m_horse->GetType() & EntityType::ABSTRACT_CHESTED_HORSE) == EntityType::ABSTRACT_CHESTED_HORSE &&
        boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->hasChest())
    {
        sectionCount = boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->getInventoryColumns() * 3 + 2;
    }
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
        const bool llama = m_horse->GetType() == EntityType::LLAMA;
        SetInventoryType(llama ? 2 : 0);
        m_armorSlot.addSlots(1, 1);
    }
    if ((m_horse->GetType() & EntityType::ABSTRACT_CHESTED_HORSE) == EntityType::ABSTRACT_CHESTED_HORSE &&
        boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->hasChest())
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
    m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    {
        m_saddleSlot.setupControl(this, panel, std::string("SlotSaddle"));
        m_controls.push_back(&m_saddleSlot);
    }
    {
        m_armorSlot.setupControl(this, panel, std::string("SlotArmor"));
        m_controls.push_back(&m_armorSlot);
    }
    {
        m_donkeyInventoryList.setupControl(this, panel, std::string("DonkeyInventoryList"));
        m_controls.push_back(&m_donkeyInventoryList);
    }
    {
        m_horseLabel.setupControl(this, panel, std::string("horseinventoryText"));
        m_controls.push_back(&m_horseLabel);
    }
    {
        m_horseControl.setupControl(this, panel, std::string("iggy_horse"));
        m_controls.push_back(&m_horseControl);
    }
    return 1;
}

std::wstring UIScene_HorseInventoryMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"HorseInventoryMenuSplit") :
        std::wstring(L"HorseInventoryMenu");
}

void UIScene_HorseInventoryMenu::handleReload()
{
    setUp(true);
}

UIControl* UIScene_HorseInventoryMenu::getSection(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    UIControl* control = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseSaddle:
        if (m_horse->canBeSaddled())
            control = &m_saddleSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseArmor:
        if (m_horse->wearsArmor())
            control = &m_armorSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseChest:
        if ((m_horse->GetType() & EntityType::ABSTRACT_CHESTED_HORSE) == EntityType::ABSTRACT_CHESTED_HORSE &&
            boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->hasChest())
            control = &m_donkeyInventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseInventory:
        control = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HorseHotbar:
        control = &m_hotbarList;
        break;
    default:
        break;
    }
    return control;
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
