#include "game/ui/FJ_LeaderboardEntry.hpp"

#include <cstdio>
#include <vector>

#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/core/StringConversion.hpp"

typedef std::vector<FJ_LeaderboardEntry*> FJLeaderboardEntryVector;
typedef FJLeaderboardEntryVector::iterator FJLeaderboardEntryIterator;

void __CPR241___Insert_n__Q2_3std86vector__tm__72_P19FJJ_LeaderboardEntryQ2_3std40allocator__tm__23_PJ36JFQ2_3std68_Vector_iterator__tm__44_Z1ZQ3_3std25_Vector_val__tm__7_Z1ZZ2Z5_AltyQ4_3stdJ151JJ178J9size_typeRCZ1Z_v(
    FJLeaderboardEntryVector& vector, FJLeaderboardEntryIterator position,
    FJLeaderboardEntryVector::size_type count, FJ_LeaderboardEntry* const& value)
{
    vector.insert(position, count, value);
}

FJLeaderboardEntryIterator __CPR264__insert__Q2_3std86vector__tm__72_P19FJJ_LeaderboardEntryQ2_3std40allocator__tm__23_PJ33JFQ2_3std68_Vector_iterator__tm__44_Z1ZQ3_3std25_Vector_val__tm__7_Z1ZZ2Z5_AltyRCZ1Z_Q2_3stdJ111J(
    FJLeaderboardEntryVector& vector, FJLeaderboardEntryIterator position,
    FJ_LeaderboardEntry* const& value)
{
    return vector.insert(position, value);
}

FJ_LeaderboardEntry::FJ_LeaderboardEntry(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode), m_state(0), m_rankText(NULL), m_gamertagText(NULL),
      m_unknown48(0), m_unknown4c(0), m_unknown50(0), m_unknown54(0),
      m_unknown58(0), m_unknown5c(0), m_unknown60(0), m_firstValuePosition(0), m_numColumns(0),
      m_rankTextWidth(0.0f), m_gamertagTextWidth(0.0f), m_columnTextWidth(0.0f)
{
    int i;
    m_columnText[0] = NULL;
    m_columnText[1] = NULL;
    m_columnText[2] = NULL;
    m_columnText[3] = NULL;
    m_columnText[4] = NULL;
    m_columnText[5] = NULL;
    m_columnText[6] = NULL;

    FJ_FuiNode* rankContainer = getChildByName("RankContainer")->getChildAt(0)->asMovieClip();
    FJ_FuiNode* gamertagContainer = getChildByName("GamertagContainer")->getChildAt(0)->asMovieClip();
    m_rankText = static_cast<fuiRenderNodeEditText*>(rankContainer->getRenderNodeChildAt(0)->asEditText());
    m_gamertagText = static_cast<fuiRenderNodeEditText*>(gamertagContainer->getRenderNodeChildAt(0)->asEditText());
    m_rankTextWidth = m_rankText->getWidth();
    m_gamertagTextWidth = m_gamertagText->getWidth();

    char name[64];
    for (i = 0; i < 7; ++i)
    {
        std::sprintf(name, "FJ_Column_%d", i);
        FJ_FuiNode* column = getChildByName(name)->getChildAt(0)->asMovieClip();
        m_columnText[i] = static_cast<fuiRenderNodeEditText*>(column->getRenderNodeChildAt(0)->asEditText());
        addChild(m_columnText[i]);
        m_columnText[i]->setX(column->getX());
        m_columnText[i]->setY(column->getY());
        m_columnTextWidth = m_columnText[i]->getWidth();
        column->getParent()->removeChild(column, true);
    }
    ChangeState(0);
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
}

FJ_LeaderboardEntry::~FJ_LeaderboardEntry()
{
}

void FJ_LeaderboardEntry::Init(int count, int firstValue, int spacing, int offset)
{
    int i;
    int position = firstValue + offset;
    int value = firstValue + spacing + offset;
    for (i = 0; i < 7; ++i)
    {
        if (i * 2 >= count)
            m_columnText[i]->setVisibility(false);
        else
        {
            m_columnText[i]->setX((float)value);
            m_columnText[i]->setVisibility(true);
        }
        value += spacing * 2;
        position += spacing * 2;
    }
    m_numColumns = count;
    m_firstValuePosition = (firstValue * 2 + offset * 2 + spacing * (count - 1)) / 2;
}

void FJ_LeaderboardEntry::SetData(FJ_LeaderboardList::CLeaderboardEntry* pEntry)
{
    int i;
    float scale = 1.0f;
    if (pEntry == NULL)
        m_rankText->setText(L"");
    else
        m_rankText->setText(_toString(pEntry->rank));
    m_rankText->setAutoSize(1);
    m_rankText->setScaleX(scale / getScaleX());
    m_rankText->setScaleY(scale / getScaleY());
    m_rankText->setX(m_rankTextWidth * 0.5f - m_rankText->getWidth() * 0.5f);

    if (pEntry == NULL)
        m_gamertagText->setText(L"");
    else
        m_gamertagText->setText(pEntry->gamertag);
    m_gamertagText->setAutoSize(1);
    m_gamertagText->setScaleX(scale / getScaleX());
    m_gamertagText->setScaleY(scale / getScaleY());
    m_gamertagText->setX(m_gamertagTextWidth * 0.5f - m_gamertagText->getWidth() * 0.5f);

    if (pEntry != NULL && pEntry->hasData)
    {
        for (i = 0; i < 7; ++i)
            m_columnText[i]->setVisibility(false);
        m_columnText[0]->setText(pEntry->values[0]);
        m_columnText[0]->setAutoSize(1);
        m_columnText[0]->setScaleX(scale / getScaleX());
        m_columnText[0]->setScaleY(scale / getScaleY());
        m_columnText[0]->setVisibility(true);
        return;
    }
    for (i = 0; i < 7; ++i)
    {
        if (pEntry == NULL || pEntry->values[i].compare(L"") == 0 || i >= m_numColumns)
            m_columnText[i]->setVisibility(false);
        else
        {
            m_columnText[i]->setText(pEntry->values[i]);
            m_columnText[i]->setAutoSize(1);
            m_columnText[i]->setScaleX(scale / getScaleX());
            m_columnText[i]->setScaleY(scale / getScaleY());
            m_columnText[i]->setVisibility(true);
        }
    }
}

void FJ_LeaderboardEntry::ChangeState(int state)
{
    gotoAndStop(state);
    m_state = state;
}

void FJ_LeaderboardEntry::focusInHandler(FJ_Event*)
{
    gotoAndStop("FocusIn");
    m_state = 1;
}

void FJ_LeaderboardEntry::_focusInHandler(FJ_FuiNode* pNode, FJ_Event* pEvent)
{
    static_cast<FJ_LeaderboardEntry*>(pNode)->focusInHandler(pEvent);
}

void FJ_LeaderboardEntry::focusOutHandler(FJ_Event*)
{
    gotoAndStop("FocusOut");
    m_state = 0;
}

void FJ_LeaderboardEntry::_focusOutHandler(FJ_FuiNode* pNode, FJ_Event* pEvent)
{
    static_cast<FJ_LeaderboardEntry*>(pNode)->focusOutHandler(pEvent);
}

unsigned int FJ_LeaderboardEntry::getType()
{
    return 108;
}

void FJ_LeaderboardEntry::UpdateHighlightColours()
{
    FJ_FuiNode* style = getChildByName("StyleContainer")->getChildAt(0)->asMovieClip();
    unsigned int color = style->getRenderNodeChildAt(0)->asEditText()->getColor();
    m_rankText->setColor(color);
    m_gamertagText->setColor(color);
    for (int i = 0; i < 7; ++i)
        m_columnText[i]->setColor(color);
}
