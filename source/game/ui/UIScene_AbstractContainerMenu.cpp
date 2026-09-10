#include "game/ui/UIScene_AbstractContainerMenu.hpp"

#include <cstdio>

#include "game/core/ConsoleUIController.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/UILayer.hpp"
#include "game/item/ItemInstance.hpp"

extern ConsoleUIController gConsoleUIController;

UIScene_AbstractContainerMenu::UIScene_AbstractContainerMenu(int type, UILayer* layer)
    : UIScene(type, layer)
{
    m_controlFocus = -1;
    m_updatePending = false;
    gConsoleUIController.OverrideSFX(m_playerId, 0, true);
    gConsoleUIController.OverrideSFX(m_playerId, 20, true);
    gConsoleUIController.OverrideSFX(m_playerId, 2, true);
    gConsoleUIController.OverrideSFX(m_playerId, 3, true);
    gConsoleUIController.OverrideSFX(m_playerId, 11, true);
    gConsoleUIController.OverrideSFX(m_playerId, 10, true);
    gConsoleUIController.OverrideSFX(m_playerId, 7, true);
    gConsoleUIController.OverrideSFX(m_playerId, 6, true);
    gConsoleUIController.OverrideSFX(m_playerId, 4, true);
    gConsoleUIController.OverrideSFX(m_playerId, 5, true);
}

UIScene_AbstractContainerMenu::~UIScene_AbstractContainerMenu()
{
}

void UIScene_AbstractContainerMenu::PlatformInitialize(bool enabled, int, int slotIndex)
{
    m_inventoryLabel.init(UIString(L10N::GetString(0x14630b9e)));
    if (m_inventorySmall)
    {
        m_hotbarList.UpdateControl();
        m_backgroundPanel.UpdateControl();
    }
    if (slotIndex >= 0)
    {
        if (!m_inventorySmall)
            m_inventoryList.addSlots(static_cast<unsigned int>(slotIndex), 27);
        m_hotbarList.addSlots(static_cast<unsigned int>(slotIndex + 27), 9);
    }
}

void UIScene_AbstractContainerMenu::InitDataAssociations(int, AbstractContainerMenu*, int)
{
}

void UIScene_AbstractContainerMenu::setSectionFocus(IUIScene_AbstractContainerMenu::ESceneSection section, int)
{
    UIControl* selectedSection = getSection(section);
    if (selectedSection != 0)
        selectedSection->setFocus(true);

    if (m_controlFocus != -1)
    {
        UIControl* previousSection = getSection(
            static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(m_controlFocus));
        if (previousSection != selectedSection && previousSection != 0)
            previousSection->setFocus(false);
    }
    m_controlFocus = static_cast<int>(section);
}

void UIScene_AbstractContainerMenu::setFocusToPointer(int)
{
    if (m_controlFocus != -1)
    {
        UIControl* previousSection = getSection(
            static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(m_controlFocus));
        if (previousSection != 0)
            previousSection->setFocus(false);
    }
    m_controlFocus = -1;
}

void UIScene_AbstractContainerMenu::SetPointerText(std::vector<HtmlString>* text, bool visible)
{
    m_cursor.setLabel(UIString(HtmlString::Compose(text)), false, visible);
}

not_null_ptr<ItemInstance> UIScene_AbstractContainerMenu::getSlotItem(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot)
{
    Slot* target = getSlot(section, slot);
    if (target != 0)
        return target->getItem();
    return ItemInstance::EMPTY;
}

Slot* UIScene_AbstractContainerMenu::getSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot)
{
    return m_menu == 0 ? 0 : m_menu->getSlot(getSectionStartOffset(section) + slot);
}

bool UIScene_AbstractContainerMenu::isSlotEmpty(IUIScene_AbstractContainerMenu::ESceneSection section, int slot)
{
    Slot* target = getSlot(section, slot);
    return target != 0 && !target->hasItem();
}

void UIScene_AbstractContainerMenu::adjustPointerForSafeZone()
{
}

unsigned int UIScene_AbstractContainerMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie == 0 ? 0 : m_movie->getRootNode();
    m_mainPanel.setupControl(this, root, "MainPanel");
    m_controls.push_back(&m_mainPanel);
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_backgroundPanel.setupControl(this, panel, "BackgroundPanel");
    m_controls.push_back(&m_backgroundPanel);
    m_hotbarList.setupControl(this, panel, "hotbarList");
    m_controls.push_back(&m_hotbarList);
    m_inventoryList.setupControl(this, panel, "inventoryList");
    m_controls.push_back(&m_inventoryList);
    m_cursor.setupControl(this, panel, "cursor");
    m_controls.push_back(&m_cursor);
    m_inventoryLabel.setupControl(this, panel, "inventoryLabel");
    m_controls.push_back(&m_inventoryLabel);
    return 1;
}

void UIScene_AbstractContainerMenu::handleDestroy()
{
    gConsoleUIController.OverrideSFX(m_playerId, 0, false);
    gConsoleUIController.OverrideSFX(m_playerId, 20, false);
    gConsoleUIController.OverrideSFX(m_playerId, 2, false);
    gConsoleUIController.OverrideSFX(m_playerId, 3, false);
    gConsoleUIController.OverrideSFX(m_playerId, 11, false);
    gConsoleUIController.OverrideSFX(m_playerId, 10, false);
    gConsoleUIController.OverrideSFX(m_playerId, 7, false);
    gConsoleUIController.OverrideSFX(m_playerId, 6, false);
    gConsoleUIController.OverrideSFX(m_playerId, 4, false);
    gConsoleUIController.OverrideSFX(m_playerId, 5, false);
}

void UIScene_AbstractContainerMenu::tick()
{
    UIScene::tick();
    onMouseTick();

    int width;
    int height;
    m_layer->getRenderDimensions(width, height);
    if (width == 1280 && height == 480)
        width = 640;
    float x = static_cast<float>(static_cast<int>(m_touchX * static_cast<float>(width) /
                                                   static_cast<float>(m_stageWidth)));
    float y = static_cast<float>(static_cast<int>(m_touchY * static_cast<float>(height) /
                                                   static_cast<float>(m_stageHeight)));
    gFui->dispatchMouseMoveEvent(m_movie, x, y);
}

void UIScene_AbstractContainerMenu::render(int width, int height, C4JRender::eViewportType viewport)
{
    m_slotDrawPending = true;
    m_updatePending = m_updatePending || m_menu->needsRendered();
    if (m_updatePending)
    {
        m_slotDrawCount = static_cast<int>(GetBaseSlotCount());
        unsigned int slotCount = static_cast<unsigned int>(m_menu->m_slots.size());
        for (unsigned int slot = 0; slot < slotCount; ++slot)
        {
            if (m_menu->getSlot(static_cast<int>(slot))->hasItem())
                ++m_slotDrawCount;
        }
    }
    UIScene::render(width, height, viewport);
    m_updatePending = false;
}

void UIScene_AbstractContainerMenu::customDraw(const char* name, fuiRect* rect)
{
    int slot = 0;
    if (std::sscanf(name, "slot_%d", &slot) != 1 || m_menu == 0)
        return;

    not_null_ptr<ItemInstance> item;
    if (slot == -1)
        item = m_menu->GetQuickCraftItem(m_playerId, -1);
    else
    {
        Slot* menuSlot = m_menu->getSlot(slot);
        if (menuSlot == 0)
            return;
        item = menuSlot->getItem();
    }

    if (!item->isEmpty())
        customDrawSlotControlFui(rect, m_playerId, item, 0.5f, false, true, false);
}

void UIScene_AbstractContainerMenu::handleInput(int, int action, bool repeat, bool released, bool pressed,
                                                bool& handled)
{
    if (!m_updatePending)
        gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (!m_updatePending && released)
        handled = handleKeyDown(m_playerId, action, repeat) != 0;
}

bool UIScene_AbstractContainerMenu::doesSectionTreeHaveFocus(IUIScene_AbstractContainerMenu::ESceneSection)
{
    return false;
}

int UIScene_AbstractContainerMenu::getPad()
{
    return m_playerId;
}

UIControl* UIScene_AbstractContainerMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection)
{
    return 0;
}

unsigned int UIScene_AbstractContainerMenu::GetBaseSlotCount()
{
    return 0;
}

void UIScene_AbstractContainerMenu::updateTooltips()
{
    UpdateTooltips();
}
