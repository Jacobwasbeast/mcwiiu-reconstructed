#include "game/ui/UIScene_AnvilMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cstring>

#include "game/client/render/Font.hpp"
#include "game/core/CensoredString.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerboundCustomPayloadPacket.hpp"
#include "game/item/Item.hpp"
#include "game/core/Slot.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Items.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/AnvilMenu.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/FJ_AnvilMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "platform/wiiu/CInput.hpp"

#include <wchar.h>

extern CInput* g_pInput;
extern CMinecraftApp* g_pApp;
extern ConsoleUIController gConsoleUIController;

struct RepairingMenuInitData
{
    boost::shared_ptr<Inventory> inventory;
    Level* level;
    int x;
    int y;
    int z;
    int player;
    bool splitscreen;
};

UIScene_AnvilMenu::UIScene_AnvilMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_sceneInterface(0), m_anvilMenu(0),
      m_nameUpdateAction(this, &UIScene_AnvilMenu::updateItemName), m_itemName(0),
      m_pendingName(0),
      m_crossVisible(false)
{
    RepairingMenuInitData* sceneData = static_cast<RepairingMenuInitData*>(data);
    m_sceneInterface = static_cast<IUIScene_AbstractContainerMenu*>(
        static_cast<UIScene_AbstractContainerMenu*>(this));
    m_itemName = new CensoredString(&m_nameUpdateAction, 0xad36b773, 0x62950b66);
    initialiseMovie();
    m_editName.init(UIString(m_itemName->get()), 0);
    m_editName.SetCharLimit(0x1e);
    m_anvilLabel.init(UIString(L10N::GetString(0xc60bb915)));

    Minecraft* minecraft = Minecraft::GetInstance();
    MultiPlayerGameMode* gameMode = minecraft->mGameModes[type];
    if (gameMode != 0)
    {
        Tutorial* tutorial = gameMode->getTutorial();
        if (tutorial != 0)
        {
            m_sceneInterface->m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_AnvilMenu, this);
        }
    }

    m_anvilMenu = new AnvilMenu(sceneData->inventory, sceneData->level,
                                BlockPos(sceneData->x, sceneData->y, sceneData->z),
                                minecraft->mLocalPlayers[type]);
    m_anvilMenu->addSlotListener(this);

    m_sceneInterface->Initialize(false, type, m_anvilMenu, true, 3,
                                 IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar,
                                 IUIScene_AbstractContainerMenu::ESceneSection_AnvilMax, false);
    m_ingredientSlot.addSlots(0, 1);
    m_ingredientSlot2.addSlots(1, 1);
    m_resultSlot.addSlots(2, 1);
    std::wstring costLabel;
    bool costLabelSafe = false;
    if (m_anvilMenu->mRepairCost > 0)
    {
        boost::shared_ptr<LocalPlayer> player = minecraft->mLocalPlayers[type];
        Slot* resultSlot = m_anvilMenu->getSlot(2);
        if (m_anvilMenu->mRepairCost >= 40 && !resultSlot->mayPickup(player))
        {
            costLabel = L10N::GetString(0xdb7b95e9);
            costLabelSafe = true;
        }
        else if (resultSlot->hasItem())
        {
            wchar_t label[256];
            swprintf(label, 0x100, L10N::GetString(0xf3a668fd));
            costLabel = label;
            costLabelSafe = false;
        }
    }
    setCostLabel(costLabel, costLabelSafe);

    delete sceneData;
    g_pApp->SetRichPresenceContext(type, 0xc);
}

UIScene_AnvilMenu::~UIScene_AnvilMenu()
{
    delete m_itemName;
    delete m_pendingName;
}

bool UIScene_AnvilMenu::IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    return section > IUIScene_AbstractContainerMenu::ESceneSection_BrewingMax &&
           section < IUIScene_AbstractContainerMenu::ESceneSection_AnvilName;
}

unsigned int UIScene_AnvilMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    static const unsigned char columns[] = { 9, 9, 1, 1, 1 };
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar || section > IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult)
        return 0;
    return columns[section - IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar];
}

unsigned int UIScene_AnvilMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar || section > IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult)
        return 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory)
        return m_sceneInterface->GetInventoryRows();
    return 1;
}

unsigned int UIScene_AnvilMenu::GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                              IUIScene_AbstractContainerMenu::ETapState state,
                                                              int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection target = section;
    int horizontalOffset = 0;

    if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (m_sceneInterface->m_smallInventory == 0)
                target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory;
            else if (*x < 4)
                target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1;
            else if (*x < 5)
                target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2;
            else
                target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = *x < 4 ? IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1 : IUIScene_AbstractContainerMenu::ESceneSection_AnvilName;
        }
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (*x < 4)
                target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1;
            else if (*x < 5)
                target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2;
            else
                target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar;
        }
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilName;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_sceneInterface->m_smallInventory != 0 ? IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar : IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory;
            horizontalOffset = 3;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2;
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilName;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_sceneInterface->m_smallInventory != 0 ? IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar : IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory;
            horizontalOffset = -4;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult;
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilName;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_sceneInterface->m_smallInventory != 0 ? IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar : IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory;
            horizontalOffset = -5;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1;
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilName)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar;
            horizontalOffset = -4;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2;
        }
    }

    m_sceneInterface->updateSlotPosition(section,
        target, state, x, y,
        horizontalOffset, 0);
    return target;
}

void UIScene_AnvilMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                              _UIVec2D* position)
{
    UIControl* control = NULL;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar:
        control = &m_hotbarList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory:
        control = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1:
        control = &m_ingredientSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2:
        control = &m_ingredientSlot2;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult:
        control = &m_resultSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilName:
        control = &m_editName;
        break;
    default:
        break;
    }
    if (control == NULL)
        return;
    position->x = static_cast<float>(control->getXPos());
    position->y = static_cast<float>(control->getYPos());
}

void UIScene_AnvilMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                           _UIVec2D* position, _UIVec2D* size)
{
    UIControl* control = NULL;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar:
        control = &m_hotbarList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory:
        control = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1:
        control = &m_ingredientSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2:
        control = &m_ingredientSlot2;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult:
        control = &m_resultSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilName:
        control = &m_editName;
        break;
    default:
        break;
    }
    if (control == NULL)
        return;
    double width = control->getWidth();
    double height = control->getHeight();
    if (!IsSectionSlotList(section))
    {
        GetPositionOfSection(section, position);
        size->x = static_cast<float>(width);
        size->y = static_cast<float>(height);
        return;
    }
    int rows = static_cast<int>(getSectionRows(section));
    int columns = static_cast<int>(getSectionColumns(section));
    size->y = static_cast<float>(height / static_cast<float>(rows));
    size->x = static_cast<float>(width / static_cast<float>(columns));
    position->x = static_cast<float>(slot % columns) * size->x;
    position->y = static_cast<float>(slot / columns) * size->y;
}

void UIScene_AnvilMenu::addPendingNameChange(const PendingNameInfo& info)
{
    delete m_pendingName;
    m_pendingName = new PendingNameInfo(info);
}

int UIScene_AnvilMenu::KeyboardCompleteCallback(void* data, bool accepted)
{
    unsigned int callbackId = static_cast<unsigned int>(reinterpret_cast<unsigned long>(data));
    UIScene_AnvilMenu* scene =
        static_cast<UIScene_AnvilMenu*>(gConsoleUIController.GetSceneFromCallbackId(callbackId));
    if (scene == 0)
        return 0;

    scene->m_ignoreInput = false;
    if (!accepted)
        return 0;

    PendingNameInfo info;
    info.itemName = Minecraft::GetInstance()->mLocalPlayers[scene->m_playerId]->getUUID();

    wchar_t buffer[128];
    std::memset(buffer, 0, sizeof(buffer));
    g_pInput->GetText(buffer, 0x7f);
    info.name.assign(buffer, wcslen(buffer));

    scene->addPendingNameChange(info);
    return 0;
}

void UIScene_AnvilMenu::handleEditNamePressed()
{
    m_ignoreInput = true;
    AnvilMenu* menu = m_anvilMenu != 0 ? m_anvilMenu : static_cast<AnvilMenu*>(m_menu);
    not_null_ptr<ItemInstance> item = menu->getSlot(0)->getItem();
    if (item->getItem() != Items::nameTag && item->getItem() != Items::spawnEgg)
    {
        g_pInput->RequestKeyboard(L10N::GetString(IDS_TITLE_RENAME), m_editName.getLabel(), m_playerId, 0x1e,
                                  KeyboardCompleteCallback,
                                  reinterpret_cast<void*>(GetCallbackUniqueId()),
                                  C_4JInput::EKeyboardMode_0);
        m_ignoreInput = false;
        return;
    }
    std::wstring label(m_editName.getLabel());
    std::wstring sanitized = Minecraft::GetInstance()->mFont->sanitize(label, true);
    g_pInput->RequestKeyboard(L10N::GetString(IDS_TITLE_RENAME), sanitized.c_str(), m_playerId, 0x1e,
                              KeyboardCompleteCallback,
                              reinterpret_cast<void*>(GetCallbackUniqueId()),
                              static_cast<C_4JInput::EKeyboardMode>(3));
    m_ignoreInput = false;
}

void UIScene_AnvilMenu::handleOtherClicked(int, IUIScene_AbstractContainerMenu::ESceneSection section,
                                            int, bool)
{
    if (section != IUIScene_AbstractContainerMenu::ESceneSection_AnvilName)
        return;
    handleEditNamePressed();
}

int UIScene_AnvilMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar) return 0x1e;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory) return 3;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2) return 1;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult) return 2;
    return 0;
}

void UIScene_AnvilMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                int slot, int direction, bool wrap)
{
    const int columns = static_cast<int>(getSectionColumns(section));
    UIControl_SlotList* list = static_cast<UIControl_SlotList*>(0);
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar)
        list = &m_hotbarList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory)
        list = &m_inventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1)
        list = &m_ingredientSlot;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2)
        list = &m_ingredientSlot2;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult)
        list = &m_resultSlot;
    if (wrap)
        list->setHighlightSlot(slot + direction * columns);
}

void UIScene_AnvilMenu::refreshContainer(AbstractContainerMenu* container,
                                           std::vector<not_null_ptr<ItemInstance> >*)
{
    slotChanged(container, 0, container->getSlot(0)->getItem());
}

void UIScene_AnvilMenu::slotChanged(AbstractContainerMenu*, int slot, not_null_ptr<ItemInstance> item)
{
    if (slot != 0)
        return;

    if (!item->isEmpty())
    {
        m_itemName->set(item->getHoverNameUnfiltered(), item->getOwnerUUID(),
                         item->wasCreatedByRestrictedPlayer(), item->wasCreatedOnHost());
    }
    else
    {
        m_itemName->set(std::wstring(), std::wstring(), false, true);
    }
}

void UIScene_AnvilMenu::setContainerData(AbstractContainerMenu*, int, int)
{
}

void UIScene_AnvilMenu::setAllContainerData(AbstractContainerMenu*, boost::shared_ptr<Container>)
{
}

void UIScene_AnvilMenu::setCostLabel(const std::wstring& label, bool safe)
{
    static_cast<FJ_AnvilMenu*>(m_movie->getRootNode()->getFuiNode())->SetCostLabel(label, safe);
}

unsigned int UIScene_AnvilMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_ingredientSlot.setupControl(this, panel, "Ingredient"); m_controls.push_back(&m_ingredientSlot);
    m_ingredientSlot2.setupControl(this, panel, "Ingredient2"); m_controls.push_back(&m_ingredientSlot2);
    m_resultSlot.setupControl(this, panel, "Result"); m_controls.push_back(&m_resultSlot);
    m_anvilLabel.setupControl(this, panel, "AnvilText"); m_controls.push_back(&m_anvilLabel);
    m_editName.setupControl(this, panel, "AnvilTextInput"); m_controls.push_back(&m_editName);
    return 1;
}

std::wstring UIScene_AnvilMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"AnvilMenu") : std::wstring(L"AnvilMenuSplit");
}

void UIScene_AnvilMenu::handleReload()
{
    if (m_sceneInterface != 0)
        m_sceneInterface->Initialize(true, m_playerId, m_menu,
                                     true, 3, IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar,
                                     IUIScene_AbstractContainerMenu::ESceneSection_AnvilMax, false);
    m_ingredientSlot.addSlots(0, 1);
    m_ingredientSlot2.addSlots(1, 1);
    m_resultSlot.addSlots(2, 1);
}

void UIScene_AnvilMenu::showCross(bool show)
{
    if (m_crossVisible != show)
    {
        static_cast<FJ_AnvilMenu*>(m_movie->getRootNode()->getFuiNode())->ShowRedCross(show);
        m_crossVisible = show;
    }
}

void UIScene_AnvilMenu::handleTick()
{
    std::wstring label;
    bool showCostLabel = true;

    if (m_pendingName != 0)
    {
        bool censored = g_pApp->IsConsoleChatCensored();
        bool isHost = CGameNetworkManager::sInstance.IsHost();
        m_itemName->set(m_pendingName->name, m_pendingName->itemName, censored, isHost);
        delete m_pendingName;
        m_pendingName = 0;
    }

    if (m_anvilMenu->mRepairCost > 0 && m_anvilMenu->mRepairCost < 40)
    {
        if (m_anvilMenu->getSlot(2)->hasItem())
        {
            wchar_t buffer[256];
            swprintf(buffer, 0x100, L10N::GetString(0xf3a668fd));
            label = buffer;
        }
    }

    setCostLabel(label, showCostLabel);

    bool showCrossFlag = false;
    if (m_anvilMenu->getSlot(0)->hasItem() || m_anvilMenu->getSlot(1)->hasItem())
    {
        if (!m_anvilMenu->getSlot(2)->hasItem())
            showCrossFlag = true;
    }
    showCross(showCrossFlag);
}

void UIScene_AnvilMenu::tick()
{
    UIScene_AbstractContainerMenu::tick();
    handleTick();
}

UIControl* UIScene_AnvilMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar) return &m_hotbarList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory) return &m_inventoryList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1) return &m_ingredientSlot;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2) return &m_ingredientSlot2;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult) return &m_resultSlot;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_AnvilName) return &m_editName;
    return 0;
}

void UIScene_AnvilMenu::setEditNameValue(const std::wstring& value)
{
    m_editName.setLabel(UIString(value), false, false);
}

void UIScene_AnvilMenu::handleDestroy()
{
    g_pInput->DestroyKeyboard();
    UIScene_AbstractContainerMenu::handleDestroy();
}

void UIScene_AnvilMenu::updateItemName()
{
    Slot* slot = m_anvilMenu->getSlot(0);
    const bool censored = g_pApp->IsConsoleChatCensored() != 0;
    std::wstring name = m_itemName->get();
    not_null_ptr<ItemInstance> item = slot->getItem();
    if (m_itemName->checkParentalFilter() == 0 && item.get() != NULL)
        name = item->getItem()->getName(item);
    setEditNameValue(name);
    m_anvilMenu->setItemName(m_itemName->raw(), censored);
    if (!slot->hasItem())
        return;

    ByteArrayOutputStream bytes;
    DataOutputStream output(&bytes);
    output.writeUTF(m_itemName->raw());
    const std::wstring channel = censored ? L"MC|RIName" : L"MC|ItemName";
    Minecraft::GetInstance()->getConnection(getPad())->send(
        boost::shared_ptr<Packet>(new ServerboundCustomPayloadPacket(channel, bytes.toByteArray())));
}

void UIScene_AnvilMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_AnvilMenu::getSceneType()
{
    return EUIScene_AnvilMenu;
}
