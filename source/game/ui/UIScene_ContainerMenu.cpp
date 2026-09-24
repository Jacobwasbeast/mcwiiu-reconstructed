#include "game/ui/UIScene_ContainerMenu.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/item/Container.hpp"
#include "game/ui/ChestMenu.hpp"
#include "game/ui/ShulkerBoxMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"

namespace
{
struct _ContainerMenuArgs
{
    boost::shared_ptr<Container> first;
    boost::shared_ptr<Container> second;
    int player;
    bool splitscreen;
};
}

UIScene_ContainerMenu::UIScene_ContainerMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer),
      m_largeChest(false),
      m_containerList(),
      m_chestLabel(),
      m_sceneInterface(0)
{
    m_sceneInterface = this;

    _ContainerMenuArgs* args = static_cast<_ContainerMenuArgs*>(data);
    m_largeChest = args->second->getContainerSize() > 27;

    initialiseMovie();

    const int openedSize = static_cast<int>(args->second->getContainerSize());
    const std::wstring openedName = args->second->getName();
    boost::shared_ptr<Player> player = Minecraft::GetInstance()->mLocalPlayers[args->player];
    AbstractContainerMenu* menu = 0;

    if (openedName.empty())
    {
        switch (openedSize)
        {
        case 0xd:
            m_chestLabel.init(UIString(L10N::GetString(0xf1fc8097)));
            break;
        case 0xf:
            m_chestLabel.init(UIString(L10N::GetString(0xdaf7bb26)));
            break;
        case 0xe:
            m_chestLabel.init(UIString(L10N::GetString(0x68521aaf)));
            break;
        case 0x10:
            m_chestLabel.init(UIString(L10N::GetString(0x917ea687)));
            break;
        case 0x15:
            m_chestLabel.init(UIString(L10N::GetString(0x7b0460c8)));
            menu = new ShulkerBoxMenu(args->first, args->second, player);
            break;
        default:
            m_chestLabel.init(UIString(L10N::GetString(0x24fee045)));
            break;
        }
    }
    else
    {
        m_chestLabel.init(UIString(openedName));
        if (openedSize == 0x15)
            menu = new ShulkerBoxMenu(args->first, args->second, player);
    }

    if (menu == 0)
        menu = new ChestMenu(args->first, args->second, player);

    m_sceneInterface->m_menu = menu;
    if (Minecraft::GetInstance()->mGameModes[args->player] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[args->player]->getTutorial();
        if (tutorial != 0)
        {
            m_sceneInterface->m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(static_cast<eTutorial_State>(10), this);
        }
    }
    const int containerSlots = openedSize == 0x15 ? 27 : (openedSize / 9) * 9;
    m_sceneInterface->Initialize(false, args->player, menu, true, containerSlots,
                                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0),
                                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(3), false);
    m_containerList.addSlots(0, containerSlots);

    delete args;
}

UIScene_ContainerMenu::~UIScene_ContainerMenu()
{
}

unsigned int UIScene_ContainerMenu::getSectionColumns(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    return section <= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2) ? 9U : 0U;
}

unsigned int UIScene_ContainerMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0))
        return 1;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1))
        return GetInventoryRows();
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2))
        return static_cast<unsigned int>((GetBaseSlotCount() - 36) / 9);
    return 0;
}

unsigned int UIScene_ContainerMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection result = section;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0))
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            updateSlotPosition(section, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2),
                               state, x, y, 0, 0);
            return 2;
        }
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            result = m_inventorySmall ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1);
            updateSlotPosition(section, result, state, x, y, 0, 0);
            return result;
        }
    }
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1))
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            updateSlotPosition(section, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0),
                               state, x, y, 0, 0);
            return 0;
        }
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            updateSlotPosition(section, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2),
                               state, x, y, 0, 0);
            return 2;
        }
    }
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2))
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            result = m_inventorySmall ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1);
            updateSlotPosition(section, result, state, x, y, 0, 0);
            return result;
        }
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            updateSlotPosition(section, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0),
                               state, x, y, 0, 0);
            return 0;
        }
    }
    updateSlotPosition(section, section, state, x, y, 0, 0);
    return section;
}

void UIScene_ContainerMenu::GetPositionOfSection(
    IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    UIControl* control = 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0))
        control = &m_hotbarList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1))
        control = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2))
        control = &m_containerList;
    if (control != 0 && position != 0)
    {
        position->x = static_cast<float>(control->getXPos());
        position->y = static_cast<float>(control->getYPos());
    }
}

void UIScene_ContainerMenu::GetItemScreenData(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
    _UIVec2D* position, _UIVec2D* size)
{
    UIControl* control = 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0))
        control = &m_hotbarList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1))
        control = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2))
        control = &m_containerList;
    if (control != 0 && position != 0 && size != 0)
    {
        const unsigned int columns = getSectionColumns(section);
        const unsigned int rows = getSectionRows(section);
        position->x = static_cast<float>(slot % static_cast<int>(columns));
        position->y = static_cast<float>(slot / static_cast<int>(columns));
        size->x = static_cast<float>(control->getWidth() / columns);
        size->y = static_cast<float>(control->getHeight() / rows);
    }
}

void UIScene_ContainerMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    UIControl_SlotList* list = 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0))
        list = &m_hotbarList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1))
        list = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2))
        list = &m_containerList;
    if (wrap && list != 0)
        list->setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
}

int UIScene_ContainerMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0))
        return GetBaseSlotCount() - 9;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1))
        return GetBaseSlotCount() - 36;
    return 0;
}

void UIScene_ContainerMenu::handleSectionClick(
    IUIScene_AbstractContainerMenu::ESceneSection)
{
}

unsigned int UIScene_ContainerMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_containerList.setupControl(this, panel, std::string("containerList"));
    m_controls.push_back(&m_containerList);
    m_chestLabel.setupControl(this, panel, std::string("chestLabel"));
    m_controls.push_back(&m_chestLabel);
    (void)root;
    return 1;
}

std::wstring UIScene_ContainerMenu::getMoviePath()
{
    if (m_largeChest)
        return g_pApp->GetLocalPlayerCount() < 2 ?
            std::wstring(L"ChestLargeMenu") : std::wstring(L"ChestLargeMenuSplit");
    return g_pApp->GetLocalPlayerCount() < 2 ?
        std::wstring(L"ChestMenu") : std::wstring(L"ChestMenuSplit");
}

void UIScene_ContainerMenu::handleReload()
{
    const int count = GetBaseSlotCount();
    m_sceneInterface->Initialize(true, m_playerId, m_menu, true, count - 36,
                                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0),
                                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(3), false);
    m_containerList.addSlots(0, count - 36);
}

UIControl* UIScene_ContainerMenu::getSection(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0))
        return &m_hotbarList;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1))
        return &m_inventoryList;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2))
        return &m_containerList;
    return 0;
}

int UIScene_ContainerMenu::getSceneType()
{
    return 9;
}
