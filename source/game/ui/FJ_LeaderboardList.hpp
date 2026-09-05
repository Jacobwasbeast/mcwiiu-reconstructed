#ifndef GAME_UI_FJ_LEADERBOARDLIST_HPP
#define GAME_UI_FJ_LEADERBOARDLIST_HPP

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "game/ui/FJ_FuiNode.hpp"
class FJ_LeaderboardHeader;
class FJ_ScrollArrow;
class FJ_Label;
class fuiRenderNode;
class FJ_LeaderboardEntry;

class FJ_LeaderboardList : public FJ_FuiNode
{
public:
    struct CLeaderboardEntry
    {
        int rank;
        std::wstring gamertag;
        bool hasData;
        std::wstring values[7];
    };

    FJ_LeaderboardList(fuiRenderNode* pRenderNode);
    virtual ~FJ_LeaderboardList();

    void SetupPleaseWait(FJ_Label* label);
    uint64_t UpdateLeaderboardCache(bool below);
    void RequestData(bool below);
    void addHeader();
    void HandleScrollListArrows(bool addArrows);
    void addNewItem(int index);
    void UpdateVisibleLeaderboard();
    uint64_t NavigateLeaderboard(bool down, bool page);
    static FJ_FuiNode* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
    virtual void ASConstructor();
    uint64_t SetupTitles(std::wstring rank, std::wstring gamertag);
    void SetDataLoading(bool loading);
    void InitLeaderboard(int firstValue, int spacing, int offset);
    void SetColumnIcon(int column, int icon);
    void ResetLeaderboard();
    uint64_t TouchHighlightEntry(int index);
    uint64_t SetTouchFocus(int x, int y, bool focus);
    uint64_t FocusOnFirstEntry();

private:
    bool m_initialized;
    int m_visibleEntries;
    int m_numColumns;
    int m_entrySpacing;
    int m_numEntries;
    std::map<int, CLeaderboardEntry> m_leaderboard;
    std::vector<FJ_LeaderboardEntry*> m_entryNodes;
    FJ_LeaderboardHeader* m_header;
    int m_firstEntry;
    int m_lastEntry;
    std::wstring m_rankTitle;
    std::wstring m_gamertagTitle;
    int m_columnIcons[7];
    FJ_FuiNode* m_upArrow;
    FJ_ScrollArrow* m_downArrow;
    FJ_Label* m_pleaseWait;
    int m_touchEntry;
    int m_touchY;
};

#endif
