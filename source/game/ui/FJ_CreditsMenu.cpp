#include "game/ui/FJ_CreditsMenu.hpp"

#include <cstdio>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_LabelOutline.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_CreditsMenu::FJ_CreditsMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
    m_blendingContainer = getChildByName(std::string("BlendingContainer"))->asMovieClip();
    m_creditsContent = m_blendingContainer->getChildByName(std::string("CreditsContent"))->asMovieClip();

    m_scrollTop = 130;
    m_scrollBottom = static_cast<int>(getStage()->getStageHeight() - 25.0f);
    if (getStage()->getStageHeight() == 408.0f)
    {
        m_scrollTop = 110;
        m_scrollBottom = static_cast<int>(getStage()->getStageHeight() - 25.0f);
    }
    if (getStage()->getStageHeight() == 720.0f)
    {
        m_scrollTop = 195;
        m_scrollBottom = static_cast<int>(getStage()->getStageHeight() - 38.0f);
    }
    else if (getStage()->getStageHeight() == 1080.0f)
    {
        m_scrollTop = 292;
        m_scrollBottom = static_cast<int>(getStage()->getStageHeight() - 56.0f);
    }

    m_scrollAmount = 1.0f;
    if (getStage()->getStageHeight() == 1080.0f || getStage()->getStageHeight() == 540.0f)
        m_scrollAmount = 1.5f;
    if (getStage()->getStageHeight() == 408.0f)
        m_scrollAmount = 0.5f;

    m_logoLanguage = 0;
}

FJ_CreditsMenu::~FJ_CreditsMenu()
{
    removeEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling);
}

FJ_Document* FJ_CreditsMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_CreditsMenu(renderNode);
}

unsigned int FJ_CreditsMenu::getType()
{
    return 0x58;
}

void FJ_CreditsMenu::RequestNextLabel()
{
    if (getRoot()->m_pCallbackScene != NULL)
        getRoot()->m_pCallbackScene->handleRequestMoreData(0, false);
}

void FJ_CreditsMenu::UpdateScrolling(FJ_Event*)
{
    if (!m_requestingLabel && (m_scrollingNodes.empty() || m_scrollingNodes.size() < 25))
        RequestNextLabel();

    for (unsigned int i = 0; i < m_scrollingNodes.size(); ++i)
    {
        m_scrollingNodes[i]->setY(m_scrollingNodes[i]->getY() -
            static_cast<float>(Math::round(m_scrollAmount + m_scrollAmount)));
    }

    if (m_scrollingNodes.empty())
    {
        m_requestingLabel = false;
    }
    else if (m_scrollingNodes[0]->getY() + m_scrollingNodes[0]->getHeight() <
             static_cast<float>(m_scrollTop))
    {
        if (m_scrollingNodes[0]->isFJ_LabelOutline())
            m_scrollingNodes[0]->asFJ_LabelOutline()->SetLabel(L"");
        m_scrollingNodes[0]->setVisible(false);
        m_scrollingNodes.erase(m_scrollingNodes.begin());
    }
}

void FJ_CreditsMenu::_UpdateScrolling(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_CreditsMenu*>(node)->UpdateScrolling(event);
}

void FJ_CreditsMenu::init(FJ_Event*)
{
    m_requestingLabel = false;
    addEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling, false);

    char name[64];
    for (int i = 0; i < 25; ++i)
    {
        std::sprintf(name, "CreditsText_S_%d", i);
        m_smallLabels[i] = m_creditsContent->getChildByName(std::string(name))->asFJ_LabelOutline();
        fuiRect smallRect = getRect(this);
        m_smallLabels[i]->setX(static_cast<float>(Math::round(
            smallRect.xMin - (m_smallLabels[i]->getWidth() * 0.5f - getWidth() * 0.5f))));
        m_smallLabels[i]->setY(0.0f);
        m_smallLabels[i]->setVisible(false);

        if (i < 12)
        {
            std::sprintf(name, "CreditsText_M_%d", i);
            m_mediumLabels[i] = m_creditsContent->getChildByName(std::string(name))->asFJ_LabelOutline();
            fuiRect mediumRect = getRect(this);
            m_mediumLabels[i]->setX(static_cast<float>(Math::round(
                mediumRect.xMin - (m_mediumLabels[i]->getWidth() * 0.5f - getWidth() * 0.5f))));
            m_mediumLabels[i]->setY(0.0f);
            m_mediumLabels[i]->setVisible(false);
        }

        if (i < 20)
        {
            std::sprintf(name, "CreditsText_L_%d", i);
            m_largeLabels[i] = m_creditsContent->getChildByName(std::string(name))->asFJ_LabelOutline();
            fuiRect largeRect = getRect(this);
            m_largeLabels[i]->setX(static_cast<float>(Math::round(
                largeRect.xMin - (m_largeLabels[i]->getWidth() * 0.5f - getWidth() * 0.5f))));
            m_largeLabels[i]->setY(0.0f);
            m_largeLabels[i]->setVisible(false);
        }

        if (i < 8)
        {
            std::sprintf(name, "CreditsText_XL_%d", i);
            m_extraLargeLabels[i] = m_creditsContent->getChildByName(std::string(name))->asFJ_LabelOutline();
            fuiRect extraLargeRect = getRect(this);
            m_extraLargeLabels[i]->setX(static_cast<float>(Math::round(
                extraLargeRect.xMin - (m_extraLargeLabels[i]->getWidth() * 0.5f - getWidth() * 0.5f))));
            m_extraLargeLabels[i]->setY(0.0f);
            m_extraLargeLabels[i]->setVisible(false);
        }
    }

    m_logoIggy = m_creditsContent->getChildByName(std::string("Logo_Iggy"));
    fuiRect iggyRect = getRect(this);
    m_logoIggy->setX(static_cast<float>(Math::round(
        iggyRect.xMin - (m_logoIggy->getWidth() * 0.5f - getWidth() * 0.5f))));
    m_logoIggy->setY(0.0f);

    m_logoMiles = m_creditsContent->getChildByName(std::string("Logo_Miles"));
    fuiRect milesRect = getRect(this);
    m_logoMiles->setX(static_cast<float>(Math::round(
        milesRect.xMin - (m_logoMiles->getWidth() * 0.5f - getWidth() * 0.5f))));
    m_logoMiles->setY(0.0f);

    m_logoDolby = m_creditsContent->getChildByName(std::string("Logo_Dolby"));
    fuiRect dolbyRect = getRect(this);
    m_logoDolby->setX(static_cast<float>(Math::round(
        dolbyRect.xMin - (m_logoDolby->getWidth() * 0.5f - getWidth() * 0.5f))));
    m_logoDolby->setY(0.0f);

    m_logoNintendo = m_creditsContent->getChildByName(std::string("Logo_Nintendo"));
    fuiRect nintendoRect = getRect(this);
    m_logoNintendo->setX(static_cast<float>(Math::round(
        nintendoRect.xMin - (m_logoNintendo->getWidth() * 0.5f - getWidth() * 0.5f))));
    m_logoNintendo->setY(0.0f);

    getChildByName(std::string("LogoObject"))->getChildAt(0)->gotoAndStop(1);
}

void FJ_CreditsMenu::AddImage(int image, bool requestNext)
{
    m_requestingLabel = requestNext;

    if (image == 0)
    {
        m_logoIggy->setVisible(true);
        m_scrollingNodes.push_back(m_logoIggy);
    }
    else if (image == 1)
    {
        m_logoMiles->setVisible(true);
        m_scrollingNodes.push_back(m_logoMiles);
    }
    else if (image == 2)
    {
        m_logoDolby->setVisible(true);
        m_scrollingNodes.push_back(m_logoDolby);
    }
    else if (image == 3)
    {
        m_logoNintendo->setVisible(true);
        m_scrollingNodes.push_back(m_logoNintendo);
    }

    if (!m_scrollingNodes.empty())
    {
        if (m_scrollingNodes.size() == 1)
        {
            m_scrollingNodes[m_scrollingNodes.size() - 1]->setY(static_cast<float>(m_scrollBottom));
        }
        else
        {
            m_scrollingNodes[m_scrollingNodes.size() - 1]->setY(
                m_scrollingNodes[m_scrollingNodes.size() - 2]->getY() +
                m_scrollingNodes[m_scrollingNodes.size() - 2]->getHeight());
        }
    }
}

void FJ_CreditsMenu::SetLogoLanguage(int language)
{
    m_logoLanguage = language;
}

unsigned int FJ_CreditsMenu::SetNextLabel(std::wstring label, int type, bool requestNext)
{
    m_requestingLabel = requestNext;
    if (label.size() == 0)
        label = L" ";

    FJ_LabelOutline** labels;
    int count;
    if (type == 0)
    {
        labels = m_extraLargeLabels;
        count = 8;
    }
    else if (type == 1)
    {
        labels = m_largeLabels;
        count = 20;
    }
    else if (static_cast<unsigned int>(type) < 3)
    {
        labels = m_mediumLabels;
        count = 12;
    }
    else if (type == 3)
    {
        labels = m_smallLabels;
        count = 25;
    }

    for (int i = 0; i < count; ++i)
    {
        if (!labels[i]->isVisible())
        {
            labels[i]->setVisible(true);
            labels[i]->SetLabel(label);
            m_scrollingNodes.push_back(labels[i]);
            if (m_scrollingNodes.size() == 1)
            {
                m_scrollingNodes[m_scrollingNodes.size() - 1]->setY(static_cast<float>(m_scrollBottom));
            }
            else
            {
                m_scrollingNodes[m_scrollingNodes.size() - 1]->setY(
                    m_scrollingNodes[m_scrollingNodes.size() - 2]->getY() +
                    m_scrollingNodes[m_scrollingNodes.size() - 2]->getHeight());
            }
            return 1;
        }
    }
    return 0;
}
