#include "game/ui/UIScene_EnchantingMenu.hpp"

#include <cstdio>
#include <cstring>

#include <boost/shared_ptr.hpp>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
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
        m_enchantLabel.init(UIString(L10N::GetString(0xf4bd500f)));
    if (Minecraft::GetInstance()->mGameModes[sceneData->playerIndex] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[sceneData->playerIndex]->getTutorial();
        if (tutorial != 0)
        {
            sceneInterface->m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(static_cast<eTutorial_State>(0x14), this);
        }
    }
    EnchantmentMenu* menu = new EnchantmentMenu(sceneData->inventory, sceneData->level,
                                                  BlockPos(sceneData->x, sceneData->y, sceneData->z));
    static_cast<IUIScene_AbstractContainerMenu*>(this)->Initialize(false, sceneData->playerIndex, menu, true, 2,
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x21),
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x28), false);
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
    return section >= 0x23 && section <= 0x24;
}

unsigned int UIScene_EnchantingMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < 0x21 || section > 0x24) return 0;
    static const unsigned int columns[4] = { 1, 1, 1, 1 };
    return columns[static_cast<int>(section) - 0x21];
}

unsigned int UIScene_EnchantingMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < 0x21 || section > 0x24) return 0;
    if (section == 0x22) return static_cast<IUIScene_AbstractContainerMenu*>(this)->GetInventoryRows();
    return 1;
}

unsigned int UIScene_EnchantingMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection target = section;
    int dx = 0;
    switch (static_cast<int>(section))
    {
    case 0x21:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            if (*x > 2) target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x25);
            else if (*x > 1) target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x24);
            else target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x23);
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (m_inventorySmall == 0)
                target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x22);
            else if (*x > 2)
                target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x27);
            else if (*x > 1)
                target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x24);
            else
                target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x23);
        }
        break;
    case 0x22:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x21);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (*x > 2) target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x27);
            else if (*x > 1) target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x24);
            else target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x23);
        }
        break;
    case 0x23:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = m_inventorySmall != 0 ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x21) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x22);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x21);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x25);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x24);
        break;
    case 0x24:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_inventorySmall != 0 ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x21) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x22);
            dx = -2;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x21);
            dx = -2;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x23);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x25);
        break;
    case 0x25:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x26);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x21);
            dx = -7;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x24);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x23);
        break;
    case 0x26:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x27);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x25);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x24);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x23);
        break;
    case 0x27:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_inventorySmall != 0 ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x21) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x22);
            dx = -7;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x26);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x24);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x23);
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
    UIControl* control = controlForSection(section);
    if (control != 0 && position != 0)
    {
        position->x = static_cast<float>(control->getXPos());
        position->y = static_cast<float>(control->getYPos());
    }
}

void UIScene_EnchantingMenu::GetItemScreenData(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
    _UIVec2D* position, _UIVec2D* size)
{
    UIControl* control = controlForSection(section);
    if (control == 0 || position == 0 || size == 0) return;
    if (!IsSectionSlotList(section))
    {
        GetPositionOfSection(section, position);
        size->x = static_cast<float>(control->getWidth());
        size->y = static_cast<float>(control->getHeight());
        return;
    }
    const unsigned int columns = getSectionColumns(section);
    const unsigned int rows = getSectionRows(section);
    if (columns == 0 || rows == 0) return;
    const float width = static_cast<float>(control->getWidth()) / columns;
    const float height = static_cast<float>(control->getHeight()) / rows;
    position->x = static_cast<float>(slot % static_cast<int>(columns)) * width;
    position->y = static_cast<float>(slot / static_cast<int>(columns)) * height;
    size->x = width;
    size->y = height;
}

void UIScene_EnchantingMenu::handleOtherClicked(int button,
    IUIScene_AbstractContainerMenu::ESceneSection section, int, bool)
{
    if (section < 0x25 || section > 0x27) return;
    const int enchantment = getSectionStartOffset(section);
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft == 0 || minecraft->mGameModes[m_playerId] == 0) return;
    minecraft->mGameModes[m_playerId]->handleInventoryButtonClick(m_menu->m_containerId, enchantment + button);
}

int UIScene_EnchantingMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < 0x21 || section > 0x24) return 0;
    static const int offsets[4] = { 0, 1, 0, 1 };
    return offsets[static_cast<int>(section) - 0x21];
}

void UIScene_EnchantingMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    if (!wrap) return;
    UIControl* control = controlForSection(section);
    UIControl_SlotList* list = dynamic_cast<UIControl_SlotList*>(control);
    if (list != 0) list->setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
}

std::vector<HtmlString>* UIScene_EnchantingMenu::GetSectionHoverText(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < 0x25 || section > 0x27 || getMenu() == 0) return 0;
    const int index = static_cast<int>(section) - 0x25;
    const int cost = getMenu()->mCosts[index];
    Enchantment* enchantment = Enchantment::byId(getMenu()->mEnchantmentIds[index]);
    if (cost < 1 || enchantment == 0) return 0;
    std::vector<HtmlString>* result = new std::vector<HtmlString>();
    result->push_back(enchantment->getFullname(cost));
    if (!IsActiveButton(section)) return result;
    std::wstring text = L10N::GetString(index == 0 ? 0x8d500014 : 0xf5706ab);
    std::wstring token = L"{*COST*}";
    std::wstring value;
    wchar_t number[16];
    std::swprintf(number, 16, L"%d", index + 1);
    value = number;
    std::wstring::size_type at = text.find(token);
    if (at != std::wstring::npos) text.replace(at, token.size(), value);
    result->push_back(HtmlString(text, eMinecraftColour_Color_EnchantText, false, false, 0));
    return result;
}

bool UIScene_EnchantingMenu::IsActiveButton(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < 0x25 || section > 0x27) return false;
    return !m_enchantmentButtons[static_cast<int>(section) - 0x25].IsInactive();
}

unsigned int UIScene_EnchantingMenu::overrideTooltips(
    IUIScene_AbstractContainerMenu::ESceneSection section, not_null_ptr<ItemInstance>, bool, bool,
    bool, int, EToolTipItem& first, EToolTipItem&, EToolTipItem&, EToolTipItem&, EToolTipItem&,
    EToolTipItem&, EToolTipItem&)
{
    if (IsActiveButton(section))
    {
        first = static_cast<EToolTipItem>(25);
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
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x21),
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x28), false);
    m_enchantmentButtons[0].SetEnchantmentLevel(1);
    m_enchantmentButtons[1].SetEnchantmentLevel(2);
    m_enchantmentButtons[2].SetEnchantmentLevel(3);
    m_ingredientSlot.addSlots(0, 1);
    m_lapisSlot.addSlots(1, 1);
}

UIControl* UIScene_EnchantingMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    return controlForSection(section);
}

EnchantmentMenu* UIScene_EnchantingMenu::getMenu()
{
    return static_cast<EnchantmentMenu*>(m_menu);
}

UIControl* UIScene_EnchantingMenu::controlForSection(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == 0x21) return &m_hotbarList;
    if (section == 0x22) return &m_inventoryList;
    if (section == 0x23) return &m_ingredientSlot;
    if (section == 0x24) return &m_lapisSlot;
    if (section == 0x25) return &m_enchantmentButtons[0];
    if (section == 0x26) return &m_enchantmentButtons[1];
    if (section == 0x27) return &m_enchantmentButtons[2];
    return 0;
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
    return 46;
}
