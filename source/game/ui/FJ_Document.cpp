#include "game/ui/FJ_Document.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_Document::FJ_Document(fuiRenderNode* renderNode)
    : FJ_FuiNode(renderNode), m_safeZoneTop(0), m_safeZoneBottom(0), m_safeZoneLeft(0), m_safeZoneRight(0),
      m_root(this), m_focus(0), m_horizontalResize(0), m_horizontalOffset(0), m_focusChange(true)
{
}

FJ_Document::~FJ_Document()
{
}

int FJ_Document::getISafeZoneTop() { return m_safeZoneTop; }
int FJ_Document::getISafeZoneBottom() { return m_safeZoneBottom; }
int FJ_Document::getISafeZoneLeft() { return m_safeZoneLeft; }
int FJ_Document::getISafeZoneRight() { return m_safeZoneRight; }

void FJ_Document::_init(FJ_FuiNode* node, FJ_Event* event)
{
    FJ_Document* document = node == NULL ? NULL : node->asFJ_Document();
    if (document != NULL) document->init(event);
}

void FJ_Document::HandleOnFocusChangeInit(FJ_Event* event)
{
    OnFocusChange(false);
}

void FJ_Document::_HandleOnFocusChangeInit(FJ_FuiNode* node, FJ_Event* event)
{
    FJ_Document* document = node == NULL ? NULL : node->asFJ_Document();
    if (document != NULL) document->HandleOnFocusChangeInit(event);
}

void FJ_Document::HandleOnFocusChangeUser(FJ_Event* event)
{
    OnFocusChange(true);
}

void FJ_Document::_HandleOnFocusChangeUser(FJ_FuiNode* node, FJ_Event* event)
{
    FJ_Document* document = node == NULL ? NULL : node->asFJ_Document();
    if (document != NULL) document->HandleOnFocusChangeUser(event);
}

void FJ_Document::_handlePress(FJ_FuiNode* node, FJ_Event* event)
{
    FJ_Document* document = node == NULL ? NULL : node->asFJ_Document();
    if (document != NULL) document->handlePress(event);
}

void FJ_Document::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    FJ_Document* document = node == NULL ? NULL : node->asFJ_Document();
    if (document != NULL) document->keyDownHandler(event);
}

void FJ_Document::init(FJ_Event* event)
{
}

void FJ_Document::SetFocus(int id)
{
    m_focus = id;
    FJ_FuiNodeStage* stage = getStage();
    if (stage == NULL) return;
    for (int i = 0; i < m_root->getNumChildren(); ++i)
    {
        FJ_FuiNode* child = m_root->getChildAt(i);
        if (child != NULL && (id < 0 || child->getTabIndex() == id))
        {
            stage->setFocus(child);
            return;
        }
    }
}

void FJ_Document::SetSafeZone(int top, int bottom, int left, int right)
{
    m_safeZoneTop = top;
    m_safeZoneBottom = bottom;
    m_safeZoneLeft = left;
    m_safeZoneRight = right;
}

void FJ_Document::handlePress(FJ_Event* event)
{
}

void FJ_Document::keyDownHandler(FJ_Event* event)
{
}

void FJ_Document::OnFocusChange(bool focused)
{
    m_focusChange = focused;
}

FJ_FuiNode* FJ_Document::GetTopObject(FJ_FuiNode* node)
{
    FJ_FuiNode* top = node;
    while (top != NULL && top->getParent() != NULL && top->getParent() != this)
    {
        top = top->getParent();
    }
    return top;
}

void FJ_Document::RepositionItems(FJ_FuiNode* first, FJ_FuiNode* second, bool horizontal)
{
    if (first == NULL || second == NULL) return;
    if (horizontal) second->setX(first->getX() + first->getWidth());
    else second->setY(first->getY() + first->getHeight());
}

void FJ_Document::FixNavigation(FJ_FuiNode* node)
{
    if (node != NULL && node->getTabIndex() < 0) node->setTabIndex(0);
}

void FJ_Document::RemoveObject(const std::string& name, bool destroy)
{
    FJ_FuiNode* node = getChildByName(name);
    if (node != NULL) removeChild(node, destroy);
}

void FJ_Document::DoHorizontalResizeCheck()
{
    m_horizontalResize = 1;
}

void FJ_Document::ResetHorizontalResizeCheck()
{
    m_horizontalResize = 0;
    m_horizontalOffset = 0;
}

void FJ_Document::SlideLeft()
{
    m_horizontalOffset -= 1;
}

void FJ_Document::SlideRight()
{
    m_horizontalOffset += 1;
}

int FJ_Document::getFrameByLabel(const std::string& label)
{
    return FJ_FuiNode::getFrameByLabel(label);
}

void FJ_Document::SliderClosed()
{
    m_horizontalResize = 0;
}

void FJ_Document::SliderOpen()
{
    m_horizontalResize = 1;
}
