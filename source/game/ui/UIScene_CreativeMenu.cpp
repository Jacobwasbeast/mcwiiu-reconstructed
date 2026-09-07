#include "game/ui/UIScene_CreativeMenu.hpp"

#include <algorithm>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/FJ_CreativeMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern ConsoleUIController gConsoleUIController;

UIScene_CreativeMenu::TabSpec::TabSpec(const wchar_t* title, int itemCount, int groupCount,
                                       ECreative_Inventory_Groups* groups, int specialCount,
                                       ECreative_Inventory_Groups* specialGroups, int extraCount,
                                       ECreative_Inventory_Groups* extraGroups)
    : m_title(title == 0 ? L"" : title), m_itemCount(itemCount), m_groupCount(groupCount),
      m_specialCount(specialCount), m_extraCount(extraCount), m_pageCount(0)
{
    if (groups != 0 && groupCount > 0)
        m_groups.assign(groups, groups + groupCount);
    if (specialGroups != 0 && specialCount > 0)
        m_specialGroups.assign(specialGroups, specialGroups + specialCount);
    if (extraGroups != 0 && extraCount > 0)
        m_extraGroups.assign(extraGroups, extraGroups + extraCount);
    const int total = static_cast<int>(m_groups.size() + m_specialGroups.size() + m_extraGroups.size());
    m_pageCount = total > 0 ? (total + 49) / 50 : 0;
}

unsigned int UIScene_CreativeMenu::TabSpec::getPageCount()
{
    return static_cast<unsigned int>(m_pageCount);
}

void UIScene_CreativeMenu::TabSpec::populateMenu(AbstractContainerMenu* menu, int page, unsigned int offset)
{
    if (menu == 0)
        return;
    const unsigned int first = offset + static_cast<unsigned int>(std::max(page, 0) * 50);
    const unsigned int last = first + 50;
    for (unsigned int slot = first; slot < last; ++slot)
        menu->setItem(slot, ItemInstance::EMPTY);
}

UIScene_CreativeMenu::ItemPickerMenu::ItemPickerMenu(boost::shared_ptr<SimpleContainer> container,
                                                     boost::shared_ptr<Inventory> inventory)
    : m_container(container), m_inventory(inventory), m_selectedSlot(0), m_resultSlot(0)
{
}

UIScene_CreativeMenu::ItemPickerMenu::~ItemPickerMenu()
{
}

bool UIScene_CreativeMenu::ItemPickerMenu::stillValid(boost::shared_ptr<Player>)
{
    return true;
}

bool UIScene_CreativeMenu::ItemPickerMenu::isOverrideResultClick(int slot, int button)
{
    return slot < 9 && button == 0;
}

bool UIScene_CreativeMenu::ItemPickerMenu::doQuickMoveLoop()
{
    return false;
}

UIScene_CreativeMenu::UIScene_CreativeMenu(int type, void*, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_currentTab(8), m_containerMenu(0),
      m_sceneInterface(this)
{
    std::fill(m_tabPages, m_tabPages + 8, 0);
    std::fill(m_tabCurrentPages, m_tabCurrentPages + 8, 0);
}

UIScene_CreativeMenu::~UIScene_CreativeMenu()
{
}

bool UIScene_CreativeMenu::IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    return section > static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x14) &&
           section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x17);
}

unsigned int UIScene_CreativeMenu::CanHaveFocus(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    return IsSectionSlotList(section) ? 1U : 0U;
}

unsigned int UIScene_CreativeMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15)) return 9;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x16)) return 10;
    return 0;
}

unsigned int UIScene_CreativeMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15)) return 1;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x16)) return 5;
    return 0;
}

unsigned int UIScene_CreativeMenu::GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                                 IUIScene_AbstractContainerMenu::ETapState state,
                                                                 int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection next = section;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15) &&
        (state == IUIScene_AbstractContainerMenu::ETapState_Down || state == IUIScene_AbstractContainerMenu::ETapState_Up))
        next = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x16);
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x16) &&
             (state == IUIScene_AbstractContainerMenu::ETapState_Down || state == IUIScene_AbstractContainerMenu::ETapState_Up))
        next = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15);
    m_sceneInterface->updateSlotPosition(section, next, state, x, y, 0, 0);
    return static_cast<unsigned int>(next);
}

void UIScene_CreativeMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    if (position == 0) return;
    UIControl* control = getSection(section);
    position->x = control == 0 ? 0.0f : control->getXPos();
    position->y = control == 0 ? 0.0f : control->getYPos();
}

void UIScene_CreativeMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                              _UIVec2D* position, _UIVec2D* size)
{
    if (position == 0 || size == 0) return;
    UIControl* control = getSection(section);
    if (control == 0) { position->x = position->y = size->x = size->y = 0.0f; return; }
    size->x = control->getWidth() / static_cast<float>(std::max(getSectionColumns(section), 1U));
    size->y = control->getHeight() / static_cast<float>(std::max(getSectionRows(section), 1U));
    position->x = static_cast<float>(slot % static_cast<int>(std::max(getSectionColumns(section), 1U))) * size->x;
    position->y = static_cast<float>(slot / static_cast<int>(std::max(getSectionColumns(section), 1U))) * size->y;
}

unsigned int UIScene_CreativeMenu::handleValidKeyPress(int key, int value, int)
{
    handleAdditionalKeyPress(key);
    return value == 0 ? 0U : 1U;
}

void UIScene_CreativeMenu::handleOutsideClicked(int, int, int)
{
}

void UIScene_CreativeMenu::updateTabHighlightAndText(ECreativeInventoryTabs tab)
{
    m_currentTab = static_cast<int>(tab);
}

void UIScene_CreativeMenu::updateScrollCurrentPage(int page, int pageCount)
{
    if (m_currentTab >= 0 && m_currentTab < 8)
        m_tabPages[m_currentTab] = pageCount;
}

void UIScene_CreativeMenu::switchTab(ECreativeInventoryTabs tab)
{
    m_currentTab = static_cast<int>(tab);
    updateScrollCurrentPage(m_tabCurrentPages[m_currentTab], m_tabPages[m_currentTab]);
}

void UIScene_CreativeMenu::ScrollBar(_UIVec2D)
{
}

void UIScene_CreativeMenu::handleOtherClicked(int button, IUIScene_AbstractContainerMenu::ESceneSection section, int, bool)
{
    if (section >= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x17) &&
        section <= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x1e))
        switchTab(static_cast<ECreativeInventoryTabs>(section - 0x17));
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x1f))
        ScrollBar(_UIVec2D());
    (void)button;
}

void UIScene_CreativeMenu::handleAdditionalKeyPress(int key)
{
    if (m_currentTab < 0 || m_currentTab >= 8) return;
    if (key == 8 || key == 10) ++m_tabCurrentPages[m_currentTab];
    if (key == 14 && m_tabCurrentPages[m_currentTab] > 0) --m_tabCurrentPages[m_currentTab];
    if (key == 15 && m_tabCurrentPages[m_currentTab] < m_tabPages[m_currentTab]) ++m_tabCurrentPages[m_currentTab];
    switchTab(static_cast<ECreativeInventoryTabs>(m_currentTab));
}

int UIScene_CreativeMenu::getEmptyInventorySlot(not_null_ptr<ItemInstance>, int& slot)
{
    slot = -1;
    return 0;
}

void UIScene_CreativeMenu::handleSlotListClicked(IUIScene_AbstractContainerMenu::ESceneSection, int, int)
{
}

int UIScene_CreativeMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15)) return 50;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x16)) return 0;
    return 0;
}

void UIScene_CreativeMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    if (wrap && (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15) ||
                 section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x16)))
        m_containerList.setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
}

unsigned int UIScene_CreativeMenu::overrideTooltips(IUIScene_AbstractContainerMenu::ESceneSection,
                                                     not_null_ptr<ItemInstance>, bool, bool, bool, int,
                                                     IUIScene_AbstractContainerMenu::EToolTipItem& first,
                                                     IUIScene_AbstractContainerMenu::EToolTipItem& second,
                                                     IUIScene_AbstractContainerMenu::EToolTipItem& third,
                                                     IUIScene_AbstractContainerMenu::EToolTipItem& fourth,
                                                     IUIScene_AbstractContainerMenu::EToolTipItem& fifth,
                                                     IUIScene_AbstractContainerMenu::EToolTipItem& sixth,
                                                     IUIScene_AbstractContainerMenu::EToolTipItem& seventh)
{
    first = second = third = fourth = fifth = sixth = seventh = IUIScene_AbstractContainerMenu::EToolTipItem_None;
    return 0;
}

void UIScene_CreativeMenu::staticCtor()
{
}

unsigned int UIScene_CreativeMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    return 1;
}

std::wstring UIScene_CreativeMenu::getMoviePath()
{
    return std::wstring(g_pApp->GetLocalPlayerCount() < 2 ? L"CreativeMenu" : L"CreativeMenuSplit");
}

void UIScene_CreativeMenu::handleReload()
{
    if (m_sceneInterface != 0)
        m_sceneInterface->Initialize(true, m_containerId, m_containerMenu, false, -1,
                                     static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15),
                                     static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x20), false);
}

UIControl* UIScene_CreativeMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15)) return &m_hotbarList;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x16)) return &m_containerList;
    return 0;
}

void UIScene_CreativeMenu::handleInput(int playerId, int action, bool repeat, bool released, bool pressed, bool& handled)
{
    UIScene_AbstractContainerMenu::handleInput(playerId, action, repeat, released, pressed, handled);
}

void UIScene_CreativeMenu::handleTouchInput(unsigned int playerId, int action, int x, int y, bool repeat, bool released, bool pressed)
{
    UIScene::handleTouchInput(playerId, action, x, y, repeat, released, pressed);
}

unsigned int UIScene_CreativeMenu::GetMainPanel()
{
    return 0;
}

void UIScene_CreativeMenu::handleTouchBoxRebuild()
{
    addTimer(0, 100);
}

void UIScene_CreativeMenu::handleTimerComplete(int timerId)
{
    if (timerId == 0) { gConsoleUIController.TouchBoxRebuild(this); killTimer(0); }
}

void UIScene_CreativeMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_CreativeMenu::getSceneType()
{
    return 0x2a;
}
