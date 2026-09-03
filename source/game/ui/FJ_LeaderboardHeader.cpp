#include "game/ui/FJ_LeaderboardHeader.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"
#include <cstdio>

namespace fui
{
    fuiRenderNode* createNodeFromSymbol(const char*, fuiRenderNode* parent = NULL);
}

class fuiRenderNodeEditText : public fuiRenderNode
{
public:
    void setAutoSize(int);
    void setText(const std::wstring&);
    float getTextWidth();
};

class fuiRenderNodeCodeGenRect : public fuiRenderNode
{
public:
    fuiRenderNodeCodeGenRect(fuiRenderNode*, fuiRect*, const char*);
};

FJ_LeaderboardHeader::FJ_LeaderboardHeader(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode), m_firstEntryPos(0), m_entrySpacing(0),
      m_rankTextWidth(0.0f), m_gamertagTextWidth(0.0f),
      m_small(false), m_slotWidth(0), m_slotAreaWidth(0)
{
    int i;
    for (i = 0; i < 7; ++i)
        m_icons[i] = NULL;
}

FJ_LeaderboardHeader::~FJ_LeaderboardHeader()
{
}

FJ_LeaderboardHeader* FJ_LeaderboardHeader::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_LeaderboardHeader(pRenderNode);
}

void FJ_LeaderboardHeader::Init(int count, FJ_FuiNode* parent, int x, int y)
{
    int i;
    char name[64];
    for (i = 0; i < 7; ++i)
    {
        std::sprintf(name, "Icon_%d", i + 1);
        m_icons[i] = getChildByName(name)->asMovieClip();
        parent->addChild(m_icons[i]);
        if (m_slotWidth == 0)
            m_slotWidth = (int)m_icons[i]->getWidth();
        m_icons[i]->setScaleX(1.0f);
        m_icons[i]->setScaleY(1.0f);
        m_icons[i]->gotoAndStop("SlotNormal");
    }
    m_slotAreaWidth = (int)m_icons[0]->getWidth();
    int left = (int)m_icons[0]->getX();
    int right = (int)m_icons[6]->getX();
    m_entrySpacing = (right - left) / 6;
    m_firstEntryPos = left + m_entrySpacing + ((right - left) - (count - 1) * m_entrySpacing) / count;
    float scale = (float)(((right - left) - (count - 1) * m_entrySpacing) / count) * 0.5f;
    m_entrySpacing = Math::round(scale);
    for (i = 0; i < 7; ++i)
    {
        if (i >= count)
            m_icons[i]->setVisible(false);
        else
        {
            m_icons[i]->setX((float)(x + Math::round((float)(m_firstEntryPos + i * m_entrySpacing - left) * getScaleX())));
            m_icons[i]->setY((float)(m_icons[0]->getY() + y));
            m_icons[i]->setVisible(true);
        }
    }
}

void FJ_LeaderboardHeader::SetupHeaders(std::wstring rank, std::wstring gamertag)
{
    FJ_FuiNode* rankContainer = getChildByName("RankContainer")->getChildAt(0)->asMovieClip();
    FJ_FuiNode* gamertagContainer = getChildByName("GamertagContainer")->getChildAt(0)->asMovieClip();
    fuiRenderNodeEditText* rankText = static_cast<fuiRenderNodeEditText*>(rankContainer->getRenderNodeChildAt(0)->asEditText());
    if (m_rankTextWidth == 0.0f)
        m_rankTextWidth = rankText->getTextWidth();
    rankText->setAutoSize(1);
    rankText->setText(rank);
    rankText->setScaleX(1.0f / getScaleX());
    rankText->setScaleY(1.0f / getScaleY());
    rankText->setX((m_rankTextWidth - rankText->getTextWidth()) * 0.5f);
    fuiRenderNodeEditText* gamertagText = static_cast<fuiRenderNodeEditText*>(gamertagContainer->getRenderNodeChildAt(0)->asEditText());
    if (m_gamertagTextWidth == 0.0f)
        m_gamertagTextWidth = gamertagText->getTextWidth();
    gamertagText->setAutoSize(1);
    gamertagText->setText(gamertag);
    gamertagText->setScaleX(1.0f / getScaleX());
    gamertagText->setScaleY(1.0f / getScaleY());
    gamertagText->setX((m_gamertagTextWidth - gamertagText->getTextWidth()) * 0.5f);
}

fuiRenderNode* FJ_LeaderboardHeader::GetBitmapData(int type)
{
    static const char* normal[] = { "", "", "LeaderBoard_Icon_Fallen", "LeaderBoard_Icon_Walked", "LeaderBoard_Icon_Swam", "LeaderBoard_Icon_Zombie", "LeaderBoard_Icon_Killed", "LeaderBoard_Icon_Creeper", "LeaderBoard_Icon_Creeper", "LeaderBoard_Icon_Skeleton", "LeaderBoard_Icon_Spider", "LeaderBoard_Icon_SpiderJockey", "LeaderBoard_Icon_Slime", "LeaderBoard_Icon_Portal", "LeaderBoard_Icon_Score", "LeaderBoard_Icon_Time", "LeaderBoard_Icon_TotalRings", "LeaderBoard_Icon_Climbed" };
    static const char* small[] = { "", "", "LeaderBoard_Icon_Fallen_Small", "LeaderBoard_Icon_Walked_Small", "LeaderBoard_Icon_Swam_Small", "LeaderBoard_Icon_Zombie_Small", "LeaderBoard_Icon_Killed_Small", "LeaderBoard_Icon_Creeper_Small", "LeaderBoard_Icon_Creeper_Small", "LeaderBoard_Icon_Skeleton_Small", "LeaderBoard_Icon_Spider_Small", "LeaderBoard_Icon_SpiderJockey_Sm", "LeaderBoard_Icon_Slime_Small", "LeaderBoard_Icon_Portal_Small", "LeaderBoard_Icon_Score_Small", "LeaderBoard_Icon_Time_Small", "LeaderBoard_Icon_TotalRings_Smal", "LeaderBoard_Icon_Climbed_Small" };
    if (type < 0 || type > 17)
        type = 17;
    return fui::createNodeFromSymbol(m_small ? small[type] : normal[type], m_pRenderNode);
}

void FJ_LeaderboardHeader::SetupSlot(int slot, int type)
{
    FJ_FuiNode* control = getChildByName("SlotControl");
    if (slot < 0 || slot >= 7)
        return;
    control = m_icons[slot]->getChildByName("SlotControl")->asMovieClip();
    control->setWidth((float)m_slotWidth);
    control->setHeight((float)m_slotWidth);
    if (type == 0)
    {
        fuiRect rect;
        rect.xMin = control->getX();
        rect.xMax = rect.xMin + control->getWidth();
        rect.yMin = control->getY();
        rect.yMax = rect.yMin + control->getHeight();
        fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(m_pRenderNode, &rect, "iggy_slot");
        renderNode->setAlpha(0.25f);
        FJ_FuiNode* blank = new FJ_FuiNode(renderNode);
        control->addChild(blank);
        control->removeChild(control->getChildAt(0), true);
        return;
    }
    fuiRenderNode* bitmap = GetBitmapData(type);
    bitmap->setWidth(control->getWidth());
    bitmap->setHeight(control->getHeight());
    bitmap->setX(control->getX());
    bitmap->setY(control->getY());
    control->addChild(bitmap);
    control->removeChild(control->getChildAt(0), true);
}

unsigned int FJ_LeaderboardHeader::getFirstEntryPos()
{
    return (unsigned int)m_firstEntryPos;
}

int FJ_LeaderboardHeader::getEntrySpacing()
{
    return m_entrySpacing;
}

unsigned int FJ_LeaderboardHeader::getIconHolderHalfWidth()
{
    return (unsigned int)Math::round((m_icons[0]->getWidth() * 0.5f) / getScaleX());
}

void FJ_LeaderboardHeader::Destroy()
{
    int i;
    for (i = 0; i < 7; ++i)
    {
        if (m_icons[i] != NULL && m_icons[i]->getParent() != NULL)
            m_icons[i]->getParent()->removeChild(m_icons[i], true);
        m_icons[i] = NULL;
    }
}

unsigned int FJ_LeaderboardHeader::getType()
{
    return 0x69;
}

void FJ_LeaderboardHeader::ASConstructor()
{
    FJ_FuiNode* node = getChildByName("SlotArea_Small");
    if (node == NULL)
        node = getChildByName("SlotArea")->asMovieClip();
    else
        m_small = true;
    node->setVisible(false);
}
