#include "game/ui/FJ_LeaderboardList.hpp"

#include <algorithm>
#include <cmath>

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_LeaderboardHeader.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

class FJ_Factory
{
public:
    static FJ_FuiNode* createObject(const char*, fuiRenderNode*, bool, const char*);
};

class FJ_LeaderboardEntry : public FJ_FuiNode
{
public:
    void Init(int, int, int, int);
    void SetData(FJ_LeaderboardList::CLeaderboardEntry*);
};

FJ_LeaderboardList::FJ_LeaderboardList(fuiRenderNode* renderNode)
    : FJ_FuiNode(renderNode), m_small(false), m_entrySpacing(4), m_visibleEntries(10),
      m_requestSize(101), m_numColumns(15), m_leaderboard(), m_needsUpdate(true), m_dataLoading(true), m_entries(), m_header(NULL),
      m_highlightEntry(0), m_firstEntry(0), m_rankTitle(), m_gamertagTitle(),
      m_upArrow(NULL), m_downArrow(NULL), m_pleaseWait(NULL), m_touchEntry(0), m_touchY(0)
{
    m_entries.reserve(0);
    m_columnIcons[0] = 0;
    m_columnIcons[1] = 0;
    m_columnIcons[2] = 0;
    m_columnIcons[3] = 0;
    m_columnIcons[4] = 0;
    m_columnIcons[5] = 0;
    m_columnIcons[6] = 0;
}

FJ_LeaderboardList::~FJ_LeaderboardList()
{
}

void FJ_LeaderboardList::SetupPleaseWait(FJ_Label* label)
{
    m_pleaseWait = label;
}

uint64_t FJ_LeaderboardList::UpdateLeaderboardCache(bool next)
{
    int i;
    if (!next)
    {
        for (i = 0; i < m_numColumns; ++i)
        {
            std::map<int, CLeaderboardEntry>::iterator first = m_leaderboard.lower_bound(m_highlightEntry);
            std::map<int, CLeaderboardEntry>::iterator last = m_leaderboard.upper_bound(m_highlightEntry);
            m_leaderboard.erase(first, last);
            --m_highlightEntry;
        }
    }
    else
    {
        for (i = 0; i < m_numColumns; ++i)
        {
            std::map<int, CLeaderboardEntry>::iterator first = m_leaderboard.lower_bound(m_firstEntry);
            std::map<int, CLeaderboardEntry>::iterator last = m_leaderboard.upper_bound(m_firstEntry);
            m_leaderboard.erase(first, last);
            ++m_firstEntry;
        }
    }
    return (uint64_t)(uintptr_t)this << 32;
}

void FJ_LeaderboardList::RequestData(bool next)
{
    if (!next)
    {
        m_firstEntry = std::max(0, m_firstEntry - m_numColumns);
        if (m_highlightEntry - m_firstEntry < m_requestSize)
            return;
    }
    else
    {
        m_highlightEntry = std::min(m_requestSize, m_highlightEntry + m_numColumns);
        if (m_highlightEntry - m_firstEntry < m_requestSize)
            return;
    }
    UpdateLeaderboardCache(next);
}

void FJ_LeaderboardList::addHeader()
{
    if (m_header != NULL)
        return;
    const char* symbol = m_small ? "fourj_listClasses_FJ_LeaderboardHeaderSmall" : "fourj_listClasses_FJ_LeaderboardHeader";
    m_header = static_cast<FJ_LeaderboardHeader*>(FJ_Factory::createObject(symbol, m_pRenderNode, true, NULL));
    m_header->setWidth(getWidth());
    m_header->Init(m_numColumns, getParent(), static_cast<int>(getX()), static_cast<int>(getY()));
    m_header->SetupHeaders(m_rankTitle, m_gamertagTitle);
    addChild(m_header);
    m_header->SetupSlot(0, m_columnIcons[0]);
    m_header->SetupSlot(1, m_columnIcons[1]);
    m_header->SetupSlot(2, m_columnIcons[2]);
    m_header->SetupSlot(3, m_columnIcons[3]);
    m_header->SetupSlot(4, m_columnIcons[4]);
    m_header->SetupSlot(5, m_columnIcons[5]);
    m_header->SetupSlot(6, m_columnIcons[6]);
}

void FJ_LeaderboardList::HandleScrollListArrows(bool add)
{
    unsigned int count = static_cast<unsigned int>(m_entries.size());
    if (add && count >= static_cast<unsigned int>(m_visibleEntries) && m_upArrow->getParent() == NULL)
    {
        getParent()->addChild(m_upArrow);
        getParent()->addChild(m_downArrow);
        m_upArrow->setX(-m_upArrow->getWidth());
        m_upArrow->setY(getY() + getHeight());
        m_downArrow->setX(getWidth());
        m_downArrow->setY(getY() + getHeight());
        m_upArrow->ChangeState(0);
        m_downArrow->ChangeState(3);
    }
    m_upArrow->setVisible(m_firstEntry > 0 && m_requestSize > m_visibleEntries);
    m_downArrow->setVisible(m_firstEntry < m_requestSize - m_visibleEntries);
}

void FJ_LeaderboardList::addNewItem(int index)
{
    const char* symbol = m_small ? "fourj_listClasses_FJ_LeaderboardEntrySmall" : "fourj_listClasses_FJ_LeaderboardEntryNormal";
    FJ_LeaderboardEntry* entry = static_cast<FJ_LeaderboardEntry*>(FJ_Factory::createObject(symbol, m_pRenderNode, true, NULL));
    entry->setWidth(getWidth());
    entry->setY(getY() + static_cast<float>(index) * entry->getHeight() + static_cast<float>(m_header->getHeight()));
    entry->Init(m_numColumns, m_header->getFirstEntryPos(), m_header->getEntrySpacing(), m_header->getIconHolderHalfWidth());
    m_entries.push_back(entry);
    getParent()->addChild(entry);
    HandleScrollListArrows(true);
}

void FJ_LeaderboardList::UpdateVisibleLeaderboard()
{
    if (m_dataLoading)
    {
        addHeader();
        int count = std::min(m_requestSize, m_visibleEntries);
        int i;
        for (i = 0; i < count; ++i)
            addNewItem(i);
    }
    int i;
    for (i = 0; i < static_cast<int>(m_entries.size()); ++i)
    {
        FJ_LeaderboardEntry* entry = static_cast<FJ_LeaderboardEntry*>(m_entries[i]);
        std::map<int, CLeaderboardEntry>::iterator it = m_leaderboard.find(m_firstEntry + i);
        entry->SetData(it == m_leaderboard.end() ? NULL : &it->second);
    }
}

uint64_t FJ_LeaderboardList::NavigateLeaderboard(bool down, bool page)
{
    if (m_entries.empty())
        return 0;
    unsigned int count = static_cast<unsigned int>(m_entries.size());
    int next = down ? m_highlightEntry + (page ? count - 1 : 1) : m_highlightEntry - (page ? count - 1 : 1);
    if (next >= 0 && next < static_cast<int>(count))
        m_highlightEntry = next;
    else if (down && m_firstEntry < m_requestSize - m_visibleEntries)
        m_firstEntry = std::min(m_requestSize - m_visibleEntries, m_firstEntry + (page ? m_visibleEntries : 1));
    else if (!down && m_firstEntry > 0)
        m_firstEntry = std::max(0, m_firstEntry - (page ? m_visibleEntries : 1));
    UpdateVisibleLeaderboard();
    HandleScrollListArrows(false);
    return (uint64_t)(uintptr_t)this << 32;
}

FJ_FuiNode* FJ_LeaderboardList::Create(fuiRenderNode* renderNode)
{
    return new FJ_LeaderboardList(renderNode);
}

unsigned int FJ_LeaderboardList::getType()
{
    return 104;
}

void FJ_LeaderboardList::ASConstructor()
{
    m_upArrow = static_cast<FJ_ScrollArrow*>(FJ_Factory::createObject("fourj.FJ_ScrollArrow", m_pRenderNode, true, NULL));
    m_downArrow = static_cast<FJ_ScrollArrow*>(FJ_Factory::createObject("fourj.FJ_ScrollArrow", m_pRenderNode, true, NULL));
    FJ_FuiNode* background = getChildByName(std::string("Background"))->asMovieClip();
    background->setVisible(false);
}

uint64_t FJ_LeaderboardList::SetupTitles(std::wstring rankTitle, std::wstring gamertagTitle)
{
    m_rankTitle.assign(rankTitle);
    m_gamertagTitle.assign(gamertagTitle);
    return (uint64_t)(uintptr_t)this << 32;
}

void FJ_LeaderboardList::SetDataLoading(bool loading)
{
    if (m_pleaseWait != NULL)
        m_pleaseWait->setVisible(loading);
}

void FJ_LeaderboardList::InitLeaderboard(int firstRank, int lastRank, int selectedRank)
{
    m_firstEntry = firstRank;
    m_requestSize = lastRank;
    m_touchY = selectedRank;
    m_dataLoading = true;
    SetDataLoading(true);
}

void FJ_LeaderboardList::SetColumnIcon(int column, int icon)
{
    m_columnIcons[column] = icon;
}

void FJ_LeaderboardList::ResetLeaderboard()
{
    if (m_header != NULL && m_header->getParent() != NULL)
    {
        m_header->Destroy();
        m_header->getParent()->removeChild(m_header, true);
        m_header = NULL;
    }
    std::vector<FJ_FuiNode*>::iterator it;
    for (it = m_entries.begin(); it != m_entries.end(); ++it)
        if (*it != NULL && (*it)->getParent() != NULL)
            (*it)->getParent()->removeChild(*it, true);
    m_upArrow->setVisible(false);
    m_downArrow->setVisible(false);
    m_entries.clear();
    m_leaderboard.clear();
    SetDataLoading(true);
    m_firstEntry = 0;
    m_highlightEntry = 0;
    m_touchEntry = 1;
    m_touchY = -1;
    m_dataLoading = true;
}

uint64_t FJ_LeaderboardList::TouchHighlightEntry(int index)
{
    if (index >= 0 && index < static_cast<int>(m_entries.size()))
    {
        m_highlightEntry = index;
        getStage()->setFocus(m_entries[index]);
        dispatchEvent(new FJ_Event(eFJEventType_FocusChangeUser, true, false));
    }
    return (uint64_t)(uintptr_t)this << 32;
}

uint64_t FJ_LeaderboardList::SetTouchFocus(int x, int y, bool down)
{
    if (!down)
    {
        unsigned int i;
        for (i = 0; i < m_entries.size(); ++i)
            if (y >= m_entries[i]->getY() && y < m_entries[i]->getY() + m_entries[i]->getHeight())
            {
                TouchHighlightEntry(static_cast<int>(i));
                break;
            }
        m_touchY = y;
    }
    else if (!m_entries.empty() && m_requestSize > static_cast<int>(m_entries.size()))
    {
        int delta = static_cast<int>((std::abs(y - m_touchY)) / (m_entries[0]->getHeight() + m_entrySpacing));
        if (delta > 0)
            m_firstEntry = std::max(0, std::min(m_requestSize - m_visibleEntries, m_firstEntry + (y > m_touchY ? delta : -delta)));
        UpdateVisibleLeaderboard();
    }
    return (uint64_t)(uintptr_t)this << 32;
}

uint64_t FJ_LeaderboardList::FocusOnFirstEntry()
{
    if (m_touchY >= m_firstEntry && m_touchY < m_firstEntry + m_visibleEntries)
        m_firstEntry = std::max(0, m_touchY - m_visibleEntries + m_firstEntry + 1);
    if (!m_entries.empty())
    {
        m_highlightEntry = 0;
        getStage()->setFocus(m_entries[0]);
        dispatchEvent(new FJ_Event(eFJEventType_FocusChangeUser, true, false));
    }
    return (uint64_t)(uintptr_t)this << 32;
}
