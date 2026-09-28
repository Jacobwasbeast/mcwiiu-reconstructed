#include "game/ui/UIControl_GridList.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_List2D.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_GridList::UIControl_GridList()
{
    m_currentSelection = 0;
    m_itemIndex = 0;
}

UIControl_GridList::~UIControl_GridList()
{
    std::vector<GenericPendingUpdate*>::iterator update = m_pendingUpdates.begin();
    while (update != m_pendingUpdates.end()) {
        delete *update;
        ++update;
    }
}

bool UIControl_GridList::CanTouchFocus(int x, int y)
{
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node->isFJ_List2D())
        return node->asFJ_List2D()->CanTouchFocus(x, y);
    return false;
}

void UIControl_GridList::SetTouchFocus(int x, int y, bool focus)
{
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node->isFJ_List2D())
        node->asFJ_List2D()->SetTouchFocus(x, y, focus);
}

bool UIControl_GridList::CanTouchTrigger(int x, int y)
{
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node->isFJ_List2D())
        return node->asFJ_List2D()->CanTouchTrigger(x, y);
    return false;
}

void UIControl_GridList::addElement(UIControl::eUIControlType type, int index)
{
    if (index < 0)
        index = m_itemIndex;
    ++m_itemIndex;
    m_elementTypes[(unsigned int)index] = type;
    m_elementEnabled[(unsigned int)index] = true;
    m_elements.push_back(index);
}

void UIControl_GridList::clearElements()
{
    m_itemIndex = 0;
    m_currentSelection = 0;
    m_elementTypes.clear();
    m_elementEnabled.clear();
    m_elements.clear();
}

int UIControl_GridList::getListIndex(int index)
{
    std::vector<int>::iterator element = m_elements.begin();
    int listIndex = 0;
    while (element != m_elements.end()) {
        if (*element == index)
            return listIndex;
        ++element;
        ++listIndex;
    }
    return -1;
}

void UIControl_GridList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    UIControl_Base::setupControl(scene, node, name);
}

void UIControl_GridList::clearList()
{
    m_fuiRenderNode->getFuiNode()->asFJ_List2D()->removeAllItems();
    clearElements();
}

void UIControl_GridList::init(int controlId)
{
    m_controlId = controlId;
    FJ_List2D* list = m_fuiRenderNode->getFuiNode()->asFJ_List2D();
    list->Init(controlId);
    int sceneType = m_scene->getSceneType();
    if (sceneType == 4 || (sceneType > 5 && sceneType < 8))
        gConsoleUIController.TouchBoxAdd(this, m_scene);
}

void UIControl_GridList::ReInit()
{
    UIControl::ReInit();
    init(m_controlId);
    clearElements();
    m_pendingUpdates.erase(m_pendingUpdates.begin(), m_pendingUpdates.end());
}

void UIControl_GridList::tick()
{
    UIControl_Base::tick();
    std::vector<GenericPendingUpdate*>::iterator update = m_pendingUpdates.begin();
    while (update != m_pendingUpdates.end()) {
        (*update)->OnTick();
        delete *update;
        ++update;
    }
    m_pendingUpdates.erase(m_pendingUpdates.begin(), m_pendingUpdates.end());
}

void UIControl_GridList::setCurrentSelection(int selection, bool immediate)
{
    if (immediate) {
        m_fuiRenderNode->getFuiNode()->asFJ_List2D()->HighlightItem(selection);
        return;
    }
    m_pendingUpdates.push_back(new PendingSetCurrentSelection(this, selection));
}

void UIControl_GridList::EnableItem(int item, bool enabled, bool immediate)
{
    if (immediate) {
        m_fuiRenderNode->getFuiNode()->asFJ_List2D()->EnableItem(getListIndex(item), enabled);
        return;
    }
    m_pendingUpdates.push_back(new PendingSetEnable(this, item, enabled));
}

UIControl_GridList::GenericPendingUpdate::~GenericPendingUpdate()
{
}

UIControl_GridList::PendingSetCurrentSelection::~PendingSetCurrentSelection()
{
}

unsigned int UIControl_GridList::PendingSetCurrentSelection::OnTick()
{
    m_list->setCurrentSelection(m_value, true);
    return 1;
}

UIControl_GridList::PendingSetEnable::~PendingSetEnable()
{
}

unsigned int UIControl_GridList::PendingSetEnable::OnTick()
{
    m_list->EnableItem(m_value, m_enabled, true);
    return 1;
}
