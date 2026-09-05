#include "game/ui/FJ_LeaderboardList.hpp"

#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_LeaderboardHeader.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/fuiRenderNode.hpp"

class FJ_Label;

class FJ_LeaderboardEntry : public FJ_FuiNode
{
public:
    void Init(int count, int firstValue, int spacing, int offset);
    void SetData(FJ_LeaderboardList::CLeaderboardEntry* entry);
};

FJ_LeaderboardList::FJ_LeaderboardList(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode), m_initialized(true), m_visibleEntries(10),
      m_numColumns(4), m_entrySpacing(101), m_numEntries(15), m_header(NULL),
      m_firstEntry(-1), m_lastEntry(-1), m_upArrow(NULL), m_downArrow(NULL),
      m_pleaseWait(NULL), m_touchEntry(0), m_touchY(0)
{
    int i;
    for (i = 0; i < 7; ++i)
        m_columnIcons[i] = 0;
}

FJ_LeaderboardList::~FJ_LeaderboardList()
{
    int i;
    for (i = 0; i < (int)m_entryNodes.size(); ++i)
    {
        if (m_entryNodes[i] != NULL && m_entryNodes[i]->getParent() != NULL)
            m_entryNodes[i]->getParent()->removeChild(m_entryNodes[i], true);
    }
    m_entryNodes.clear();
    if (m_header != NULL)
    {
        m_header->Destroy();
        delete m_header;
        m_header = NULL;
    }
}

void FJ_LeaderboardList::SetupPleaseWait(FJ_Label* label)
{
    m_pleaseWait = label;
}

uint64_t FJ_LeaderboardList::UpdateLeaderboardCache(bool below)
{
    int i;
    if (below)
    {
        for (i = 0; i < m_numEntries; ++i)
            m_leaderboard.erase(m_lastEntry + i + 1);
        m_lastEntry -= m_numEntries;
    }
    else
    {
        for (i = 0; i < m_numEntries; ++i)
            m_leaderboard.erase(m_firstEntry - i - 1);
        m_firstEntry += m_numEntries;
    }
    return 0;
}

void FJ_LeaderboardList::RequestData(bool below)
{
    if (below)
    {
        m_lastEntry += m_visibleEntries;
        if (m_lastEntry > m_numEntries)
            m_lastEntry = m_numEntries;
    }
    else
    {
        m_firstEntry -= m_visibleEntries;
        if (m_firstEntry < 0)
            m_firstEntry = 0;
    }
    if (m_lastEntry - m_firstEntry >= m_entrySpacing)
        UpdateLeaderboardCache(below);
}

void FJ_LeaderboardList::addHeader()
{
    if (m_header != NULL)
        return;
    FJ_FuiNode* object = FJ_Factory::createObject("fourj.listClasses.FJ_LeaderboardHeaderNormal", m_pRenderNode, true, NULL);
    m_header = static_cast<FJ_LeaderboardHeader*>(object);
    if (m_header != NULL)
    {
        m_header->Init(m_numColumns, this, 0, 0);
        m_header->SetupHeaders(m_rankTitle, m_gamertagTitle);
        for (int i = 0; i < 7; ++i)
            m_header->SetupSlot(i, m_columnIcons[i]);
        addChild(m_header);
    }
}

void FJ_LeaderboardList::HandleScrollListArrows(bool addArrows)
{
    if (addArrows && m_entryNodes.size() >= (unsigned int)m_visibleEntries && m_upArrow == NULL)
    {
        m_upArrow = FJ_Factory::createObject("fourj.listClasses.FJ_ScrollArrow", m_pRenderNode, true, NULL);
        m_downArrow = static_cast<FJ_ScrollArrow*>(FJ_Factory::createObject("fourj.listClasses.FJ_ScrollArrow", m_pRenderNode, true, NULL));
        if (getParent() != NULL)
        {
            getParent()->addChild(m_upArrow);
            getParent()->addChild(m_downArrow);
        }
        static_cast<FJ_ScrollArrow*>(m_upArrow)->ChangeState(0);
        m_downArrow->ChangeState(3);
    }
    if (m_upArrow != NULL)
        m_upArrow->setVisible(m_firstEntry > 0 && m_numEntries > m_visibleEntries);
    if (m_downArrow != NULL)
        m_downArrow->setVisible(m_firstEntry < m_numEntries - m_visibleEntries);
}

void FJ_LeaderboardList::addNewItem(int index)
{
    const char* symbol = m_numColumns <= 4 ? "fourj.listClasses.FJ_LeaderboardEntryNormal" : "fourj.listClasses.FJ_LeaderboardEntrySmall";
    FJ_LeaderboardEntry* entry = static_cast<FJ_LeaderboardEntry*>(FJ_Factory::createObject(symbol, m_pRenderNode, true, NULL));
    if (entry == NULL)
        return;
    entry->setWidth(getWidth());
    entry->setX(getX() + (getWidth() - entry->getWidth()) * 0.5f);
    entry->setY(getY() + (entry->getHeight() + (float)m_entrySpacing) * (float)index);
    entry->Init(m_numColumns, m_header->getFirstEntryPos(), m_header->getEntrySpacing(), m_header->getIconHolderHalfWidth());
    m_entryNodes.push_back(entry);
    if (getParent() != NULL)
        getParent()->addChild(entry);
    HandleScrollListArrows(true);
}

void FJ_LeaderboardList::UpdateVisibleLeaderboard()
{
    unsigned int i;
    if (m_initialized)
    {
        int count = m_numEntries < m_visibleEntries ? m_numEntries : m_visibleEntries;
        addHeader();
        for (i = 0; i < (unsigned int)count; ++i)
            addNewItem((int)i);
        m_initialized = false;
    }
    for (i = 0; i < m_entryNodes.size(); ++i)
    {
        std::map<int, CLeaderboardEntry>::iterator found = m_leaderboard.find(m_firstEntry + (int)i);
        if (found == m_leaderboard.end())
            m_entryNodes[i]->SetData(NULL);
        else
            m_entryNodes[i]->SetData(&found->second);
    }
}

uint64_t FJ_LeaderboardList::NavigateLeaderboard(bool down, bool page)
{
    int amount = page ? m_visibleEntries : 1;
    int old = m_firstEntry;
    if (down)
        m_firstEntry += amount;
    else
        m_firstEntry -= amount;
    if (m_firstEntry < 0)
        m_firstEntry = 0;
    if (m_firstEntry > m_numEntries - m_visibleEntries)
        m_firstEntry = m_numEntries - m_visibleEntries;
    if (m_firstEntry != old)
    {
        RequestData(down);
        UpdateVisibleLeaderboard();
        HandleScrollListArrows(false);
    }
    return 0;
}

FJ_FuiNode* FJ_LeaderboardList::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_LeaderboardList(pRenderNode);
}

unsigned int FJ_LeaderboardList::getType()
{
    return 104;
}

void FJ_LeaderboardList::ASConstructor()
{
    addHeader();
    m_upArrow = FJ_Factory::createObject("fourj.listClasses.FJ_ScrollArrow", m_pRenderNode, true, NULL);
    m_downArrow = static_cast<FJ_ScrollArrow*>(FJ_Factory::createObject("fourj.listClasses.FJ_ScrollArrow", m_pRenderNode, true, NULL));
    m_initialized = true;
}

uint64_t FJ_LeaderboardList::SetupTitles(std::wstring rank, std::wstring gamertag)
{
    m_rankTitle = rank;
    m_gamertagTitle = gamertag;
    return 0;
}

void FJ_LeaderboardList::SetDataLoading(bool loading)
{
    m_initialized = loading;
}

void FJ_LeaderboardList::InitLeaderboard(int firstValue, int spacing, int offset)
{
    m_firstEntry = firstValue;
    m_entrySpacing = spacing;
    m_lastEntry = offset;
    m_initialized = true;
    SetDataLoading(true);
}

void FJ_LeaderboardList::SetColumnIcon(int column, int icon)
{
    if (column >= 0 && column < 7)
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
    for (unsigned int i = 0; i < m_entryNodes.size(); ++i)
        if (m_entryNodes[i] != NULL && m_entryNodes[i]->getParent() != NULL)
            m_entryNodes[i]->getParent()->removeChild(m_entryNodes[i], true);
    m_entryNodes.clear();
    m_leaderboard.clear();
    if (m_upArrow != NULL)
        m_upArrow->setVisible(false);
    if (m_downArrow != NULL)
        m_downArrow->setVisible(false);
    m_firstEntry = 0;
    m_lastEntry = -1;
    m_touchEntry = 0;
    m_initialized = true;
    SetDataLoading(true);
}

uint64_t FJ_LeaderboardList::TouchHighlightEntry(int index)
{
    if (index >= 0 && index < (int)m_entryNodes.size())
    {
        m_touchEntry = index;
        getStage()->setFocus(m_entryNodes[index]);
        FJ_Event event(eFJEventType_FocusChangeUser, true, false);
        dispatchEvent(&event);
    }
    return 0;
}

uint64_t FJ_LeaderboardList::SetTouchFocus(int x, int y, bool focus)
{
    if (!focus)
    {
        for (unsigned int i = 0; i < m_entryNodes.size(); ++i)
        {
            if ((float)y >= m_entryNodes[i]->getY() && (float)y < m_entryNodes[i]->getY() + m_entryNodes[i]->getHeight())
            {
                TouchHighlightEntry((int)i);
                break;
            }
        }
    }
    else if (y > m_touchY)
        NavigateLeaderboard(true, true);
    else if (y < m_touchY)
        NavigateLeaderboard(false, true);
    m_touchY = y;
    return 0;
}

uint64_t FJ_LeaderboardList::FocusOnFirstEntry()
{
    if (m_firstEntry >= m_visibleEntries && m_firstEntry < m_lastEntry)
        m_firstEntry -= m_visibleEntries - 1;
    if (!m_entryNodes.empty())
        TouchHighlightEntry(0);
    return 0;
}
