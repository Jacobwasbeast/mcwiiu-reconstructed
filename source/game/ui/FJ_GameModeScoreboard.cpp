#include "game/ui/FJ_GameModeScoreboard.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_IconSlot.hpp"
#include "game/ui/FJ_List_ListButtonScoreBoard.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_GameModeScoreboard::FJ_GameModeScoreboard(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
    const char* names[5] = {
        "PanelAward.Stat1Icon", "PanelAward.Stat2Icon", "PanelAward.Stat3Icon",
        "PanelAward.Stat4Icon", "PanelAward.Stat5Icon"
    };
    int i;
    for (i = 0; i < 5; ++i)
        m_statIconHolders.push_back(getChildByName(std::string(names[i])));
    for (i = 0; i < 5; ++i)
    {
        FJ_IconSlot* icon = m_statIconHolders[i]->asFJ_IconSlot();
        icon->SetParams(0, false);
    }

    m_statIconCount = 0;

    m_killedByLabelY = static_cast<int>(getChildByName(std::string("PanelAward.KilledByLabel"))->getY());
    m_killedByLabelHeight = static_cast<int>(getChildByName(std::string("PanelAward.KilledByLabel"))->getHeight());
}

FJ_GameModeScoreboard::~FJ_GameModeScoreboard()
{
}

FJ_GameModeScoreboard* FJ_GameModeScoreboard::Create(fuiRenderNode* renderNode)
{
    return new FJ_GameModeScoreboard(renderNode);
}

unsigned int FJ_GameModeScoreboard::getType()
{
    return 79;
}

void FJ_GameModeScoreboard::SetupStatIconHolders()
{
    int i;
    for (i = 0; i < 5; ++i)
    {
        FJ_IconSlot* icon = m_statIconHolders[i]->asFJ_IconSlot();
        icon->setWidth(m_statIconHolders[i]->getWidth());
        icon->Init(std::wstring(L""));
        icon->SetSlotID(std::string("stat"), i);
        icon->ShowSlotHolder(false);
    }
}

void FJ_GameModeScoreboard::SetScoreboardType(int type)
{
    FJ_FuiNode* panel = getChildByName(std::string("PanelScoreboard"));
    m_scoreboardType = type;
    if (type == 0)
    {
        FJ_FuiNode* award = getChildByName(std::string("PanelAward"));
        removeChild(award, true);
        m_statIconHolders.clear();
        const float stageWidth = static_cast<float>(getStage()->getStageWidth());
        const float width = panel->getWidth();
        panel->setX(static_cast<float>(-Math::round((width - stageWidth) * 0.5f) - 176));
        setX(0.0f);
    }
    else
        SetupStatIconHolders();
}

void FJ_GameModeScoreboard::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    const float stageHeight = getStage()->getStageHeight();
    if (stageHeight != 360.0 && stageHeight != 540.0)
        return;
    setX(static_cast<float>(-right - 176));
    setY(static_cast<float>(-bottom));
}

void FJ_GameModeScoreboard::handlePress(FJ_Event* event)
{
    FJ_Button* button = event->m_pTarget->asFJ_Button();
    if (button != 0)
    {
        m_pCallbackScene->handlePress(static_cast<FJ_Base*>(button->getObjListParent())->getId(), button->getId());
        button->setButtonPressed(false);
    }
}

void FJ_GameModeScoreboard::InitNumColumns(int numColumns)
{
    FJ_FuiNode* panel = getChildByName(std::string("PanelScoreboard"));
    FJ_FuiNode* listNode = getChildByName(std::string("PanelScoreboard.Scoreboard"));
    FJ_List_ListButtonScoreBoard* list = listNode->asFJ_List_ListButtonScoreBoard();
    int i;
    for (i = 0; i < numColumns; ++i)
    {
        FJ_IconSlot* icon = static_cast<FJ_IconSlot*>(FJ_Factory::createObject("fourj.listClasses.FJ_IconSlot", m_pRenderNode, true, 0));
        icon->SetParams(list->GetButtonHeight(), false);
        icon->ShowSlotHolder(false);
        icon->Init(std::wstring(L""));
        icon->setX(-(icon->getWidth() * static_cast<float>(numColumns - i) - (listNode->getX() + listNode->getWidth())) - static_cast<float>((numColumns - i - 1) * 2));
        icon->setY(listNode->getY() - static_cast<float>(icon->getHeight() * 1.1f));
        m_titleSlots.push_back(icon);
        panel->addChild(icon);
        icon->SetSlotID(std::string("header"), i);
    }
    list->SetNumColumns(numColumns);
}

void FJ_GameModeScoreboard::SetTitleSlotText(int slot, const std::wstring& text)
{
    m_titleSlots[slot]->asFJ_IconSlot()->SetLabel(text);
}

void FJ_GameModeScoreboard::ShowTitleSlot(int slot, bool show)
{
    m_titleSlots[slot]->setVisible(show);
}

void FJ_GameModeScoreboard::SetTitleIconTextureName(int slot, const std::wstring& textureName)
{
    m_titleSlots[slot]->asFJ_IconSlot()->SetTextureName(textureName);
}

void FJ_GameModeScoreboard::SetStatIconTextureName(int slot, const std::wstring& textureName)
{
    if (slot >= 0 && slot < static_cast<int>(m_statIconHolders.size()))
        m_statIconHolders[slot]->asFJ_IconSlot()->SetTextureName(textureName);
}

void FJ_GameModeScoreboard::SetHasPlayerBeenKilled(bool killed)
{
    FJ_FuiNode* award = getChildByName(std::string("PanelAward"));
    if (award == 0)
        return;
    FJ_FuiNode* killedBy = award->getChildByName(std::string("KilledByLabel"));
    FJ_FuiNode* block = award->getChildByName(std::string("KilledByBlock"));
    FJ_FuiNode* gamertag = award->getChildByName(std::string("GamertagLabel"));
    FJ_FuiNode* icon = award->getChildByName(std::string("Stat5Icon"));
    FJ_FuiNode* label = award->getChildByName(std::string("Stat5Label"));
    if (!killed)
    {
        killedBy->setY(block->getY() + block->getHeight() * 0.5f - static_cast<float>(m_killedByLabelHeight / 2));
        gamertag->setVisible(false);
        icon->setVisible(false);
        label->setVisible(false);
    }
    else
    {
        killedBy->setY(static_cast<float>(m_killedByLabelY));
        gamertag->setVisible(true);
        icon->setVisible(true);
        label->setVisible(true);
    }
}
