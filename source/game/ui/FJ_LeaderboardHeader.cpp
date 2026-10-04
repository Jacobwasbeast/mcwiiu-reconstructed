#include "game/ui/FJ_LeaderboardHeader.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"
#include <cstdio>

FJ_LeaderboardHeader::FJ_LeaderboardHeader(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode), m_rankTextWidth(0.0f), m_gamertagTextWidth(0.0f), m_small(false), m_slotWidth(0),
      m_slotAreaWidth(0)
{
    for (int i = 0; i < 7; ++i)
        m_icons[i] = NULL;
}

FJ_LeaderboardHeader* FJ_LeaderboardHeader::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_LeaderboardHeader(pRenderNode);
}

void FJ_LeaderboardHeader::Init(int count, FJ_FuiNode* parent, int x, int y)
{
    char name[64];
    for (int i = 0; i < 7; ++i)
    {
        std::sprintf(name, "Icon_%d", i);
        m_icons[i] = getChildByName(std::string(name))->asMovieClip();
        parent->addChild(m_icons[i]);
        if (m_slotWidth == 0)
            m_slotWidth = static_cast<int>(m_icons[0]->getWidth());
        m_icons[i]->setScaleX(1.0f);
        m_icons[i]->setScaleY(1.0f);
        m_icons[i]->gotoAndStop(std::string("SlotNormal"));
    }

    float iconY = static_cast<float>(static_cast<int>(m_icons[0]->getY()) + y);
    m_slotAreaWidth = static_cast<int>(m_icons[0]->getWidth());
    int left = static_cast<int>(m_icons[0]->getX());
    int right = static_cast<int>(m_icons[6]->getX());
    int spacing = (right - left) / 6;
    int slotWidth = ((right - left) - (count - 1) * spacing) / count;
    float halfSlotWidth = static_cast<float>(slotWidth) * 0.5f;
    for (int i = 0; i < 7; ++i)
    {
        if (i < count)
        {
            m_entryPositions[i] = Math::round(static_cast<float>(left + i * (spacing + slotWidth)) + halfSlotWidth);
            m_icons[i]->setX(static_cast<float>(x + Math::round(static_cast<float>(m_entryPositions[i]) * getScaleX())));
            m_icons[i]->setY(iconY);
            m_icons[i]->setVisible(true);
        }
        else
        {
            m_icons[i]->setVisible(false);
        }
    }
}

void FJ_LeaderboardHeader::SetupHeaders(std::wstring rank, std::wstring gamertag)
{
    FJ_FuiNode* rankContainer = getChildByName("RankContainer")->getChildAt(0)->asMovieClip();
    FJ_FuiNode* gamertagContainer = getChildByName("GamertagContainer")->getChildAt(0)->asMovieClip();
    fuiRenderNodeEditText* rankText = rankContainer->getRenderNodeChildAt(0)->asEditText();
    if (m_rankTextWidth == 0.0f)
        m_rankTextWidth = rankText->getWidth();
    rankText->setAutoSize(TextFieldAutoSizeLeft);
    rankText->setText(rank);
    rankText->setScaleX(1.0f / getScaleX());
    rankText->setScaleY(1.0f / getScaleY());
    rankText->setX(-rankText->getWidth() * 0.5f + m_rankTextWidth * 0.5f);
    fuiRenderNodeEditText* gamertagText = gamertagContainer->getRenderNodeChildAt(0)->asEditText();
    if (m_gamertagTextWidth == 0.0f)
        m_gamertagTextWidth = gamertagText->getWidth();
    gamertagText->setAutoSize(TextFieldAutoSizeLeft);
    gamertagText->setText(gamertag);
    gamertagText->setScaleX(1.0f / getScaleX());
    gamertagText->setScaleY(1.0f / getScaleY());
    gamertagText->setX(-gamertagText->getWidth() * 0.5f + m_gamertagTextWidth * 0.5f);
}

fuiRenderNode* FJ_LeaderboardHeader::GetBitmapData(int type)
{
    static const char* normal[] = {
        "LeaderBoard_Icon_Fallen", "LeaderBoard_Icon_Walked", "LeaderBoard_Icon_Swam",
        "LeaderBoard_Icon_Zombie", "LeaderBoard_Icon_ZombiePigman", "LeaderBoard_Icon_Ghast",
        "LeaderBoard_Icon_Creeper", "LeaderBoard_Icon_Skeleton", "LeaderBoard_Icon_Spider",
        "LeaderBoard_Icon_SpiderJockey", "LeaderBoard_Icon_Slime", "LeaderBoard_Icon_Portal",
        "LeaderBoard_Icon_Score", "LeaderBoard_Icon_Time", "LeaderBoard_Icon_TotalRings"
    };
    static const char* small[] = {
        "LeaderBoard_Icon_Fallen_Small", "LeaderBoard_Icon_Walked_Small",
        "LeaderBoard_Icon_Swam_Small", "LeaderBoard_Icon_Zombie_Small",
        "LeaderBoard_Icon_ZombiePigman_Small", "LeaderBoard_Icon_Ghast_Small",
        "LeaderBoard_Icon_Creeper_Small", "LeaderBoard_Icon_Skeleton_Small",
        "LeaderBoard_Icon_Spider_Small", "LeaderBoard_Icon_SpiderJockey_Small",
        "LeaderBoard_Icon_Slime_Small", "LeaderBoard_Icon_Portal_Small",
        "LeaderBoard_Icon_Score_Small", "LeaderBoard_Icon_Time_Small",
        "LeaderBoard_Icon_TotalRings_Small"
    };
    const char* name;
    if (type >= 2 && type <= 16)
        name = m_small ? small[type - 2] : normal[type - 2];
    else
        name = m_small ? "LeaderBoard_Icon_Climbed_Small" : "LeaderBoard_Icon_Climbed";
    return gFui->createNodeFromSymbol(name, m_pRenderNode);
}

void FJ_LeaderboardHeader::SetupSlot(int slot, int type)
{
    FJ_FuiNode* icon = m_icons[slot];
    FJ_FuiNode* control = m_icons[slot]->getChildByName(std::string("SlotControl"))->asMovieClip();
    m_icons[slot]->setWidth(static_cast<float>(m_slotWidth));
    m_icons[slot]->setHeight(static_cast<float>(m_slotWidth));
    if (type == 0)
    {
        fuiRect rect;
        rect.xMin = control->getX();
        rect.yMin = control->getY();
        rect.xMax = rect.xMin + control->getWidth();
        rect.yMax = rect.yMin + control->getHeight();
        char name[64];
        std::sprintf(name, "iggy_slot_%d", slot);
        fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, name);
        renderNode->setAlpha(0.25f);
        renderNode->setFuiNode(new FJ_FuiNode(renderNode));
        m_icons[slot]->addChild(renderNode->getFuiNode());
        m_icons[slot]->removeChild(control, true);
        return;
    }

    fuiRenderNode* bitmap = GetBitmapData(type);
    bitmap->setWidth(control->getWidth());
    bitmap->setHeight(control->getHeight());
    bitmap->setX(control->getX());
    bitmap->setY(control->getY());
    m_icons[slot]->addChild(bitmap);
    m_icons[slot]->removeChild(control, true);
}

unsigned int FJ_LeaderboardHeader::getFirstEntryPos()
{
    return m_entryPositions[0];
}

int FJ_LeaderboardHeader::getEntrySpacing()
{
    return m_entryPositions[1] - m_entryPositions[0];
}

unsigned int FJ_LeaderboardHeader::getIconHolderHalfWidth()
{
    return Math::round(m_icons[0]->getWidth() * 0.5f / getScaleX());
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
