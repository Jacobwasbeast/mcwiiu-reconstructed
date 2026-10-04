#include "game/ui/IUIScene_AbstractContainerMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cmath>

#include "game/client/render/Renderer.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Slot.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/ArmorRecipes.hpp"
#include "game/item/FurnaceRecipes.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionBrewing.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/ShulkerBoxMenu.hpp"
#include "game/util/D3DXVECTOR3.hpp"
#include "game/world/block/FurnaceBlockEntity.hpp"
#include "game/world/block/ShulkerBoxBlockItem.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/Rarity.hpp"
#include "platform/wiiu/CInput.hpp"

static const int s_toolTipStringIds[] = {
    IDS_TOOLTIPS_PICKUPPLACE, (int)IDS_TOOLTIPS_EXIT, IDS_TOOLTIPS_PICKUP_GENERIC, 0x1d6a6d67, 0x23a35725, (int)0x987f8d08,
    (int)IDS_TOOLTIPS_PLACE_ONE, (int)IDS_TOOLTIPS_DISTRIBUTE, (int)IDS_TOOLTIPS_PLACE_ALL, (int)0x92835202, (int)IDS_TOOLTIPS_DROP_ONE,
    (int)IDS_TOOLTIPS_DROP_ALL, (int)IDS_TOOLTIPS_SWAP, IDS_TOOLTIPS_QUICK_MOVE, (int)IDS_TOOLTIPS_QUICK_MOVE_INGREDIENT, IDS_TOOLTIPS_QUICK_MOVE_FUEL, 0x2fccdffb,
    (int)IDS_TOOLTIPS_EQUIP, (int)IDS_TOOLTIPS_CLEAR_QUICK_SELECT, IDS_TOOLTIPS_QUICK_MOVE_TOOL, (int)IDS_TOOLTIPS_QUICK_MOVE_ARMOR, (int)IDS_TOOLTIPS_QUICK_MOVE_WEAPON, IDS_TOOLTIPS_DYE,
    IDS_TOOLTIPS_REPAIR, IDS_TOOLTIPS_TAKE_EVERYTHING, (int)IDS_TOOLTIPS_SELECT};

IUIScene_AbstractContainerMenu::IUIScene_AbstractContainerMenu()
{
    m_menu = NULL;
    m_takeAllEnabled = false;
    m_quickCraftDisabled = false;
    m_pointerPos.x = 0.0f;
    m_pointerLabelSlot = NULL;
    m_pointerPos.y = 0.0f;
    m_autoDeleteMenu = false;
    m_previousTutorialState = eTutorial_State_Gameplay;
    m_smallInventory = false;
}

IUIScene_AbstractContainerMenu::~IUIScene_AbstractContainerMenu()
{
    if (m_autoDeleteMenu)
        delete m_menu;
}

bool IUIScene_AbstractContainerMenu::canTakeAll() const
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    return m_takeAllEnabled && Minecraft::InMiniGame(eMiniGameId_Any, true) &&
           miniGame->IsTakeEverythingEnabled() &&
           CGameNetworkManager::sInstance.IsInActiveCoopOrCompetitive(-1);
}

bool IUIScene_AbstractContainerMenu::canQuickCraft() const
{
    return !m_quickCraftDisabled && !Minecraft::InMiniGame(eMiniGameId_Any, true);
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

void IUIScene_AbstractContainerMenu::updateSlotPosition(ESceneSection section, ESceneSection newSection,
                                                         ETapState tapState, int* slotX, int* slotY,
                                                         int xOffset, int yOffset)
{
    int columns;
    int rows;
    GetSectionDimensions(newSection, &columns, &rows);

    if (newSection != section)
    {
        int newY;
        if (tapState == ETapState_Up)
            newY = rows - 1 - yOffset;
        else if (tapState == ETapState_Down)
            newY = -yOffset;
        else
            newY = *slotY - yOffset;

        if (newY < 0)
            *slotY = 0;
        else if (newY < rows)
            *slotY = newY;
        else
            *slotY = rows - 1;
    }

    int newX = *slotX - xOffset;
    if (newX < 0)
        *slotX = 0;
    else if (newX < columns)
        *slotX = newX;
    else
        *slotX = columns - 1;
}

IUIScene_AbstractContainerMenu::ETapState IUIScene_AbstractContainerMenu::GetTapInputType(float inputX,
                                                                                           float inputY)
{
    if (std::fabs(inputX) < 0.3f && std::fabs(inputY) < 0.3f)
        return ETapState_None;
    if (inputX < -0.3f && std::fabs(inputY) < 0.3f)
        return ETapState_Left;
    if (inputX > 0.3f && std::fabs(inputY) < 0.3f)
        return ETapState_Right;
    if (inputY < -0.3f && std::fabs(inputX) < 0.3f)
        return ETapState_Down;
    if (inputY > 0.3f && std::fabs(inputX) < 0.3f)
        return ETapState_Up;
    return ETapState_Cancelled;
}

void IUIScene_AbstractContainerMenu::UpdateTooltips()
{
    unsigned char pad = getPad();
    for (int button = 0; button < eToolTipButton_Count; ++button)
    {
        if (m_toolTips[button] == eToolTipNone)
        {
            gConsoleUIController.ShowTooltip(pad, button, false);
        }
        else
        {
            gConsoleUIController.SetTooltipText(pad, button, s_toolTipStringIds[m_toolTips[button]]);
            gConsoleUIController.ShowTooltip(pad, button, true);
        }
    }
}

void IUIScene_AbstractContainerMenu::SetToolTip(EToolTipButton button, EToolTipItem item)
{
    if (m_toolTips[button] != item)
    {
        m_toolTips[button] = item;
        UpdateTooltips();
    }
}

void IUIScene_AbstractContainerMenu::Initialize(bool navigateBack, int pad, AbstractContainerMenu* menu,
                                                 bool autoDeleteMenu, int startIndex,
                                                 ESceneSection firstSection, ESceneSection maxSection,
                                                 bool navigateBackOnClose)
{
    m_autoDeleteMenu = autoDeleteMenu;
    m_menu = menu;

    bool smallInventory = false;
    if (Minecraft::InMiniGame(eMiniGameId_Any, true) &&
        Minecraft::GetInstance()->GetMiniGame()->IsSmallInventoryEnabled())
    {
        smallInventory = true;
    }
    m_smallInventory = smallInventory;

    Minecraft::GetInstance()->mLocalPlayers[pad]->mContainerMenu = menu;
    m_menu->m_uiQuickCraftActive = false;
    m_menu->m_uiQuickCraftSlots.clear();

    for (int button = 0; button < eToolTipButton_Count; ++button)
        m_toolTips[button] = eToolTipNone;

    SetToolTip((EToolTipButton)1, eToolTipExit);
    SetToolTip((EToolTipButton)0, eToolTipNone);
    SetToolTip((EToolTipButton)2, eToolTipNone);
    SetToolTip((EToolTipButton)3, eToolTipNone);

    m_navigateBack = navigateBackOnClose;
    m_consecutiveInputTicks = 0;
    m_currentSection = firstSection;
    m_firstSection = firstSection;
    m_pointerOutsideMenu = false;
    m_currentSlotY = 0;
    m_tapState = ETapState_None;
    m_maxSection = maxSection;
    m_currentSlotX = 0;

    PlatformInitialize(navigateBack, pad, startIndex);
}

std::vector<HtmlString>* IUIScene_AbstractContainerMenu::GetItemDescription(Slot* slot)
{
    if (slot == NULL)
        return NULL;

    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[getPad()];
    std::vector<HtmlString>* description = slot->getItem()->getTooltipLines(player, false);
    if (description->size() != 0)
    {
        description->at(0).mColour = slot->getItem()->getRarity()->colour;
        if (slot->getItem()->hasCustomHoverName())
            description->at(0).mColour = eMinecraftColour_Color_RenamedItemTitle;
    }
    return description;
}

bool IUIScene_AbstractContainerMenu::IsSameItemAs(not_null_ptr<ItemInstance> first,
                                                  not_null_ptr<ItemInstance> second)
{
    if (!first->isEmpty() && !second->isEmpty())
    {
        bool same = false;
        if (first->getItem() == second->getItem())
        {
            if (second->isStackedByData() && second->getAuxValue() != first->getAuxValue())
                return false;
            if (ItemInstance::tagMatches(second, first))
                same = true;
        }
        return same;
    }
    return false;
}

int IUIScene_AbstractContainerMenu::GetEmptyStackSpace(Slot* slot)
{
    if (slot == NULL || !slot->hasItem())
        return 0;

    not_null_ptr<ItemInstance> item = slot->getItem();
    if (!item->isStackable())
        return 0;

    int count = item->getCount();
    int maxStackSize = item->getMaxStackSize();
    int slotMaxStackSize = slot->getMaxStackSize();
    if (slotMaxStackSize < maxStackSize)
        maxStackSize = slotMaxStackSize;
    int space = maxStackSize - count;
    return space < 0 ? 0 : space;
}

void IUIScene_AbstractContainerMenu::recalculateQuickCraftRemaining()
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[getPad()];
    not_null_ptr<ItemInstance> carried = ItemInstance::EMPTY;
    if (player.get() != NULL)
        carried = player->mInventory->getCarried();

    if (carried->isEmpty() || !m_menu->m_uiQuickCraftActive)
        return;

    m_menu->m_uiQuickCraftRemaining = carried->getCount();

    AbstractContainerMenu::SlotSet::iterator it = m_menu->m_uiQuickCraftSlots.begin();
    while (it != m_menu->m_uiQuickCraftSlots.end())
    {
        Slot* slot = *it;
        if (!slot->getItem()->isEmpty() && !IsSameItemAs(slot->getItem(), carried))
            it = m_menu->m_uiQuickCraftSlots.erase(it);
        else
            ++it;
    }

    for (it = m_menu->m_uiQuickCraftSlots.begin(); it != m_menu->m_uiQuickCraftSlots.end(); ++it)
    {
        Slot* slot = *it;
        not_null_ptr<ItemInstance> item = carried->copy();
        int existingCount = slot->getItem()->isEmpty() ? 0 : slot->getItem()->getCount();
        AbstractContainerMenu::getQuickCraftSlotCount(&m_menu->m_uiQuickCraftSlots, 0, item, existingCount);
        if (item->getCount() > item->getMaxStackSize())
            item->setCount(item->getMaxStackSize());
        if (item->getCount() > slot->getMaxStackSize(item))
            item->setCount(slot->getMaxStackSize(item));
        m_menu->m_uiQuickCraftRemaining -= item->getCount() - existingCount;
    }
}

void IUIScene_AbstractContainerMenu::onMouseTick()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[getPad()] != NULL)
    {
        Tutorial* tutorial = minecraft->mGameModes[getPad()]->getTutorial();
        if (tutorial != NULL && gConsoleUIController.IsTutorialVisible(getPad()) && !tutorial->popupPending &&
            !minecraft->mGameModes[getPad()]->getGameModeConstraints()->isInputAllowed(EControllerActions_MenuUp))
        {
            return;
        }
    }

    float pointerX = m_pointerPos.x + m_pointerImageOffset.x;
    float pointerY = m_pointerPos.y + m_pointerImageOffset.y;
    float newPointerX = m_pointerPos.x;
    float newPointerY = m_pointerPos.y;

    int pad = getPad();
    bool pointerMoved = false;
    float inputX = (float)(g_pInput->GetJoypadStick_Menu_LX(pad) *
                           ((float)GameSettings::GetGameSettings(pad, eGameSetting_InterfaceSensitivity) / 100.0f));
    float inputY = (float)(g_pInput->GetJoypadStick_Menu_LY(pad) *
                           ((float)GameSettings::GetGameSettings(pad, eGameSetting_InterfaceSensitivity) / 100.0f));
    float xDirection = 0.0f;
    float yDirection = 0.0f;

    ESceneSection section = ESceneSection_None;
    int slotIndex = -1;
    bool onSlot = false;

    if (std::fabs(inputX) >= 0.01f || std::fabs(inputY) >= 0.01f)
    {
        if (inputX > 0.0f)
            xDirection = 1.0f;
        else if (inputX < 0.0f)
            xDirection = -1.0f;
        if (inputY > 0.0f)
            yDirection = 1.0f;
        else if (inputY < 0.0f)
            yDirection = -1.0f;

        ETapState tapState = GetTapInputType(inputX, inputY);
        if (m_tapState == ETapState_None)
            m_tapState = tapState;
        else if (m_tapState < ETapState_NoMove && tapState != m_tapState && tapState != ETapState_None)
            m_tapState = ETapState_Cancelled;

        float moveX = inputX * inputX * xDirection * 13.0f;
        float moveY = inputY * inputY * yDirection * 13.0f;
        float speed = 1.0f;
        if (m_consecutiveInputTicks < 7)
        {
            ++m_consecutiveInputTicks;
            speed = (float)m_consecutiveInputTicks / 7.0f;
        }
        else if (m_consecutiveInputTicks < 15)
        {
            ++m_consecutiveInputTicks;
        }
        else
        {
            m_tapState = ETapState_Cancelled;
        }
        if (!gRenderer->IsHiDef() || gApp.GetLocalPlayerCount() > 1)
            speed *= 0.6f;

        pointerX = moveX * speed + pointerX;
        pointerY = pointerY - moveY * speed;
        if (pointerX < m_pointerMinX)
            pointerX = m_pointerMinX;
        else if (pointerX > m_pointerMaxX)
            pointerX = m_pointerMaxX;
        if (pointerY < m_pointerMinY)
            pointerY = m_pointerMinY;
        else if (pointerY > m_pointerMaxY)
            pointerY = m_pointerMaxY;
        pointerMoved = true;
    }
    else
    {
        m_consecutiveInputTicks = 0;
    }

    D3DXVECTOR3 snapPos;
    int slotX = -1;
    int slotY = -1;
    for (int eSection = m_firstSection; eSection < m_maxSection; ++eSection)
    {
        ESceneSection candidate = static_cast<ESceneSection>(eSection);
        if (m_tapState == ETapState_NoMove)
        {
            section = m_currentSection;
            continue;
        }
        if (section != ESceneSection_None)
            continue;

        _UIVec2D sectionPos;
        GetPositionOfSection(candidate, &sectionPos);
        if (!IsSectionSlotList(candidate))
        {
            _UIVec2D itemPos;
            _UIVec2D itemSize;
            GetItemScreenData(candidate, 0, &itemPos, &itemSize);
            if (sectionPos.x <= pointerX && pointerX <= itemSize.x + itemPos.x &&
                sectionPos.y <= pointerY && pointerY <= itemSize.y + itemPos.y)
            {
                snapPos.x = itemSize.x * 0.5f + itemPos.x;
                snapPos.y = itemSize.y * 0.5f + itemPos.y;
                if (!doesSectionTreeHaveFocus(candidate))
                    setSectionFocus(candidate, getPad());
                onSlot = false;
                if (candidate != m_currentSection || slotX != m_currentSlotX || slotY != m_currentSlotY)
                    m_tapState = ETapState_Cancelled;
                m_currentSection = candidate;
                section = candidate;
            }
            continue;
        }

        int columns;
        int rows;
        int itemCount = GetSectionDimensions(candidate, &columns, &rows);
        for (int item = 0; item < itemCount; ++item)
        {
            _UIVec2D itemPos;
            _UIVec2D itemSize;
            GetItemScreenData(candidate, item, &itemPos, &itemSize);
            itemPos.x += sectionPos.x;
            itemPos.y += sectionPos.y;
            if (itemPos.x <= pointerX && pointerX <= itemSize.x + itemPos.x && itemPos.y <= pointerY &&
                pointerY <= itemSize.y + itemPos.y)
            {
                slotY = item / columns;
                slotX = item - slotY * columns;
                snapPos.x = itemSize.x * 0.5f + itemPos.x;
                snapPos.y = itemSize.y * 0.5f + itemPos.y;
                if (!doesSectionTreeHaveFocus(candidate))
                    setSectionFocus(candidate, getPad());
                setSectionSelectedSlot(candidate, slotX, slotY, true);
                onSlot = true;
                if (candidate != m_currentSection || slotX != m_currentSlotX || slotY != m_currentSlotY)
                    m_tapState = ETapState_Cancelled;
                m_currentSection = candidate;
                m_currentSlotX = slotX;
                m_currentSlotY = slotY;
                slotIndex = item;
                section = candidate;
                break;
            }
        }
    }

    if (!IsVisible(section))
    {
        setFocusToPointer(getPad());
        section = ESceneSection_None;
        m_tapState = ETapState_Cancelled;
        m_currentSection = ESceneSection_None;
        m_currentSlotX = -1;
        m_currentSlotY = -1;
    }
    else if (section == ESceneSection_None)
    {
        setFocusToPointer(getPad());
        m_tapState = ETapState_Cancelled;
        m_currentSection = ESceneSection_None;
        m_currentSlotX = -1;
        m_currentSlotY = -1;
    }
    else if (!pointerMoved)
    {
        int columns;
        int rows;
        switch (m_tapState)
        {
        case ETapState_Up:
            slotX = m_currentSlotX;
            slotY = m_currentSlotY - 1;
            break;
        case ETapState_Down:
            slotX = m_currentSlotX;
            slotY = m_currentSlotY + 1;
            break;
        case ETapState_Left:
            slotX = m_currentSlotX - 1;
            slotY = m_currentSlotY;
            break;
        case ETapState_Right:
            slotX = m_currentSlotX + 1;
            slotY = m_currentSlotY;
            break;
        case ETapState_NoMove:
            slotX = m_currentSlotX;
            slotY = m_currentSlotY;
            break;
        default:
            slotX = -1;
            slotY = -1;
            break;
        }
        GetSectionDimensions(section, &columns, &rows);

        if (m_tapState != ETapState_Cancelled && m_tapState != ETapState_None &&
            (!IsSectionSlotList(section) || slotX < 0 || slotX >= columns || slotY < 0 || slotY >= rows))
        {
            section = static_cast<ESceneSection>(GetSectionAndSlotInDirection(section, m_tapState, &slotX, &slotY));
            if (!IsSectionSlotList(section))
                onSlot = false;
            GetSectionDimensions(section, &columns, &rows);
        }

        if (!IsSectionSlotList(section) || (slotX >= 0 && slotX < columns && slotY >= 0 && slotY < rows))
        {
            _UIVec2D sectionPos;
            _UIVec2D itemPos;
            _UIVec2D itemSize;
            GetPositionOfSection(section, &sectionPos);
            slotIndex = slotY * columns + slotX;
            GetItemScreenData(section, slotIndex, &itemPos, &itemSize);
            if (IsSectionSlotList(section))
            {
                itemPos.x += sectionPos.x;
                itemPos.y += sectionPos.y;
            }
            snapPos.x = itemSize.x * 0.5f + itemPos.x;
            snapPos.y = itemSize.y * 0.5f + itemPos.y;
            m_currentSection = section;
            m_currentSlotX = slotX;
            m_currentSlotY = slotY;
        }

        m_tapState = ETapState_None;
        if (CanHaveFocus(section))
        {
            pointerX = snapPos.x;
            pointerY = snapPos.y;
        }
    }

    if (pointerX < m_pointerMinX)
        newPointerX = m_pointerMinX;
    else if (pointerX > m_pointerMaxX)
        newPointerX = m_pointerMaxX;
    else
        newPointerX = pointerX;
    if (pointerY < m_pointerMinY)
        newPointerY = m_pointerMinY;
    else if (pointerY > m_pointerMaxY)
        newPointerY = m_pointerMaxY;
    else
        newPointerY = pointerY;

    bool pointerOutsideMenu = newPointerX < m_panelMinX || newPointerX > m_panelMaxX ||
                              newPointerY < m_panelMinY || newPointerY > m_panelMaxY;

    boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[getPad()];
    not_null_ptr<ItemInstance> carried = ItemInstance::EMPTY;
    if (player.get() != NULL)
        carried = player->mInventory->getCarried();

    not_null_ptr<ItemInstance> slotItem = ItemInstance::EMPTY;
    Slot* slot = NULL;
    int slotId = 0;
    if (onSlot)
    {
        slotId = slotIndex + getSectionStartOffset(section);
        slot = m_menu->getSlot(slotId);
        if (slot == NULL)
            onSlot = false;
    }

    bool carriedIsItem = !carried->isEmpty();
    int carriedCount = 0;
    bool carriedSameAsSlot = false;
    if (carriedIsItem)
        carriedCount = carried->getCount();

    bool slotIsItem = false;
    bool canPlaceOne = false;
    bool slotItemIsWearable = false;
    bool canCombine = false;
    int slotSpace = 0;
    bool canPlaceMultiple = false;
    bool mayPlace = false;
    int slotCount = 0;
    if (onSlot)
    {
        slotItem = slot->getItem();
        slotIsItem = !slotItem->isEmpty();
        if (slotIsItem)
        {
            slotCount = slotItem->getCount();
            if (carriedIsItem)
            {
                carriedSameAsSlot = IsSameItemAs(carried, slotItem);
                canCombine = m_menu->mayCombine(slot, carried);
                if (canCombine)
                {
                    Item* item = slot->getItem()->getItem();
                    if (item != NULL && dynamic_cast<ArmorItem*>(item) != NULL)
                    {
                        slotItemIsWearable = true;
                    }
                    else
                    {
                        item = slot->getItem()->getItem();
                        if (item != NULL && dynamic_cast<ShulkerBoxBlockItem*>(item) != NULL)
                            slotItemIsWearable = true;
                    }
                }
                if (carriedSameAsSlot)
                    slotSpace = GetEmptyStackSpace(m_menu->getSlot(slotId));
            }
        }
        if (carriedIsItem)
        {
            mayPlace = slot->mayPlace(carried);
            if (!slotIsItem)
                slotSpace = slot->getMaxStackSize();
            else
                slotSpace = GetEmptyStackSpace(slot);
            if (mayPlace && slotSpace > 0)
            {
                canPlaceOne = true;
                if (slotSpace > 1 && carried->getCount() > 1)
                    canPlaceMultiple = true;
            }
        }
    }

    if (onSlot && slotIsItem)
    {
        std::vector<HtmlString>* description = GetItemDescription(slot);
        SetPointerText(description, slot != m_pointerLabelSlot);
        m_pointerLabelSlot = slot;
        delete description;
    }
    else if (section == ESceneSection_None || IsSectionSlotList(section))
    {
        SetPointerText(NULL, false);
        m_pointerLabelSlot = NULL;
    }
    else
    {
        std::vector<HtmlString>* description = GetSectionHoverText(section);
        SetPointerText(description, false);
        m_pointerLabelSlot = NULL;
        delete description;
    }

    EToolTipItem toolTip0 = eToolTipNone;
    EToolTipItem toolTip2 = eToolTipNone;
    EToolTipItem toolTip3 = eToolTipNone;
    EToolTipItem toolTip4 = eToolTipNone;
    EToolTipItem toolTip5 = eToolTipNone;
    EToolTipItem toolTip10 = eToolTipNone;
    EToolTipItem toolTip6 = eToolTipNone;
    EToolTipItem toolTip7 = eToolTipNone;

    MultiPlayerGameMode* gameMode = Minecraft::GetInstance()->mGameModes[pad];
    bool tutorialActive = gameMode != NULL && gameMode->getTutorial() != NULL;

    bool describeSection = false;
    bool checkTutorial = false;
    if (onSlot)
    {
        if (!m_menu->m_uiQuickCraftActive)
        {
            if (!carriedIsItem)
            {
                if (slotIsItem)
                {
                    m_pointerOutsideMenu = false;
                    if (slotCount == 1)
                    {
                        toolTip0 = eToolTipTake;
                    }
                    else
                    {
                        toolTip0 = eToolTipTakeAll;
                        if (m_currentSection != ESceneSection_InventoryResult && m_currentSection != ESceneSection_ClassicCraftingResult)
                            toolTip2 = eToolTipTakeHalf;
                    }
                    checkTutorial = true;
                }
            }
            else if (slotIsItem)
            {
                m_pointerOutsideMenu = false;
                if (!carriedSameAsSlot)
                {
                    if (mayPlace)
                        toolTip0 = eToolTipSwap;
                }
                else if (slotSpace != 0)
                {
                    if (slotSpace == 1)
                    {
                        toolTip0 = eToolTipPlace;
                    }
                    else if (canPlaceMultiple)
                    {
                        toolTip4 = eToolTipDistribute;
                        toolTip0 = eToolTipPlaceAll;
                        toolTip2 = eToolTipPlaceOne;
                    }
                    else if (canPlaceOne)
                    {
                        if (carriedCount < 2)
                        {
                            toolTip0 = eToolTipPlace;
                        }
                        else
                        {
                            toolTip4 = eToolTipDistribute;
                            toolTip0 = eToolTipPlaceOne;
                        }
                    }
                }
                if (slotItemIsWearable)
                    toolTip2 = eToolTipDye;
                else if (canCombine)
                    toolTip2 = eToolTipRepair;
                checkTutorial = true;
            }
            else if (mayPlace)
            {
                if (carriedCount != 1)
                {
                    if (canPlaceMultiple)
                    {
                        toolTip4 = eToolTipDistribute;
                        toolTip0 = eToolTipPlaceAll;
                        toolTip2 = eToolTipPlaceOne;
                    }
                    else if (canPlaceOne)
                    {
                        toolTip4 = eToolTipDistribute;
                        toolTip0 = eToolTipPlaceOne;
                    }
                }
                else
                {
                    toolTip0 = eToolTipPlace;
                }
            }
        }
        else
        {
            toolTip4 = eToolTipDistribute;
            if (slotIsItem)
            {
                m_pointerOutsideMenu = false;
                describeSection = true;
            }
        }
    }

    if (checkTutorial)
    {
        if (tutorialActive)
            toolTip5 = eToolTipWhatIsThis;
        describeSection = true;
    }

    if (describeSection)
    {
        if (section == ESceneSection_InventoryHotbar || section == ESceneSection_InventoryInventory)
        {
            not_null_ptr<ItemInstance> item = getSlotItem(section, slotIndex);
            int armorType = ArmorRecipes::GetArmorType(item->getItem());
            if (armorType == 0)
            {
                const EquipmentSlot* equipmentSlot = Mob::getEquipmentSlotForItem(item);
                if (equipmentSlot->getType() == EquipmentSlot::ARMOR &&
                    isSlotEmpty(ESceneSection_InventoryArmor, 0))
                {
                    toolTip3 = eToolTipEquip;
                }
                else
                {
                    toolTip3 = eToolTipQuickMove;
                }
            }
            else
            {
                bool armorSlotEmpty = false;
                if (armorType == 1)
                    armorSlotEmpty = isSlotEmpty(ESceneSection_InventoryArmor, 0);
                else if (armorType == 2)
                    armorSlotEmpty = isSlotEmpty(ESceneSection_InventoryArmor, 1);
                else if (armorType == 3)
                    armorSlotEmpty = isSlotEmpty(ESceneSection_InventoryArmor, 2);
                else if (armorType == 4)
                    armorSlotEmpty = isSlotEmpty(ESceneSection_InventoryArmor, 3);
                toolTip3 = armorSlotEmpty ? eToolTipEquip : eToolTipQuickMove;
            }
        }
        else if (section == ESceneSection_FurnaceHotbar || section == ESceneSection_FurnaceInventory)
        {
            not_null_ptr<ItemInstance> item = getSlotItem(section, slotIndex);
            bool isFuel = FurnaceBlockEntity::isFuel(item);
            if (FurnaceRecipes::getInstance()->getResult(item)->isEmpty())
            {
                if (!isFuel)
                {
                    toolTip3 = eToolTipQuickMove;
                }
                else if (!isSlotEmpty(ESceneSection_FurnaceFuel, 0))
                {
                    not_null_ptr<ItemInstance> fuel = getSlotItem(ESceneSection_FurnaceFuel, 0);
                    toolTip3 = fuel->getItem() == item->getItem() ? eToolTipQuickMoveFuel : eToolTipQuickMove;
                }
                else
                {
                    toolTip3 = eToolTipQuickMoveFuel;
                }
            }
            else if (isSlotEmpty(ESceneSection_FurnaceIngredient, 0))
            {
                toolTip3 = eToolTipQuickMoveIngredient;
            }
            else
            {
                not_null_ptr<ItemInstance> ingredient = getSlotItem(ESceneSection_FurnaceIngredient, 0);
                if (ingredient->getItem() == item->getItem() ||
                    !FurnaceRecipes::getInstance()->getResult(item)->isEmpty())
                {
                    toolTip3 = eToolTipQuickMoveIngredient;
                }
                else
                {
                    toolTip3 = eToolTipQuickMove;
                }
            }
        }
        else if (section == ESceneSection_BrewingHotbar || section == ESceneSection_BrewingInventory)
        {
            not_null_ptr<ItemInstance> item = getSlotItem(section, slotIndex);
            bool isIngredient = PotionBrewing::isIngredient(item);
            bool notBlazePowder = item->getItem() != Items::blazePowder;
            bool isBottle = item->getItem() == Items::potion || item->getItem() == Items::splashPotion ||
                            item->getItem() == Items::lingeringPotion ||
                            item->getItem() == Items::glassBottle;
            if (!isIngredient)
            {
                if (!isBottle)
                {
                    toolTip3 = eToolTipQuickMove;
                }
                else if (isSlotEmpty(ESceneSection_BrewingBottle1, 0) || isSlotEmpty(ESceneSection_BrewingBottle2, 0) ||
                         isSlotEmpty(ESceneSection_BrewingBottle3, 0))
                {
                    toolTip3 = eToolTipQuickMoveIngredient;
                }
                else
                {
                    toolTip3 = eToolTipNone;
                }
            }
            else if (isSlotEmpty(ESceneSection_BrewingIngredient, 0))
            {
                toolTip3 = eToolTipQuickMoveIngredient;
            }
            else if (notBlazePowder)
            {
                toolTip3 = eToolTipQuickMove;
            }
            else if (isSlotEmpty(ESceneSection_BrewingFuel, 0))
            {
                toolTip3 = eToolTipQuickMoveFuel;
            }
            else
            {
                not_null_ptr<ItemInstance> fuel = getSlotItem(ESceneSection_BrewingFuel, 0);
                toolTip3 = fuel->getCount() < fuel->getMaxStackSize() ? eToolTipQuickMoveFuel : eToolTipNone;
            }
        }
        else if (section == ESceneSection_EnchantHotbar || section == ESceneSection_EnchantInventory)
        {
            not_null_ptr<ItemInstance> item = getSlotItem(section, slotIndex);
            unsigned int id = (unsigned int)Item::getId(item->getItem());
            if (!item->getItem()->isEnchantable(item) || !isSlotEmpty(ESceneSection_EnchantItem, 0))
            {
                toolTip3 = eToolTipQuickMove;
            }
            else if (id == 0x105 || id == 0x10b || id == 0x10c || id == 0x110 || id == 0x114)
            {
                toolTip3 = eToolTipQuickMoveWeapon;
            }
            else if (id >= 0x12a && id < 0x13e)
            {
                toolTip3 = eToolTipQuickMoveArmor;
            }
            else if (id == 0x154)
            {
                toolTip3 = eToolTipQuickMove;
            }
            else
            {
                toolTip3 = eToolTipQuickMoveTool;
            }
        }
        else if (dynamic_cast<ShulkerBoxMenu*>(m_menu) == NULL)
        {
            toolTip3 = eToolTipQuickMove;
        }
        else
        {
            not_null_ptr<ItemInstance> item = getSlotItem(section, slotIndex);
            if (item->getItem() == NULL || dynamic_cast<ShulkerBoxBlockItem*>(item->getItem()) == NULL)
                toolTip3 = eToolTipQuickMove;
        }
    }

    if (pointerOutsideMenu)
    {
        m_pointerOutsideMenu = true;
        if (carriedIsItem)
        {
            if (!m_menu->m_uiQuickCraftActive)
            {
                if (carriedCount < 2)
                {
                    toolTip0 = eToolTipDrop;
                }
                else
                {
                    toolTip0 = eToolTipDropAll;
                    toolTip2 = eToolTipDropOne;
                }
            }
            else
            {
                toolTip0 = eToolTipDropAll;
            }
        }
    }
    else
    {
        m_pointerOutsideMenu = false;
    }

    not_null_ptr<ItemInstance> pointerItem = ItemInstance::EMPTY;
    if (onSlot && slotIsItem)
        pointerItem = getSlotItem(section, slotIndex);

    if (Minecraft::InMiniGame(eMiniGameId_Any, true) &&
        Minecraft::GetInstance()->GetMiniGame()->IsSmallInventoryEnabled())
    {
        if (toolTip3 != eToolTipEquip && player->mContainerMenu == player->mInventoryMenu)
            toolTip3 = eToolTipNone;

        if (pointerItem.get() != NULL && pointerItem->getItem() != NULL && pointerItem->getItem()->IsArmor() &&
            section != ESceneSection_ContainerHotbar)
        {
            int armorType = pointerItem->getItem()->GetArmorType();
            const EquipmentSlot* equipmentSlot = NULL;
            if (armorType == 1)
                equipmentSlot = EquipmentSlot::HEAD;
            else if (armorType == 2)
                equipmentSlot = EquipmentSlot::CHEST;
            else if (armorType == 3)
                equipmentSlot = EquipmentSlot::LEGS;
            else if (armorType == 4)
                equipmentSlot = EquipmentSlot::FEET;
            if (player->getItemSlot(equipmentSlot)->isEmpty())
                toolTip3 = eToolTipEquip;
            if (section == ESceneSection_InventoryArmor)
                toolTip3 = eToolTipQuickMoveArmor;
        }
    }

    if (canTakeAll())
        toolTip6 = eToolTipTakeEverything;

    overrideTooltips(section, pointerItem, carriedIsItem, slotIsItem, carriedSameAsSlot, slotSpace, toolTip0,
                     toolTip2, toolTip3, toolTip5, toolTip10, toolTip6, toolTip7);

    if (canTakeAll())
        toolTip5 = eToolTipNone;
    if (!canQuickCraft())
        toolTip4 = eToolTipNone;

    SetToolTip((EToolTipButton)0, toolTip0);
    SetToolTip((EToolTipButton)2, toolTip2);
    SetToolTip((EToolTipButton)3, toolTip3);
    SetToolTip((EToolTipButton)4, toolTip4);
    SetToolTip((EToolTipButton)5, toolTip5);
    SetToolTip((EToolTipButton)10, toolTip10);
    SetToolTip((EToolTipButton)6, toolTip6);
    SetToolTip((EToolTipButton)7, toolTip7);

    float pointerPosX = newPointerX - m_pointerImageOffset.x;
    float pointerPosY = newPointerY - m_pointerImageOffset.y;
    if (xDirection != 0.0f)
    {
        if (xDirection == 1.0f)
            pointerPosX += 0.999999f;
        else
            pointerPosX -= 0.999999f;
    }
    if (yDirection != 0.0f)
    {
        if (yDirection == 1.0f)
            pointerPosY += 0.999999f;
        else
            pointerPosY -= 0.999999f;
    }
    pointerPosX = floorf(pointerPosX);
    pointerPosX += (float)((int)pointerPosX % 2);
    pointerPosY = floorf(pointerPosY);
    pointerPosY += (float)((int)pointerPosY % 2);
    m_pointerPos.x = pointerPosX;
    m_pointerPos.y = pointerPosY;
    adjustPointerForSafeZone();

    if (m_menu->m_uiQuickCraftActive)
    {
        bool addSlot = false;
        if (slot != NULL && m_menu->m_uiQuickCraftSlots.size() < (unsigned int)carried->getCount() && canPlaceOne)
        {
            not_null_ptr<ItemInstance> dragged = minecraft->mLocalPlayers[getPad()]->mInventory->getCarried();
            if (AbstractContainerMenu::canItemQuickReplace(slot, dragged, true) && m_menu->canDragTo(slot))
                addSlot = true;
        }
        if (addSlot && m_menu->m_uiQuickCraftSlots.find(slot) == m_menu->m_uiQuickCraftSlots.end())
            m_menu->m_uiQuickCraftSlots.insert(slot);
        recalculateQuickCraftRemaining();
    }
}

int IUIScene_AbstractContainerMenu::getCurrentIndex(ESceneSection section)
{
    int columns;
    int rows;
    GetSectionDimensions(section, &columns, &rows);
    return m_currentSlotY * columns + m_currentSlotX + getSectionStartOffset(section);
}

void IUIScene_AbstractContainerMenu::slotClicked(int slotId, int buttonNum, ClickType::eClickType clickType)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    minecraft->mGameModes[getPad()]->handleInventoryMouseClick(m_menu->m_containerId, slotId, buttonNum,
                                                               clickType, minecraft->mLocalPlayers[getPad()]);
}

void IUIScene_AbstractContainerMenu::handleQuickCraft()
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[getPad()];

    not_null_ptr<ItemInstance> carried = ItemInstance::EMPTY;
    if (player.get() != NULL)
        carried = player->mInventory->getCarried();

    if (carried->isEmpty() || carried->getCount() <= 1)
        return;

    m_menu->m_uiQuickCraftActive = !m_menu->m_uiQuickCraftActive;
    if (!m_menu->m_uiQuickCraftActive)
    {
        if (!m_menu->m_uiQuickCraftSlots.empty())
        {
            slotClicked(-999, AbstractContainerMenu::getQuickcraftMask(0, 0), ClickType::eClickType_QuickCraft);
            if (m_menu->m_uiQuickCraftSlots.empty())
            {
                slotClicked(-999, AbstractContainerMenu::getQuickcraftMask(2, 0), ClickType::eClickType_QuickCraft);
            }
            else
            {
                AbstractContainerMenu::SlotSet::iterator it;
                for (it = m_menu->m_uiQuickCraftSlots.begin(); it != m_menu->m_uiQuickCraftSlots.end(); ++it)
                {
                    slotClicked((*it)->mIndex, AbstractContainerMenu::getQuickcraftMask(1, 0),
                                ClickType::eClickType_QuickCraft);
                }
                slotClicked(-999, AbstractContainerMenu::getQuickcraftMask(2, 0), ClickType::eClickType_QuickCraft);
            }
        }
        m_menu->m_uiQuickCraftSlots.clear();
    }
}

bool IUIScene_AbstractContainerMenu::handleKeyDown(int pad, int action, bool repeat)
{
    bool handled = false;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[getPad()] != NULL)
    {
        Tutorial* tutorial = minecraft->mGameModes[getPad()]->getTutorial();
        if (tutorial != NULL)
        {
            tutorial->handleUIInput(action);
            if (gConsoleUIController.IsTutorialVisible(getPad()) && !tutorial->popupPending &&
                !minecraft->mGameModes[getPad()]->getGameModeConstraints()->isInputAllowed(action))
            {
                return false;
            }
        }
    }

    int buttonNum = 0;
    int quickKeyHeld = 0;
    bool clicked = false;
    switch (action)
    {
    case EControllerActions_MenuA:
        if (!repeat && (!m_menu->m_uiQuickCraftActive || m_pointerOutsideMenu))
        {
            m_menu->m_uiQuickCraftActive = false;
            m_menu->m_uiQuickCraftSlots.clear();
            buttonNum = 0;
            quickKeyHeld = 0;
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
            clicked = true;
        }
        break;
    case EControllerActions_MenuB:
    case EControllerActions_MenuStart:
        gConsoleUIController.SetTooltips(pad, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                          -1, false, false);
        if (!m_navigateBack)
        {
            gConsoleUIController.CloseUIScenes(pad, false);
            return false;
        }
        gConsoleUIController.NavigateBack(pad, false, EUIScene_None, EUILayer_Count);
        return false;
    case EControllerActions_MenuX:
        if (!repeat && !m_menu->m_uiQuickCraftActive)
        {
            buttonNum = 1;
            quickKeyHeld = 0;
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
            clicked = true;
        }
        break;
    case EControllerActions_MenuY:
        if (!repeat && (!m_menu->m_uiQuickCraftActive || !m_pointerOutsideMenu))
        {
            buttonNum = 0;
            quickKeyHeld = 1;
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
            clicked = true;
        }
        break;
    case EControllerActions_MenuUp:
        m_tapState = ETapState_Up;
        handleAdditionalKeyPress(action);
        UpdateTooltips();
        return false;
    case EControllerActions_MenuDown:
        m_tapState = ETapState_Down;
        handleAdditionalKeyPress(action);
        UpdateTooltips();
        return false;
    case EControllerActions_MenuRight:
        m_tapState = ETapState_Right;
        handleAdditionalKeyPress(action);
        UpdateTooltips();
        return false;
    case EControllerActions_MenuLeft:
        m_tapState = ETapState_Left;
        handleAdditionalKeyPress(action);
        UpdateTooltips();
        return false;
    case EControllerActions_MenuPageUp:
        if (canQuickCraft())
            handleQuickCraft();
        handled = true;
        break;
    case EControllerActions_MenuPageDown:
        if (IsSectionSlotList(m_currentSection))
        {
            int currentIndex = getCurrentIndex(m_currentSection);
            int slotIndex = currentIndex - getSectionStartOffset(m_currentSection);
            if (!isSlotEmpty(m_currentSection, slotIndex) && Minecraft::GetInstance()->mGameModes[pad] != NULL)
            {
                Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[pad]->getTutorial();
                if (tutorial != NULL)
                {
                    not_null_ptr<ItemInstance> item = getSlotItem(m_currentSection, slotIndex)->copy();
                    Tutorial::PopupMessageDetails* details = new Tutorial::PopupMessageDetails();
                    details->titleId = item->getUseDescriptionId();
                    if (item->getItem() != NULL)
                        details->itemDescription = item->getHoverName();
                    details->itemDescriptionId = item->getDescriptionId(-1);
                    details->itemInstance = item;
                    details->itemId = Item::VALIDgetId(item->getItem());
                    details->itemAuxValue = item->getAuxValue();
                    details->force = true;
                    details->allowFade = true;
                    tutorial->setMessage(NULL, details);
                }
                gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
            }
        }
        handled = true;
        break;
    case EControllerActions_MenuLeftScroll:
        if (!repeat && canTakeAll())
        {
            handleTakeAll(0);
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
            handleAdditionalKeyPress(action);
            UpdateTooltips();
            return false;
        }
        break;
    default:
        break;
    }

    if (clicked)
    {
        if (!handleValidKeyPress(pad, buttonNum, quickKeyHeld))
        {
            if (IsSectionSlotList(m_currentSection))
            {
                handleSlotListClicked(m_currentSection, buttonNum, quickKeyHeld);
            }
            else if (m_pointerOutsideMenu)
            {
                handleOutsideClicked(pad, buttonNum, quickKeyHeld);
            }
            else
            {
                handleOtherClicked(pad, m_currentSection, buttonNum, quickKeyHeld != 0);
            }
        }
        UpdateTooltips();
        return true;
    }

    handleAdditionalKeyPress(action);
    UpdateTooltips();
    return handled;
}

unsigned int IUIScene_AbstractContainerMenu::handleValidKeyPress(int pad, int buttonNum, int quickKeyHeld)
{
    return 0;
}

void IUIScene_AbstractContainerMenu::handleOutsideClicked(int pad, int buttonNum, int quickKeyHeld)
{
    slotClicked(-999, buttonNum, (ClickType::eClickType)(quickKeyHeld != 0));
}

void IUIScene_AbstractContainerMenu::handleOtherClicked(int pad, ESceneSection section, int buttonNum,
                                                        bool quickKey)
{
}

void IUIScene_AbstractContainerMenu::handleAdditionalKeyPress(int action)
{
}

void IUIScene_AbstractContainerMenu::handleSlotListClicked(ESceneSection section, int buttonNum,
                                                           int quickKeyHeld)
{
    slotClicked(getCurrentIndex(section), buttonNum, (ClickType::eClickType)(quickKeyHeld != 0));
    handleSectionClick(section);
}

void IUIScene_AbstractContainerMenu::handleTakeAll(int buttonNum)
{
    int columns;
    int rows;
    GetSectionDimensions(ESceneSection_ContainerChest, &columns, &rows);
    int startIndex = getSectionStartOffset(ESceneSection_ContainerChest);

    Minecraft* minecraft = Minecraft::GetInstance();
    minecraft->mGameModes[getPad()]->handleTakeAllInvenetory(m_menu->m_containerId, buttonNum, startIndex,
                                                             rows * columns, minecraft->mLocalPlayers[getPad()]);
    handleSectionClick(ESceneSection_ContainerChest);
}

std::vector<HtmlString>* IUIScene_AbstractContainerMenu::GetSectionHoverText(ESceneSection section)
{
    return NULL;
}

int IUIScene_AbstractContainerMenu::GetInventoryRows()
{
    return m_smallInventory ? 0 : 3;
}

bool IUIScene_AbstractContainerMenu::IsSectionSlotList(ESceneSection section)
{
    return section != ESceneSection_None;
}

unsigned int IUIScene_AbstractContainerMenu::CanHaveFocus(ESceneSection section)
{
    return true;
}

unsigned int IUIScene_AbstractContainerMenu::IsVisible(ESceneSection section)
{
    return true;
}

unsigned int IUIScene_AbstractContainerMenu::overrideTooltips(ESceneSection section, not_null_ptr<ItemInstance> slotItem,
                                                      bool carriedIsItem, bool slotIsItem,
                                                      bool carriedSameAsSlot, int slotSpace,
                                                      EToolTipItem& toolTip0, EToolTipItem& toolTip2,
                                                      EToolTipItem& toolTip3, EToolTipItem& toolTip5,
                                                      EToolTipItem& toolTip10, EToolTipItem& toolTip6,
                                                      EToolTipItem& toolTip7)
{
    return false;
}
