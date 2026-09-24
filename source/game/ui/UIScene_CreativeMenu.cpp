#include "game/ui/UIScene_CreativeMenu.hpp"

#include <algorithm>
#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Items.hpp"
#include "game/item/Potions.hpp"
#include "game/item/SimpleContainer.hpp"
#include "game/core/Slot.hpp"
#include "game/util/Math.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/FJ_CreativeMenu.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/MenuBuilder.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Block.hpp"

extern ConsoleUIController gConsoleUIController;

namespace
{
struct CreativeMenuSceneData
{
    boost::shared_ptr<LocalPlayer> localPlayer;
    bool creative;
    int playerIndex;
    bool splitscreen;
};
}

UIScene_CreativeMenu::TabSpec* UIScene_CreativeMenu::s_tabSpecs[8] = {};
std::vector<const UIScene_CreativeMenu::ItemBuilder*> UIScene_CreativeMenu::s_groupItems[12];

UIScene_CreativeMenu::ItemBuilder::ItemBuilder(const ItemInstance& value)
    : item(value)
{
}

UIScene_CreativeMenu::ItemBuilder::~ItemBuilder()
{
}


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
    int ordinaryTotal = 0;
    for (std::vector<ECreative_Inventory_Groups>::const_iterator it = m_groups.begin();
         it != m_groups.end(); ++it)
        ordinaryTotal += static_cast<int>(UIScene_CreativeMenu::s_groupItems[*it].size());
    int specialTotal = 0;
    for (std::vector<ECreative_Inventory_Groups>::const_iterator it = m_specialGroups.begin();
         it != m_specialGroups.end(); ++it)
        specialTotal += static_cast<int>(UIScene_CreativeMenu::s_groupItems[*it].size());
    m_pageCount = static_cast<int>(
        std::ceil(static_cast<float>(ordinaryTotal) / static_cast<float>(50 - specialTotal)));
}

unsigned int UIScene_CreativeMenu::TabSpec::getPageCount()
{
    return static_cast<unsigned int>(m_pageCount);
}

unsigned int UIScene_CreativeMenu::TabSpec::getTitleStringId()
{
    return static_cast<unsigned int>(m_itemCount);
}

unsigned int UIScene_CreativeMenu::TabSpec::getSpecialGroupCount()
{
    return static_cast<unsigned int>(m_specialCount);
}

UIScene_CreativeMenu::TabSpec::~TabSpec()
{
}

void UIScene_CreativeMenu::TabSpec::populateMenu(AbstractContainerMenu* menu, int page, unsigned int offset)
{
    if (menu == 0)
        return;
    unsigned int emitted = 0;
    const unsigned int pageStart = offset;

    if (page >= 0 && page < static_cast<int>(m_specialGroups.size()))
    {
        const ECreative_Inventory_Groups group = m_specialGroups[page];
        const std::vector<const ItemBuilder*>& specialItems = UIScene_CreativeMenu::s_groupItems[group];
        for (std::vector<const ItemBuilder*>::const_iterator itemIt = specialItems.begin();
             itemIt != specialItems.end(); ++itemIt)
        {
            if (emitted >= 50)
                break;
            menu->setItem(++emitted, not_null_ptr<ItemInstance>(new ItemInstance((*itemIt)->item)));
        }
    }
    int extraTotal = 0;
    for (std::vector<ECreative_Inventory_Groups>::const_iterator groupIt = m_extraGroups.begin();
         groupIt != m_extraGroups.end(); ++groupIt)
        extraTotal += static_cast<int>(UIScene_CreativeMenu::s_groupItems[*groupIt].size());
    const unsigned int ordinarySlots = static_cast<unsigned int>(std::max(1, 50 - extraTotal));

    unsigned int skip = pageStart * ordinarySlots;
    bool full = false;
    for (std::vector<ECreative_Inventory_Groups>::const_iterator groupIt = m_groups.begin();
         !full && groupIt != m_groups.end(); ++groupIt)
    {
        const std::vector<const ItemBuilder*>& items = UIScene_CreativeMenu::s_groupItems[*groupIt];
        for (std::vector<const ItemBuilder*>::const_iterator itemIt = items.begin();
             itemIt != items.end(); ++itemIt)
        {
            if (skip != 0) { --skip; continue; }
            if (emitted >= 50) { full = true; break; }
            menu->setItem(++emitted, not_null_ptr<ItemInstance>(new ItemInstance((*itemIt)->item)));
        }
    }
    while (emitted < 50)
        menu->setItem(emitted++, not_null_ptr<ItemInstance>(new ItemInstance(*ItemInstance::EMPTY)));
}

UIScene_CreativeMenu::ItemPickerMenu::ItemPickerMenu(boost::shared_ptr<SimpleContainer> container,
                                                     boost::shared_ptr<Inventory> inventory)
    : m_container(container), m_inventory(inventory), m_selectedSlot(0), m_resultSlot(0)
{
    for (int i = 0; i < 50; ++i)
        addSlot(new Slot(m_container, i, -1, -1));
    for (int i = 0; i < 9; ++i)
        addSlot(new Slot(m_inventory, i, -1, -1));
    m_containerId = -2;
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

UIScene_CreativeMenu::UIScene_CreativeMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_currentTab(8), m_containerMenu(0),
      m_sceneInterface(this)
{
    std::fill(m_tabPages, m_tabPages + 8, 0);
    std::fill(m_tabCurrentPages, m_tabCurrentPages + 8, 0);
    initialiseMovie();

    CreativeMenuSceneData* sceneData = static_cast<CreativeMenuSceneData*>(data);
    boost::shared_ptr<SimpleContainer> container(
        new SimpleContainer(0, std::wstring(L""), false, 50));
    m_containerMenu = new ItemPickerMenu(container, sceneData->localPlayer->mInventory);
    m_sceneInterface->Initialize(false, sceneData->playerIndex, m_containerMenu, true, -1,
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15),
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x20),
                                 sceneData->creative);
    m_inventoryLabel.setLabel(UIString(L""), false, false);

    for (int i = 0; i < 50; ++i)
        m_hotbarList.addSlot(i);
    for (int i = 50; i < 59; ++i)
        UIScene_AbstractContainerMenu::m_hotbarList.addSlot(i);

    if (Minecraft::GetInstance()->mGameModes[sceneData->playerIndex] != 0)
    {
        MultiPlayerGameMode* gameMode = Minecraft::GetInstance()->mGameModes[type];
        Tutorial* tutorial = gameMode->getTutorial();
        if (tutorial != 0)
        {
            m_sceneInterface->m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(static_cast<eTutorial_State>(0xe), this);
        }
    }

    delete sceneData;
    m_currentTab = 8;
    switchTab(ECreativeInventoryTabs_Building);
    for (int i = 0; i < 9; ++i)
        m_touchPanels[i].init(i);
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

    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15))
    {
        position->x = UIScene_AbstractContainerMenu::m_hotbarList.getXPos();
        position->y = UIScene_AbstractContainerMenu::m_hotbarList.getYPos();
    }
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x16))
    {
        position->x = m_hotbarList.getXPos();
        position->y = m_hotbarList.getYPos();
    }
    else if (section >= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x17) &&
             section <= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x1f))
    {
        UIControl_Touch& panel = m_touchPanels[section - 0x17];
        position->x = panel.getXPos();
        position->y = panel.getYPos();
    }
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
    if (value != 1)
        return 0;

    Minecraft* minecraft = Minecraft::GetInstance();
    for (int index = 50; index < 59; ++index)
    {
        Slot* slot = m_sceneInterface->m_menu->getSlot(index);
        not_null_ptr<ItemInstance> item = slot->getItem();
        if (!item->isEmpty())
        {
            not_null_ptr<ItemInstance> copy = item->copy();
            slot->set(ItemInstance::EMPTY);
            const int itemsSize = static_cast<int>(m_sceneInterface->m_menu->getItems()->size());
            minecraft->mGameModes[key]->handleCreativeModeItemAdd(copy, (index - itemsSize) + 45);
        }
    }
    return 1;
}

void UIScene_CreativeMenu::handleOutsideClicked(int playerId, int, int button)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    Inventory* inventory = minecraft->mLocalPlayers[playerId]->mInventory.get();
    not_null_ptr<ItemInstance> carried = inventory->getCarried();
    if (carried->isEmpty())
        return;

    if (button == 0)
    {
        minecraft->mGameModes[playerId]->handleCreativeModeItemDrop(inventory->getCarried());
        inventory->setCarried(ItemInstance::EMPTY);
    }
    if (button == 1)
    {
        not_null_ptr<ItemInstance> one = inventory->getCarried()->split(1);
        minecraft->mGameModes[playerId]->handleCreativeModeItemDrop(one);
    }
}

void UIScene_CreativeMenu::updateTabHighlightAndText(ECreativeInventoryTabs tab)
{
    fuiRenderNode* root = m_movie->getRootNode();
    static_cast<FJ_CreativeMenu*>(root->getFuiNode()->asFJ_Document())->SetActiveTab(static_cast<int>(tab));
}

void UIScene_CreativeMenu::updateScrollCurrentPage(int page, int pageCount)
{
    if (m_currentTab >= 0 && m_currentTab < 8)
        m_tabPages[m_currentTab] = pageCount;
}

void UIScene_CreativeMenu::switchTab(ECreativeInventoryTabs tab)
{
    if (static_cast<int>(tab) != m_currentTab)
        updateTabHighlightAndText(tab);

    TabSpec* spec = s_tabSpecs[tab];
    m_currentTab = static_cast<int>(tab);
    updateScrollCurrentPage(m_tabCurrentPages[m_currentTab] + 1, spec->getPageCount());
    spec->populateMenu(m_containerMenu, m_tabPages[m_currentTab], m_tabCurrentPages[m_currentTab]);
}

void UIScene_CreativeMenu::ScrollBar(_UIVec2D position)
{
    _UIVec2D origin = { 0.0f, 0.0f };
    _UIVec2D size = { 0.0f, 0.0f };
    m_sceneInterface->GetItemScreenData(
        static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x1f), 0, &origin, &size);

    float fraction = (position.y - origin.y) / size.y;
    if (fraction < 0.0f)
        fraction = 0.0f;
    if (fraction > 1.0f)
        fraction = 1.0f;

    if (m_currentTab >= 0 && m_currentTab < 8)
    {
        const int pageCount = static_cast<int>(m_tabPages[m_currentTab]);
        m_tabCurrentPages[m_currentTab] = Math::round(
            fraction * static_cast<float>(pageCount - 1));
        switchTab(static_cast<ECreativeInventoryTabs>(m_currentTab));
    }
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
    const int tab = m_currentTab;
    if (key < 0xb)
    {
        if (key == 8)
        {
            const int nextGroup = m_tabPages[tab] + 1;
            m_tabPages[tab] = nextGroup < static_cast<int>(s_tabSpecs[tab]->getSpecialGroupCount()) ? nextGroup : 0;
            switchTab(static_cast<ECreativeInventoryTabs>(tab));
            return;
        }
        if (key != 10)
            return;
    }
    else
    {
        if (key != 0xb)
        {
            if (key == 0xe)
            {
                const int previousPage = m_tabCurrentPages[tab] - 1;
                if (previousPage >= 0)
                {
                    m_tabCurrentPages[tab] = previousPage;
                    switchTab(static_cast<ECreativeInventoryTabs>(tab));
                    return;
                }
                m_tabCurrentPages[tab] = 0;
                return;
            }
            if (key == 0xf)
            {
                const unsigned int pageCount = s_tabSpecs[tab]->getPageCount();
                const unsigned int nextPage = static_cast<unsigned int>(m_tabCurrentPages[tab] + 1);
                m_tabCurrentPages[tab] = static_cast<int>(nextPage);
                if (nextPage < pageCount)
                {
                    switchTab(static_cast<ECreativeInventoryTabs>(tab));
                    return;
                }
                m_tabCurrentPages[tab] = static_cast<int>(pageCount - 1);
                return;
            }
            return;
        }
    }

    const int direction = (key == 0xb) ? -1 : 1;
    int newTab = tab + direction;
    if (newTab >= 0)
    {
        if (newTab > 7)
            newTab = 0;
        switchTab(static_cast<ECreativeInventoryTabs>(newTab));
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
        return;
    }
    switchTab(static_cast<ECreativeInventoryTabs>(7));
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
}

int UIScene_CreativeMenu::getEmptyInventorySlot(not_null_ptr<ItemInstance> item, int& slot)
{
    int menuSlot = 50;
    int relativeSlot = 0;
    for (int remaining = 9; remaining > 0; --remaining, ++menuSlot, ++relativeSlot)
    {
        not_null_ptr<ItemInstance> slotItem = m_containerMenu->getSlot(menuSlot)->getItem();
        if (!slotItem->isEmpty() && slotItem->sameItemWithTags(item, 0) &&
            slotItem->getCount() + item->getCount() <= item->getMaxStackSize())
        {
            slot = relativeSlot;
            return 1;
        }
    }

    int groupsRemaining = 3;
    menuSlot = 50;
    int emptySlot = menuSlot;
    while (true)
    {
        emptySlot = menuSlot;
        if (m_containerMenu->getSlot(emptySlot)->getItem()->isEmpty())
            break;
        emptySlot = menuSlot + 1;
        if (m_containerMenu->getSlot(emptySlot)->getItem()->isEmpty())
            break;
        emptySlot = menuSlot + 2;
        if (m_containerMenu->getSlot(emptySlot)->getItem()->isEmpty())
            break;
        groupsRemaining--;
        menuSlot += 3;
        if (groupsRemaining == 0)
            return 0;
    }
    slot = emptySlot - 50;
    return 1;
}

void UIScene_CreativeMenu::handleSlotListClicked(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                 int slot, int button)
{
    int effectiveSlot = m_sceneInterface->getCurrentIndex(section);
    Minecraft* minecraft = Minecraft::GetInstance();

    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x16) && slot == 0)
    {
        const int playerId = getPad();
        Inventory* inventory = minecraft->mLocalPlayers[playerId]->mInventory.get();
        Slot* hotbarSlot = m_sceneInterface->m_menu->getSlot(effectiveSlot);
        not_null_ptr<ItemInstance> heldItem = hotbarSlot->getItem();
        if (heldItem->isEmpty())
            return;

        not_null_ptr<ItemInstance> newCarried = heldItem->copy();
        inventory->setCarried(newCarried);
        newCarried = inventory->getCarried();
        if (button == 1)
            newCarried->setCount(newCarried->getMaxStackSize());

        m_sceneInterface->m_dragEnd = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15);
        m_sceneInterface->m_dragStart = 5;

        int emptySlot = 0;
        if (!getEmptyInventorySlot(newCarried, emptySlot))
            return;

        m_sceneInterface->m_quickCraftRemaining = emptySlot;
        m_sceneInterface->setSectionSelectedSlot(static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15),
                                                  emptySlot, m_sceneInterface->m_quickCraftSlots, false);

        effectiveSlot = m_sceneInterface->getCurrentIndex(
            static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15));
        slot = 0;
        button = 0;
    }
    else if (section != static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15))
    {
        return;
    }

    const int playerId = getPad();
    boost::shared_ptr<Player> player = minecraft->mLocalPlayers[playerId];
    m_sceneInterface->m_menu->clicked(effectiveSlot, slot,
                                      static_cast<ClickType::eClickType>(button != 0), player);

    const int itemsSize = static_cast<int>(m_sceneInterface->m_menu->getItems()->size());
    minecraft->mGameModes[playerId]->handleCreativeModeItemAdd(
        m_sceneInterface->m_menu->getSlot(effectiveSlot)->getItem(), (effectiveSlot - itemsSize) + 45);
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

namespace
{
    void AddCreativeBlock(MenuBuilder& builder, Block* block)
    {
        builder.ITEM_BLOCKSTATE(block->defaultBlockState());
    }

    void AddCreativeBlockItem(MenuBuilder& builder, Block* block, int aux)
    {
        builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(block, 1, aux)));
    }
}

void UIScene_CreativeMenu::staticCtor()
{
    MenuBuilder builder;
    builder.mItems = &s_groupItems[0];

    AddCreativeBlock(builder, Blocks::stone);
    AddCreativeBlock(builder, Blocks::grass);
    AddCreativeBlock(builder, Blocks::grass_path);
    AddCreativeBlock(builder, Blocks::dirt);
    AddCreativeBlock(builder, Blocks::cobblestone);
    AddCreativeBlock(builder, Blocks::sand);
    AddCreativeBlock(builder, Blocks::sandstone);
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::woodenDoor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::ironDoor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::spruceDoor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::birchDoor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::jungleDoor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::acaciaDoor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::darkOakDoor, 1, 0)));
    {
        Block* stairsBlocks[] = {
            Blocks::oak_stairs, Blocks::spruce_stairs, Blocks::jungle_stairs,
            Blocks::acacia_stairs, Blocks::stone_stairs, Blocks::brick_stairs,
            Blocks::stone_brick_stairs, Blocks::nether_brick_stairs,
            Blocks::red_sandstone_stairs, Blocks::quartz_stairs,
            Blocks::purpur_stairs, Blocks::prismarine_stairs
        };
        const int numValues = sizeof(stairsBlocks) / sizeof(stairsBlocks[0]);
        for (int i = 0; i < numValues; ++i)
            AddCreativeBlockItem(builder, stairsBlocks[i], 2);
    }
    {
        int auxValues[] = {15, 11, 3, 13, 5, 6, 2, 10, 12, 4, 0, 7};
        const int numValues = sizeof(auxValues) / sizeof(auxValues[0]);
        for (int i = 0; i < numValues; ++i)
            AddCreativeBlockItem(builder, Blocks::stained_hardened_clay, auxValues[i]);
    }
    builder.mItems = &s_groupItems[1];
    {
        int auxValues[] = {9, 13, 5, 6, 2, 10, 14, 12, 0, 4, 0, 8, 7};
        const int numValues = sizeof(auxValues) / sizeof(auxValues[0]);
        for (int i = 0; i < numValues; ++i)
            AddCreativeBlockItem(builder, Blocks::stained_glass, auxValues[i]);
    }
    {
        int auxValues[] = {15, 11, 3, 9, 13, 5, 6, 2, 10, 14, 12, 0, 4, 0, 8, 7};
        const int numValues = sizeof(auxValues) / sizeof(auxValues[0]);
        for (int i = 0; i < numValues; ++i)
            AddCreativeBlockItem(builder, Blocks::stained_glass_pane, auxValues[i]);
    }
    {
        int auxValues[] = {0, 0, 2, 3, 4};
        const int numValues = sizeof(auxValues) / sizeof(auxValues[0]);
        for (int i = 0; i < numValues; ++i)
            AddCreativeBlockItem(builder, Blocks::coral, auxValues[i]);
    }
    {
        int auxValues[] = {0, 0, 2, 3, 4};
        const int numValues = sizeof(auxValues) / sizeof(auxValues[0]);
        for (int i = 0; i < numValues; ++i)
            AddCreativeBlockItem(builder, Blocks::coral_fan, auxValues[i]);
    }
    {
        int auxValues[] = {0, 0, 2, 3, 4};
        const int numValues = sizeof(auxValues) / sizeof(auxValues[0]);
        for (int i = 0; i < numValues; ++i)
            AddCreativeBlockItem(builder, Blocks::coral_fan_dead, auxValues[i]);
    }
    {
        int auxValues[] = {0, 0, 2, 3, 4, 8, 9, 10, 11, 12};
        const int numValues = sizeof(auxValues) / sizeof(auxValues[0]);
        for (int i = 0; i < numValues; ++i)
            AddCreativeBlockItem(builder, Blocks::coral_block, auxValues[i]);
    }
    AddCreativeBlockItem(builder, Blocks::piston, 5);
    AddCreativeBlockItem(builder, Blocks::sticky_piston, 5);
    AddCreativeBlockItem(builder, Blocks::stone_button, 4);
    AddCreativeBlockItem(builder, Blocks::wooden_button, 4);
    AddCreativeBlockItem(builder, Blocks::acacia_button, 4);
    AddCreativeBlockItem(builder, Blocks::birch_button, 4);
    AddCreativeBlockItem(builder, Blocks::dark_oak_button, 4);
    AddCreativeBlockItem(builder, Blocks::jungle_button, 4);
    AddCreativeBlockItem(builder, Blocks::spruce_button, 4);
    AddCreativeBlockItem(builder, Blocks::redstone_torch, 5);
    builder.mItems = &s_groupItems[8];
    builder.POTION(Items::potion, Potions::WATER);
    builder.POTION(Items::potion, Potions::AWKWARD);

    builder.mItems = &s_groupItems[1];
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 5)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::flowerPot, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::painting, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::itemFrame, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::sign, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::kelp, 1, 0)));
    builder.mItems = &s_groupItems[2];
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::redstone, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::repeater, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::comparator, 1, 0)));
    builder.mItems = &s_groupItems[3];
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::minecart, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::chestMinecart, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::furnaceMinecart, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::hopperMinecart, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::tntMinecart, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::elytra, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::saddle, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::boat, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::spruceBoat, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::birchBoat, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::jungleBoat, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::acaciaBoat, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::darkOakBoat, 1, 0)));
    builder.mItems = &s_groupItems[11];
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::brewingStand, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::brewingStand, 1, 15)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 11)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 9)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 13)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 5)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 6)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 10)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 14)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 12)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 8)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 7)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::endCrystal, 1, 0)));

    static ECreative_Inventory_Groups structures[] = {static_cast<ECreative_Inventory_Groups>(0)};
    static ECreative_Inventory_Groups decoration[] = {static_cast<ECreative_Inventory_Groups>(1)};
    static ECreative_Inventory_Groups redstone[] = {static_cast<ECreative_Inventory_Groups>(3), static_cast<ECreative_Inventory_Groups>(2)};
    static ECreative_Inventory_Groups materials[] = {static_cast<ECreative_Inventory_Groups>(4)};
    static ECreative_Inventory_Groups food[] = {static_cast<ECreative_Inventory_Groups>(5)};
    static ECreative_Inventory_Groups tools[] = {static_cast<ECreative_Inventory_Groups>(6)};
    static ECreative_Inventory_Groups brewing[] = {static_cast<ECreative_Inventory_Groups>(7), static_cast<ECreative_Inventory_Groups>(8), static_cast<ECreative_Inventory_Groups>(9), static_cast<ECreative_Inventory_Groups>(10)};
    static ECreative_Inventory_Groups misc[] = {static_cast<ECreative_Inventory_Groups>(11)};
    s_tabSpecs[0] = new TabSpec(L"Structures", 0x0FBC6E08, 1, structures, 0, 0, 0, 0);
    s_tabSpecs[1] = new TabSpec(L"Decoration", static_cast<int>(0xAC698282), 1, decoration, 0, 0, 0, 0);
    s_tabSpecs[2] = new TabSpec(L"RedstoneAndTransport", 0x060380CB, 2, redstone, 3, 0, 0, 0);
    s_tabSpecs[3] = new TabSpec(L"Materials", 0x448C117B, 1, materials, 0, 0, 0, 0);
    s_tabSpecs[4] = new TabSpec(L"Food", static_cast<int>(0xEF31F309), 1, food, 0, 0, 0, 0);
    s_tabSpecs[5] = new TabSpec(L"Tools", static_cast<int>(0xAD2EBC52), 1, tools, 0, 0, 0, 0);
    s_tabSpecs[6] = new TabSpec(L"Brewing", 0x50D06098, 4, brewing, 0, 0, 0, 0);
    s_tabSpecs[7] = new TabSpec(L"Misc", static_cast<int>(0xBC0FD661), 1, misc, 0, 0, 0, 0);

}

unsigned int UIScene_CreativeMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    m_movie->getRootNode();
    fuiRenderNode* mainNode = GetMainPanel()->getFuiRenderNode();

    for (int i = 0; i < 8; ++i)
    {
        std::string name("TouchPanel_");
        name += static_cast<char>('0' + i);
        m_touchPanels[i].setupControl(this, mainNode, name);
        m_controls.push_back(&m_touchPanels[i]);
    }
    m_touchPanels[8].setupControl(this, mainNode, "TouchPanel_Slider");
    m_controls.push_back(&m_touchPanels[8]);

    m_hotbarList.setupControl(this, mainNode, "containerList");
    m_controls.push_back(&m_hotbarList);

    return 1;
}

std::wstring UIScene_CreativeMenu::getMoviePath()
{
    return std::wstring(g_pApp->GetLocalPlayerCount() < 2 ? L"CreativeMenu" : L"CreativeMenuSplit");
}

void UIScene_CreativeMenu::handleReload()
{
    m_sceneInterface->Initialize(true, m_playerId, m_sceneInterface->m_menu, false, -1,
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15),
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x20),
                                 m_sceneInterface->m_canNavigateBack != 0);

    for (int i = 0; i < 50; ++i)
        m_hotbarList.addSlot(i);
    for (int i = 50; i < 59; ++i)
        UIScene_AbstractContainerMenu::m_hotbarList.addSlot(i);

    const int previousTab = m_currentTab;
    m_currentTab = 8;
    switchTab(static_cast<ECreativeInventoryTabs>(previousTab));
}

UIControl* UIScene_CreativeMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x15)) return &m_hotbarList;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x16)) return &m_containerList;
    return 0;
}

void UIScene_CreativeMenu::handleInput(int playerId, int action, bool repeat, bool released, bool pressed, bool& handled)
{
    if (released)
        return;

    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (action < 0x5804)
    {
        UIScene_AbstractContainerMenu::handleInput(playerId, action, false, released, pressed, handled);
        return;
    }
    if (action < 0x5806)
    {
        if (!pressed)
            return;
        const int direction = action == 0x5804 ? 1 : -1;
        int tab = m_currentTab + direction + 8;
        tab -= ((tab >> 31) & 7) + tab & 0xfffffff8;
        switchTab(static_cast<ECreativeInventoryTabs>(tab));
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
        return;
    }
    if (action == 0x5806)
    {
        if (!pressed)
            return;
        int nextPage = m_tabCurrentPages[m_currentTab] + 1;
        if (nextPage >= static_cast<int>(s_tabSpecs[m_currentTab]->getPageCount()))
            nextPage = 0;
        m_tabCurrentPages[m_currentTab] = nextPage;
        switchTab(static_cast<ECreativeInventoryTabs>(m_currentTab));
        return;
    }
    UIScene_AbstractContainerMenu::handleInput(playerId, action, false, released, pressed, handled);
}

void UIScene_CreativeMenu::handleTouchInput(unsigned int, int action, int, int y, bool, bool released, bool pressed)
{
    if (pressed && action < 8)
    {
        switchTab(static_cast<ECreativeInventoryTabs>(action));
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    }
    if (released && action == 8)
    {
        TabSpec* spec = s_tabSpecs[m_currentTab];
        if (spec->getPageCount() > 1)
        {
            float fraction = static_cast<float>(
                (static_cast<double>(y) - m_containerList.getYPos() - m_mainPanel.getYPos()) /
                m_containerList.getHeight());
            if (fraction > 1.0f)
                fraction = 1.0f;
            else if (fraction < 0.0f)
                fraction = 0.0f;
            const int pageCount = static_cast<int>(spec->getPageCount());
            m_tabCurrentPages[m_currentTab] =
                Math::round(fraction * static_cast<float>(pageCount - 1));
            switchTab(static_cast<ECreativeInventoryTabs>(m_currentTab));
        }
    }
}

UIControl* UIScene_CreativeMenu::GetMainPanel()
{
    return &m_mainPanel;
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
