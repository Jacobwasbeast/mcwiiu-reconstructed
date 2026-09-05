#include "game/ui/FJ_ListButtonScoreBoard.hpp"

#include <vector>

#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_IconSlot.hpp"

FJ_ListButtonScoreBoard::FJ_ListButtonScoreBoard(fuiRenderNode* renderNode)
    : FJ_Button(renderNode), m_playerIcons(NULL), m_voipIcon(NULL), m_numIcons(0),
      m_externalHighlight(NULL)
{
    m_playerIcons = getChildByName(std::string("PlayerIcons"))->asMovieClip();
    m_voipIcon = getChildByName(std::string("VOIP"))->asMovieClip();
}

FJ_ListButtonScoreBoard::~FJ_ListButtonScoreBoard()
{
}

void FJ_ListButtonScoreBoard::Init(const std::wstring& name, int id)
{
    SetLabelAlignment(4);
    m_isLabel = true;
    FJ_Button::Init(name, id);

    m_playerIcons->setScaleX(m_playerIcons->getScaleX() / getScaleX());
    m_playerIcons->setScaleY(m_playerIcons->getScaleY() / getScaleY());
    m_voipIcon->setScaleX(m_voipIcon->getScaleX() / getScaleX());
    m_voipIcon->setScaleY(m_voipIcon->getScaleY() / getScaleY());
}

void FJ_ListButtonScoreBoard::SetNumIcons(int numIcons, int width, int highlightWidth)
{
    setWidth(getWidth() - static_cast<float>(numIcons) * (getHeight() + 2.0f));

    int i;
    for (i = 0; i < numIcons; ++i)
    {
        FJ_IconSlot* slot = static_cast<FJ_IconSlot*>(FJ_Factory::createObject(
            "fourj.listClasses.FJ_IconSlot", m_pRenderNode, true, NULL));
        slot->SetParams(static_cast<int>(getHeight()), false);
        slot->Init(std::wstring(L""));
        slot->ShowSlotHolder(false);
        slot->setX(getX() + getWidth() + static_cast<float>(i) * (slot->getWidth() + 2.0f));
        slot->setY(getY());
        m_objects.push_back(slot);
        getParent()->addChild(slot);
        slot->SetSlotID(std::string("list"), m_id * 10 + i);

        std::vector<FJ_FuiNode*>::iterator object = m_objects.begin();
        while (object != m_objects.end())
        {
            (*object)->setVisible(isVisible());
            ++object;
        }
    }

    FJ_FuiNode* highlight = FJ_Factory::createObject(
        "fourj.Misc.FJ_ExternalHighlight", m_pRenderNode, true, NULL);
    highlight->setX(-static_cast<float>(highlightWidth) / getScaleX());
    highlight->setY(0.5f);
    highlight->setScaleX(highlight->getScaleX() / getScaleX());
    highlight->setScaleY(highlight->getScaleY() / getScaleY());
    highlight->getChildAt(0)->setWidth(static_cast<float>(width));
    highlight->getChildAt(0)->setHeight(getHeight());
    highlight->setAlpha(0.5f);
    highlight->setVisible(true);
    addChild(highlight);

    m_externalHighlight = FJ_Factory::createObject(
        "fourj.Misc.FJ_ExternalHighlight", m_pRenderNode, true, NULL);
    m_externalHighlight->setX(-static_cast<float>(highlightWidth) / getScaleX());
    m_externalHighlight->setY(0.5f);
    m_externalHighlight->setScaleX(m_externalHighlight->getScaleX() / getScaleX());
    m_externalHighlight->setScaleY(m_externalHighlight->getScaleY() / getScaleY());
    m_externalHighlight->getChildAt(0)->setWidth(static_cast<float>(width));
    m_externalHighlight->getChildAt(0)->setHeight(getHeight());
    m_externalHighlight->setVisible(false);
    addChild(m_externalHighlight);
}

void FJ_ListButtonScoreBoard::SetPlayerIcon(int frame)
{
    m_playerIcons->gotoAndStop(frame);
}

void FJ_ListButtonScoreBoard::SetVOIPIcon(int frame)
{
    m_voipIcon->gotoAndStop(frame);
}

void FJ_ListButtonScoreBoard::SetSlotText(int slot, const std::wstring& text)
{
    if (slot >= 0 && slot < static_cast<int>(m_objects.size()))
        m_objects[slot]->asFJ_IconSlot()->SetLabel(text);
}

void FJ_ListButtonScoreBoard::SetSlotTextureName(int slot, const std::wstring& textureName)
{
    m_objects[slot]->asFJ_IconSlot()->SetTextureName(textureName);
}

void FJ_ListButtonScoreBoard::ChangeState(int state)
{
    if (m_externalHighlight != NULL)
    {
        if ((state == 0 || state == 1 || state == 3) && getLabelTooWide())
        {
            m_externalHighlight->setVisible(true);
            FJ_Button::ChangeState(state);
            return;
        }
        m_externalHighlight->setVisible(false);
    }
    FJ_Button::ChangeState(state);
}
