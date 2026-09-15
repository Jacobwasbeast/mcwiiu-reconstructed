#include "game/ui/FJ_Document.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/FJ_List2D.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_Document::FJ_Document(fuiRenderNode* renderNode)
    : FJ_FuiNode(renderNode), m_safeZoneTop(0), m_safeZoneBottom(0), m_safeZoneLeft(0), m_safeZoneRight(0),
      m_root(this), m_focus(NULL), m_horizontalResize(0), m_horizontalOffset(0), m_focusChange(true)
{
    addEventListener(eFJEventType_Init, this, _init, false);
    getStage()->addEventListener(eFJEventType_FocusChangeInit, this, _HandleOnFocusChangeInit, false);
    getStage()->addEventListener(eFJEventType_FocusChangeUser, this, _HandleOnFocusChangeUser, false);
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
    if (!stage->getFocus()->isFJ_Base())
        return;

    FJ_FuiNode* previous = stage->getFocus();
    if (previous == NULL)
        return;
    if (previous->asFJ_Base() == NULL)
        return;

    FJ_Base* base = previous->asFJ_Base();

    const int code = static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode;
    if (code == 3)
    {
        if (!base->m_navLinkName[0].empty() && m_root->getChildByName(base->m_navLinkName[0]) != NULL)
        {
            FJ_FuiNode* child = m_root->getChildByName(base->m_navLinkName[0]);
            stage->setFocus(child);
            FJ_FuiNode* focus = stage->getFocus();
            if (focus->asFJ_Base()->getObjListParent() != NULL && focus->isFJ_Label())
                keyDownHandler(event);
        }
    }
    else if (code == 4)
    {
        if (!base->m_navLinkName[1].empty() && m_root->getChildByName(base->m_navLinkName[1]) != NULL)
        {
            FJ_FuiNode* child = m_root->getChildByName(base->m_navLinkName[1]);
            stage->setFocus(child);
            FJ_FuiNode* focus = stage->getFocus();
            if (focus->asFJ_Base()->getObjListParent() != NULL && focus->isFJ_Label())
                keyDownHandler(event);
        }
    }
    else if (code == 5)
    {
        if (!base->m_navLinkName[2].empty() && m_root->getChildByName(base->m_navLinkName[2]) != NULL)
        {
            FJ_FuiNode* child = m_root->getChildByName(base->m_navLinkName[2]);
            stage->setFocus(child);
        }
    }
    else if (code == 6)
    {
        if (!base->m_navLinkName[3].empty() && m_root->getChildByName(base->m_navLinkName[3]) != NULL)
        {
            FJ_FuiNode* child = m_root->getChildByName(base->m_navLinkName[3]);
            stage->setFocus(child);
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
    if (getStage()->getFocus() == NULL)
        return;

    const bool changed = m_focus != getStage()->getFocus();
    m_focus = getStage()->getFocus();

    FJ_Base* base = getStage()->getFocus()->asFJ_Base();
    if (m_pCallbackScene == NULL || base == NULL)
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
            {
                m_pCallbackScene->_handleFocusChange(base->getId(), 0);
                return;
            }
            m_pCallbackScene->_handleInitFocus(base->getId(), 0);
        }
        return;
    }

    if (parent->isFJ_List())
    {
        const int parentId = parent->asFJ_List()->getId();
        const int childId = base->getId();
        if (changed || parentId != m_horizontalResize || childId != m_horizontalOffset)
        {
            m_horizontalOffset = childId;
            m_horizontalResize = parentId;
            if (focused)
            {
                m_pCallbackScene->_handleFocusChange(parentId, childId);
                return;
            }
            m_pCallbackScene->_handleInitFocus(parentId, childId);
            return;
        }
    }
    else if (parent->isFJ_List2D())
    {
        const int parentId = parent->asFJ_List2D()->getId();
        const int childId = base->getId();
        if (changed || parentId != m_horizontalResize || childId != m_horizontalOffset)
        {
            m_horizontalOffset = childId;
            m_horizontalResize = parentId;
            if (focused)
            {
                m_pCallbackScene->_handleFocusChange(parentId, childId);
                return;
            }
            m_pCallbackScene->_handleInitFocus(parentId, childId);
            return;
        }
    }
}

FJ_FuiNode* FJ_Document::GetTopObject(FJ_FuiNode* node)
{
    int y = static_cast<int>(node->getY());
    for (;;)
    {
        FJ_Base* base = node->asFJ_Base();
        if (base->m_navLinkName[0].empty())
            return node;
        FJ_FuiNode* candidate = FJ_FuiNode::asMovieClip(m_root->getChildByName(base->m_navLinkName[0]));
        FJ_Base* candidateBase = candidate->asFJ_Base();
        if (base->m_navLinkName[0] == candidateBase->m_navLinkName[0])
            return node;
        if (candidate == NULL)
            return node;
        int candidateY = static_cast<int>(candidate->getY());
        if (y < candidateY)
            break;
        y = static_cast<int>(candidate->getY());
        node = candidate;
    }
    return node;
}

void FJ_Document::RepositionItems(FJ_FuiNode* first, FJ_FuiNode* second, bool horizontal)
{
    int prevY = 0;
    int prevHeight = 0;
    int bottomA = 0;
    int bottomB = 0;
    int shiftedCount = 0;
    FJ_FuiNode* node = second;
    for (;;)
    {
        bool hasListParent = false;
        if (node->isFJ_Base() && node->asFJ_Base()->getObjListParent() != NULL)
            hasListParent = true;

        if (node->isFJ_List() || node->isFJ_List2D() || hasListParent)
        {
            node = FJ_FuiNode::asMovieClip(m_root->getChildByName(node->asFJ_Base()->m_navLinkName[1]));
        }
        else if (node == second)
        {
            prevHeight = Math::round(node->getHeight());
            prevY = Math::round(node->getY());
            node = FJ_FuiNode::asMovieClip(m_root->getChildByName(node->asFJ_Base()->m_navLinkName[1]));
        }
        else
        {
            const int curY = Math::round(node->getY());
            const int shift = Math::round(second->getHeight() + static_cast<float>((curY - prevY) - prevHeight));
            node->decY(static_cast<float>(shift));
            bottomB = Math::round(node->getY() + node->getHeight());
            prevHeight = Math::round(node->getHeight());
            ++shiftedCount;
            node = FJ_FuiNode::asMovieClip(m_root->getChildByName(node->asFJ_Base()->m_navLinkName[1]));
            prevY = curY;
        }

        if (node == NULL || node == first)
            break;
        bottomA = Math::round(node->getY() + node->getHeight());
    }

    if (bottomA == 0 && bottomB == 0)
    {
        FJ_FuiNode* candidate = FJ_FuiNode::asMovieClip(m_root->getChildByName(second->asFJ_Base()->m_navLinkName[0]));
        if (candidate != NULL)
        {
            bottomA = Math::round(second->getY() + second->getHeight());
            bottomB = Math::round(candidate->getY() + candidate->getHeight());
        }
    }

    if (shiftedCount < 1 && horizontal)
        horizontal = false;

    if (horizontal)
    {
        const float amount = static_cast<float>(bottomA - bottomB) * 0.5f;
        int i = 0;
        int count = m_root->getNumChildren();
        while (i < count)
        {
            FJ_FuiNode* child = m_root->getChildAt(i);
            if (!child->isFJ_HtmlText())
            {
                bool skip = child->isFJ_Base() && child->asFJ_Base()->getObjListParent() != NULL;
                if (!skip)
                {
                    bool shouldMove = true;
                    if (!child->isFJ_Base())
                    {
                        std::string name = child->getName();
                        if (name != std::string("BackgroundPanel") && name != std::string("PlayerIcon"))
                            shouldMove = false;
                    }
                    if (shouldMove)
                        child->incY(static_cast<float>(Math::round(amount)));
                }
            }
            ++i;
            count = m_root->getNumChildren();
        }
    }

    FJ_FuiNode* background = m_root->getChildByName(std::string("BackgroundPanel"));
    if (background != NULL)
    {
        FJ_FuiNode* fill = background->getChildAt(0)->asMovieClip();
        if (fill != NULL)
            fill->decHeight(static_cast<float>(bottomA - bottomB));
    }
}

void FJ_Document::FixNavigation(FJ_FuiNode* node)
{
    FJ_Base* base = node->asFJ_Base();
    if (!base->m_navLinkName[1].empty())
    {
        FJ_FuiNode* candidate = FJ_FuiNode::asMovieClip(m_root->getChildByName(base->m_navLinkName[1]));
        if (candidate != NULL)
        {
            FJ_Base* candidateBase = candidate->asFJ_Base();
            base = node->asFJ_Base();
            candidateBase->m_navLinkName[0] = base->m_navLinkName[0];
            if (getStage()->getFocus() == node)
                getStage()->setFocus(candidate);
            if (node->getTabIndex() == 1)
            {
                node->setTabIndex(2);
                candidate->setTabIndex(1);
            }
        }
    }
    base = node->asFJ_Base();
    if (!base->m_navLinkName[0].empty())
    {
        FJ_FuiNode* candidate = FJ_FuiNode::asMovieClip(m_root->getChildByName(base->m_navLinkName[0]));
        if (candidate != NULL)
        {
            FJ_Base* candidateBase = candidate->asFJ_Base();
            base = node->asFJ_Base();
            candidateBase->m_navLinkName[1] = base->m_navLinkName[1];
            if (getStage()->getFocus() == node)
                getStage()->setFocus(candidate);
            if (node->getTabIndex() == 1)
                candidate->setTabIndex(1);
        }
    }
}

void FJ_Document::RemoveObject(const std::string& name, bool destroy)
{
    FJ_FuiNode* node = FJ_FuiNode::asMovieClip(getChildByName(name));
    if (node != NULL && node->isFJ_Base())
    {
        FJ_FuiNode* top = GetTopObject(node);
        RepositionItems(top, node, destroy);
        FixNavigation(node);
        node->getParent()->removeChild(node, true);
    }
}

void FJ_Document::DoHorizontalResizeCheck()
{
    bool resized = false;
    int widest = 0;
    int left = 0;
    int oldWidth = 0;
    FJ_FuiNode* backgroundMovie = getChildByName("BackgroundPanel");
    if (backgroundMovie != NULL)
        backgroundMovie = backgroundMovie->asMovieClip();

    for (int i = 0; i < m_root->getNumChildren(); ++i)
    {
        FJ_FuiNode* child = m_root->getChildAt(i);
        FJ_Base* base = child == NULL ? NULL : child->asFJ_Base();
        if (base == NULL)
            continue;

        if (!base->m_manualWrapFits || !base->m_multilineWrapFits)
        {
            if (static_cast<int>(base->getNewWidth()) > widest)
            {
                widest = static_cast<int>(base->getNewWidth());
                left = static_cast<int>(child->getX());
                oldWidth = base->getOldWidth();
            }
            if (base->getLabelTooWide() != 0)
                resized = true;
        }
    }

    if (backgroundMovie != NULL &&
        static_cast<float>(widest) + (static_cast<float>(left) - backgroundMovie->getX()) * 2.0F > backgroundMovie->getWidth())
    {
        oldWidth = static_cast<int>(backgroundMovie->getWidth() -
                                    (static_cast<float>(left) - backgroundMovie->getX()) * 2.0F);
        resized = true;
    }

    if (resized)
    {
        const float offset = static_cast<float>(widest - oldWidth);
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
                child->decX(static_cast<float>(Math::round(offset * 0.5F)));
            else
                child->setX(parent->getX());
        }

        if (backgroundMovie != NULL)
        {
            backgroundMovie->getChildAt(0)->setWidth(
                static_cast<float>(Math::round(backgroundMovie->getWidth() +
                                                static_cast<float>(widest) - oldWidth)));
            backgroundMovie->decX(static_cast<float>(Math::round(
                (static_cast<float>(widest) - oldWidth) * 0.5F)));
        }

        FJ_FuiNode* worldOptions = getChildByName("WorldOptionsBox");
        FJ_FuiNode* worldMovie = worldOptions == NULL ? NULL : worldOptions->asMovieClip();
        if (worldMovie != NULL)
        {
            const float delta = static_cast<float>(widest - oldWidth);
            worldMovie->incWidth(delta);
            worldMovie->decX(static_cast<float>(Math::round(delta * 0.5F)));
        }
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
