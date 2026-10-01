#include "game/ui/UIScene_SuperFlatEditLayer.hpp"
#include "game/core/StringIds.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "game/client/render/Renderer.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/FJ_SuperFlatEditLayerMenu.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/SuperflatMenu.hpp"
#include "game/util/Math.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Rarity.hpp"
#include "platform/wiiu/CInput.hpp"

extern CInput* g_pInput;

UIScene_SuperFlatEditLayer::UIScene_SuperFlatEditLayer(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer), m_layer(1, Blocks::air),
      m_generator(static_cast<SuperflatConfig*>(0)), m_cursor(), m_title(), m_heightSlider(),
      m_confirmButton(), m_heightLabel(), m_typeLabel(), m_slotList(), m_sliderTouch(),
      m_listTouch(), m_mainPanel()
{
    m_pendingSnap = ESnap_None;
    m_scrollPage = 0;
    m_moveTicks = 0;
    m_focusedControl = 0;
    struct LayerData
    {
        bool newLayer;
        int index;
        SuperflatConfig* config;
    };
    m_customBlocks = new SuperflatMenu::CustomSuperflatBlocks();
    initialiseMovie();
    LayerData* values = static_cast<LayerData*>(data);
    m_newLayer = values->newLayer;
    m_layerIndex = values->index;
    m_generator = FlatGeneratorInfo(values->config);
    m_confirmButton.init(UIString(IDS_OK), 2);
    m_title.init(UIString(m_newLayer ? IDS_SUPERFLAT_ADD_LAYER : IDS_SUPERFLAT_EDIT_LAYER));
    _init();
}

UIScene_SuperFlatEditLayer::~UIScene_SuperFlatEditLayer()
{
}

unsigned int UIScene_SuperFlatEditLayer::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_mainPanel.setupControl(this, root, "MainPanel");
    m_controls.push_back(&m_mainPanel);
    m_heightSlider.setupControl(this, m_mainPanel.getFuiRenderNode(), "LayerHeight");
    m_controls.push_back(&m_heightSlider);
    m_confirmButton.setupControl(this, m_mainPanel.getFuiRenderNode(), "ConfirmLayer");
    m_controls.push_back(&m_confirmButton);
    m_slotList.setupControl(this, m_mainPanel.getFuiRenderNode(), "containerList");
    m_controls.push_back(&m_slotList);
    m_title.setupControl(this, m_mainPanel.getFuiRenderNode(), "EditLayerTitle");
    m_controls.push_back(&m_title);
    m_heightLabel.setupControl(this, m_mainPanel.getFuiRenderNode(), "LayerHeightLabel");
    m_controls.push_back(&m_heightLabel);
    m_typeLabel.setupControl(this, m_mainPanel.getFuiRenderNode(), "LayerTypeLabel");
    m_controls.push_back(&m_typeLabel);
    m_cursor.setupControl(this, m_mainPanel.getFuiRenderNode(), "cursor");
    m_controls.push_back(&m_cursor);
    m_sliderTouch.setupControl(this, m_mainPanel.getFuiRenderNode(), "TouchPanel_Slider");
    m_controls.push_back(&m_sliderTouch);
    m_listTouch.setupControl(this, m_mainPanel.getFuiRenderNode(), "TouchPanel_containerList");
    m_controls.push_back(&m_listTouch);
    return 1;
}

void UIScene_SuperFlatEditLayer::updateLayerButtons()
{
    m_typeLabel.setLabel(UIString(m_layer.getNameText()), false, false);
    if (SuperflatMenu::CustomSuperflatBlocks::hasRestrictedPlacement(m_layer.getId()))
    {
        m_heightSlider.SetSliderValue(1);
        m_heightLabel.setLabel(UIString(L"1x"), false, false);
        return;
    }
    m_heightSlider.SetSliderValue(static_cast<int>(m_layer.getHeight()));
    m_heightLabel.setLabel(UIString(m_layer.getHeightText()), false, false);
}

void UIScene_SuperFlatEditLayer::updateScrollCurrentPage(int page)
{
    fuiRenderNode* root = m_movie->getRootNode();
    if (page > 5)
        page = 5;
    static_cast<FJ_SuperFlatEditLayerMenu*>(root->getFuiNode())->SetScrollBar(5, page);
}

int UIScene_SuperFlatEditLayer::getSlotId(int column, int row)
{
    return m_scrollPage * 50 + row * 10 + column;
}

int UIScene_SuperFlatEditLayer::getSlotId(float x, float y)
{
    float left = m_slotList.getXPos();
    float width = m_slotList.getWidth();
    float top = m_slotList.getYPos();
    float height = m_slotList.getHeight();
    return getSlotId(static_cast<int>(std::floor((x - left) / (width + 0.01f) * 10.0f)),
                     static_cast<int>(std::floor((y - top) / (height + 0.01f) * 5.0f)));
}

void UIScene_SuperFlatEditLayer::componentExtents(UIControl& control, float& left, float& right, float& top, float& bottom)
{
    left = static_cast<float>(control.getXPos());
    top = static_cast<float>(control.getYPos());
    right = left + static_cast<float>(control.getWidth());
    bottom = top + static_cast<float>(control.getHeight());
}

bool UIScene_SuperFlatEditLayer::isOverComponent(UIControl& control, float x, float y)
{
    float left = 0.0f;
    float right = 0.0f;
    float top = 0.0f;
    float bottom = 0.0f;
    componentExtents(control, left, right, top, bottom);
    return x >= left && x <= right && y >= top && y <= bottom;
}

int UIScene_SuperFlatEditLayer::getComponentByLocation(float x, float y)
{
    if (isOverComponent(m_confirmButton, x, y)) return 2;
    if (isOverComponent(m_heightSlider, x, y)) return 1;
    if (isOverComponent(m_slotList, x, y)) return 0;
    return -1;
}

int UIScene_SuperFlatEditLayer::getComponentByLocation(const _UIVec2D& point)
{
    return getComponentByLocation(point.x, point.y);
}

int UIScene_SuperFlatEditLayer::getSlotId(const _UIVec2D& point)
{
    return getSlotId(point.x, point.y);
}

void UIScene_SuperFlatEditLayer::SetPointerText(not_null_ptr<ItemInstance> item)
{
    if (item->isEmpty())
    {
        m_cursor.setLabel(UIString(L""), false, false);
        return;
    }

    m_cursor.setLabel(UIString(IDS_SUPERFLAT_DESC_AIR_LAYER), false, false);

    std::vector<HtmlString>* lines;
    if (Item::getId(item->getItem()) == 0)
    {
        lines = new std::vector<HtmlString>();
        lines->push_back(HtmlString(L10N::GetString(IDS_SUPERFLAT_AIR_LAYER), eMinecraftColour_NOTSET, false, false, 0));
    }
    else
    {
        lines = item->getTooltipLines(boost::shared_ptr<Player>(), false);
        if (lines != NULL && !lines->empty())
        {
            lines->at(0).mColour = item->getRarity()->colour;
            if (item->hasCustomHoverName())
                lines->at(0).mColour = eMinecraftColour_Color_RenamedItemTitle;
        }
    }
    m_cursor.setLabel(UIString(HtmlString::Compose(lines)), false, true);
    delete lines;
}

unsigned int UIScene_SuperFlatEditLayer::switchFocus(UIControl* control)
{
    if (m_focusedControl == control) return 0;
    if (control != 0) control->setFocus(true);
    if (m_focusedControl != 0) m_focusedControl->setFocus(false);
    m_focusedControl = control;
    return 1;
}

void UIScene_SuperFlatEditLayer::setNewPointerPos(float x, float y)
{
    bool changed = false;
    int component = getComponentByLocation(x, y);
    if (component == 0)
    {
        int slotId = getSlotId(x, y);
        if (getComponentByLocation(m_pointerPosition) != 0 || slotId != getSlotId(m_pointerPosition))
        {
            m_slotList.setHighlightSlot(slotId % 50);
            SetPointerText(m_customBlocks->item(slotId));
            switchFocus(0);
            changed = true;
        }
        m_cursor.setVisible(true);
    }
    else if (component == 1)
    {
        SetPointerText(ItemInstance::EMPTY);
        changed = switchFocus(&m_heightSlider) != 0;
        m_cursor.setVisible(false);
    }
    else if (component == 2)
    {
        SetPointerText(ItemInstance::EMPTY);
        changed = switchFocus(&m_confirmButton) != 0;
        m_cursor.setVisible(false);
    }
    else
    {
        SetPointerText(ItemInstance::EMPTY);
        switchFocus(0);
        m_cursor.setVisible(false);
        changed = true;
    }
    m_pointerPosition.x = x;
    m_pointerPosition.y = y;
    if (changed)
        updateTooltips();
}

void UIScene_SuperFlatEditLayer::setNewPointerPos(const _UIVec2D& point)
{
    setNewPointerPos(point.x, point.y);
}

void UIScene_SuperFlatEditLayer::_init()
{
    if (!m_newLayer)
        m_layer = m_generator.getLayer(m_layerIndex);
    int maximumHeight = 128 - m_generator.getHeight();
    int height = 1;
    if (!m_newLayer)
    {
        maximumHeight += static_cast<int>(m_layer.getHeight());
        height = static_cast<int>(m_generator.getLayer(m_layerIndex).getHeight());
    }
    if (maximumHeight > 128)
        maximumHeight = 128;
    m_maxLayerHeight = maximumHeight;
    m_heightSlider.init(UIString(IDS_SUPERFLAT_LAYER_HEIGHT), 1, 1, m_maxLayerHeight, height, false, 10, 1);
    m_heightLabel.init(UIString(L""));
    m_typeLabel.init(UIString(L""));
    updateLayerButtons();

    float left = static_cast<float>(m_slotList.getXPos());
    float top = static_cast<float>(m_slotList.getYPos());
    m_listLeft = left;
    m_listRight = left + static_cast<float>(m_slotList.getWidth());
    m_listTop = top;
    m_listBottom = top + static_cast<float>(m_slotList.getHeight());
    m_pointerOffset.x = 0.0f;
    m_pointerOffset.y = 0.0f;
    m_pointerPosition.x = (m_listRight - m_listLeft) * 0.5f;
    m_pointerPosition.y = (m_listBottom - m_listTop) * 0.5f;
    m_slotList.addSlots(0, 50);
    updateScrollCurrentPage(0);
    setNewPointerPos(m_pointerPosition);
    m_sliderTouch.init(3);
    m_listTouch.init(4);
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_SuperFlatEditLayer::updateTooltips()
{
    int component = getComponentByLocation(m_pointerPosition);
    int select = -1;
    if (component == 0 || component == 2)
        select = IDS_TOOLTIPS_SELECT;
    gConsoleUIController.SetTooltips(0, select, IDS_TOOLTIPS_CANCEL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, false, false);
}

void UIScene_SuperFlatEditLayer::updateComponents()
{
}

UIScene_SuperFlatEditLayer::ESnap UIScene_SuperFlatEditLayer::getSnap(float x, float y) const
{
    if (y > 0.0f) return ESnap_Up;
    if (y < 0.0f) return ESnap_Down;
    if (x > 0.0f) return ESnap_Right;
    if (x < 0.0f) return ESnap_Left;
    return ESnap_None;
}

void UIScene_SuperFlatEditLayer::dmouse(float& moveX, float& moveY, float& directionX, float& directionY)
{
    float sensitivity =
        static_cast<float>(GameSettings::GetGameSettings(m_playerId, eGameSetting_InterfaceSensitivity)) / 100.0f;
    moveX = g_pInput->GetJoypadStick_Menu_LX(m_playerId) * sensitivity;
    moveY = g_pInput->GetJoypadStick_Menu_LY(m_playerId) * sensitivity;
    if (std::fabs(moveX) < 0.01f)
        directionX = 0.0f;
    else
        directionX = moveX / std::fabs(moveX);
    if (std::fabs(moveY) < 0.01f)
        directionY = 0.0f;
    else
        directionY = moveY / std::fabs(moveY);
}

void UIScene_SuperFlatEditLayer::componentCentre(UIControl& control, float& x, float& y)
{
    float left = control.getXPos();
    float width = control.getWidth();
    x = width * 0.5f + left;
    float top = control.getYPos();
    float height = control.getHeight();
    y = height * 0.5f + top;
}

void UIScene_SuperFlatEditLayer::slotCentre(int slot, float& x, float& y)
{
    float column = static_cast<float>((slot % 50) % 10) / 10.0f + 0.05f;
    float row = static_cast<float>((slot % 50) / 10) / 5.0f + 0.1f;
    float left = m_slotList.getXPos();
    float width = m_slotList.getWidth();
    x = width * column + left;
    float top = m_slotList.getYPos();
    float height = m_slotList.getHeight();
    y = height * row + top;
}

bool UIScene_SuperFlatEditLayer::onRow(int slot, int row)
{
    return (slot % 50) / 10 == row;
}

int UIScene_SuperFlatEditLayer::modulo(int value, int divisor)
{
    if (value < 0)
        return divisor - std::abs(value) % divisor;
    return value % divisor;
}

int UIScene_SuperFlatEditLayer::getNextSlot(int slot, ESnap snap)
{
    int cell = modulo(slot, 50);
    int page = slot / 50;
    switch (snap)
    {
    case ESnap_Up:
        return modulo(cell + 40, 50) + page * 50;
    case ESnap_Down:
        return modulo(cell + 10, 50) + page * 50;
    case ESnap_Left:
        return (cell / 10) * 10 + modulo(cell - 1, 10) + page * 50;
    case ESnap_Right:
        return (cell / 10) * 10 + modulo(cell + 1, 10) + page * 50;
    default:
        return cell + page * 50;
    }
}

void UIScene_SuperFlatEditLayer::doSnap(const _UIVec2D& point, ESnap snap)
{
    float x = point.x;
    float y = point.y;
    int component = getComponentByLocation(x, y);
    if (component == 0)
    {
        if (snap == ESnap_None)
        {
            slotCentre(getSlotId(x, y), x, y);
            setNewPointerPos(x, y);
            return;
        }
        int slot = getSlotId(x, y);
        if (onRow(slot, 0) && snap == ESnap_Up)
        {
            componentCentre(m_confirmButton, x, y);
            setNewPointerPos(x, y);
            return;
        }
        if (!onRow(slot, 4) || snap != ESnap_Down)
        {
            slotCentre(getNextSlot(getSlotId(x, y), snap), x, y);
            setNewPointerPos(x, y);
            return;
        }
    }
    else if (component == 1)
    {
        switch (snap)
        {
        case ESnap_None:
            break;
        case ESnap_Up:
            slotCentre(45, x, y);
            setNewPointerPos(x, y);
            return;
        case ESnap_Down:
            componentCentre(m_confirmButton, x, y);
            setNewPointerPos(x, y);
            return;
        case ESnap_Left:
        case ESnap_Right:
            componentCentre(m_heightSlider, x, y);
            if (!SuperflatMenu::CustomSuperflatBlocks::hasRestrictedPlacement(m_layer.getId()))
            {
                m_heightSlider.MoveSliderValue(snap == ESnap_Right ? 1 : -1);
                handleSliderMove(1, m_heightSlider.getCurrentValue());
            }
            setNewPointerPos(x, y);
            return;
        default:
            componentCentre(m_heightSlider, x, y);
            setNewPointerPos(x, y);
            return;
        }
    }
    else if (component == 2)
    {
        if (snap == ESnap_Down)
        {
            slotCentre(5, x, y);
            setNewPointerPos(x, y);
            return;
        }
        if (snap != ESnap_Up)
        {
            componentCentre(m_confirmButton, x, y);
            setNewPointerPos(x, y);
            return;
        }
    }
    else
    {
        setNewPointerPos(x, y);
        return;
    }
    componentCentre(m_heightSlider, x, y);
    setNewPointerPos(x, y);
}

void UIScene_SuperFlatEditLayer::setNewPointerPos(UIControl* control)
{
    if (control != 0)
    {
        float left = control->getXPos();
        float width = control->getWidth();
        float top = control->getYPos();
        float height = control->getHeight();
        setNewPointerPos(width * 0.5f + left, height * 0.5f + top);
    }
}

void UIScene_SuperFlatEditLayer::onMouseTick()
{
    float moveX = 0.0f;
    float moveY = 0.0f;
    float directionX = 0.0f;
    float directionY = 0.0f;
    dmouse(moveX, moveY, directionX, directionY);
    ESnap snap = getSnap(directionX, directionY);

    int component = getComponentByLocation(m_pointerPosition);
    int ticks = m_moveTicks;
    if (component != 0 && ticks > 10 && snap != ESnap_None)
    {
        doSnap(m_pointerPosition, snap);
        if (getComponentByLocation(m_pointerPosition) == 1 && (snap == ESnap_Left || snap == ESnap_Right))
        {
            m_pendingSnap = ESnap_None;
            m_moveTicks = 4;
            return;
        }
        m_pendingSnap = ESnap_None;
        m_moveTicks = -10;
        return;
    }

    if (directionX == 0.0f && directionY == 0.0f)
    {
        if (ticks > 0)
        {
            if (ticks < 10 && getComponentByLocation(m_moveStartPosition) >= 0)
            {
                doSnap(m_moveStartPosition, m_pendingSnap);
                m_pendingSnap = ESnap_None;
                m_moveTicks = 0;
                return;
            }
            doSnap(m_pointerPosition, ESnap_None);
            m_pendingSnap = ESnap_None;
            m_moveTicks = 0;
            return;
        }
        if (m_pendingSnap != ESnap_None)
        {
            doSnap(m_pointerPosition, m_pendingSnap);
            m_pendingSnap = ESnap_None;
            return;
        }
        ticks = ticks + 1;
        if (ticks > 0)
            ticks = 0;
        m_moveTicks = ticks;
        return;
    }

    if (ticks == 0)
    {
        m_moveStartPosition = m_pointerPosition;
        if (std::fabs(moveY) < std::fabs(moveX))
        {
            directionY = 0.0f;
            m_pendingSnap = getSnap(directionX, 0.0f);
        }
        else
            m_pendingSnap = getSnap(directionX, directionY);
    }
    m_moveTicks = ticks + 1;

    if (getComponentByLocation(m_pointerPosition) != 0)
        return;

    moveX = moveX * moveX * directionX * 13.0f;
    _UIVec2D position;
    position.x = m_pointerPosition.x + m_pointerOffset.x;
    position.y = m_pointerPosition.y + m_pointerOffset.y;
    moveY = moveY * moveY * directionY * 13.0f;
    float scale = 1.0f;
    if (!gRenderer->IsHiDef() || gApp.GetLocalPlayerCount() > 1)
        scale = 0.6f;
    moveX = moveX * scale;
    moveY = moveY * scale;
    position.x = position.x + moveX;
    position.y = position.y - moveY;
    position.x = std::max(m_listLeft, position.x);
    if (m_listRight - position.x < 0.0f)
        position.x = m_listRight;
    position.y = std::max(m_listTop, position.y);
    float bottom = m_listBottom;
    if (bottom - position.y < 0.0f)
        position.y = bottom;
    if (position.y != bottom)
    {
        setNewPointerPos(position);
        return;
    }
    setNewPointerPos(&m_heightSlider);
    m_pendingSnap = ESnap_None;
    m_moveTicks = -10;
}

void UIScene_SuperFlatEditLayer::tick()
{
    UIScene::tick();
    onMouseTick();
    int width = 0;
    int height = 0;
    UIScene::m_layer->getRenderDimensions(width, height);
    if (width == 1280 && height == 480)
        width = 640;
    int mouseX = static_cast<int>(m_pointerPosition.x * (static_cast<float>(width) / static_cast<float>(m_stageWidth)));
    int mouseY = static_cast<int>(m_pointerPosition.y * (static_cast<float>(height) / static_cast<float>(m_stageHeight)));
    gFui->dispatchMouseMoveEvent(m_movie, static_cast<float>(mouseX), static_cast<float>(mouseY));
}

void UIScene_SuperFlatEditLayer::handleReload()
{
    _init();
}

void UIScene_SuperFlatEditLayer::exit(EExit result)
{
    if (result == EExit_Save)
    {
        if (SuperflatMenu::CustomSuperflatBlocks::hasRestrictedPlacement(m_layer.getId()))
            m_layer.setHeight(1);
        if (m_newLayer)
        {
            m_generator.insertLayer(m_layerIndex + 1, m_layer);
            navigateBack();
            return;
        }
        m_generator.setLayer(m_layerIndex, m_layer);
    }
    navigateBack();
}

UIScene_SuperFlatEditLayer::ESnap UIScene_SuperFlatEditLayer::getSnap(int action) const
{
    switch (action)
    {
    case EControllerActions_MenuUp:
        return ESnap_Up;
    case EControllerActions_MenuDown:
        return ESnap_Down;
    case EControllerActions_MenuRight:
        return ESnap_Right;
    case EControllerActions_MenuLeft:
        return ESnap_Left;
    default:
        return ESnap_None;
    }
}

void UIScene_SuperFlatEditLayer::handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
        if (pressed && m_moveTicks == 0)
            m_pendingSnap = getSnap(action);
        return;
    case EControllerActions_MenuOtherStickUp:
        if (!pressed)
            return;
        m_scrollPage = m_scrollPage - 1 < 0 ? 0 : m_scrollPage - 1;
        updateScrollCurrentPage(m_scrollPage);
        if (getComponentByLocation(m_pointerPosition) == 0)
            SetPointerText(m_customBlocks->item(getSlotId(m_pointerPosition)));
        break;
    case EControllerActions_MenuOtherStickDown:
        if (!pressed)
            return;
        m_scrollPage = m_scrollPage + 1 > 4 ? 4 : m_scrollPage + 1;
        updateScrollCurrentPage(m_scrollPage);
        if (getComponentByLocation(m_pointerPosition) == 0)
            SetPointerText(m_customBlocks->item(getSlotId(m_pointerPosition)));
        break;
    case EControllerActions_MenuOk:
    {
        if (!pressed)
            return;
        int component = getComponentByLocation(m_pointerPosition);
        if (component != 0)
        {
            if (component == 2)
                exit(EExit_Save);
            break;
        }
        not_null_ptr<ItemInstance> item = m_customBlocks->item(getSlotId(m_pointerPosition));
        int itemId = 0;
        int auxValue = 0;
        if (!item.get()->isEmpty())
        {
            itemId = Item::getId(item.get()->getItem());
            auxValue = item.get()->getAuxValue();
        }
        m_layer.setBlockState(Block::byId(itemId)->getBlockState(auxValue));
        updateLayerButtons();
        break;
    }
    case EControllerActions_MenuCancel:
        if (!pressed)
            return;
        exit(EExit_Cancel);
        break;
    default:
        sendInputToMovie(action, repeat, pressed, released);
        return;
    }
    handled = true;
}

void UIScene_SuperFlatEditLayer::handlePress(int, int)
{
}

void UIScene_SuperFlatEditLayer::handleSliderMove(int id, int value)
{
    if (id != 1)
        return;
    int height = std::min(m_maxLayerHeight, value);
    if (height < 1)
        height = 1;
    m_heightSlider.handleSliderMove(height);
    m_layer.setHeight(height);
    updateLayerButtons();
}

void UIScene_SuperFlatEditLayer::customDraw(const char* name, fuiRect* rect)
{
    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    m_slotDrawPending = false;
    int slot = -1;
    std::sscanf(name, "slot_%d", &slot);
    if (slot != -1)
    {
        item = m_customBlocks->item(getSlotId(slot % 10, slot / 10));
    }
    else if (std::strcmp(name, "pointerIcon") != 0)
    {
        std::sscanf(name, "layerslot_%d", &slot);
        if (slot == 0 && m_layer.getId() != 0)
        {
            int itemId = m_layer.getId();
            if (itemId >= 256 && itemId < 1024)
                itemId += 768;
            item = not_null_ptr<ItemInstance>(new ItemInstance(Item::byId(itemId), 1, m_layer.getData()));
        }
    }
    if (!item.get()->isEmpty())
        customDrawSlotControlFui(rect, m_playerId, item, 1.0f, false, true, false);
    else
        item = ItemInstance::EMPTY;
}

void UIScene_SuperFlatEditLayer::handleTouchInput(unsigned int playerId, int touchX, int touchY,
                                                  int touchBox, bool pressed, bool repeat,
                                                  bool released)
{
    if (touchBox == 4)
    {
        if (pressed || repeat)
        {
            setNewPointerPos(static_cast<float>(touchX) - m_mainPanel.getXPos(),
                             static_cast<float>(touchY) - m_mainPanel.getYPos());
        }
        else
        {
            if (!released)
                return;
            not_null_ptr<ItemInstance> item = m_customBlocks->item(getSlotId(m_pointerPosition));
            if (!item.get()->isEmpty())
            {
                Block* block = Block::byItem(item.get()->getItem());
                m_layer.setBlockState(block->getBlockState(item.get()->getAuxValue()));
            }
            else
            {
                static const BlockState* const airState = Blocks::air->defaultBlockState();
                m_layer.setBlockState(airState);
            }
            doSnap(m_pointerPosition, ESnap_None);
            updateLayerButtons();
        }
    }

    if (repeat && touchBox == 3)
    {
        float position = (static_cast<float>(touchY) - m_sliderTouch.getYPos() - m_mainPanel.getYPos()) /
                         m_sliderTouch.getHeight();
        if (position > 1.0f)
            position = 1.0f;
        else if (position < 0.0f)
            position = 0.0f;
        m_scrollPage = Math::round(position * 4.0f);
        updateScrollCurrentPage(m_scrollPage);
        if (getComponentByLocation(m_pointerPosition) == 0)
            SetPointerText(m_customBlocks->item(getSlotId(m_pointerPosition)));
    }
}

UIControl* UIScene_SuperFlatEditLayer::GetMainPanel()
{
    return &m_mainPanel;
}

void UIScene_SuperFlatEditLayer::SetFocusToElement(int focus)
{
    UIControl* control;
    if (focus == 1)
        control = &m_heightSlider;
    else if (focus == 2)
        control = &m_confirmButton;
    else
        return;
    setNewPointerPos(control);
}

std::wstring UIScene_SuperFlatEditLayer::getMoviePath()
{
    return L"SuperFlatEditLayerMenu";
}

int UIScene_SuperFlatEditLayer::getSceneType()
{
    return EUIScene_SuperFlatEditLayer;
}
