#include "game/ui/FJ_MessageBox.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_MessageBox::FJ_MessageBox(fuiRenderNode* renderNode)
    : FJ_Document(renderNode), m_buttonCount(0), m_titleHeight(0), m_contentY(0)
{
    m_titleHeight = static_cast<int>(getChildByName(std::string("Title"))->getHeight());
    m_contentY = static_cast<int>(getChildByName(std::string("Content"))->getY());
}

FJ_Document* FJ_MessageBox::Create(fuiRenderNode* renderNode)
{
    return new FJ_MessageBox(renderNode);
}

unsigned int FJ_MessageBox::getType()
{
    return 15;
}

void FJ_MessageBox::Init(int buttonCount, int focusIndex)
{
    FJ_FuiNode* button0 = getChildByName(std::string("Button0"));
    FJ_FuiNode* button1 = getChildByName(std::string("Button1"));
    FJ_FuiNode* button2 = getChildByName(std::string("Button2"));
    FJ_FuiNode* button3 = getChildByName(std::string("Button3"));

    m_buttonCount = buttonCount;
    m_focusChange = false;
    if (buttonCount > 4)
    {
        return;
    }

    if (buttonCount < 4)
    {
        button0->setVisible(false);
        if (buttonCount < 3)
        {
            button1->setVisible(false);
        }
        if (buttonCount < 2)
        {
            button2->setVisible(false);
        }
        if (buttonCount == 1)
        {
            FJ_Base* base = button3->asFJ_Base();
            base->m_navUpName.clear();
            base->m_navDownName.clear();
            getStage()->setFocus(button3);
            return;
        }
    }

    FJ_FuiNode* focus = NULL;
    if (buttonCount == 2)
    {
        FJ_Base* button2Base = button2->asFJ_Base();
        FJ_Base* button3Base = button3->asFJ_Base();
        button2Base->m_navUpName = button3->getName();
        button3Base->m_navDownName = button2->getName();
        if (focusIndex == 0)
        {
            focus = button2;
        }
        else if (focusIndex == 1)
        {
            focus = button3;
        }
    }
    else if (buttonCount == 3)
    {
        FJ_Base* button1Base = button1->asFJ_Base();
        FJ_Base* button3Base = button3->asFJ_Base();
        button1Base->m_navUpName = button3->getName();
        button3Base->m_navDownName = button1->getName();
        if (focusIndex == 0)
        {
            focus = button1;
        }
        else if (focusIndex == 1)
        {
            focus = button2;
        }
        else if (focusIndex == 2)
        {
            focus = button3;
        }
    }
    else if (buttonCount == 4)
    {
        if (focusIndex == 0)
        {
            focus = button0;
        }
        else if (focusIndex == 1)
        {
            focus = button1;
        }
        else if (focusIndex == 2)
        {
            focus = button2;
        }
        else if (focusIndex == 3)
        {
            focus = button3;
        }
    }
    if (focus != NULL)
    {
        getStage()->setFocus(focus);
    }
}

void FJ_MessageBox::AutoResize()
{
    FJ_FuiNode* button0 = getChildByName(std::string("Button0"));
    FJ_FuiNode* button1 = getChildByName(std::string("Button1"));
    FJ_FuiNode* button2 = getChildByName(std::string("Button2"));
    FJ_FuiNode* button3 = getChildByName(std::string("Button3"));
    FJ_Base* content = getChildByName(std::string("Content"))->asFJ_Base();
    FJ_Base* title = getChildByName(std::string("Title"))->asFJ_Base();
    FJ_FuiNode* background = getChildByName(std::string("BackgroundPanel"));

    const int oldBackgroundWidth = static_cast<int>(background->getWidth());
    DoHorizontalResizeCheck();
    const int backgroundWidth = static_cast<int>(background->getWidth());
    if (oldBackgroundWidth < backgroundWidth)
    {
        const int widthDelta = backgroundWidth - oldBackgroundWidth;
        title->SetNewObjectWidth(static_cast<int>(title->getWidth()) + widthDelta);
        content->SetNewObjectWidth(static_cast<int>(content->getWidth()) + widthDelta);
    }

    const float titleHeight = title->getHeight();
    if (titleHeight > static_cast<float>(m_titleHeight))
    {
        const int extraHeight = static_cast<int>(titleHeight) - m_titleHeight;
        if (extraHeight > 0)
        {
            content->setY(static_cast<float>(m_contentY + extraHeight));
        }
        m_titleHeight = static_cast<int>(titleHeight);
    }

    float offset = 0.0f;
    if (m_buttonCount == 1)
    {
        offset = static_cast<float>(Math::round(button3->getHeight() * 0.5f + content->getY() + content->getHeight() - button3->getY()));
        button3->incY(offset);
    }
    else if (m_buttonCount == 2)
    {
        offset = static_cast<float>(Math::round(button2->getHeight() * 0.5f + content->getY() + content->getHeight() - button2->getY()));
        button2->incY(offset);
        button3->incY(offset);
    }
    else if (m_buttonCount == 3)
    {
        offset = static_cast<float>(Math::round(button1->getHeight() * 0.5f + content->getY() + content->getHeight() - button1->getY()));
        button1->incY(offset);
        button2->incY(offset);
        button3->incY(offset);
    }
    else if (m_buttonCount == 4)
    {
        offset = static_cast<float>(Math::round(button0->getHeight() * 0.5f + content->getY() + content->getHeight() - button0->getY()));
        button0->incY(offset);
        button1->incY(offset);
        button2->incY(offset);
        button3->incY(offset);
    }

    background->getChildAt(0)->asMovieClip()->incHeight(offset);
    const float halfOffset = static_cast<float>(Math::round(offset * 0.5f));
    int i;
    for (i = 0; i < getNumChildren(); ++i)
    {
        getChildAt(i)->decY(halfOffset);
    }
}
