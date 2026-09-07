#include "game/ui/UIControl_List.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_List::UIControl_List()
{
    m_currentSelection = 0;
    m_itemIndex = 0;
}

UIControl_List::~UIControl_List()
{
    std::vector<GenericPendingUpdate*>::iterator update = m_pendingUpdates.begin();
    while (update != m_pendingUpdates.end()) {
        delete *update;
        ++update;
    }
}

void UIControl_List::addElement(UIControl::eUIControlType type, int index)
{
    if (index < 0)
        index = m_itemIndex;
    ++m_itemIndex;
    m_elementTypes[(unsigned int)index] = type;
    m_elementEnabled[(unsigned int)index] = true;
    m_elements.push_back(index);
}

void UIControl_List::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    UIControl_Base::setupControl(scene, node, name);
}

void UIControl_List::GetTopItem()
{
    m_fuiRenderNode->getFuiNode()->asFJ_List()->GetTopItem();
}

void UIControl_List::setCurrentSelection(int selection, bool update, bool immediate)
{
    if (immediate) {
        m_fuiRenderNode->getFuiNode()->asFJ_List()->HighlightItem(selection, update);
        return;
    }
    m_pendingUpdates.push_back(new PendingSetCurrentSelection(this, selection, update));
}

void UIControl_List::updateChildFocus(int index)
{
    m_currentSelection = index;
}

unsigned int UIControl_List::getItemCount()
{
    return (unsigned int)m_itemIndex;
}

unsigned int UIControl_List::getCurrentSelection() const
{
    return (unsigned int)m_currentSelection;
}

void UIControl_List::init(int controlId)
{
    m_controlType = controlId;
    FJ_List* list = m_fuiRenderNode->getFuiNode()->asFJ_List();
    list->Init(controlId);
    int sceneType = m_scene->getSceneType();
    if (sceneType == 4 || (sceneType > 5 && sceneType < 8))
        gConsoleUIController.TouchBoxAdd(this, m_scene);
}

int UIControl_List::getListIndex(int index)
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

void UIControl_List::EnableItem(int item, bool enabled, bool immediate)
{
    if (!immediate) {
        m_pendingUpdates.push_back(new PendingSetEnable(this, item, enabled));
        return;
    }
    m_elementEnabled[(unsigned int)item] = enabled;
    FJ_List* list = m_fuiRenderNode->getFuiNode()->asFJ_List();
    if (list != NULL)
        list->EnableItem(getListIndex(item), enabled);
}

bool UIControl_List::istype(UIControl::eUIControlType type, int index)
{
    return m_elementTypes[(unsigned int)index] == type;
}

void UIControl_List::clearElements()
{
    m_itemIndex = 0;
    m_currentSelection = 0;
    m_elementTypes.clear();
    m_elementEnabled.clear();
    m_elements.clear();
}

void UIControl_List::clearList()
{
    m_fuiRenderNode->getFuiNode()->asFJ_List()->removeAllItems();
    clearElements();
}

void UIControl_List::SetItemLabel(int item, UIString label, bool immediate)
{
    FJ_List* list = m_fuiRenderNode->getFuiNode()->asFJ_List();
    if (list == NULL)
        return;
    if (!immediate) {
        m_pendingUpdates.push_back(new PendingSetLabel(this, item, label));
        return;
    }
    if (m_scene->hasMovie()) {
        list->SetItemLabel(getListIndex(item), label.getString());
    }
}

void UIControl_List::remElement(int index)
{
    --m_itemIndex;
    m_elementTypes.erase((unsigned int)index);
    m_elementEnabled.erase((unsigned int)index);
    std::vector<int>::iterator element = m_elements.begin();
    while (element != m_elements.end()) {
        if (*element == index) {
            m_elements.erase(element);
            return;
        }
        ++element;
    }
}

void UIControl_List::ReInit()
{
    UIControl::ReInit();
    init(m_controlType);
    clearElements();
    m_pendingUpdates.erase(m_pendingUpdates.begin(), m_pendingUpdates.end());
}

void UIControl_List::tick()
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

void UIControl_List::removeItem(int item, bool immediate)
{
    if (immediate) {
        m_fuiRenderNode->getFuiNode()->asFJ_List()->RemoveItem(item);
        remElement(item);
        return;
    }
    m_pendingUpdates.push_back(new PendingRemoveItem(this, item));
}

unsigned char UIControl_List::IsEnabled(int index)
{
    return (unsigned char)m_elementEnabled[(unsigned int)index];
}

bool UIControl_List::CheckElementExists(int index)
{
    return m_elementTypes.find((unsigned int)index) != m_elementTypes.end();
}

int UIControl_List::GetActiveItem()
{
    return m_fuiRenderNode->getFuiNode()->asFJ_List()->GetActiveItem();
}

void UIControl_List::UpdateControl()
{
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node->isFJ_List())
        node->asFJ_List()->UpdateTouchSize();
    UIControl::UpdateControl();
}

void UIControl_List::SetTouchFocus(int x, int y, bool focus)
{
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node->isFJ_List())
        node->asFJ_List()->SetTouchFocus(x, y, focus);
}

bool UIControl_List::CanTouchFocus(int x, int y)
{
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node->isFJ_List())
        return node->asFJ_List()->CanTouchFocus(x, y);
    return false;
}

bool UIControl_List::CanTouchTrigger(int x, int y)
{
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node->isFJ_List())
        return node->asFJ_List()->CanTouchTrigger(x, y);
    return false;
}

UIControl_List::GenericPendingUpdate::~GenericPendingUpdate()
{
}

UIControl_List::PendingRemoveItem::~PendingRemoveItem()
{
}

unsigned int UIControl_List::PendingRemoveItem::OnTick()
{
    m_list->removeItem(m_item, true);
    return 1;
}

UIControl_List::PendingSetCurrentSelection::~PendingSetCurrentSelection()
{
}

unsigned int UIControl_List::PendingSetCurrentSelection::OnTick()
{
    m_list->setCurrentSelection(m_item, m_update, true);
    return 1;
}

UIControl_List::PendingSetEnable::~PendingSetEnable()
{
}

unsigned int UIControl_List::PendingSetEnable::OnTick()
{
    m_list->EnableItem(m_item, m_enabled, true);
    return 1;
}

UIControl_List::PendingSetLabel::~PendingSetLabel()
{
}

unsigned int UIControl_List::PendingSetLabel::OnTick()
{
    m_list->SetItemLabel(m_item, m_label, true);
    return 1;
}
