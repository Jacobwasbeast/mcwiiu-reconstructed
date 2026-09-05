#include "game/ui/FJ_CreditsMenu.hpp"

#include <cstdio>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_CreditsMenu::FJ_CreditsMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode), m_scrollAmount(1.0f), m_scrollTop(130), m_scrollBottom(0),
      m_creditsContent(0), m_unused(0), m_requestingLabel(false)
{
    m_creditsContent = getChildByName("BlendingContainer");
    if (m_creditsContent != 0)
        m_creditsContent = m_creditsContent->getChildByName("CreditsContent");

    const double height = getStage()->getStageHeight();
    m_scrollBottom = static_cast<int>(height - 25.0);
    if (height == 408.0)
        m_scrollTop = 130;
    if (height == 1080.0)
        m_scrollTop = 110;
    if (height == 720.0)
    {
        m_scrollTop = 195;
        m_scrollBottom = static_cast<int>(height - 38.0);
    }
    if (height == 540.0)
        m_scrollAmount = 0.5f;
    if (height == 720.0)
        m_scrollAmount = 1.5f;
}

FJ_CreditsMenu::~FJ_CreditsMenu()
{
    removeEventListener(eFJEventType_AnimationStep, this, _UpdateScrolling);
}

FJ_Document* FJ_CreditsMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_CreditsMenu(renderNode);
}

unsigned int FJ_CreditsMenu::getType()
{
    return 88;
}

void FJ_CreditsMenu::RequestNextLabel()
{
    FJ_FuiNode* root = getRoot();
    if (root != 0 && root->m_pRenderNode != 0)
        root->getChildAt(0);
}

void FJ_CreditsMenu::UpdateScrolling(FJ_Event*)
{
    if (!m_requestingLabel && (m_scrollingNodes.empty() || m_scrollingNodes.size() < 25))
        RequestNextLabel();

    for (std::vector<FJ_FuiNode*>::iterator it = m_scrollingNodes.begin();
         it != m_scrollingNodes.end(); ++it)
    {
        FJ_FuiNode* node = *it;
        if (node != 0)
            node->setY(node->getY() - static_cast<float>(Math::round(m_scrollAmount * 2.0f)));
    }

    if (!m_scrollingNodes.empty())
    {
        FJ_FuiNode* first = m_scrollingNodes.front();
        if (first != 0 && first->getY() + first->getHeight() < static_cast<float>(m_scrollTop))
        {
            first->setVisible(false);
            m_scrollingNodes.erase(m_scrollingNodes.begin());
        }
    }
    if (m_scrollingNodes.empty())
        m_requestingLabel = false;
}

void FJ_CreditsMenu::_UpdateScrolling(FJ_FuiNode* node, FJ_Event* event)
{
    if (node != 0)
    {
        static_cast<FJ_CreditsMenu*>(node)->UpdateScrolling(event);
    }
}

void FJ_CreditsMenu::init(FJ_Event*)
{
    m_requestingLabel = false;
    addEventListener(eFJEventType_AnimationStep, this, _UpdateScrolling, false);

    char name[64];
    int i;
    for (i = 0; i < 25; ++i)
    {
        std::sprintf(name, "CreditsText_S_%d", i);
        m_smallLabels[i] = m_creditsContent == 0 ? 0 : m_creditsContent->getChildByName(name);
        if (m_smallLabels[i] != 0)
        {
            fuiRect rect = getRect(this);
            m_smallLabels[i]->setX(static_cast<float>(Math::round(rect.xMin -
                (m_smallLabels[i]->getWidth() * 0.5f - getWidth() * 0.5f))));
            m_smallLabels[i]->setY(0.0f);
            m_smallLabels[i]->setVisible(false);
        }
        if (i < 12)
        {
            std::sprintf(name, "CreditsText_M_%d", i);
            m_mediumLabels[i] = m_creditsContent == 0 ? 0 : m_creditsContent->getChildByName(name);
            if (m_mediumLabels[i] != 0)
            {
                fuiRect rect = getRect(this);
                m_mediumLabels[i]->setX(static_cast<float>(Math::round(rect.xMin -
                    (m_mediumLabels[i]->getWidth() * 0.5f - getWidth() * 0.5f))));
                m_mediumLabels[i]->setY(0.0f);
                m_mediumLabels[i]->setVisible(false);
            }
        }
        std::sprintf(name, "CreditsText_L_%d", i);
        m_largeLabels[i] = m_creditsContent == 0 ? 0 : m_creditsContent->getChildByName(name);
        if (m_largeLabels[i] != 0)
        {
            fuiRect rect = getRect(this);
            m_largeLabels[i]->setX(static_cast<float>(Math::round(rect.xMin -
                (m_largeLabels[i]->getWidth() * 0.5f - getWidth() * 0.5f))));
            m_largeLabels[i]->setY(0.0f);
            m_largeLabels[i]->setVisible(false);
        }
        if (i < 8)
        {
            std::sprintf(name, "CreditsText_XL_%d", i);
            m_largeLabels[20 + i] = m_creditsContent == 0 ? 0 : m_creditsContent->getChildByName(name);
            if (m_largeLabels[20 + i] != 0)
                m_largeLabels[20 + i]->setVisible(false);
        }
    }

    m_logoIggy = m_creditsContent == 0 ? 0 : m_creditsContent->getChildByName("Logo_Iggy");
    m_logoMiles = m_creditsContent == 0 ? 0 : m_creditsContent->getChildByName("Logo_Miles");
    m_logoDolby = m_creditsContent == 0 ? 0 : m_creditsContent->getChildByName("Logo_Dolby");
    m_logoNintendo = m_creditsContent == 0 ? 0 : m_creditsContent->getChildByName("Logo_Nintendo");
    fuiRect logoRect = getRect(this);
    FJ_FuiNode* logos[4] = { m_logoIggy, m_logoMiles, m_logoDolby, m_logoNintendo };
    for (i = 0; i < 4; ++i)
    {
        if (logos[i] != 0)
        {
            logos[i]->setX(static_cast<float>(Math::round(logoRect.xMin -
                (logos[i]->getWidth() * 0.5f - getWidth() * 0.5f))));
            logos[i]->setY(0.0f);
        }
    }
    FJ_FuiNode* logoObject = getChildByName("LogoObject");
    if (logoObject != 0 && logoObject->getChildAt(0) != 0)
        logoObject->getChildAt(0)->gotoAndStop(1);
}

void FJ_CreditsMenu::AddImage(int image, bool requestNext)
{
    m_requestingLabel = requestNext;
    FJ_FuiNode* node = 0;
    if (image == 0) node = m_logoIggy;
    else if (image == 1) node = m_logoMiles;
    else if (image == 2) node = m_logoDolby;
    else if (image == 3) node = m_logoNintendo;
    if (node != 0)
    {
        node->setVisible(true);
        m_scrollingNodes.push_back(node);
    }
    if (!m_scrollingNodes.empty())
        m_scrollingNodes.back()->setY(static_cast<float>(m_scrollBottom));
}

unsigned int FJ_CreditsMenu::SetNextLabel(std::wstring label, int type, bool requestNext)
{
    m_requestingLabel = requestNext;
    if (label.empty())
        label.assign(L" ");
    FJ_FuiNode** labels = m_smallLabels;
    int count = 25;
    if (type == 1) { labels = m_mediumLabels; count = 12; }
    else if (type < 3) { labels = m_largeLabels; count = 20; }
    else if (type == 3) { labels = m_smallLabels; count = 25; }
    int i;
    for (i = 0; i < count; ++i)
    {
        if (labels[i] != 0 && !labels[i]->isVisible())
        {
            labels[i]->setVisible(true);
            m_scrollingNodes.push_back(labels[i]);
            if (m_scrollingNodes.size() == 1)
                labels[i]->setY(static_cast<float>(m_scrollBottom));
            else
                labels[i]->setY(m_scrollingNodes[m_scrollingNodes.size() - 2]->getY() +
                    m_scrollingNodes[m_scrollingNodes.size() - 2]->getHeight());
            return 1;
        }
    }
    return 0;
}
