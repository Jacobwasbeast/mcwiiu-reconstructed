#include "game/ui/FJ_TutorialPopup.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_HtmlText.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"
#include "game/util/Math.hpp"

FJ_TutorialPopup::FJ_TutorialPopup(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode), m_backgroundPanel(NULL), m_iconHolder(NULL), m_bitmap(NULL)
{
    m_titleY = (int)getChildByName(std::string("Title"))->getY();
    m_exitScreenShotY = (int)getChildByName(std::string("ExitScreenShot"))->getY();
    m_descriptionY = (int)getChildByName(std::string("Description"))->getY();
    m_iconHolderY = (int)getChildByName(std::string("IconHolder"))->getY();
    m_backgroundPanel = getChildByName(std::string("BackgroundPanel"))->getChildAt(0)->asMovieClip();
    m_titleHeight = (int)getChildByName(std::string("Title"))->getHeight();
}

FJ_TutorialPopup* FJ_TutorialPopup::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_TutorialPopup(pRenderNode);
}

unsigned int FJ_TutorialPopup::getType()
{
    return 90;
}

void FJ_TutorialPopup::SetSafeZone(int, int, int, int)
{
}

void FJ_TutorialPopup::ShowTitle(bool visible)
{
    getChildByName(std::string("Title"))->setVisible(visible);
}

void FJ_TutorialPopup::ShowExitScreenShot(bool visible)
{
    getChildByName(std::string("ExitScreenShot"))->setVisible(visible);
}

void FJ_TutorialPopup::ShowDescription(bool visible)
{
    getChildByName(std::string("Description"))->setVisible(visible);
}

void FJ_TutorialPopup::ShowIconHolder(bool visible)
{
    getChildByName(std::string("IconHolder"))->setVisible(visible);
}

void FJ_TutorialPopup::AdjustLayout(bool reposition)
{
    setY(0.0f);
    if (reposition)
    {
        float stageHeight = getStage()->getStageHeight();
        if (stageHeight == 360.0 || stageHeight == 540.0)
            setY((float)Math::round((float)m_titleHeight * 1.4f));
    }

    FJ_FuiNode* title = getChildByName(std::string("Title"));
    unsigned int y = 0;
    unsigned int height = 0;
    if (title->isVisible())
    {
        y = (unsigned int)m_titleY;
        height = (unsigned int)(title->getHeight() < (float)m_titleHeight ? (float)m_titleHeight : title->getHeight());
    }

    FJ_FuiNode* exitScreenShot = getChildByName(std::string("ExitScreenShot"));
    if (exitScreenShot->isVisible())
    {
        if (y == 0)
            y = (unsigned int)m_exitScreenShotY;
        exitScreenShot->setY((float)(y + height));
        height += (unsigned int)exitScreenShot->getHeight();
    }

    FJ_FuiNode* description = getChildByName(std::string("Description"));
    if (description->isVisible())
    {
        if (y == 0)
            y = (unsigned int)m_descriptionY;
        description->setY((float)(y + height));
        height += (unsigned int)static_cast<FJ_HtmlText*>(description)->getTextHeight();
    }

    FJ_FuiNode* iconHolder = getChildByName(std::string("IconHolder"));
    if (iconHolder->isVisible())
    {
        if (y == 0)
            y = (unsigned int)m_iconHolderY;
        iconHolder->setY((float)(y + height));
        height += (unsigned int)iconHolder->getHeight();
    }

    FJ_FuiNode* background = getChildByName(std::string("BackgroundPanel"));
    m_backgroundPanel->setHeight(2.0f * ((float)y - background->getY()) + (float)height);
}

fuiRenderNode* FJ_TutorialPopup::GetBitmapData(int type)
{
    const char* symbol = "icon_armour";
    switch (type)
    {
    case 2: symbol = "icon_brewing"; break;
    case 3: symbol = "icon_decoration"; break;
    case 4: symbol = "icon_food"; break;
    case 5: symbol = "icon_Materials"; break;
    case 6: symbol = "icon_mechanisms"; break;
    case 7: symbol = "icon_misc"; break;
    case 8: symbol = "icon_Redstone_and_Transport"; break;
    case 9: symbol = "icon_structures"; break;
    case 10: symbol = "icon_tools"; break;
    case 11: symbol = "icon_transport"; break;
    default: break;
    }
    return (fuiRenderNode*)gFui->createNodeFromSymbol(symbol, m_pRenderNode);
}

void FJ_TutorialPopup::SetupIconHolder(int type)
{
    FJ_FuiNode* holder = getChildByName(std::string("IconHolder"));
    FJ_FuiNode* slotControl = holder->getChildAt(0)->asMovieClip()->getChildByName(std::string("SlotControl"))->asMovieClip();
    int width = Math::round(holder->getChildAt(0)->getScaleX());

    if (m_iconHolder != NULL && m_iconHolder->getParent() != NULL)
    {
        m_iconHolder->getParent()->removeChild(m_iconHolder, true);
        m_iconHolder = NULL;
    }
    if (m_bitmap != NULL && m_bitmap->getParent() != NULL)
    {
        static_cast<fuiRenderNodeTimeline*>(m_bitmap->getParent()->asTimeline())->removeChild(m_bitmap, true);
        m_bitmap = NULL;
    }

    if (type == 0)
    {
        fuiRect rect;
        rect.xMin = (float)width;
        rect.xMax = rect.xMin + holder->getWidth() - (float)(width * 2);
        rect.yMin = (float)width;
        rect.yMax = rect.yMin + holder->getHeight() - (float)(width * 2);
        fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, "iggy_slot_IconHolder");
        renderNode->setAlpha(0.25f);
        m_iconHolder = new FJ_FuiNode(renderNode);
        renderNode->setFuiNode(m_iconHolder);
        holder->addChildAt(m_iconHolder, holder->getNumChildren());
        slotControl->setVisible(false);
        return;
    }

    m_bitmap = GetBitmapData(type);
    m_bitmap->setX((float)width);
    m_bitmap->setY((float)width);
    m_bitmap->setWidth(holder->getWidth() - (float)(width * 2));
    m_bitmap->setHeight(holder->getHeight() - (float)(width * 2));
    holder->addChildAt(m_bitmap, holder->getNumChildren());
    slotControl->setVisible(false);
}
