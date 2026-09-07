#include "game/ui/UIScene_HorseInventoryMenu.hpp"

#include "game/entity/mob/AbstractChestedHorse.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/FJ_HorseInventoryMenu.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern CMinecraftApp* g_pApp;

UIScene_HorseInventoryMenu::UIScene_HorseInventoryMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_sceneInterface(0), m_container(),
      m_playerInventory(), m_horse(), m_saddleSlot(), m_armorSlot(), m_inventoryList(),
      m_horseLabel(), m_horseControl()
{
    (void)data;
    m_sceneInterface = static_cast<IUIScene_AbstractContainerMenu*>(
        static_cast<UIScene_AbstractContainerMenu*>(this));
    initialiseMovie();
    setUp(false);
}

UIScene_HorseInventoryMenu::~UIScene_HorseInventoryMenu()
{
}

bool UIScene_HorseInventoryMenu::IsSectionSlotList(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section >= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x48) &&
        section <= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x49))
        return true;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4a))
        return m_horse && boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->hasChest();
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4b))
        return m_horse && m_horse->wearsArmor();
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4c))
        return m_horse && m_horse->isSaddled();
    return false;
}

unsigned int UIScene_HorseInventoryMenu::IsVisible(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x48) ||
        section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x49))
        return 1;
    return IsSectionSlotList(section) ? 1U : 0U;
}

unsigned int UIScene_HorseInventoryMenu::getSectionColumns(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x48) ||
        section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x49))
        return 9;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4a))
        return m_horse ? static_cast<unsigned int>(boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->getInventoryColumns()) : 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4b) ||
        section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4c))
        return 1;
    return 0;
}

unsigned int UIScene_HorseInventoryMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x48))
        return 1;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x49))
        return m_sceneInterface->GetInventoryRows();
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4a))
        return 3;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4b) ||
        section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4c))
        return 1;
    return 0;
}

unsigned int UIScene_HorseInventoryMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    const IUIScene_AbstractContainerMenu::ESceneSection hotbar =
        static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x48);
    const IUIScene_AbstractContainerMenu::ESceneSection inventory =
        static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x49);
    const IUIScene_AbstractContainerMenu::ESceneSection chest =
        static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4a);
    const IUIScene_AbstractContainerMenu::ESceneSection armor =
        static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4b);
    const IUIScene_AbstractContainerMenu::ESceneSection saddle =
        static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4c);
    if (section == hotbar && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        m_sceneInterface->updateSlotPosition(section, chest, state, x, y, 3, 0);
        return chest;
    }
    if (section == hotbar && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        const IUIScene_AbstractContainerMenu::ESceneSection target =
            m_sceneInterface->m_inventorySmall ? chest : inventory;
        m_sceneInterface->updateSlotPosition(section, target, state, x, y, 0, 0);
        return target;
    }
    if (section == inventory && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        m_sceneInterface->updateSlotPosition(section, hotbar, state, x, y, 0, 0);
        return hotbar;
    }
    if (section == inventory && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        m_sceneInterface->updateSlotPosition(section, chest, state, x, y, 3, 0);
        return chest;
    }
    if (section == chest && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        m_sceneInterface->updateSlotPosition(section, hotbar, state, x, y, -3, 0);
        return hotbar;
    }
    if (section == chest && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        const IUIScene_AbstractContainerMenu::ESceneSection target =
            m_sceneInterface->m_inventorySmall ? hotbar : inventory;
        m_sceneInterface->updateSlotPosition(section, target, state, x, y, -3, 0);
        return target;
    }
    if (section == armor && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        m_sceneInterface->updateSlotPosition(section, chest, state, x, y, -1, 0);
        return chest;
    }
    if (section == saddle && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        m_sceneInterface->updateSlotPosition(section, chest, state, x, y, -1, 0);
        return chest;
    }
    m_sceneInterface->updateSlotPosition(section, section, state, x, y, 0, 0);
    return section;
}

void UIScene_HorseInventoryMenu::GetPositionOfSection(
    IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    if (!position)
        return;
    UIControl* control = 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x48)) control = &m_hotbarList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x49)) control = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4a)) control = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4b)) control = &m_armorSlot;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4c)) control = &m_saddleSlot;
    if (control)
    {
        position->x = static_cast<float>(control->getXPos());
        position->y = static_cast<float>(control->getYPos());
    }
}

void UIScene_HorseInventoryMenu::GetItemScreenData(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
    _UIVec2D* position, _UIVec2D* size)
{
    UIControl* control = 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x48)) control = &m_hotbarList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x49)) control = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4a)) control = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4b)) control = &m_armorSlot;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4c)) control = &m_saddleSlot;
    const unsigned int columns = getSectionColumns(section);
    const unsigned int rows = getSectionRows(section);
    if (control && position && size && columns && rows)
    {
        position->x = static_cast<float>(slot % static_cast<int>(columns));
        position->y = static_cast<float>(slot / static_cast<int>(columns));
        size->x = static_cast<float>(control->getWidth() / columns);
        size->y = static_cast<float>(control->getHeight() / rows);
    }
}

void UIScene_HorseInventoryMenu::handleSectionClick(
    IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_HorseInventoryMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x48))
        return m_horse ? m_horse->getInventorySize() + 27 : 27;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x49))
        return m_horse ? m_horse->getInventorySize() : 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4a))
        return 2;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4b))
        return 1;
    return 0;
}

void UIScene_HorseInventoryMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    if (!wrap)
        return;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x48)) m_hotbarList.setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x49)) m_inventoryList.setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4a)) m_inventoryList.setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4b)) m_armorSlot.setHighlightSlot(slot + direction);
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4c)) m_saddleSlot.setHighlightSlot(slot + direction);
}

void UIScene_HorseInventoryMenu::SetInventoryType(int type)
{
    fuiRenderNode* root = m_movie->getRootNode();
    FJ_HorseInventoryMenu* document = static_cast<FJ_HorseInventoryMenu*>(root->getFuiNode());
    if (document)
        document->SetInventoryType(type);
}

void UIScene_HorseInventoryMenu::SetHasInventory(bool hasInventory)
{
    fuiRenderNode* root = m_movie->getRootNode();
    FJ_HorseInventoryMenu* document = static_cast<FJ_HorseInventoryMenu*>(root->getFuiNode());
    if (document)
        document->SetHasInventory(hasInventory);
}

void UIScene_HorseInventoryMenu::SetHasSaddle(bool hasSaddle)
{
    fuiRenderNode* root = m_movie->getRootNode();
    FJ_HorseInventoryMenu* document = static_cast<FJ_HorseInventoryMenu*>(root->getFuiNode());
    if (document)
        document->SetCanHaveSaddle(hasSaddle);
}

void UIScene_HorseInventoryMenu::setUp(bool enabled)
{
    const bool hasChest = m_horse && boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->hasChest();
    const int columns = hasChest ? boost::static_pointer_cast<AbstractChestedHorse>(m_horse)->getInventoryColumns() : 2;
    m_sceneInterface->Initialize(enabled, m_sceneInterface->m_containerId,
                                 m_sceneInterface->m_menu, true, columns * 3 + 2,
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x48),
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4d), false);
    if (!hasChest)
        SetInventoryType(1);
    else
    {
        SetInventoryType(0);
        m_armorSlot.addSlots(1, 1);
        m_inventoryList.addSlots(2, columns * 3);
        SetHasInventory(true);
    }
    SetHasSaddle(m_horse && m_horse->isSaddled());
    m_saddleSlot.addSlots(0, 1);
}

unsigned int UIScene_HorseInventoryMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_saddleSlot.setupControl(this, panel, std::string("SlotSaddle"));
    m_controls.push_back(&m_saddleSlot);
    m_armorSlot.setupControl(this, panel, std::string("SlotArmor"));
    m_controls.push_back(&m_armorSlot);
    m_inventoryList.setupControl(this, panel, std::string("DonkeyInventoryList"));
    m_controls.push_back(&m_inventoryList);
    m_horseLabel.setupControl(this, panel, std::string("horseinventoryText"));
    m_controls.push_back(&m_horseLabel);
    m_horseControl.setupControl(this, panel, std::string("iggy_horse"));
    m_controls.push_back(&m_horseControl);
    return 1;
}

std::wstring UIScene_HorseInventoryMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"HorseInventoryMenu") :
        std::wstring(L"HorseInventoryMenuSplit");
}

void UIScene_HorseInventoryMenu::handleReload()
{
    setUp(true);
}

int UIScene_HorseInventoryMenu::getSection(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x48)) return 0x128;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x49)) return 0x188;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4a)) return 0x430;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4b)) return 0x3d0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4c)) return 0x370;
    return 0;
}

void UIScene_HorseInventoryMenu::customDraw(const char* name, fuiRect* rect)
{
    if (Minecraft::GetInstance() && m_horseControl.getVisible() &&
        std::string(name) == std::string("iggy_horse"))
        m_horseControl.render(rect);
    else
        UIScene_AbstractContainerMenu::customDraw(name, rect);
}

int UIScene_HorseInventoryMenu::getSceneType()
{
    return 55;
}
