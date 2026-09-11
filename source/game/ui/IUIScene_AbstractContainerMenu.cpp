#include "game/ui/IUIScene_AbstractContainerMenu.hpp"

#include <cmath>

#include "game/core/Minecraft.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/Player.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/core/Slot.hpp"
#include "game/core/HtmlString.hpp"

IUIScene_AbstractContainerMenu::IUIScene_AbstractContainerMenu()
    : m_menu(0), m_initialized(0), m_takeAllEnabled(0), m_quickCraftDisabled(0),
      m_containerId(0), m_touchX(0.0f), m_touchY(0.0f), m_sectionStart(0),
      m_sectionSize(0), m_sectionRows(0), m_sectionColumns(0), m_selectedSlot(0),
      m_activeTooltip(0), m_slotIndex(0), m_slotCount(0), m_currentIndex(0), m_dragStart(0),
      m_dragEnd(0), m_quickCraftRemaining(0), m_quickCraftSlots(0), m_pointerX(0),
      m_pointerY(0), m_pointerSection(0), m_pointerSlot(0), m_pointerButton(0),
      m_pointerItem(0), m_pointerFlags(0), m_lastInput(0), m_inventorySmall(0),
      m_dragSection(0), m_dragSlot(0), m_dragButton(0), m_dragCount(0),
      m_hoverSection(0), m_hoverSlot(0), m_hoverX(0), m_hoverY(0), m_hoverFlags(0)
{
    m_tooltipButtons[0] = 0;
    m_tooltipButtons[1] = 0;
    m_tooltipButtons[2] = 0;
    m_tooltipButtons[3] = 0;
    m_tooltipItems[0] = 0;
    m_tooltipItems[1] = 0;
    m_tooltipItems[2] = 0;
    m_tooltipItems[3] = 0;
    m_inputState[0] = 0;
    m_inputState[1] = 0;
    m_inputState[2] = 0;
    m_inputState[3] = 0;
    m_inputState[4] = 0;
    m_inputState[5] = 0;
    m_inputState[6] = 0;
    m_inputState[7] = 0;
}

IUIScene_AbstractContainerMenu::~IUIScene_AbstractContainerMenu()
{
}

unsigned int IUIScene_AbstractContainerMenu::canTakeAll() const
{
    if (m_takeAllEnabled == 0 || !Minecraft::InMiniGame((EMiniGameId)15, true)) return 0;
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (miniGame == 0 || !miniGame->IsTakeEverythingEnabled()) return 0;
    return 1;
}

unsigned int IUIScene_AbstractContainerMenu::canQuickCraft() const
{
    return m_quickCraftDisabled == 0 && !Minecraft::InMiniGame((EMiniGameId)15, true);
}

int IUIScene_AbstractContainerMenu::GetSectionDimensions(ESceneSection section, int* columns, int* rows)
{
    if (!IsSectionSlotList(section))
    {
        *rows = 0;
        *columns = 0;
        return 0;
    }
    *rows = (int)getSectionRows(section);
    *columns = (int)getSectionColumns(section);
    return *rows * *columns;
}

void IUIScene_AbstractContainerMenu::updateSlotPosition(ESceneSection from, ESceneSection to,
                                                         ETapState state, int* x, int* y,
                                                         int dx, int dy)
{
    int columns;
    int rows;
    GetSectionDimensions(to, &columns, &rows);
    if (to == from)
    {
        *x -= dx;
        if (*x < 0) *x = 0;
        if (*x >= columns) *x = columns - 1;
        return;
    }
    if (state == ETapState_Up)
        *y = rows - 1 - dy;
    else if (state == ETapState_Down)
        *y = -dy;
    else
    {
        *y -= dy;
        if (*y < 0) *y = 0;
    }
    if (*y >= rows) *y = rows - 1;
    *x -= dx;
    if (*x < 0) *x = 0;
    if (*x >= columns) *x = columns - 1;
}

unsigned int IUIScene_AbstractContainerMenu::GetTapInputType(float x, float y)
{
    if (std::fabs(x) < 0.3f && std::fabs(y) < 0.3f) return 0;
    if (x < -0.3f && std::fabs(y) < 0.3f) return 3;
    if (0.3f < x && std::fabs(y) < 0.3f) return 4;
    if (y < -0.3f && std::fabs(x) < 0.3f) return 2;
    if (0.3f < y && std::fabs(x) < 0.3f) return 1;
    return 6;
}

void IUIScene_AbstractContainerMenu::UpdateTooltips()
{
    unsigned int playerId = getPad();
    for (int i = 0; i < 4; ++i)
    {
        int tooltip = m_tooltipItems[i];
        if (tooltip == -1)
            gConsoleUIController.ShowTooltip(playerId, i, false);
        else
        {
            gConsoleUIController.SetTooltipText(playerId, i, tooltip);
            gConsoleUIController.ShowTooltip(playerId, i, true);
        }
    }
}

void IUIScene_AbstractContainerMenu::SetToolTip(EToolTipButton button, EToolTipItem item)
{
    if ((int)button < 0 || (int)button >= 4) return;
    if (m_tooltipItems[(int)button] == (unsigned char)item) return;
    m_tooltipItems[(int)button] = (unsigned char)item;
    UpdateTooltips();
}

void IUIScene_AbstractContainerMenu::Initialize(bool takeAll, int containerId, AbstractContainerMenu* menu,
                                                 bool smallInventory, int slotIndex,
                                                 ESceneSection firstSection, ESceneSection lastSection,
                                                 bool touchEnabled)
{
    m_takeAllEnabled = takeAll ? 1 : 0;
    m_containerId = containerId;
    m_menu = menu;
    m_inventorySmall = smallInventory ? 1 : 0;
    m_slotIndex = slotIndex;
    m_sectionStart = (int)firstSection;
    m_sectionSize = (int)lastSection;
    m_initialized = touchEnabled ? 1 : 0;
}

std::vector<HtmlString>* IUIScene_AbstractContainerMenu::GetItemDescription(Slot* slot)
{
    if (slot == 0) return 0;
    not_null_ptr<ItemInstance> item = slot->getItem();
    std::vector<HtmlString>* description = item->getTooltipLines(
        Minecraft::GetInstance()->GetPlayerByPlayerIndex(getPad()), false);
    if (description != 0 && !description->empty())
    {
        description->at(0).mColour = static_cast<eMinecraftColour>(item->getRarity());
        if (item->hasCustomHoverName())
            description->at(0).mColour = static_cast<eMinecraftColour>(0x1b9);
    }
    return description;
}

unsigned int IUIScene_AbstractContainerMenu::IsSameItemAs(not_null_ptr<ItemInstance> first,
                                                          not_null_ptr<ItemInstance> second)
{
    if (first->isEmpty() || second->isEmpty()) return 0;
    if (first->getItem() != second->getItem()) return 0;
    if (second->isStackedByData() && second->getAuxValue() != first->getAuxValue()) return 0;
    return ItemInstance::tagMatches(second, first);
}

int IUIScene_AbstractContainerMenu::GetEmptyStackSpace(Slot* slot)
{
    if (slot == 0 || !slot->hasItem()) return 0;
    not_null_ptr<ItemInstance> item = slot->getItem();
    if (!item->isStackable()) return 0;
    int maximum = item->getMaxStackSize();
    if (slot->getMaxStackSize() < maximum) maximum = slot->getMaxStackSize();
    maximum -= item->getCount();
    return maximum < 0 ? 0 : maximum;
}

void IUIScene_AbstractContainerMenu::recalculateQuickCraftRemaining()
{
    m_quickCraftRemaining = m_quickCraftSlots > 0 ? m_quickCraftRemaining / m_quickCraftSlots : 0;
}

void IUIScene_AbstractContainerMenu::onMouseTick()
{
    unsigned int input = GetTapInputType(m_touchX, m_touchY);
    if (input != 0) m_lastInput = (int)input;
    if (m_quickCraftRemaining > 0) recalculateQuickCraftRemaining();
}

int IUIScene_AbstractContainerMenu::getCurrentIndex(ESceneSection section)
{
    int columns;
    int rows;
    GetSectionDimensions(section, &columns, &rows);
    return getSectionStartOffset(section) + m_sectionStart * columns + m_currentIndex;
}

void IUIScene_AbstractContainerMenu::slotClicked(int slot, int button, ClickType::eClickType clickType)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    int playerIndex = getPad();
    boost::shared_ptr<Player> player = minecraft->GetPlayerByPlayerIndex(playerIndex);
    MultiPlayerGameMode* gameMode = minecraft->mGameModes[playerIndex];
    gameMode->handleInventoryMouseClick(m_containerId, slot, button, clickType, player);
}

void IUIScene_AbstractContainerMenu::handleQuickCraft()
{
    if (!canQuickCraft()) return;
    m_quickCraftSlots = 0;
    m_quickCraftRemaining = 0;
}

unsigned int IUIScene_AbstractContainerMenu::handleKeyDown(int key, int value, bool repeat)
{
    m_lastInput = key;
    if (repeat) m_pointerFlags |= value;
    if (key == 0 && canTakeAll())
        handleTakeAll(value);
    else if (key == 1 && canQuickCraft())
        handleQuickCraft();
    else
        handleAdditionalKeyPress(key);
    return 1;
}

unsigned int IUIScene_AbstractContainerMenu::handleValidKeyPress(int key, int value, int player)
{
    return 0;
}

void IUIScene_AbstractContainerMenu::handleOutsideClicked(int x, int y, int button)
{
    slotClicked(-999, y, static_cast<ClickType::eClickType>(button != 0));
}

void IUIScene_AbstractContainerMenu::handleOtherClicked(int button, ESceneSection section, int slot, bool pressed)
{
}

void IUIScene_AbstractContainerMenu::handleAdditionalKeyPress(int key)
{
}

void IUIScene_AbstractContainerMenu::handleSlotListClicked(ESceneSection section, int slot, int button)
{
    int index = getCurrentIndex(section);
    slotClicked(index, slot, static_cast<ClickType::eClickType>(button != 0));
    handleSectionClick(section);
}

void IUIScene_AbstractContainerMenu::handleTakeAll(int button)
{
    handleSectionClick(static_cast<ESceneSection>(2));
}

std::vector<HtmlString>* IUIScene_AbstractContainerMenu::GetSectionHoverText(ESceneSection section)
{
    return 0;
}

int IUIScene_AbstractContainerMenu::GetInventoryRows()
{
    return m_inventorySmall == 0 ? 3 : 0;
}

bool IUIScene_AbstractContainerMenu::IsSectionSlotList(ESceneSection section)
{
    return (int)section != -1;
}

unsigned int IUIScene_AbstractContainerMenu::CanHaveFocus(ESceneSection section)
{
    return 1;
}

unsigned int IUIScene_AbstractContainerMenu::IsVisible(ESceneSection section)
{
    return 1;
}

unsigned int IUIScene_AbstractContainerMenu::overrideTooltips(ESceneSection section,
                                                               not_null_ptr<ItemInstance> item,
                                                               bool itemValid, bool canTake,
                                                               bool canPlace, int count,
                                                               EToolTipItem& first,
                                                               EToolTipItem& second,
                                                               EToolTipItem& third,
                                                               EToolTipItem& fourth,
                                                               EToolTipItem& fifth,
                                                               EToolTipItem& sixth,
                                                               EToolTipItem& seventh)
{
    return 0;
}
