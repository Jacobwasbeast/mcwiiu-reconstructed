#include "game/ui/FJ_PressStartToPlay.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"

FJ_PressStartToPlay::FJ_PressStartToPlay(fuiRenderNode* renderNode)
    : FJ_Document(renderNode),
      m_panelWidth(0),
      m_panelHeight(0),
      m_panelX(0),
      m_mainPanel(0),
      m_achievementToast(0),
      m_achievementToastX(0)
{
    std::string name("MainPanel");
    m_mainPanel = getChildByName(name);
    if (m_mainPanel != 0)
    {
        std::string quadrant("ControllerQuadrant");
        const char* controllers[] = { "Controller1", "Controller2", "Controller3", "Controller4" };
        int i;
        for (i = 0; i < 4; ++i)
        {
            FJ_FuiNode* controller = m_mainPanel->getChildByName(quadrant);
            if (controller != 0)
            {
                controller = controller->getChildAt(0);
                if (controller != 0)
                {
                    FJ_FuiNode* icon = controller->getChildByName(std::string(controllers[i]));
                    if (icon != 0)
                    {
                        icon->setVisible(false);
                    }
                }
            }
        }

        FJ_FuiNode* pressStart = m_mainPanel->getChildByName(std::string("PressStartLabel"));
        if (pressStart != 0)
        {
            m_panelWidth = static_cast<int>(pressStart->getWidth());
            pressStart->setVisible(true);
        }

        FJ_FuiNode* background = m_mainPanel->getChildByName(std::string("BackgroundPanel"));
        if (background != 0)
        {
            background = background->getChildAt(0);
            if (background != 0)
            {
                m_panelX = static_cast<int>(background->getX());
                m_panelHeight = static_cast<int>(background->getHeight());
            }
        }
    }

    std::string toastAnimation("AchievementToastAnimation");
    FJ_FuiNode* animation = getChildByName(toastAnimation);
    if (animation != 0)
    {
        m_achievementToast = animation->getChildByName(std::string("AchievementToast"));
        if (m_achievementToast != 0)
        {
            m_achievementToast = m_achievementToast->asMovieClip();
            if (m_achievementToast != 0)
            {
                FJ_FuiNode* unlocked = m_achievementToast->getChildByName(std::string("Achievement_Unlocked"));
                if (unlocked != 0)
                {
                    m_achievementToastX = static_cast<int>(unlocked->getX());
                }
            }
        }
    }

    FJ_FuiNode* chest = getChildByName(std::string("ChestRefillAnimation"));
    if (chest != 0)
    {
        FJ_FuiNode* movie = FJ_FuiNode::asMovieClip(chest->getChildAt(0));
        if (movie != 0)
        {
            movie->setVisible(false);
        }
    }

    FJ_FuiNode* watermark = getChildByName(std::string("BETAWatermark"));
    if (watermark != 0)
    {
        watermark->setVisible(false);
    }
}

FJ_PressStartToPlay::~FJ_PressStartToPlay()
{
}

FJ_PressStartToPlay* FJ_PressStartToPlay::Create(fuiRenderNode* renderNode)
{
    return new FJ_PressStartToPlay(renderNode);
}

unsigned int FJ_PressStartToPlay::getType()
{
    return 0x26;
}

void FJ_PressStartToPlay::ShowAchievementToast(const std::wstring& title, const std::wstring& description, const std::wstring& gamerPic, const std::wstring& achievementIcon)
{
    (void)title;
    (void)description;
    (void)gamerPic;
    (void)achievementIcon;
    if (m_achievementToast != 0)
    {
        m_achievementToast->setVisible(true);
        gotoAndPlay(std::string("ShowToast"));
    }
}

void FJ_PressStartToPlay::HideAchievementToast()
{
    if (m_achievementToast != 0)
    {
        gotoAndPlay(std::string("HideToast"));
    }
}

void FJ_PressStartToPlay::ResizePanel(bool multiLine)
{
    if (m_mainPanel != 0)
    {
        FJ_FuiNode* pressStart = m_mainPanel->getChildByName(std::string("PressStartLabel"));
        FJ_FuiNode* wordWrap = m_mainPanel->getChildByName(std::string("WordWrapLabel"));
        FJ_FuiNode* background = m_mainPanel->getChildByName(std::string("BackgroundPanel"));
        if (pressStart != 0 && wordWrap != 0 && background != 0)
        {
            FJ_FuiNode* panel = background->getChildAt(0);
            float width = multiLine ? wordWrap->getWidth() : pressStart->getWidth();
            if (panel != 0)
            {
                panel->setX(static_cast<float>(m_panelX));
                panel->setY(static_cast<float>(m_panelHeight));
                if (wordWrap->getHeight() > static_cast<float>(m_panelHeight))
                {
                    panel->incWidth(width - static_cast<float>(m_panelWidth));
                    panel->decY((wordWrap->getHeight() - static_cast<float>(m_panelHeight)) * 0.5f);
                }
                if (width != static_cast<float>(m_panelWidth))
                {
                    panel->incWidth(width - static_cast<float>(m_panelWidth));
                    m_panelWidth = static_cast<int>(width);
                }
            }
        }
    }
}

void FJ_PressStartToPlay::SetText(const std::wstring& text)
{
    (void)text;
    ResizePanel(false);
}

void FJ_PressStartToPlay::SetMultiLineText(const std::wstring& text)
{
    (void)text;
    ResizePanel(true);
}

void FJ_PressStartToPlay::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    FJ_FuiNode* saveIcon = getChildByName(std::string("SaveIcon"));
    if (saveIcon != 0)
    {
        saveIcon->setX(getStage()->getStageWidth() - saveIcon->getWidth() - static_cast<float>(right) - 15.0f);
        saveIcon->setY(static_cast<float>(top + 15));
    }
    FJ_FuiNode* playerName = getChildByName(std::string("PlayerName"));
    if (playerName != 0)
    {
        playerName->setX(getStage()->getStageWidth() - playerName->getWidth() - static_cast<float>(getISafeZoneRight()));
        playerName->setY(getStage()->getStageHeight() - playerName->getHeight() - static_cast<float>(getISafeZoneBottom()));
    }
}

void FJ_PressStartToPlay::ShowController(int controller, bool show)
{
    if (m_mainPanel != 0)
    {
        std::string quadrant("ControllerQuadrant");
        const char* names[] = { "Controller1", "Controller2", "Controller3", "Controller4" };
        int i;
        for (i = 0; i < 4; ++i)
        {
            FJ_FuiNode* group = m_mainPanel->getChildByName(quadrant);
            if (group != 0)
            {
                group = group->getChildAt(0);
                if (group != 0)
                {
                    FJ_FuiNode* node = group->getChildByName(std::string(names[i]));
                    if (node != 0)
                    {
                        node->setVisible(i == controller && show);
                    }
                }
            }
        }
        FJ_FuiNode* label = m_mainPanel->getChildByName(std::string("PressStartLabel"));
        FJ_FuiNode* background = m_mainPanel->getChildByName(std::string("BackgroundPanel"));
        if (label != 0 && background != 0)
        {
            FJ_FuiNode* panel = background->getChildAt(0);
            if (panel != 0)
            {
                panel->setY((label->getHeight() - background->getHeight()) * 0.5f);
            }
        }
    }
}

void FJ_PressStartToPlay::HideChestRefillAnimation()
{
    FJ_FuiNode* animation = getChildByName(std::string("ChestRefillAnimation"));
    if (animation != 0)
    {
        FJ_FuiNode* movie = FJ_FuiNode::asMovieClip(animation->getChildAt(0));
        if (movie != 0)
        {
            movie->setVisible(false);
        }
    }
}

void FJ_PressStartToPlay::ShowChestRefillAnimation(bool left, int yOffset)
{
    FJ_FuiNode* animation = getChildByName(std::string("ChestRefillAnimation"));
    if (animation != 0)
    {
        FJ_FuiNode* movie = FJ_FuiNode::asMovieClip(animation->getChildAt(0));
        if (movie != 0)
        {
            movie->setVisible(true);
        }
        if (!left)
        {
            animation->setX(getStage()->getStageWidth() - animation->getWidth() - static_cast<float>(getISafeZoneRight()) - 15.0f);
            animation->setY(static_cast<float>(getISafeZoneTop() + 15 + yOffset));
        }
        else
        {
            animation->setX(-(animation->getWidth() - getStage()->getStageWidth()));
            animation->setY(-(animation->getHeight() - getStage()->getStageHeight()));
        }
        if (movie != 0)
        {
            movie->gotoAndPlay(2);
        }
    }
}

void FJ_PressStartToPlay::ShowBetaWatermark()
{
    FJ_FuiNode* watermark = getChildByName(std::string("BETAWatermark"));
    if (watermark != 0)
    {
        FJ_FuiNode* movie = FJ_FuiNode::asMovieClip(watermark);
        if (movie != 0)
        {
            movie->setVisible(true);
        }
        watermark->setX(getStage()->getStageWidth() - watermark->getWidth() - static_cast<float>(getISafeZoneRight()) - 15.0f);
        watermark->setY(static_cast<float>(getISafeZoneTop() + 15));
    }
}
