#include "game/ui/UIScene_SuperFlatEditLayer.hpp"

#include <cmath>

#include "game/core/ConsoleUIController.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/FJ_SuperFlatEditLayerMenu.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/SuperflatMenu.hpp"
#include "game/util/Math.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

UIScene_SuperFlatEditLayer::UIScene_SuperFlatEditLayer(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer), m_unknownC8(0), m_pointerScale(0.0f), m_unknownD0(0),
      m_unknownD4(0), m_maxLayerHeight(0), m_newLayer(false), m_layerIndex(0),
      m_layer(1, static_cast<Block*>(0)), m_generator(static_cast<SuperflatConfig*>(0)),
      m_customBlocks(new SuperflatMenu::CustomSuperflatBlocks()), m_extentLeft(0.0f),
      m_extentTop(0.0f), m_listLeft(0.0f), m_listTop(0.0f), m_listRight(0.0f),
      m_listBottom(0.0f), m_scrollPage(0), m_focusedControl(0), m_cursor(),
      m_title(), m_heightSlider(), m_confirmButton(), m_heightLabel(), m_typeLabel(), m_slotList(),
      m_sliderTouch(), m_listTouch(), m_mainPanel()
{
    struct LayerData
    {
        bool newLayer;
        int index;
        SuperflatConfig* config;
    };
    initialiseMovie();
    LayerData* values = static_cast<LayerData*>(data);
    m_newLayer = values->newLayer;
    m_layerIndex = values->index;
    m_generator = FlatGeneratorInfo(values->config);
    m_confirmButton.init(UIString(0x0ad4a4f0), 2);
    m_title.init(UIString(m_newLayer ? 0x984506e8 : 0x70e1a4d5));
    _init();
}

UIScene_SuperFlatEditLayer::~UIScene_SuperFlatEditLayer()
{
    delete m_customBlocks;
}

unsigned int UIScene_SuperFlatEditLayer::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_mainPanel.setupControl(this, root, "MainPanel");
    m_heightSlider.setupControl(this, m_mainPanel.getFuiRenderNode(), "LayerHeight");
    m_confirmButton.setupControl(this, m_mainPanel.getFuiRenderNode(), "ConfirmLayer");
    m_slotList.setupControl(this, m_mainPanel.getFuiRenderNode(), "containerList");
    m_title.setupControl(this, m_mainPanel.getFuiRenderNode(), "EditLayerTitle");
    m_heightLabel.setupControl(this, m_mainPanel.getFuiRenderNode(), "LayerHeightLabel");
    m_typeLabel.setupControl(this, m_mainPanel.getFuiRenderNode(), "LayerTypeLabel");
    m_cursor.setupControl(this, m_mainPanel.getFuiRenderNode(), "cursor");
    m_sliderTouch.setupControl(this, m_mainPanel.getFuiRenderNode(), "TouchPanel_Slider");
    m_listTouch.setupControl(this, m_mainPanel.getFuiRenderNode(), "TouchPanel_containerList");
    return 1;
}

void UIScene_SuperFlatEditLayer::updateLayerButtons()
{
    m_heightSlider.SetSliderValue(static_cast<int>(m_layer.getHeight()));
    m_heightLabel.setLabel(UIString(m_layer.getHeightText()), false, false);
    m_typeLabel.setLabel(UIString(m_layer.getNameText()), false, false);
}

void UIScene_SuperFlatEditLayer::updateScrollCurrentPage(int page)
{
    if (page > 5) page = 5;
    m_scrollPage = page;
    if (m_movie != 0)
    {
        m_movie->getRootNode();
    }
}

int UIScene_SuperFlatEditLayer::getSlotId(int column, int row)
{
    return m_scrollPage * 50 + row * 10 + column;
}

int UIScene_SuperFlatEditLayer::getSlotId(float x, float y)
{
    return getSlotId(static_cast<int>(std::floor((x - m_slotList.getXPos()) / (m_slotList.getWidth() + 0.01f) * 10.0f)),
                     static_cast<int>(std::floor((y - m_slotList.getYPos()) / (m_slotList.getHeight() + 0.01f) * 5.0f)));
}

void UIScene_SuperFlatEditLayer::componentExtents(UIControl& control, float& left, float& right, float& top, float& bottom)
{
    left = static_cast<float>(control.getXPos()); top = static_cast<float>(control.getYPos());
    right = left + static_cast<float>(control.getWidth()); bottom = top + static_cast<float>(control.getHeight());
}

bool UIScene_SuperFlatEditLayer::isOverComponent(UIControl& control, float x, float y)
{
    float left, right, top, bottom; componentExtents(control, left, right, top, bottom);
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
    m_cursor.setVisible(item.get() != 0);
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
    int component = getComponentByLocation(x, y);
    if (component == 0)
    {
        int slotId = getSlotId(x, y);
        bool changed = getComponentByLocation(m_pointerPosition) != 0 ||
                       slotId != getSlotId(m_pointerPosition);
        if (changed)
        {
            m_slotList.setHighlightSlot(slotId % 50);
            not_null_ptr<ItemInstance> item = m_customBlocks->item(slotId);
            SetPointerText(item);
            switchFocus(0);
        }
        m_cursor.setVisible(true);
        m_pointerPosition.x = x;
        m_pointerPosition.y = y;
        if (!changed)
            return;
        updateTooltips();
    }
    else if (component == 1)
    {
        SetPointerText(m_customBlocks->item(0));
        unsigned int changed = switchFocus(&m_heightSlider);
        m_cursor.setVisible(false);
        m_pointerPosition.x = x;
        m_pointerPosition.y = y;
        if (changed == 0)
            return;
        updateTooltips();
    }
    else if (component != 2)
    {
        SetPointerText(m_customBlocks->item(0));
        switchFocus(0);
        m_cursor.setVisible(false);
        m_pointerPosition.y = y;
        m_pointerPosition.x = x;
        updateTooltips();
    }
    else
    {
        SetPointerText(m_customBlocks->item(0));
        unsigned int changed = switchFocus(&m_confirmButton);
        m_cursor.setVisible(false);
        m_pointerPosition.x = x;
        m_pointerPosition.y = y;
        if (changed == 0)
            return;
        updateTooltips();
    }
}

void UIScene_SuperFlatEditLayer::setNewPointerPos(const _UIVec2D& point)
{
    float x = point.x;
    float y = point.y;
    int component = getComponentByLocation(x, y);
    if (component == 0)
    {
        int slot = getSlotId(x, y);
        int oldComponent = getComponentByLocation(m_pointerPosition);
        bool changed = false;
        if (oldComponent != 0 || slot != getSlotId(m_pointerPosition))
        {
            m_slotList.setHighlightSlot(slot % 50);
            SetPointerText(m_customBlocks->item(slot));
            switchFocus(0);
            changed = true;
        }
        m_cursor.setVisible(true);
        m_pointerPosition = point;
        if (!changed)
            return;
    }
    else if (component == 1)
    {
        SetPointerText(ItemInstance::EMPTY);
        unsigned int result = switchFocus(&m_heightSlider);
        m_cursor.setVisible(false);
        m_pointerPosition = point;
        if (result == 0)
            return;
    }
    else if (component == 2)
    {
        SetPointerText(ItemInstance::EMPTY);
        unsigned int result = switchFocus(&m_confirmButton);
        m_cursor.setVisible(false);
        m_pointerPosition = point;
        if (result == 0)
            return;
    }
    else
    {
        SetPointerText(ItemInstance::EMPTY);
        switchFocus(0);
        m_cursor.setVisible(false);
        m_pointerPosition = point;
    }
    updateTooltips();
}

void UIScene_SuperFlatEditLayer::_init()
{
    int height = 1;
    if (!m_newLayer)
        m_layer = m_generator.getLayer(m_layerIndex);
    m_maxLayerHeight = 128 - m_generator.getHeight();
    if (!m_newLayer)
    {
        m_maxLayerHeight += static_cast<int>(m_layer.getHeight());
        height = static_cast<int>(m_layer.getHeight());
    }
    if (m_maxLayerHeight > 128) m_maxLayerHeight = 128;
    m_heightSlider.init(UIString(0x9431d6c6), 1, 1, m_maxLayerHeight, height, false, 10, 1);
    m_heightLabel.init(UIString(L"")); m_typeLabel.init(UIString(L""));
    updateLayerButtons();
    m_extentLeft = 0.0f;
    m_extentTop = 0.0f;
    m_listLeft = static_cast<float>(m_slotList.getXPos());
    m_listTop = static_cast<float>(m_slotList.getYPos());
    m_listRight = m_listLeft + static_cast<float>(m_slotList.getWidth());
    m_listBottom = m_listTop + static_cast<float>(m_slotList.getHeight());
    m_pointerPosition.x = (m_listRight - m_listLeft) * 0.5f;
    m_pointerPosition.y = (m_listBottom - m_listTop) * 0.5f;
    m_slotList.addSlots(0, 50);
    updateScrollCurrentPage(0);
    setNewPointerPos(m_pointerPosition);
    m_sliderTouch.init(3); m_listTouch.init(4);
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_SuperFlatEditLayer::updateTooltips()
{
    int tooltip = getComponentByLocation(m_pointerPosition) == 0 ? 0x8d6323ed : -1;
    int tooltips[16] = { tooltip, static_cast<int>(0xe21d8023), -1, -1, -1, -1, -1, -1,
                         -1, -1, -1, -1, -1, -1, -1, -1 };
    gConsoleUIController.SetTooltips(0, tooltips, false, false);
}

void UIScene_SuperFlatEditLayer::updateComponents() { }

int UIScene_SuperFlatEditLayer::getSnap(float x, float y) const
{
    if (y > 0.0f) return 1; if (y < 0.0f) return 2; if (x > 0.0f) return 4; return x < 0.0f ? 3 : 0;
}

void UIScene_SuperFlatEditLayer::dmouse(float& x, float& y, float& dx, float& dy)
{
    dx = x == 0.0f ? 0.0f : x / std::fabs(x); dy = y == 0.0f ? 0.0f : y / std::fabs(y);
}

void UIScene_SuperFlatEditLayer::componentCentre(UIControl& control, float& x, float& y)
{
    x = static_cast<float>(control.getXPos() + control.getWidth() * 0.5); y = static_cast<float>(control.getYPos() + control.getHeight() * 0.5);
}

void UIScene_SuperFlatEditLayer::slotCentre(int slot, float& x, float& y)
{
    x = static_cast<float>(m_slotList.getXPos() + m_slotList.getWidth() * ((slot % 50) % 10 / 10.0f + 0.05f));
    y = static_cast<float>(m_slotList.getYPos() + m_slotList.getHeight() * ((slot % 50) / 10 / 5.0f + 0.1f));
}

bool UIScene_SuperFlatEditLayer::onRow(int slot, int row) { return (slot % 50) / 10 == row; }
int UIScene_SuperFlatEditLayer::modulo(int value, int divisor) { int result = value % divisor; return result < 0 ? divisor + result : result; }

int UIScene_SuperFlatEditLayer::getNextSlot(int slot, ESnap snap)
{
    int page = slot / 50; int cell = modulo(slot, 50); int column = cell % 10; int row = cell / 10;
    if (snap == ESnap_Left) column = modulo(column - 1, 10); else if (snap == ESnap_Right) column = modulo(column + 1, 10);
    else if (snap == ESnap_Up) row = modulo(row - 1, 5); else if (snap == ESnap_Down) row = modulo(row + 1, 5);
    return page * 50 + row * 10 + column;
}

void UIScene_SuperFlatEditLayer::doSnap(const _UIVec2D& point, ESnap snap)
{
    float x = point.x;
    float y = point.y;
    int component = getComponentByLocation(x, y);
    if (component == 0)
    {
        int slot = getSlotId(x, y);
        if (snap == ESnap_None)
        {
            slotCentre(slot, x, y);
            setNewPointerPos(x, y);
            return;
        }
        if (onRow(slot, 0) && snap == ESnap_Left)
        {
            componentCentre(m_confirmButton, x, y);
            setNewPointerPos(x, y);
            return;
        }
        if (!onRow(slot, 4) || snap != ESnap_Right)
        {
            slot = getNextSlot(slot, snap);
            slotCentre(slot, x, y);
            setNewPointerPos(x, y);
            return;
        }
    }
    else if (component == 1)
    {
        if (snap != ESnap_None)
        {
            if (snap == ESnap_Left)
            {
                slotCentre(0x2d, x, y);
                setNewPointerPos(x, y);
                return;
            }
            if (snap == ESnap_Right)
            {
                componentCentre(m_confirmButton, x, y);
                setNewPointerPos(x, y);
                return;
            }
            componentCentre(m_heightSlider, x, y);
            if (snap > ESnap_Down)
            {
                setNewPointerPos(x, y);
                return;
            }
            if (!SuperflatMenu::CustomSuperflatBlocks::hasRestrictedPlacement(m_layer.getId()))
            {
                m_heightSlider.MoveSliderValue(snap == ESnap_Down ? 1 : -1);
                handleSliderMove(1, m_heightSlider.getCurrentValue());
                setNewPointerPos(x, y);
                return;
            }
            setNewPointerPos(x, y);
            return;
        }
    }
    else if (component == 2)
    {
        if (snap != ESnap_Left)
        {
            if (snap == ESnap_Right)
            {
                slotCentre(5, x, y);
                setNewPointerPos(x, y);
                return;
            }
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
        float x = control->getXPos() + control->getWidth() * 0.5f;
        float y = control->getYPos() + control->getHeight() * 0.5f;
        setNewPointerPos(x, y);
    }
}

void UIScene_SuperFlatEditLayer::onMouseTick()
{
    int component = getComponentByLocation(m_pointerPosition);
    if (component == 0)
    {
        int slot = getSlotId(m_pointerPosition);
        m_slotList.setHighlightSlot(slot % 50);
        if (m_focusedControl != &m_slotList) switchFocus(&m_slotList);
    }
    else if (component == 1)
    {
        if (m_focusedControl != &m_heightSlider) switchFocus(&m_heightSlider);
    }
    else if (component == 2)
    {
        if (m_focusedControl != &m_confirmButton) switchFocus(&m_confirmButton);
    }
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
    gFui->dispatchMouseMoveEvent(m_movie, m_pointerPosition.x * static_cast<float>(width) / static_cast<float>(m_stageWidth),
                                 m_pointerPosition.y * static_cast<float>(height) / static_cast<float>(m_stageHeight));
}
void UIScene_SuperFlatEditLayer::handleReload() { _init(); }

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

int UIScene_SuperFlatEditLayer::getSnap(int action) const { return action > 3 && action < 8 ? action - 3 : 0; }

void UIScene_SuperFlatEditLayer::handleInput(int playerId, int action, bool repeat, bool released, bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (action < 15)
    {
        if (action < 4)
        {
            sendInputToMovie(action, repeat, released, pressed);
            return;
        }
        if (action < 8)
        {
            if (!released)
                return;
            if (m_unknownD4 != 0)
                return;
            m_unknownC8 = getSnap(action);
            return;
        }
        if (action != 14)
        {
            sendInputToMovie(action, repeat, released, pressed);
            return;
        }
        if (!released)
            return;
        m_scrollPage = m_scrollPage - 1;
        if (m_scrollPage < 0)
            m_scrollPage = 0;
        updateScrollCurrentPage(m_scrollPage);
        if (getComponentByLocation(m_pointerPosition) == 0)
        {
            int slotId = getSlotId(m_pointerPosition);
            not_null_ptr<ItemInstance> item = m_customBlocks->item(slotId);
            SetPointerText(item);
        }
    }
    else if (action == 15)
    {
        if (!released)
            return;
        m_scrollPage = m_scrollPage + 1;
        if (m_scrollPage > 4)
            m_scrollPage = 4;
        updateScrollCurrentPage(m_scrollPage);
        if (getComponentByLocation(m_pointerPosition) == 0)
        {
            int slotId = getSlotId(m_pointerPosition);
            not_null_ptr<ItemInstance> item = m_customBlocks->item(slotId);
            SetPointerText(item);
            handled = true;
            return;
        }
    }
    else
    {
        if (action != 0x14)
        {
            if (action == 0x15)
            {
                if (!released)
                    return;
                exit(EExit_Cancel);
                handled = true;
                return;
            }
            sendInputToMovie(action, repeat, released, pressed);
            return;
        }
        if (!released)
            return;
        int component = getComponentByLocation(m_pointerPosition);
        if (component != 0)
        {
            if (component == 2)
            {
                exit(EExit_Save);
                handled = true;
                return;
            }
            handled = true;
            return;
        }
        int slotId = getSlotId(m_pointerPosition);
        not_null_ptr<ItemInstance> item = m_customBlocks->item(slotId);
        int itemId = 0;
        int auxValue = 0;
        if (!item->isEmpty())
        {
            itemId = Item::getId(item->getItem());
            auxValue = item->getAuxValue();
        }
        Block* block = Block::byId(itemId);
        const BlockState* state = block->getBlockState(auxValue);
        m_layer.setBlockState(state);
        updateLayerButtons();
    }
    handled = true;
}

void UIScene_SuperFlatEditLayer::handlePress(int, int) { }
void UIScene_SuperFlatEditLayer::handleSliderMove(int id, int value) { if (id == 1) { m_heightSlider.handleSliderMove(value); m_layer.setHeight(value < 1 ? 1 : value); updateLayerButtons(); } }
void UIScene_SuperFlatEditLayer::customDraw(const char* name, fuiRect* rect)
{
    UIScene::customDraw(name, rect);
}

void UIScene_SuperFlatEditLayer::handleTouchInput(unsigned int playerId, int touchX, int touchY,
                                                  int touchBox, bool repeat, bool released,
                                                  bool pressed)
{
    if (touchBox == 4)
    {
        if (!repeat && !released)
        {
            if (!pressed) return;
            int slotId = getSlotId(m_pointerPosition);
            not_null_ptr<ItemInstance> item = m_customBlocks->item(slotId);
            if (!item->isEmpty())
            {
                Block* block = Block::byItem(item->getItem());
                const BlockState* state = block->getBlockState(item->getAuxValue());
                m_layer.setBlockState(state);
                doSnap(m_pointerPosition, ESnap_None);
                updateLayerButtons();
            }
            else
            {
                m_layer.setBlockState(Blocks::AIR->defaultBlockState());
                doSnap(m_pointerPosition, ESnap_None);
                updateLayerButtons();
            }
        }
        else
        {
            setNewPointerPos(static_cast<float>(touchX) - m_mainPanel.getXPos(),
                             static_cast<float>(touchY) - m_mainPanel.getYPos());
        }
    }

    if (released && touchBox == 3)
    {
        float relY = (static_cast<float>(touchY) - m_sliderTouch.getYPos() - m_mainPanel.getYPos()) / m_sliderTouch.getHeight();
        if (relY <= 1.0f)
        {
            if (relY < 0.0f)
                relY = 0.0f;
            m_scrollPage = Math::round(relY * 4.0f);
        }
        else
        {
            m_scrollPage = Math::round(1.0f * 4.0f);
        }
        updateScrollCurrentPage(m_scrollPage);
        int comp = getComponentByLocation(m_pointerPosition);
        if (comp == 0)
        {
            int slotId = getSlotId(m_pointerPosition);
            not_null_ptr<ItemInstance> item = m_customBlocks->item(slotId);
            SetPointerText(item);
        }
    }
}

UIControl* UIScene_SuperFlatEditLayer::GetMainPanel() { return &m_mainPanel; }
void UIScene_SuperFlatEditLayer::SetFocusToElement(int focus) { setNewPointerPos(focus == 1 ? static_cast<UIControl*>(&m_heightSlider) : static_cast<UIControl*>(&m_confirmButton)); }
std::wstring UIScene_SuperFlatEditLayer::getMoviePath() { return L"SuperFlatEditLayerMenu"; }
int UIScene_SuperFlatEditLayer::getSceneType() { return 58; }
