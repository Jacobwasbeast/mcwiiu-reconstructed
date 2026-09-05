#ifndef GAME_UI_FJ_LEADERBOARDENTRY_HPP
#define GAME_UI_FJ_LEADERBOARDENTRY_HPP

#include <string>

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;
class fuiRenderNodeEditText;
class FJ_LeaderboardList
{
public:
    struct CLeaderboardEntry
    {
        int rank;
        std::wstring gamertag;
        bool hasData;
        std::wstring values[7];
    };
};

class FJ_LeaderboardEntry : public FJ_FuiNode
{
public:
    FJ_LeaderboardEntry(fuiRenderNode* pRenderNode);
    virtual ~FJ_LeaderboardEntry();

    static FJ_LeaderboardEntry* Create(fuiRenderNode* pRenderNode);

    void Init(int count, int firstValue, int spacing, int offset);
    void SetData(FJ_LeaderboardList::CLeaderboardEntry* pEntry);
    void ChangeState(int state);
    void focusInHandler(FJ_Event* pEvent);
    static void _focusInHandler(FJ_FuiNode* pNode, FJ_Event* pEvent);
    void focusOutHandler(FJ_Event* pEvent);
    static void _focusOutHandler(FJ_FuiNode* pNode, FJ_Event* pEvent);
    virtual unsigned int getType();
    void UpdateHighlightColours();

private:
    int m_state;
    fuiRenderNodeEditText* m_rankText;
    fuiRenderNodeEditText* m_gamertagText;
    fuiRenderNodeEditText* m_columnText[7];
    int m_unknown48;
    int m_unknown4c;
    int m_unknown50;
    int m_unknown54;
    int m_unknown58;
    int m_unknown5c;
    int m_unknown60;
    int m_firstValuePosition;
    int m_numColumns;
    float m_rankTextWidth;
    float m_gamertagTextWidth;
    float m_columnTextWidth;
};

#endif
