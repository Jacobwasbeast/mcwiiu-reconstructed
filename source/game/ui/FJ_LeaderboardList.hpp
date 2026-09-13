#ifndef GAME_UI_FJ_LEADERBOARDLIST_HPP
#define GAME_UI_FJ_LEADERBOARDLIST_HPP

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "game/ui/FJ_FuiNode.hpp"

class FJ_Label;
class FJ_LeaderboardHeader;
class FJ_ScrollArrow;
class fuiRenderNode;

class FJ_LeaderboardList : public FJ_FuiNode
{
    friend class fuiRenderNodeTimeline;
public:
    struct CLeaderboardEntry
    {
        int rank;
        std::wstring gamertag;
        bool hasData;
        std::wstring values[7];
    };

    FJ_LeaderboardList(fuiRenderNode* renderNode);
    virtual ~FJ_LeaderboardList();

    void SetupPleaseWait(FJ_Label* label);
    uint64_t UpdateLeaderboardCache(bool next);
    void RequestData(bool next);
    void addHeader();
    void HandleScrollListArrows(bool add);
    void addNewItem(int index);
    void UpdateVisibleLeaderboard();
    uint64_t NavigateLeaderboard(bool down, bool page);
    static FJ_FuiNode* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void ASConstructor();
    uint64_t SetupTitles(std::wstring rankTitle, std::wstring gamertagTitle);
    void SetDataLoading(bool loading);
    void InitLeaderboard(int firstRank, int lastRank, int selectedRank);
    void SetColumnIcon(int column, int icon);
    void ResetLeaderboard();
    uint64_t TouchHighlightEntry(int index);
    uint64_t SetTouchFocus(int x, int y, bool down);
    uint64_t FocusOnFirstEntry();

    friend uint64_t __CPR220__AddDataSet__18FJJ_LeaderboardListFbiT2RCQ2_3std78basic_string__tm__58_wQ2_3std20char_traits__tm__2_wQ2_3std18allocator__tm__2_wT1PCQ2_3stdJ46JN66(
        FJ_LeaderboardList*, bool, int, int, const std::wstring&, bool,
        const std::wstring*, const std::wstring*, const std::wstring*,
        const std::wstring*, const std::wstring*, const std::wstring*,
        const std::wstring*);

private:
    bool m_small;
    int m_entrySpacing;
    int m_visibleEntries;
    int m_requestSize;
    int m_numColumns;
    std::map<int, CLeaderboardEntry> m_leaderboard;
    bool m_needsUpdate;
    bool m_dataLoading;
    std::vector<FJ_FuiNode*> m_entries;
    FJ_LeaderboardHeader* m_header;
    int m_highlightEntry;
    int m_firstEntry;
    std::wstring m_rankTitle;
    std::wstring m_gamertagTitle;
    int m_columnIcons[7];
    FJ_ScrollArrow* m_upArrow;
    FJ_ScrollArrow* m_downArrow;
    FJ_Label* m_pleaseWait;
    int m_touchEntry;
    int m_touchY;
};

#endif
