#include "game/ui/UIScene_EnchantingMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cstdio>
#include <cstring>

#include <boost/shared_ptr.hpp>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StringUtils.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/Inventory.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/EnchantmentMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/ui/UILayer.hpp"

extern std::wstring _toString(int value);
extern CMinecraftApp* g_pApp;

struct UIScene_EnchantingMenuData
{
    boost::shared_ptr<Inventory> inventory;
    Level* level;
    int x;
    int y;
    int z;
    int playerIndex;
    int alignment;
    std::wstring name;
};

UIScene_EnchantingMenu::UIScene_EnchantingMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_ingredientSlot(), m_lapisSlot(),
      m_enchantLabel(), m_enchantmentButtons(), m_enchantmentBook()
{
    UIScene_EnchantingMenuData* sceneData = static_cast<UIScene_EnchantingMenuData*>(data);
    IUIScene_AbstractContainerMenu* sceneInterface =
        static_cast<IUIScene_AbstractContainerMenu*>(this);
    initialiseMovie();
    m_enchantmentButtons[0].init(0);
    m_enchantmentButtons[1].init(1);
    m_enchantmentButtons[2].init(2);
    m_enchantmentButtons[0].SetEnchantmentLevel(1);
    m_enchantmentButtons[1].SetEnchantmentLevel(2);
    m_enchantmentButtons[2].SetEnchantmentLevel(3);
    if (!sceneData->name.empty())
        m_enchantLabel.init(UIString(sceneData->name));
    else
        m_enchantLabel.init(UIString(L10N::GetString(IDS_ENCHANT)));
    if (Minecraft::GetInstance()->mGameModes[sceneData->playerIndex] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[sceneData->playerIndex]->getTutorial();
        if (tutorial != 0)
        {
            sceneInterface->m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_EnchantingMenu, this);
        }
    }
    EnchantmentMenu* menu = new EnchantmentMenu(sceneData->inventory, sceneData->level,
                                                  BlockPos(sceneData->x, sceneData->y, sceneData->z));
    static_cast<IUIScene_AbstractContainerMenu*>(this)->Initialize(false, sceneData->playerIndex, menu, true, 2,
                                 IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar,
                                 IUIScene_AbstractContainerMenu::ESceneSection_EnchantMax, false);
    m_ingredientSlot.addSlots(0, 1);
    m_lapisSlot.addSlots(1, 1);
    g_pApp->SetRichPresenceContext(m_playerId, 10);
    delete sceneData;
}

UIScene_EnchantingMenu::~UIScene_EnchantingMenu()
{
}

bool UIScene_EnchantingMenu::IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    return section >= IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem && section <= IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis;
}

unsigned int UIScene_EnchantingMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar || section > IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis) return 0;
    static const unsigned int columns[4] = { 1, 1, 1, 1 };
    return columns[static_cast<int>(section) - IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar];
}

unsigned int UIScene_EnchantingMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar || section > IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis) return 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory) return static_cast<IUIScene_AbstractContainerMenu*>(this)->GetInventoryRows();
    return 1;
}

unsigned int UIScene_EnchantingMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection target = section;
    int dx = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            if (*x > 2) target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1;
            else if (*x > 1) target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis;
            else target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (m_smallInventory == 0)
                target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory;
            else if (*x > 2)
                target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3;
            else if (*x > 1)
                target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis;
            else
                target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (*x > 2) target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3;
            else if (*x > 1) target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis;
            else target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = m_smallInventory != 0 ?
                IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar :
                IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_smallInventory != 0 ?
                IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar :
                IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory;
            dx = -2;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar;
            dx = -2;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton2;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar;
            dx = -7;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton2:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_smallInventory != 0 ?
                IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar :
                IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory;
            dx = -7;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton2;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem;
        break;
    default:
        break;
    }
    static_cast<IUIScene_AbstractContainerMenu*>(this)->updateSlotPosition(section, target, state, x, y, dx, 0);
    return target;
}

void UIScene_EnchantingMenu::GetPositionOfSection(
    IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    UIControl* control = NULL;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar:
        control = &m_hotbarList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory:
        control = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem:
        control = &m_ingredientSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis:
        control = &m_lapisSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1:
        control = &m_enchantmentButtons[0];
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton2:
        control = &m_enchantmentButtons[1];
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3:
        control = &m_enchantmentButtons[2];
        break;
    default:
        break;
    }
    if (control == NULL)
        return;
    position->x = static_cast<float>(control->getXPos());
    position->y = static_cast<float>(control->getYPos());
}

void UIScene_EnchantingMenu::GetItemScreenData(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
    _UIVec2D* position, _UIVec2D* size)
{
    UIControl* control = NULL;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar:
        control = &m_hotbarList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory:
        control = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem:
        control = &m_ingredientSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis:
        control = &m_lapisSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1:
        control = &m_enchantmentButtons[0];
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton2:
        control = &m_enchantmentButtons[1];
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3:
        control = &m_enchantmentButtons[2];
        break;
    default:
        break;
    }
    if (control == NULL)
        return;
    const float width = static_cast<float>(control->getWidth());
    const float height = static_cast<float>(control->getHeight());
    if (!IsSectionSlotList(section))
    {
        GetPositionOfSection(section, position);
        size->x = width;
        size->y = height;
        return;
    }
    const int rows = static_cast<int>(getSectionRows(section));
    const int columns = static_cast<int>(getSectionColumns(section));
    size->x = width / static_cast<float>(columns);
    size->y = height / static_cast<float>(rows);
    position->x = static_cast<float>(slot % columns) * size->x;
    position->y = static_cast<float>(slot / columns) * size->y;
}

void UIScene_EnchantingMenu::handleOtherClicked(int button,
    IUIScene_AbstractContainerMenu::ESceneSection section, int, bool)
{
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1 || section > IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3) return;
    const int enchantment = getSectionStartOffset(section);
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft == 0 || minecraft->mGameModes[m_playerId] == 0) return;
    minecraft->mGameModes[m_playerId]->handleInventoryButtonClick(m_menu->m_containerId, enchantment + button);
}

int UIScene_EnchantingMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar || section > IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis) return 0;
    static const int offsets[4] = { 0, 1, 0, 1 };
    return offsets[static_cast<int>(section) - IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar];
}

void UIScene_EnchantingMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    const int columns = static_cast<int>(getSectionColumns(section));
    UIControl_SlotList* list = NULL;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar:
        list = &m_hotbarList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory:
        list = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem:
        list = &m_ingredientSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis:
        list = &m_lapisSlot;
        break;
    default:
        break;
    }
    if (wrap)
        list->setHighlightSlot(direction * columns + slot);
}

std::vector<HtmlString>* UIScene_EnchantingMenu::GetSectionHoverText(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1 ||
        section > IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3)
        return NULL;

    const int index = static_cast<int>(section) - IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1;
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[getPad()];
    const bool instabuild = player->mAbilities.mInstabuild;
    const int lapisCount = getMenu()->getGoldCount();
    const int cost = getMenu()->mCosts[index];
    Enchantment* enchantment = Enchantment::byId(getMenu()->mEnchantmentIds[index]);
    const int level = getMenu()->mEnchantmentLevels[index];
    const int lapisCost = index + 1;
    if (cost < 1 || level < 0 || enchantment == NULL)
        return NULL;

    std::vector<HtmlString>* result = new std::vector<HtmlString>();
    result->push_back(HtmlString(enchantment->getFullname(level).mText + L"...?", eMinecraftColour_HTMLColor_White, true,
                                 false, 0));
    if (instabuild)
        return result;

    result->push_back(HtmlString(L"", static_cast<eMinecraftColour>(0), false, false, 0));
    if (player->mExperienceLevel < cost)
    {
        std::wstring requirement =
            replaceAll(L10N::GetString(IDS_ENCHANT_LEVEL_REQUIREMENT), L"{*LEVEL*}", _toString(cost));
        result->push_back(HtmlString(requirement, eMinecraftColour_HTMLColor_c, false, false, 0));
        return result;
    }

    std::wstring lapisText;
    if (lapisCost == 1)
        lapisText = L10N::GetString(0x8d500014);
    else
        lapisText = replaceAll(L10N::GetString(0xf5706ab), L"{*COST*}", _toString(lapisCost));
    result->push_back(HtmlString(lapisText,
                                 lapisCount < lapisCost ? eMinecraftColour_HTMLColor_c : eMinecraftColour_HTMLColor_7,
                                 false, false, 0));

    std::wstring levelText;
    if (lapisCost == 1)
        levelText = L10N::GetString(0x3acd0b7d);
    else
        levelText = replaceAll(L10N::GetString(0x2d47653a), L"{*COST*}", _toString(lapisCost));
    result->push_back(HtmlString(levelText, eMinecraftColour_HTMLColor_7, false, false, 0));
    return result;
}

bool UIScene_EnchantingMenu::IsActiveButton(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1 || section > IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3) return false;
    return !m_enchantmentButtons[static_cast<int>(section) - IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1].IsInactive();
}

unsigned int UIScene_EnchantingMenu::overrideTooltips(
    IUIScene_AbstractContainerMenu::ESceneSection section, not_null_ptr<ItemInstance>, bool, bool,
    bool, int, EToolTipItem& first, EToolTipItem&, EToolTipItem&, EToolTipItem&, EToolTipItem&,
    EToolTipItem&, EToolTipItem&)
{
    if (IsActiveButton(section))
    {
        first = eToolTipSelect;
        return 1;
    }
    return 0;
}

unsigned int UIScene_EnchantingMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_ingredientSlot.setupControl(this, panel, "ingredient");
    m_lapisSlot.setupControl(this, panel, "lapis");
    m_enchantmentButtons[0].setupControl(this, panel, "Button1");
    m_enchantmentButtons[1].setupControl(this, panel, "Button2");
    m_enchantmentButtons[2].setupControl(this, panel, "Button3");
    m_enchantLabel.setupControl(this, panel, "enchantLabel");
    m_enchantmentBook.setupControl(this, panel, "iggy_EnchantmentBook");
    m_controls.push_back(&m_ingredientSlot);
    m_controls.push_back(&m_lapisSlot);
    m_controls.push_back(&m_enchantmentButtons[0]);
    m_controls.push_back(&m_enchantmentButtons[1]);
    m_controls.push_back(&m_enchantmentButtons[2]);
    m_controls.push_back(&m_enchantLabel);
    m_controls.push_back(&m_enchantmentBook);
    return 1;
}

std::wstring UIScene_EnchantingMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"EnchantingMenu") :
        std::wstring(L"EnchantingMenuSplit");
}

void UIScene_EnchantingMenu::handleReload()
{
    static_cast<IUIScene_AbstractContainerMenu*>(this)->Initialize(true, m_playerId, m_menu, true, 2,
                                 IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar,
                                 IUIScene_AbstractContainerMenu::ESceneSection_EnchantMax, false);
    m_enchantmentButtons[0].SetEnchantmentLevel(1);
    m_enchantmentButtons[1].SetEnchantmentLevel(2);
    m_enchantmentButtons[2].SetEnchantmentLevel(3);
    m_ingredientSlot.addSlots(0, 1);
    m_lapisSlot.addSlots(1, 1);
}

UIControl* UIScene_EnchantingMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    UIControl* control = NULL;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar:
        control = &m_hotbarList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory:
        control = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem:
        control = &m_ingredientSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis:
        control = &m_lapisSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1:
        control = &m_enchantmentButtons[0];
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton2:
        control = &m_enchantmentButtons[1];
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3:
        control = &m_enchantmentButtons[2];
        break;
    default:
        break;
    }
    return control;
}

EnchantmentMenu* UIScene_EnchantingMenu::getMenu()
{
    return static_cast<EnchantmentMenu*>(m_menu);
}

void UIScene_EnchantingMenu::customDraw(const char* name, fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mLocalPlayers[m_playerId] == 0 || minecraft->mGameModes[m_playerId] == 0)
        return;
    if (std::strcmp(name, "EnchantmentBook") == 0)
    {
        m_enchantmentBook.render(rect);
        return;
    }
    int button = 0;
    std::sscanf(name, "slot_Button%d", &button);
    UIScene_AbstractContainerMenu::customDraw(name, rect);
}

void UIScene_EnchantingMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_EnchantingMenu::getSceneType()
{
    return EUIScene_EnchantingMenu;
}
