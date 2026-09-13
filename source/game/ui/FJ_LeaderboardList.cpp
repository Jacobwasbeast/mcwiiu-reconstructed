#include "game/ui/FJ_LeaderboardList.hpp"
#include "game/ui/FJ_LeaderboardEntry.hpp"
#include "game/ui/FJ_Factory.hpp"

#include <algorithm>
#include <cmath>

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_LeaderboardHeader.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

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
    const char* symbol = m_small ? "fourj.listClasses.FJ_LeaderboardHeaderSmall"
                                  : "fourj.listClasses.FJ_LeaderboardHeaderNormal";
    m_header = static_cast<FJ_LeaderboardHeader*>(FJ_Factory::createObject(symbol, m_pRenderNode, true, NULL));
    m_header->setWidth(getWidth());
    m_header->Init(m_numColumns, getParent(), static_cast<int>(getX()), static_cast<int>(getY()));
    m_header->SetupHeaders(m_rankTitle, m_gamertagTitle);
    int offset = Math::round((getWidth() - m_header->getWidth()) * 0.5f);
    m_header->setX(getX() + static_cast<float>(offset));
    m_header->setY(getY());
    getParent()->addChild(m_header);
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
    if (add)
    {
        unsigned int count = static_cast<unsigned int>(m_entries.size());
        if (static_cast<unsigned int>(m_visibleEntries) <= count && m_upArrow->getParent() == NULL)
        {
            getParent()->addChild(m_upArrow);
            getParent()->addChild(m_downArrow);
            FJ_FuiNode* lastEntry = m_entries.back();
            float rightEdge = lastEntry->getX() + lastEntry->getWidth();
            m_upArrow->setX(rightEdge - m_upArrow->getWidth() * 1.5f);
            m_upArrow->setY(lastEntry->getY() + lastEntry->getHeight() + m_upArrow->getHeight() * 0.5f);
            m_downArrow->setX(rightEdge - m_downArrow->getWidth() * 0.5f);
            m_downArrow->setY(m_upArrow->getY());
            m_upArrow->ChangeState(0);
            m_downArrow->ChangeState(3);
        }
    }
    m_upArrow->setVisible(m_firstEntry > 0 && m_requestSize > m_visibleEntries);
    m_downArrow->setVisible(m_firstEntry < m_requestSize - m_visibleEntries);
}

void FJ_LeaderboardList::addNewItem(int index)
{
    const char* symbol = m_small ? "fourj.listClasses.FJ_LeaderboardEntrySmall"
                                  : "fourj.listClasses.FJ_LeaderboardEntryNormal";
    FJ_LeaderboardEntry* entry = static_cast<FJ_LeaderboardEntry*>(FJ_Factory::createObject(symbol, m_pRenderNode, true, NULL));
    entry->setWidth(getWidth());
    int offset = Math::round((getWidth() - entry->getWidth()) * 0.5f);
    entry->setX(getX() + static_cast<float>(offset));
    entry->setY((entry->getHeight() + static_cast<float>(m_entrySpacing)) * static_cast<float>(index) +
                (getY() + m_header->getHeight()) + static_cast<float>(m_entrySpacing));
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
        return (uint64_t)(uintptr_t)this << 32;
    int count = static_cast<int>(m_entries.size());
    bool firstEntryChanged = false;

    if (!down)
    {
        int newHighlight = page ? ((m_highlightEntry < count - 1) ? count - 1 : count)
                                 : m_highlightEntry + 1;
        if (newHighlight >= 0 && newHighlight < count)
        {
            m_highlightEntry = newHighlight;
        }
        else
        {
            int maxFirst = m_requestSize - m_visibleEntries;
            firstEntryChanged = (m_firstEntry != maxFirst);
            m_highlightEntry = count - 1;
            if (m_visibleEntries < m_requestSize)
            {
                int newFirst = page ? (m_firstEntry + m_visibleEntries) : (m_firstEntry + 1);
                if (page && newFirst + m_visibleEntries > m_requestSize)
                    newFirst = maxFirst;
                if (newFirst + m_visibleEntries > m_requestSize)
                {
                    m_firstEntry = maxFirst;
                }
                else
                {
                    m_firstEntry = newFirst;
                    if (newFirst + count - 1 > m_requestSize - 1)
                        RequestData(true);
                    UpdateVisibleLeaderboard();
                    if (m_downArrow != NULL)
                        m_downArrow->ChangeState(4);
                }
            }
        }
    }
    else
    {
        int newHighlight = page ? ((m_highlightEntry > 0) ? 0 : -1) : m_highlightEntry - 1;
        if (newHighlight >= 0)
        {
            m_highlightEntry = newHighlight;
        }
        else
        {
            firstEntryChanged = (m_firstEntry != 0);
            m_highlightEntry = 0;
            if (m_visibleEntries < m_requestSize)
            {
                int newFirst = page ? (m_firstEntry - m_visibleEntries) : (m_firstEntry - 1);
                if (newFirst < 0)
                    newFirst = 0;
                m_firstEntry = newFirst;
                if (newFirst < m_numColumns)
                    RequestData(false);
                UpdateVisibleLeaderboard();
                if (m_upArrow != NULL)
                    m_upArrow->ChangeState(1);
            }
        }
    }

    if (firstEntryChanged)
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
    (void)x;
    if (!down)
    {
        unsigned int index = 0;
        for (std::vector<FJ_FuiNode*>::iterator it = m_entries.begin(); it != m_entries.end(); ++it, ++index)
        {
            if (y >= (*it)->getY() && y < (*it)->getY() + (*it)->getHeight())
            {
                m_touchEntry = static_cast<int>(index);
                TouchHighlightEntry(static_cast<int>(index));
                m_touchY = y;
                return (uint64_t)(uintptr_t)this << 32;
            }
        }
        m_touchY = y;
        return (uint64_t)(uintptr_t)this << 32;
    }

    if (m_entries.empty())
    {
        HandleScrollListArrows(false);
        return (uint64_t)(uintptr_t)this << 32;
    }

    float rowHeight = m_entries[0]->getHeight() + static_cast<float>(m_entrySpacing);
    if (y > m_touchY)
    {
        int rows = static_cast<int>(static_cast<float>(y - m_touchY) / rowHeight);
        if (rows >= 1)
        {
            int oldFirst = m_firstEntry;
            int newFirst = oldFirst - rows;
            if (newFirst < 0)
                newFirst = 0;
            m_firstEntry = newFirst;
            if (oldFirst != newFirst)
            {
                if (newFirst < m_numColumns)
                    RequestData(false);
                UpdateVisibleLeaderboard();
                if (m_upArrow != NULL)
                    m_upArrow->ChangeState(1);
                int highlight = m_highlightEntry + rows;
                if (highlight < 0 || highlight >= static_cast<int>(m_entries.size()))
                    highlight = static_cast<int>(m_entries.size()) - 1;
                m_highlightEntry = highlight;
                getStage()->setFocus(m_entries[highlight]);
            }
            m_touchY = y;
        }
        else
        {
            HandleScrollListArrows(false);
        }
    }
    else if (y < m_touchY)
    {
        int rows = static_cast<int>(static_cast<float>(m_touchY - y) / rowHeight);
        if (rows >= 1)
        {
            int oldFirst = m_firstEntry;
            int newFirst = oldFirst + rows;
            int maxFirst = m_requestSize - m_visibleEntries;
            if (newFirst > maxFirst)
                newFirst = maxFirst;
            m_firstEntry = newFirst;
            if (oldFirst != newFirst)
            {
                if (static_cast<unsigned int>(newFirst + static_cast<int>(m_entries.size()) - 1) >
                    static_cast<unsigned int>(m_requestSize))
                    RequestData(true);
                UpdateVisibleLeaderboard();
                if (m_downArrow != NULL)
                    m_downArrow->ChangeState(4);
                int highlight = m_highlightEntry - rows;
                if (highlight < 0)
                    highlight = 0;
                m_highlightEntry = highlight;
                getStage()->setFocus(m_entries[highlight]);
            }
            m_touchY = y;
        }
        else
        {
            HandleScrollListArrows(false);
        }
    }
    else
    {
        HandleScrollListArrows(false);
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

typedef std::map<int, FJ_LeaderboardList::CLeaderboardEntry> LeaderboardMap;
typedef LeaderboardMap::iterator LeaderboardIterator;

LeaderboardIterator __CPR340___Lbound__Q2_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J70JJ90JXCbL_1_0CFRCQ2_Z1Z8key_type_Q5_Z1Z14allocator_type50rebind__tm__36_Q3_3std20_Tree_nod__tm__4_Z1Z5_Node5other7pointer(LeaderboardMap& tree, const int& key)
{
    return tree.lower_bound(key);
}

LeaderboardIterator __CPR340___Ubound__Q2_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J70JJ90JXCbL_1_0CFRCQ2_Z1Z8key_type_Q5_Z1Z14allocator_type50rebind__tm__36_Q3_3std20_Tree_nod__tm__4_Z1Z5_Node5other7pointer(LeaderboardMap& tree, const int& key)
{
    return tree.upper_bound(key);
}

void __CPR249___Inc__Q3_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J67JJ87JXCbL_1_014const_iteratorFv_v(LeaderboardIterator& iterator)
{
    ++iterator;
}

void __CPR234__clear__Q2_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J68JJ88JXCbL_1_0Fv_v(LeaderboardMap& tree)
{
    tree.clear();
}

void __CPR324___Lrotate__Q2_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J71JJ91JXCbL_1_0FQ5_Z1Z14allocator_type50rebind__tm__36_Q3_3std20_Tree_nod__tm__4_Z1Z5_Node5other7pointer_v(LeaderboardMap& tree, LeaderboardIterator iterator)
{
    if (iterator != tree.end())
        tree.insert(*iterator);
}

void __CPR324___Rrotate__Q2_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J71JJ91JXCbL_1_0FQ5_Z1Z14allocator_type50rebind__tm__36_Q3_3std20_Tree_nod__tm__4_Z1Z5_Node5other7pointer_v(LeaderboardMap& tree, LeaderboardIterator iterator)
{
    if (iterator != tree.end())
        tree.insert(*iterator);
}

void __CPR300__erase__Q2_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J68JJ88JXCbL_1_0FQ3_3std16_Tree__tm__4_Z1Z8iterator_Q3_3stdJ238JJ256J(LeaderboardMap& tree, LeaderboardIterator iterator)
{
    if (iterator != tree.end())
        tree.erase(iterator);
}

void __CPR302__erase__Q2_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J68JJ88JXCbL_1_0FQ3_3std16_Tree__tm__4_Z1Z8iteratorT1_Q3_3stdJ238JJ256J(LeaderboardMap& tree, LeaderboardIterator first, LeaderboardIterator last)
{
    tree.erase(first, last);
}

LeaderboardIterator __CPR282__find__Q2_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J67JJ87JXCbL_1_0FRCQ2_Z1Z8key_type_Q3_3std16_Tree__tm__4_Z1Z8iterator(LeaderboardMap& tree, const int& key)
{
    return tree.find(key);
}

FJ_LeaderboardList::CLeaderboardEntry* __CPR93____ct__Q2_18FJJ_LeaderboardList17CLeaderboardEntryFRCQ2_J9JJ29J(FJ_LeaderboardList::CLeaderboardEntry* entry, const FJ_LeaderboardList::CLeaderboardEntry& source)
{
    if (entry != NULL)
        *entry = source;
    return entry;
}

LeaderboardIterator __CPR377___Insert__Q2_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J70JJ90JXCbL_1_0FbQ5_Z1Z14allocator_type50rebind__tm__36_Q3_3std20_Tree_nod__tm__4_Z1Z5_Node5other7pointerRCQ2_Z1Z10value_type_Q3_3std16_Tree__tm__J297J8iterator(LeaderboardMap& tree, LeaderboardIterator position, const LeaderboardMap::value_type& value, bool)
{
    return tree.insert(position, value);
}

FJ_LeaderboardList::CLeaderboardEntry& __CPR198____vc__Q2_3std171map__tm__159_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J33JJ53JFRCZ1Z_RZ2Z(LeaderboardMap& tree, const int& key)
{
    return tree[key];
}

FJ_LeaderboardList::CLeaderboardEntry* __CPR324___Buynode__Q2_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J71JJ91JXCbL_1_0Fv_Q5_Z1Z14allocator_type50rebind__tm__36_Q3_3std20_Tree_nod__tm__4_Z1Z5_Node5other7pointer()
{
    return new FJ_LeaderboardList::CLeaderboardEntry();
}

void __CPR234___Init__Q2_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J68JJ88JXCbL_1_0Fv_v(LeaderboardMap& tree)
{
    tree.clear();
}

FJ_LeaderboardList::CLeaderboardEntry& __CPR93____as__Q2_18FJJ_LeaderboardList17CLeaderboardEntryFRCQ2_J9JJ29J(FJ_LeaderboardList::CLeaderboardEntry& entry, const FJ_LeaderboardList::CLeaderboardEntry& source)
{
    entry = source;
    return entry;
}

void __CPR234___Tidy__Q2_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J68JJ88JXCbL_1_0Fv_v(LeaderboardMap& tree)
{
    tree.clear();
}

void __CPR249___Dec__Q3_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J67JJ87JXCbL_1_014const_iteratorFv_v(LeaderboardIterator& iterator)
{
    --iterator;
}

void __CPR322___Erase__Q2_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J69JJ89JXCbL_1_0FQ5_Z1Z14allocator_type50rebind__tm__36_Q3_3std20_Tree_nod__tm__4_Z1Z5_Node5other7pointer_v(LeaderboardMap& tree, LeaderboardIterator iterator)
{
    if (iterator != tree.end())
        tree.erase(iterator);
}

std::pair<LeaderboardIterator, bool> __CPR310__insert__Q2_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J69JJ89JXCbL_1_0FRCQ2_Z1Z10value_type_Q2_3std48pair__tm__36_Q3_3std16_Tree__tm__4_Z1Z8iteratorb(LeaderboardMap& tree, const LeaderboardMap::value_type& value, bool)
{
    return tree.insert(value);
}

LeaderboardIterator __CPR321__insert__Q2_3std213_Tree__tm__199_Q2_3std188_Tmap_traits__tm__167_iQ2_18FJJ_LeaderboardList17CLeaderboardEntryQ2_3std13less__tm__2_iQ2_3std84allocator__tm__67_Q2_3std57pair__tm__45_CiQ2_J69JJ89JXCbL_1_0FQ3_3std16_Tree__tm__4_Z1Z8iteratorRCQ2_Z1Z10value_type_Q3_3stdJ239JJ257J(LeaderboardMap& tree, LeaderboardIterator position, const LeaderboardMap::value_type& value)
{
    return tree.insert(position, value);
}

uint64_t __CPR220__AddDataSet__18FJJ_LeaderboardListFbiT2RCQ2_3std78basic_string__tm__58_wQ2_3std20char_traits__tm__2_wQ2_3std18allocator__tm__2_wT1PCQ2_3stdJ46JN66(
    FJ_LeaderboardList* list, bool loading, int rank, int value,
    const std::wstring& gamertag, bool complete, const std::wstring* first,
    const std::wstring* second, const std::wstring* third,
    const std::wstring* fourth, const std::wstring* fifth,
    const std::wstring* sixth, const std::wstring* seventh)
{
    FJ_LeaderboardList::CLeaderboardEntry entry;
    entry.rank = value;
    entry.gamertag = gamertag.size() > 16 ? gamertag.substr(0, 16) + L"..." : gamertag;
    entry.hasData = complete;
    const std::wstring* columns[7] = {first, second, third, fourth, fifth, sixth, seventh};
    for (int i = 0; i < 7; ++i)
        entry.values[i] = columns[i] == NULL ? std::wstring() : *columns[i];
    FJ_LeaderboardList::CLeaderboardEntry& stored = list->m_leaderboard[rank];
    stored = entry;
    if (complete)
    {
        if (list->m_firstEntry < 0 || rank < list->m_firstEntry)
            list->m_firstEntry = rank;
        if (rank > list->m_requestSize)
            list->m_requestSize = rank;
    }
    if (loading)
    {
        list->UpdateVisibleLeaderboard();
        if (list->m_needsUpdate)
            list->FocusOnFirstEntry();
        list->m_needsUpdate = false;
        list->SetDataLoading(false);
    }
    (void)value;
    return (uint64_t)(uintptr_t)list << 32;
}
