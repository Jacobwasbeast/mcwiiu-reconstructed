#include "game/ui/FJ_Document.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/FJ_List2D.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_Document::FJ_Document(fuiRenderNode* renderNode)
    : FJ_FuiNode(renderNode), m_safeZoneTop(0), m_safeZoneBottom(0), m_safeZoneLeft(0), m_safeZoneRight(0),
      m_root(this), m_focus(NULL), m_horizontalResize(0), m_horizontalOffset(0), m_focusChange(true)
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
    m_root->addEventListener(eFJEventType_Press, this, FJ_Document::_handlePress, false);
    FJ_FuiNodeStage* stage = getStage();
    if (stage != NULL)
        stage->addEventListener(eFJEventType_KeyDown, this, FJ_Document::_keyDownHandler, false);
}

void FJ_Document::SetFocus(int id)
{
    FJ_FuiNodeStage* stage = getStage();
    if (stage == NULL)
        return;

    if (id >= 0)
    {
        for (int i = 0; i < m_root->getNumChildren(); ++i)
        {
            FJ_FuiNode* child = m_root->getChildAt(i);
            FJ_Base* base = child == NULL ? NULL : child->asFJ_Base();
            if (base != NULL && base->getObjListParent() == NULL && base->getId() == id)
            {
                stage->setFocus(child);
                return;
            }
        }
    }

    if (m_focusChange)
    {
        FJ_FuiNode* selected = NULL;
        for (int i = 0; i < m_root->getNumChildren(); ++i)
        {
            FJ_FuiNode* child = m_root->getChildAt(i);
            if (child != NULL && child->getTabIndex() == 1)
            {
                selected = child;
                break;
            }
        }
        if (selected == NULL && m_root->getNumChildren() > 0)
            selected = m_root->getChildAt(0);
        if (selected != NULL)
            stage->setFocus(selected);
        if (m_pCallbackScene != NULL)
            m_pCallbackScene->handleInitFocus(0, 0);
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
    FJ_Base* base = event->m_pTarget->asFJ_Base();
    FJ_Button* button = event->m_pTarget->asFJ_Button();

    FJ_FuiNode* parent = base->getObjListParent();
    if (button == NULL || parent == NULL)
    {
        m_pCallbackScene->handlePress(base->getId(), 0);
        if (button != NULL)
            button->setButtonPressed(false);
        return;
    }

    if (parent->isFJ_List())
    {
        m_pCallbackScene->handlePress(parent->asFJ_Base()->getId(), base->getId());
        button->setButtonPressed(false);
        return;
    }

    if (parent->isFJ_List2D())
    {
        m_pCallbackScene->handlePress(parent->asFJ_Base()->getId(), base->getId());
        button->setButtonPressed(false);
        return;
    }

    if (button != NULL)
        button->setButtonPressed(false);
}

void FJ_Document::keyDownHandler(FJ_Event* event)
{
    FJ_FuiNodeStage* stage = getStage();
    FJ_FuiNode* previous = stage->getFocus();
    if (!previous->isFJ_Base())
        return;

    FJ_Base* base = stage->getFocus()->asFJ_Base();

    const int code = static_cast<int>(event->m_eType);
    if (code == 3 || code == 4 || code == 5 || code == 6)
    {
        const char* names[] = {"FocusOut", "FocusIn", "FocusChangeInit", "FocusChangeUser"};
        FJ_FuiNode* child = base->getChildByName(names[code - 3]);
        if (child != NULL)
        {
            stage->setFocus(child);
            FJ_FuiNode* focus = stage->getFocus();
            if (focus->asFJ_Base()->getObjListParent() != NULL && focus->isFJ_Label())
                dispatchEvent(event);
        }
    }
    else if (code == 1 || code == 2)
    {
        FJ_FuiNode* parent = base->getObjListParent();
        if (parent != NULL && parent->isFJ_List())
            parent->asFJ_List()->HandlePageUpDownNavigation(code == 1);
        else if (parent != NULL && parent->isFJ_List2D())
            parent->asFJ_List2D()->HandlePageUpDownNavigation(code == 1);
    }

    if (previous->isFJ_Base())
        previous->asFJ_Base()->LostFocus();
    OnFocusChange(true);
}

void FJ_Document::OnFocusChange(bool focused)
{
    FJ_FuiNode* focus = getStage()->getFocus();
    if (focus == NULL)
        return;

    const bool changed = m_focus != focus;
    m_focus = focus;
    FJ_Base* base = focus->asFJ_Base();
    if (base == NULL)
        return;

    FJ_FuiNode* parent = base->getObjListParent();
    if (parent == NULL)
    {
        const int id = base->getId();
        if (changed || id != m_horizontalOffset)
        {
            m_horizontalOffset = id;
            m_horizontalResize = 0;
            if (focused)
                m_pCallbackScene->handleFocusChange(id, 0);
            else
                m_pCallbackScene->handleInitFocus(id, 0);
        }
        return;
    }

    FJ_Base* parentBase = parent->asFJ_Base();
    if (parentBase == NULL || (!parent->isFJ_List() && !parent->isFJ_List2D()))
        return;

    const int parentId = parentBase->getId();
    const int childId = base->getId();
    if (changed || parentId != m_horizontalResize || childId != m_horizontalOffset)
    {
        m_horizontalResize = parentId;
        m_horizontalOffset = childId;
        if (focused)
            m_pCallbackScene->handleFocusChange(parentId, childId);
        else
            m_pCallbackScene->handleInitFocus(parentId, childId);
    }
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
    bool resized = false;
    unsigned int widest = 0;
    unsigned int left = 0;
    unsigned int oldWidth = 0;
    FJ_FuiNode* backgroundMovie = getChildByName("BackgroundPanel");
    if (backgroundMovie != NULL)
        backgroundMovie = backgroundMovie->asMovieClip();

    for (int i = 0; i < m_root->getNumChildren(); ++i)
    {
        FJ_FuiNode* child = m_root->getChildAt(i);
        FJ_Base* base = child == NULL ? NULL : child->asFJ_Base();
        if (base == NULL)
            continue;

        if (base->getNewWidth() > widest)
        {
            widest = base->getNewWidth();
            left = static_cast<unsigned int>(child->getX());
            oldWidth = static_cast<unsigned int>(base->getOldWidth());
        }
        if (base->getLabelTooWide() != 0)
            resized = true;
    }

    const float offset = static_cast<float>(widest - left);
    if (backgroundMovie != NULL &&
        static_cast<float>(widest - left) + (static_cast<float>(left) - backgroundMovie->getX()) * 2.0F > backgroundMovie->getWidth())
    {
        const float innerWidth = backgroundMovie->getWidth() -
                                 (static_cast<float>(left) - backgroundMovie->getX()) * 2.0F;
        const float width = static_cast<float>(Math::round(backgroundMovie->getWidth() +
                                                            static_cast<float>(widest) - innerWidth));
        backgroundMovie->getChildAt(0)->setWidth(width);
        backgroundMovie->decX(static_cast<float>(Math::round((static_cast<float>(widest) - innerWidth) * 0.5F)));
        resized = true;
    }

    if (resized)
    {
        for (int i = 0; i < m_root->getNumChildren(); ++i)
        {
            FJ_FuiNode* child = m_root->getChildAt(i);
            FJ_Base* base = child == NULL ? NULL : child->asFJ_Base();
            if (base == NULL)
                continue;

            if (child->isFJ_Button() || child->isFJ_Slider() || child->isFJ_List() || child->isFJ_List2D())
                base->SetNewObjectWidth(static_cast<int>(widest));

            FJ_FuiNode* parent = base->getObjListParent();
            if (parent == NULL)
                child->decX(static_cast<float>(Math::round(static_cast<float>(widest - left) * 0.5F)));
            else
                child->setX(parent->getX());
        }
    }

    FJ_FuiNode* worldOptions = getChildByName("WorldOptionsBox");
    FJ_FuiNode* worldMovie = worldOptions == NULL ? NULL : worldOptions->asMovieClip();
    if (worldMovie != NULL && widest > left)
    {
        const float delta = static_cast<float>(widest - left);
        worldMovie->incWidth(delta);
        worldMovie->decX(static_cast<float>(Math::round(delta * 0.5F)));
    }

    for (int i = 0; i < m_root->getNumChildren(); ++i)
    {
        FJ_FuiNode* child = m_root->getChildAt(i);
        FJ_Base* base = child == NULL ? NULL : child->asFJ_Base();
        if (base != NULL)
            base->ResizingComplete();
    }
}

void FJ_Document::ResetHorizontalResizeCheck()
{
    for (int i = 0; i < m_root->getNumChildren(); ++i)
    {
        FJ_FuiNode* child = m_root->getChildAt(i);
        FJ_Base* base = child == NULL ? NULL : child->asFJ_Base();
        if (base != NULL)
            base->ResetResizing();
    }
}

void FJ_Document::SlideLeft()
{
    if (getChildByName(std::string("MainPanel")) != NULL)
        gotoAndPlay(std::string("Slide_Left"));
}

void FJ_Document::SlideRight()
{
    if (getChildByName(std::string("MainPanel")) != NULL)
        gotoAndPlay(std::string("Slide_Right"));
}
