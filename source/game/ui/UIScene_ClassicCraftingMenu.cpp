#include "game/ui/UIScene_ClassicCraftingMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/CraftingMenu.hpp"
#include "game/ui/FireworksMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/UILayer.hpp"

extern CMinecraftApp* g_pApp;

UIScene_ClassicCraftingMenu::UIScene_ClassicCraftingMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_resultSlot(), m_craftingSlots(), m_titleLabel()
{
    initialiseMovie();

    ClassicCraftingMenuInitData* sceneData = static_cast<ClassicCraftingMenuInitData*>(data);
    const bool fireworks = sceneData->fireworks != 0;
    const int player = sceneData->player;
    if (fireworks)
        m_titleLabel.init(UIString(L10N::GetString(0xb9c0e92)));
    else
        m_titleLabel.init(UIString(L10N::GetString(IDS_CONTROLS_CRAFTING)));

    Minecraft* minecraft = Minecraft::GetInstance();
    const BlockPos position(sceneData->x, sceneData->y, sceneData->z);
    AbstractContainerMenu* menu = fireworks
        ? static_cast<AbstractContainerMenu*>(new FireworksMenu(
              sceneData->localPlayer->mInventory, minecraft->mLevel, position))
        : static_cast<AbstractContainerMenu*>(new CraftingMenu(
              sceneData->localPlayer->mInventory, minecraft->mLevel, position));

    MultiPlayerGameMode* gameMode = minecraft->mGameModes[player];
    if (gameMode != 0)
    {
        Tutorial* tutorial = gameMode->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(
                fireworks ? eTutorial_State_FireworksMenu : eTutorial_State_SurvivalCrafting, this);
        }
    }

    Initialize(false, player, menu, true, 10,
               IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingMax, false);
    m_resultSlot.addSlots(0, 1);
    m_craftingSlots.addSlots(1, 9);

    delete sceneData;
    g_pApp->SetRichPresenceContext(type, 5);
}

UIScene_ClassicCraftingMenu::~UIScene_ClassicCraftingMenu()
{
}

unsigned int UIScene_ClassicCraftingMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    static const unsigned char columns[] = { 9, 9, 1, 3 };
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar ||
        section > IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid)
        return 0;
    return columns[static_cast<int>(section) - IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar];
}

unsigned int UIScene_ClassicCraftingMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar) return 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar) return 1;
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult) return GetInventoryRows();
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult) return 1;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid) return 3;
    return 0;
}

unsigned int UIScene_ClassicCraftingMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection result = section;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar &&
        state == IUIScene_AbstractContainerMenu::ETapState_Up)
        result = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory &&
             state == IUIScene_AbstractContainerMenu::ETapState_Down)
        result = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult &&
             state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        result = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory;
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult &&
             state == IUIScene_AbstractContainerMenu::ETapState_Up)
        result = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid &&
             state == IUIScene_AbstractContainerMenu::ETapState_Down)
        result = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid &&
             state == IUIScene_AbstractContainerMenu::ETapState_Up)
        result = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult &&
        (state == IUIScene_AbstractContainerMenu::ETapState_Down ||
         state == IUIScene_AbstractContainerMenu::ETapState_Up))
        updateSlotPosition(section, result, state, x, y, -6, 0);
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult &&
             state == IUIScene_AbstractContainerMenu::ETapState_Left)
        updateSlotPosition(section, IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid, state, x, y, 0, -1);
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult &&
             state == IUIScene_AbstractContainerMenu::ETapState_Right)
    {
        if (x != 0) *x = 0;
        updateSlotPosition(section, IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid, state, x, y, 0, -1);
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid &&
             (state == IUIScene_AbstractContainerMenu::ETapState_Left ||
              state == IUIScene_AbstractContainerMenu::ETapState_Right))
        updateSlotPosition(section, IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult, state, x, y, 0, 0);
    else
        updateSlotPosition(section, result, state, x, y, 0, 0);
    return static_cast<unsigned int>(result);
}

void UIScene_ClassicCraftingMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    UIControl* control = 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar) control = &m_hotbarList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory) control = &m_inventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult) control = &m_resultSlot;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid) control = &m_craftingSlots;
    if (control != 0 && position != 0) { position->x = static_cast<float>(control->getXPos()); position->y = static_cast<float>(control->getYPos()); }
}

void UIScene_ClassicCraftingMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, _UIVec2D* position, _UIVec2D* size)
{
    UIControl* control = 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar) control = &m_hotbarList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory) control = &m_inventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult) control = &m_resultSlot;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid) control = &m_craftingSlots;
    unsigned int columns = getSectionColumns(section), rows = getSectionRows(section);
    if (control == 0 || position == 0 || size == 0 || columns == 0 || rows == 0) return;
    size->x = static_cast<float>(control->getWidth()) / columns;
    size->y = static_cast<float>(control->getHeight()) / rows;
    position->x = static_cast<float>(slot % static_cast<int>(columns)) * size->x;
    position->y = static_cast<float>(slot / static_cast<int>(columns)) * size->y;
}

void UIScene_ClassicCraftingMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_ClassicCraftingMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    static const unsigned char offsets[] = { 37, 10, 0, 1 };
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar || section > IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid) return 0;
    return offsets[static_cast<int>(section) - IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar];
}

void UIScene_ClassicCraftingMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    if (!wrap) return;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar) m_hotbarList.setHighlightSlot(slot + direction * getSectionColumns(section));
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory) m_inventoryList.setHighlightSlot(slot + direction * getSectionColumns(section));
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult) m_resultSlot.setHighlightSlot(slot + direction * getSectionColumns(section));
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid) m_craftingSlots.setHighlightSlot(slot + direction * getSectionColumns(section));
}

std::wstring UIScene_ClassicCraftingMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"ClassicCraftingMenu") : std::wstring(L"ClassicCraftingMenuSplit");
}

unsigned int UIScene_ClassicCraftingMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_resultSlot.setupControl(this, panel, "Result");
    m_controls.push_back(&m_resultSlot);
    m_craftingSlots.setupControl(this, panel, "CraftingList");
    m_controls.push_back(&m_craftingSlots);
    m_titleLabel.setupControl(this, panel, "TitleLabel");
    m_controls.push_back(&m_titleLabel);
    return 1;
}

void UIScene_ClassicCraftingMenu::handleReload()
{
    Initialize(true, m_playerId, m_menu, true, 10,
               IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingMax, false);
    m_resultSlot.addSlots(0, 1);
    m_craftingSlots.addSlots(1, 9);
}

UIControl* UIScene_ClassicCraftingMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar) return &m_hotbarList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory) return &m_inventoryList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult) return &m_resultSlot;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid) return &m_craftingSlots;
    return 0;
}

int UIScene_ClassicCraftingMenu::getSceneType()
{
    return EUIScene_ClassicCraftingMenu;
}
