#include "game/ui/UIScene_AnvilMenu.hpp"

#include "game/core/CensoredString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/AnvilMenu.hpp"
#include "game/ui/FJ_AnvilMenu.hpp"
#include "game/ui/fuiFile.hpp"

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
    return IsSectionSlotList(section) ? 1U : 0U;
}

unsigned int UIScene_AnvilMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x32))
        return UIScene_AbstractContainerMenu::GetInventoryRows();
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x31) ||
        IsSectionSlotList(section))
        return 1U;
    return 0U;
}

unsigned int UIScene_AnvilMenu::GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                              IUIScene_AbstractContainerMenu::ETapState state,
                                                              int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection result = section;
    if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        result = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(section - 1);
    else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        result = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(section + 1);
    if (m_sceneInterface != 0)
        m_sceneInterface->updateSlotPosition(section, result, state, x, y, 0, 0);
    return result;
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
    UIScene_AnvilMenu* scene = static_cast<UIScene_AnvilMenu*>(data);
    if (scene != 0 && accepted)
        scene->updateItemName();
    return 0;
}

void UIScene_AnvilMenu::handleEditNamePressed()
{
    m_editingName = true;
    setEditNameEditable(true);
}

void UIScene_AnvilMenu::handleOtherClicked(int, IUIScene_AbstractContainerMenu::ESceneSection section,
                                            int, bool pressed)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x36) && pressed)
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
    int columns = getSectionColumns(section);
    if (wrap && IsSectionSlotList(section))
        m_resultSlot.setHighlightSlot(slot + direction * columns);
}

void UIScene_AnvilMenu::refreshContainer(AbstractContainerMenu*,
                                           std::vector<not_null_ptr<ItemInstance> >*)
{
    updateItemName();
}

void UIScene_AnvilMenu::slotChanged(AbstractContainerMenu*, int, not_null_ptr<ItemInstance>)
{
    updateItemName();
}

void UIScene_AnvilMenu::setContainerData(AbstractContainerMenu*, int, int)
{
}

void UIScene_AnvilMenu::setAllContainerData(AbstractContainerMenu*, boost::shared_ptr<Container>)
{
}

void UIScene_AnvilMenu::setCostLabel(const std::wstring& label, bool safe)
{
    if (m_movie != 0 && m_movie->getRootNode() != 0)
        FJ_AnvilMenu::Create(m_movie->getRootNode())->SetCostLabel(label, safe);
}

unsigned int UIScene_AnvilMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    return 1;
}

std::wstring UIScene_AnvilMenu::getMoviePath()
{
    return L"AnvilMenu";
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
        m_crossVisible = show;
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
    UIScene_AbstractContainerMenu::handleDestroy();
}

void UIScene_AnvilMenu::updateItemName()
{
    if (m_pendingName != 0)
        setEditNameValue(m_pendingName->name);
}

void UIScene_AnvilMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x36))
        handleEditNamePressed();
}

int UIScene_AnvilMenu::getSceneType()
{
    return 51;
}
