#include "game/ui/UIScene_AnvilMenu.hpp"

#include <cstring>

#include "game/client/render/Font.hpp"
#include "game/core/CensoredString.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/AnvilMenu.hpp"
#include "game/ui/FJ_AnvilMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CInput.hpp"

extern CInput* g_pInput;
extern CMinecraftApp* g_pApp;
extern ConsoleUIController gConsoleUIController;

UIScene_AnvilMenu::UIScene_AnvilMenu(int type, void*, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_sceneInterface(0), m_anvilMenu(0),
      m_itemName(0), m_pendingName(0), m_editingName(false), m_crossVisible(false)
{
}

UIScene_AnvilMenu::~UIScene_AnvilMenu()
{
    delete m_pendingName;
}

bool UIScene_AnvilMenu::IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    return section > static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x30) &&
           section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x36);
}

unsigned int UIScene_AnvilMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    static const unsigned char columns[] = { 9, 9, 1, 1, 1 };
    const int value = static_cast<int>(section);
    if (value < 0x31 || value > 0x35)
        return 0;
    return columns[value - 0x31];
}

unsigned int UIScene_AnvilMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    const int value = static_cast<int>(section);
    if (value < 0x31 || value > 0x35)
        return 0;
    if (value == 0x32)
        return m_sceneInterface->GetInventoryRows();
    return 1;
}

unsigned int UIScene_AnvilMenu::GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                              IUIScene_AbstractContainerMenu::ETapState state,
                                                              int* x, int* y)
{
    const int value = static_cast<int>(section);
    const int direction = static_cast<int>(state);
    int target = value;
    int horizontalOffset = 0;

    if (value == 0x31)
    {
        if (direction == 1)
        {
            if (m_sceneInterface->m_inventorySmall == 0)
                target = 0x32;
            else if (*x < 4)
                target = 0x33;
            else if (*x < 5)
                target = 0x34;
            else
                target = 0x35;
        }
        else if (direction == 2)
        {
            target = *x < 4 ? 0x33 : 0x36;
        }
    }
    else if (value == 0x32)
    {
        if (direction == 1)
        {
            if (*x < 4)
                target = 0x33;
            else if (*x < 5)
                target = 0x34;
            else
                target = 0x35;
        }
        else if (direction == 2)
        {
            target = 0x31;
        }
    }
    else if (value == 0x33)
    {
        if (direction == 1)
            target = 0x36;
        else if (direction == 2)
        {
            target = m_sceneInterface->m_inventorySmall != 0 ? 0x31 : 0x32;
            horizontalOffset = 3;
        }
        else if (direction == 3)
            target = 0x35;
        else if (direction == 4)
            target = 0x34;
    }
    else if (value == 0x34)
    {
        if (direction == 1)
            target = 0x36;
        else if (direction == 2)
        {
            target = m_sceneInterface->m_inventorySmall != 0 ? 0x31 : 0x32;
            horizontalOffset = -4;
        }
        else if (direction == 3)
            target = 0x33;
        else if (direction == 4)
            target = 0x35;
    }
    else if (value == 0x35)
    {
        if (direction == 1)
            target = 0x36;
        else if (direction == 2)
        {
            target = m_sceneInterface->m_inventorySmall != 0 ? 0x31 : 0x32;
            horizontalOffset = -5;
        }
        else if (direction == 3)
            target = 0x34;
        else if (direction == 4)
            target = 0x33;
    }
    else if (value == 0x36)
    {
        if (direction == 1)
        {
            target = 0x31;
            horizontalOffset = -4;
        }
        else if (direction == 2)
        {
            target = 0x34;
        }
    }

    m_sceneInterface->updateSlotPosition(section,
        static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(target), state, x, y,
        horizontalOffset, 0);
    return static_cast<unsigned int>(target);
}

void UIScene_AnvilMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                              _UIVec2D* position)
{
    UIControl* control = static_cast<UIControl*>(0);
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x31))
        control = &m_ingredientSlot;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x32))
        control = &m_ingredientSlot2;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x33))
        control = &m_ingredientSlot;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x34))
        control = &m_ingredientSlot2;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x35))
        control = &m_resultSlot;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x36))
        control = &m_editName;
    if (control != 0 && position != 0)
    {
        position->x = static_cast<float>(control->getXPos());
        position->y = static_cast<float>(control->getYPos());
    }
}

void UIScene_AnvilMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                           _UIVec2D* position, _UIVec2D* size)
{
    UIControl* control = static_cast<UIControl*>(0);
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x31))
        control = &m_ingredientSlot;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x32))
        control = &m_ingredientSlot2;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x33))
        control = &m_ingredientSlot;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x34))
        control = &m_ingredientSlot2;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x35))
        control = &m_resultSlot;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x36))
        control = &m_editName;
    if (control != 0 && position != 0 && size != 0)
    {
        const unsigned int columns = getSectionColumns(section);
        const unsigned int rows = getSectionRows(section);
        position->x = static_cast<float>(slot % (columns == 0 ? 1 : columns));
        position->y = static_cast<float>(slot / (columns == 0 ? 1 : columns));
        size->x = static_cast<float>(control->getWidth() / (columns == 0 ? 1 : columns));
        size->y = static_cast<float>(control->getHeight() / (rows == 0 ? 1 : rows));
    }
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

    scene->m_editingName = false;
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
    m_editingName = true;
    AnvilMenu* menu = m_anvilMenu != 0 ? m_anvilMenu : static_cast<AnvilMenu*>(m_menu);
    not_null_ptr<ItemInstance> item = menu->getSlot(0)->getItem();
    if (item->getItem() != Items::air && item->getItem() != Items::paper)
    {
        g_pInput->RequestKeyboard(L10N::GetString(0xbdc6c1eb), m_editName.getLabel(), m_playerId, 0x1e,
                                  KeyboardCompleteCallback,
                                  reinterpret_cast<void*>(GetCallbackUniqueId()),
                                  C_4JInput::EKeyboardMode_0);
        m_editingName = false;
        return;
    }
    std::wstring label(m_editName.getLabel());
    std::wstring sanitized = Minecraft::GetInstance()->mFont->sanitize(label, true);
    g_pInput->RequestKeyboard(L10N::GetString(0xbdc6c1eb), sanitized.c_str(), m_playerId, 0x1e,
                              KeyboardCompleteCallback,
                              reinterpret_cast<void*>(GetCallbackUniqueId()),
                              static_cast<C_4JInput::EKeyboardMode>(3));
    m_editingName = false;
}

void UIScene_AnvilMenu::handleOtherClicked(int, IUIScene_AbstractContainerMenu::ESceneSection section,
                                            int, bool)
{
    if (section != 0x36)
        return;
    handleEditNamePressed();
}

int UIScene_AnvilMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x31)) return 0x1e;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x32)) return 3;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x34)) return 1;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x35)) return 2;
    return 0;
}

void UIScene_AnvilMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                int slot, int direction, bool wrap)
{
    const int columns = static_cast<int>(getSectionColumns(section));
    const int value = static_cast<int>(section);
    UIControl_SlotList* list = static_cast<UIControl_SlotList*>(0);
    if (value == 0x31)
        list = &m_hotbarList;
    else if (value == 0x32)
        list = &m_inventoryList;
    else if (value == 0x33)
        list = &m_ingredientSlot;
    else if (value == 0x34)
        list = &m_ingredientSlot2;
    else if (value == 0x35)
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
        m_sceneInterface->Initialize(true, m_containerId, 0,
                                     true, 3, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x31),
                                     static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x37), false);
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
    updateItemName();
    showCross(false);
}

void UIScene_AnvilMenu::tick()
{
    UIScene_AbstractContainerMenu::tick();
    handleTick();
}

UIControl* UIScene_AnvilMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x31)) return &m_hotbarList;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x32)) return &m_inventoryList;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x33)) return &m_ingredientSlot;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x34)) return &m_ingredientSlot2;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x35)) return &m_resultSlot;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x36)) return &m_editName;
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
    if (m_pendingName != 0)
        setEditNameValue(m_pendingName->name);
}

void UIScene_AnvilMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_AnvilMenu::getSceneType()
{
    return 51;
}
