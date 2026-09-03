#include "game/ui/FJ_FuiNode.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeStage.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"

FJ_FuiNode::FJ_FuiNode(fuiRenderNode* pRenderNode)
{
    m_pRenderNode = pRenderNode;
    m_pCallbackScene = NULL;
    m_iTabIndex = -1;
}

FJ_FuiNode* FJ_FuiNode::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_FuiNode(pRenderNode);
}

FJ_FuiNodeStage* FJ_FuiNode::getStage()
{
    return (FJ_FuiNodeStage*)m_pRenderNode->getStage()->getFuiNode();
}

FJ_FuiNode::~FJ_FuiNode()
{
    g_pFui->removeEventListenerForNode(this);

    if (m_pRenderNode != NULL)
    {
        if (m_pRenderNode->getStage() != NULL)
            getStage()->removeFocus(this);
    }
}

void FJ_FuiNode::ASConstructor()
{
}

float FJ_FuiNode::getX()
{
    return m_pRenderNode->getX();
}

float FJ_FuiNode::getY()
{
    return m_pRenderNode->getY();
}

float FJ_FuiNode::getWidth()
{
    return m_pRenderNode->getWidth();
}

float FJ_FuiNode::getHeight()
{
    return m_pRenderNode->getHeight();
}

void FJ_FuiNode::setScaleX(float fScaleX)
{
    m_pRenderNode->setScaleX(fScaleX);
}

void FJ_FuiNode::setScaleY(float fScaleY)
{
    m_pRenderNode->setScaleY(fScaleY);
}

float FJ_FuiNode::getScaleX()
{
    return m_pRenderNode->getScaleX();
}

float FJ_FuiNode::getScaleY()
{
    return m_pRenderNode->getScaleY();
}

const std::string& FJ_FuiNode::getName()
{
    return m_pRenderNode->getName();
}

fuiRect FJ_FuiNode::getRect(FJ_FuiNode* pNode)
{
    return m_pRenderNode->getBounds();
}

int FJ_FuiNode::getTabIndex()
{
    return m_iTabIndex;
}

void FJ_FuiNode::setScrollRect(fuiRect& rect)
{
    m_pRenderNode->setScrollRect(rect);
}

int FJ_FuiNode::getNumChildren()
{
    if (m_pRenderNode == NULL)
        return 0;

    fuiRenderNodeTimeline* pTimeline = m_pRenderNode->asTimeline();
    if (pTimeline == NULL)
        return 0;

    return pTimeline->getNumChildren();
}

FJ_FuiNode* FJ_FuiNode::getChildByName(const std::string& name)
{
    fuiRenderNodeTimeline* pTimeline = m_pRenderNode->asTimeline();
    if (pTimeline != NULL)
    {
        fuiRenderNode* pChild = pTimeline->findNode(name.c_str());
        if (pChild != NULL)
            return pChild->getFuiNode();
    }

    return NULL;
}

fuiRenderNode* FJ_FuiNode::getRenderNodeChildByName(const std::string& name)
{
    fuiRenderNodeTimeline* pTimeline = m_pRenderNode->asTimeline();
    if (pTimeline != NULL)
    {
        fuiRenderNode* pChild = pTimeline->findNode(name.c_str());
        if (pChild != NULL)
            return pChild;
    }

    return NULL;
}

fuiRenderNode* FJ_FuiNode::getRenderNodeChildAt(int iIndex)
{
    fuiRenderNodeTimeline* pTimeline = m_pRenderNode->asTimeline();
    if (pTimeline == NULL)
        return NULL;

    return pTimeline->getChildAt(iIndex);
}

fuiRenderNode* FJ_FuiNode::getRenderNodeParent()
{
    return m_pRenderNode->getParent();
}

FJ_FuiNode* FJ_FuiNode::getChildAt(int iIndex)
{
    fuiRenderNodeTimeline* pTimeline = m_pRenderNode->asTimeline();
    if (pTimeline == NULL)
        return NULL;

    return pTimeline->getChildAt(iIndex)->getFuiNode();
}

FJ_FuiNode* FJ_FuiNode::getParent()
{
    if (m_pRenderNode->getParent() == NULL)
        return NULL;

    return m_pRenderNode->getParent()->getFuiNode();
}

void FJ_FuiNode::removeChild(FJ_FuiNode* pNode, bool bDestroy)
{
    m_pRenderNode->asTimeline()->removeChild(pNode->m_pRenderNode, bDestroy);
}

void FJ_FuiNode::removeChild(fuiRenderNode* pRenderNode, bool bDestroy)
{
    m_pRenderNode->asTimeline()->removeChild(pRenderNode, bDestroy);
}

void FJ_FuiNode::removeChildAt(int iIndex, bool bDestroy)
{
    m_pRenderNode->asTimeline()->removeChildAt(iIndex, bDestroy);
}

void FJ_FuiNode::addMask(FJ_FuiNode* pNode)
{
    m_pRenderNode->asTimeline()->addMask(pNode->m_pRenderNode);
}

void FJ_FuiNode::addChild(FJ_FuiNode* pNode)
{
    m_pRenderNode->asTimeline()->addChild(pNode->m_pRenderNode);
}

void FJ_FuiNode::addChildAt(FJ_FuiNode* pNode, int iIndex)
{
    m_pRenderNode->asTimeline()->addChildAt(pNode->m_pRenderNode, iIndex);
}

void FJ_FuiNode::addChild(fuiRenderNode* pRenderNode)
{
    m_pRenderNode->asTimeline()->addChild(pRenderNode);
}

void FJ_FuiNode::addChildAt(fuiRenderNode* pRenderNode, int iIndex)
{
    m_pRenderNode->asTimeline()->addChildAt(pRenderNode, iIndex);
}

int FJ_FuiNode::getChildIndex(FJ_FuiNode* pNode)
{
    return m_pRenderNode->asTimeline()->getChildIndex(pNode->m_pRenderNode);
}

FJ_FuiNode* FJ_FuiNode::asMovieClip()
{
    if (m_pRenderNode->asTimeline() != NULL)
        return this;

    return NULL;
}

FJ_FuiNode* FJ_FuiNode::asMovieClip(FJ_FuiNode* pNode)
{
    if (pNode != NULL)
        return pNode->asMovieClip();

    return NULL;
}

FJ_Label* FJ_FuiNode::asFJ_Label()
{
    return dynamic_cast<FJ_Label*>(this);
}

FJ_Base* FJ_FuiNode::asFJ_Base()
{
    return dynamic_cast<FJ_Base*>(this);
}

bool FJ_FuiNode::isFJ_Base()
{
    return asFJ_Base() != NULL;
}

FJ_Button* FJ_FuiNode::asFJ_Button()
{
    return dynamic_cast<FJ_Button*>(this);
}

bool FJ_FuiNode::isFJ_Button()
{
    return asFJ_Button() != NULL;
}

FJ_MenuButton* FJ_FuiNode::asFJ_MenuButton()
{
    return dynamic_cast<FJ_MenuButton*>(this);
}

bool FJ_FuiNode::isFJ_Label()
{
    return asFJ_Label() != NULL;
}

FJ_LabelOutline* FJ_FuiNode::asFJ_LabelOutline()
{
    return dynamic_cast<FJ_LabelOutline*>(this);
}

bool FJ_FuiNode::isFJ_LabelOutline()
{
    return asFJ_LabelOutline() != NULL;
}

FJ_Label_HUD_Splash* FJ_FuiNode::asFJ_Label_HUD_Splash()
{
    return dynamic_cast<FJ_Label_HUD_Splash*>(this);
}

bool FJ_FuiNode::isFJ_Label_HUD_Splash()
{
    return asFJ_Label_HUD_Splash() != NULL;
}

FJ_Label_HUD_SplashBig* FJ_FuiNode::asFJ_Label_HUD_SplashBig()
{
    return dynamic_cast<FJ_Label_HUD_SplashBig*>(this);
}

bool FJ_FuiNode::isFJ_Label_HUD_SplashBig()
{
    return asFJ_Label_HUD_SplashBig() != NULL;
}

FJ_Label_HUD_SplashVita* FJ_FuiNode::asFJ_Label_HUD_SplashVita()
{
    return dynamic_cast<FJ_Label_HUD_SplashVita*>(this);
}

FJ_Label_Blink* FJ_FuiNode::asFJ_Label_Blink()
{
    return dynamic_cast<FJ_Label_Blink*>(this);
}

bool FJ_FuiNode::isFJ_Label_Blink()
{
    return asFJ_Label_Blink() != NULL;
}

FJ_List* FJ_FuiNode::asFJ_List()
{
    return dynamic_cast<FJ_List*>(this);
}

bool FJ_FuiNode::isFJ_List()
{
    return asFJ_List() != NULL;
}

FJ_List2D* FJ_FuiNode::asFJ_List2D()
{
    return dynamic_cast<FJ_List2D*>(this);
}

bool FJ_FuiNode::isFJ_List2D()
{
    return asFJ_List2D() != NULL;
}

FJ_List_MenuButton* FJ_FuiNode::asFJ_List_MenuButton()
{
    return dynamic_cast<FJ_List_MenuButton*>(this);
}

FJ_ListButton* FJ_FuiNode::asFJ_ListButton()
{
    return dynamic_cast<FJ_ListButton*>(this);
}

bool FJ_FuiNode::isFJ_ListButton()
{
    return asFJ_ListButton() != NULL;
}

FJ_ListButtonController* FJ_FuiNode::asFJ_ListButtonController()
{
    return dynamic_cast<FJ_ListButtonController*>(this);
}

bool FJ_FuiNode::isFJ_ListButtonController()
{
    return asFJ_ListButtonController() != NULL;
}

FJ_List_ListButton* FJ_FuiNode::asFJ_List_ListButton()
{
    return dynamic_cast<FJ_List_ListButton*>(this);
}

bool FJ_FuiNode::isFJ_List_ListButton()
{
    return asFJ_List_ListButton() != NULL;
}

FJ_List_ListButtonPlayerList* FJ_FuiNode::asFJ_List_ListButtonPlayerList()
{
    return dynamic_cast<FJ_List_ListButtonPlayerList*>(this);
}

FJ_List_ListButtonScoreBoard* FJ_FuiNode::asFJ_List_ListButtonScoreBoard()
{
    return dynamic_cast<FJ_List_ListButtonScoreBoard*>(this);
}

FJ_HtmlText* FJ_FuiNode::asFJ_HtmlText()
{
    return dynamic_cast<FJ_HtmlText*>(this);
}

bool FJ_FuiNode::isFJ_HtmlText()
{
    return asFJ_HtmlText() != NULL;
}

FJ_DynamicHtmlText* FJ_FuiNode::asFJ_DynamicHtmlText()
{
    return dynamic_cast<FJ_DynamicHtmlText*>(this);
}

bool FJ_FuiNode::isFJ_DynamicHtmlText()
{
    return asFJ_DynamicHtmlText() != NULL;
}

FJ_HtmlText_WithPanel* FJ_FuiNode::asFJ_HtmlText_WithPanel()
{
    return dynamic_cast<FJ_HtmlText_WithPanel*>(this);
}

bool FJ_FuiNode::isFJ_HtmlText_WithPanel()
{
    return asFJ_HtmlText_WithPanel() != NULL;
}

FJ_Slider* FJ_FuiNode::asFJ_Slider()
{
    return dynamic_cast<FJ_Slider*>(this);
}

bool FJ_FuiNode::isFJ_Slider()
{
    return asFJ_Slider() != NULL;
}

FJ_CheckBox* FJ_FuiNode::asFJ_CheckBox()
{
    return dynamic_cast<FJ_CheckBox*>(this);
}

bool FJ_FuiNode::isFJ_CheckBox()
{
    return asFJ_CheckBox() != NULL;
}

FJ_TextInput* FJ_FuiNode::asFJ_TextInput()
{
    return dynamic_cast<FJ_TextInput*>(this);
}

bool FJ_FuiNode::isFJ_TextInput()
{
    return asFJ_TextInput() != NULL;
}

FJ_HorseJumpBar* FJ_FuiNode::asFJ_HorseJumpBar()
{
    return dynamic_cast<FJ_HorseJumpBar*>(this);
}

FJ_ExperienceBar* FJ_FuiNode::asFJ_ExperienceBar()
{
    return dynamic_cast<FJ_ExperienceBar*>(this);
}

FJ_Cursor* FJ_FuiNode::asFJ_Cursor()
{
    return dynamic_cast<FJ_Cursor*>(this);
}

bool FJ_FuiNode::isFJ_Cursor()
{
    return asFJ_Cursor() != NULL;
}

FJ_SlotList* FJ_FuiNode::asFJ_SlotList()
{
    return dynamic_cast<FJ_SlotList*>(this);
}

FJ_ScrollArrow* FJ_FuiNode::asFJ_ScrollArrow()
{
    return dynamic_cast<FJ_ScrollArrow*>(this);
}

FJ_MobEffect* FJ_FuiNode::asFJ_MobEffect()
{
    return dynamic_cast<FJ_MobEffect*>(this);
}

FJ_ProgressBar* FJ_FuiNode::asFJ_ProgressBar()
{
    return dynamic_cast<FJ_ProgressBar*>(this);
}

FJ_BitmapIcon* FJ_FuiNode::asFJ_BitmapIcon()
{
    return dynamic_cast<FJ_BitmapIcon*>(this);
}

FJ_IconSlot* FJ_FuiNode::asFJ_IconSlot()
{
    return dynamic_cast<FJ_IconSlot*>(this);
}

FJ_Label_Book* FJ_FuiNode::asFJ_Label_Book()
{
    return dynamic_cast<FJ_Label_Book*>(this);
}

bool FJ_FuiNode::isFJ_Label_Book()
{
    return asFJ_Label_Book() != NULL;
}

FJ_List_MultiList* FJ_FuiNode::asFJ_List_MultiList()
{
    return dynamic_cast<FJ_List_MultiList*>(this);
}

FJ_Document* FJ_FuiNode::asFJ_Document()
{
    return dynamic_cast<FJ_Document*>(this);
}

bool FJ_FuiNode::isFJ_Document()
{
    return asFJ_Document() != NULL;
}

FJ_HtmlTextPanel* FJ_FuiNode::asFJ_HtmlTextPanel()
{
    return dynamic_cast<FJ_HtmlTextPanel*>(this);
}

bool FJ_FuiNode::isFJ_HtmlTextPanel()
{
    return asFJ_HtmlTextPanel() != NULL;
}

FJ_HtmlTextPanelLarge* FJ_FuiNode::asFJ_HtmlTextPanelLarge()
{
    return dynamic_cast<FJ_HtmlTextPanelLarge*>(this);
}

bool FJ_FuiNode::isFJ_HtmlTextPanelLarge()
{
    return asFJ_HtmlTextPanelLarge() != NULL;
}

FJ_ListButtonAchievement* FJ_FuiNode::asFJ_ListButtonAchievement()
{
    return dynamic_cast<FJ_ListButtonAchievement*>(this);
}

bool FJ_FuiNode::isFJ_ListButtonAchievement()
{
    return asFJ_ListButtonAchievement() != NULL;
}

FJ_SignInput* FJ_FuiNode::asFJ_SignInput()
{
    return dynamic_cast<FJ_SignInput*>(this);
}

bool FJ_FuiNode::isFJ_SignInput()
{
    return asFJ_SignInput() != NULL;
}

FJ_LeaderboardList* FJ_FuiNode::asFJ_LeaderboardList()
{
    return dynamic_cast<FJ_LeaderboardList*>(this);
}

bool FJ_FuiNode::isFJ_LeaderboardList()
{
    return asFJ_LeaderboardList() != NULL;
}

FJ_Hud* FJ_FuiNode::asFJ_Hud()
{
    return dynamic_cast<FJ_Hud*>(this);
}

bool FJ_FuiNode::isFJ_Hud()
{
    return asFJ_Hud() != NULL;
}

void FJ_FuiNode::gotoAndPlay(int iFrame)
{
    fuiRenderNodeTimeline* pTimeline = m_pRenderNode->asTimeline();
    if (pTimeline != NULL)
    {
        pTimeline->setFrame(iFrame);
        pTimeline->setPlaying(true);
    }
}

void FJ_FuiNode::gotoAndPlay(const std::string& sLabel)
{
    fuiRenderNodeTimeline* pTimeline = m_pRenderNode->asTimeline();
    if (pTimeline != NULL)
    {
        pTimeline->setFrame(sLabel.c_str());
        pTimeline->setPlaying(true);
    }
}

void FJ_FuiNode::gotoAndStop(int iFrame)
{
    fuiRenderNodeTimeline* pTimeline = m_pRenderNode->asTimeline();
    if (pTimeline != NULL)
    {
        pTimeline->setFrame(iFrame);
        pTimeline->setPlaying(false);
    }
}

void FJ_FuiNode::gotoAndStop(const std::string& sLabel)
{
    fuiRenderNodeTimeline* pTimeline = m_pRenderNode->asTimeline();
    if (pTimeline != NULL)
    {
        pTimeline->setFrame(sLabel.c_str());
        pTimeline->setPlaying(false);
    }
}

std::string FJ_FuiNode::currentLabel()
{
    fuiRenderNodeTimeline* pTimeline = m_pRenderNode->asTimeline();
    if (pTimeline != NULL)
        return pTimeline->currentLabel();

    return "";
}

unsigned int FJ_FuiNode::currentFrame()
{
    fuiRenderNodeTimeline* pTimeline = m_pRenderNode->asTimeline();
    if (pTimeline == NULL)
        return 1;

    return pTimeline->currentFrame();
}

void FJ_FuiNode::setX(float fX)
{
    m_pRenderNode->setX(fX);
}

void FJ_FuiNode::decX(float fAmount)
{
    setX(getX() - fAmount);
}

void FJ_FuiNode::incX(float fAmount)
{
    setX(getX() + fAmount);
}

void FJ_FuiNode::setY(float fY)
{
    m_pRenderNode->setY(fY);
}

void FJ_FuiNode::decY(float fAmount)
{
    setY(getY() - fAmount);
}

void FJ_FuiNode::incY(float fAmount)
{
    setY(getY() + fAmount);
}

void FJ_FuiNode::setHeight(float fHeight)
{
    m_pRenderNode->setHeight(fHeight);
}

void FJ_FuiNode::decHeight(float fAmount)
{
    setHeight(getHeight() - fAmount);
}

void FJ_FuiNode::incHeight(float fAmount)
{
    setHeight(getHeight() + fAmount);
}

void FJ_FuiNode::setWidth(float fWidth)
{
    m_pRenderNode->setWidth(fWidth);
}

void FJ_FuiNode::incWidth(float fAmount)
{
    setWidth(getWidth() + fAmount);
}

void FJ_FuiNode::setTabIndex(int iTabIndex)
{
    m_iTabIndex = iTabIndex;
}

void FJ_FuiNode::setVisible(bool bVisible)
{
    m_pRenderNode->setVisibility(bVisible);
}

bool FJ_FuiNode::isVisible()
{
    return m_pRenderNode->isVisible();
}

float FJ_FuiNode::getAlpha()
{
    return m_pRenderNode->getAlpha();
}

void FJ_FuiNode::setAlpha(float fAlpha)
{
    m_pRenderNode->setAlpha(fAlpha);
}

void FJ_FuiNode::incAlpha(float fAmount)
{
    setAlpha(getAlpha() + fAmount);
}

void FJ_FuiNode::eventListen(FJ_Event* pEvent, bool bCapture)
{
    for (unsigned int i = 0; i < m_eventListeners.size(); ++i)
    {
        const FJ_EventListener& listener = m_eventListeners[i];
        if (listener.m_bCapture == bCapture)
        {
            if (listener.m_eType == pEvent->m_eType)
                listener.m_pfnHandler(listener.m_pNode, pEvent);
        }
    }
}

void FJ_FuiNode::dispatchEvent(FJ_Event* pEvent)
{
    std::vector<FJ_FuiNode*> nodes;

    pEvent->m_pTarget = this;

    for (fuiRenderNode* pNode = m_pRenderNode; pNode != NULL; pNode = pNode->getParent())
    {
        if (pNode->getFuiNode() != NULL)
            nodes.push_back(pNode->getFuiNode());
    }

    if (nodes.size() != 0)
    {
        for (unsigned int i = nodes.size() - 1; i != 0; --i)
            nodes[i]->eventListen(pEvent, true);

        nodes[0]->eventListen(pEvent, false);

        if (pEvent->m_bBubbles)
        {
            for (unsigned int j = 1; j < nodes.size(); ++j)
                nodes[j]->eventListen(pEvent, false);
        }
    }

    delete pEvent;
}

FJ_Label* FJ_FuiNode::asFJ_Label(FJ_FuiNode* pNode)
{
    if (pNode != NULL)
        return pNode->asFJ_Label();
    return NULL;
}

bool FJ_FuiNode::isFJ_MenuButton()
{
    return asFJ_MenuButton() != NULL;
}

bool FJ_FuiNode::isFJ_Label_HUD_SplashVita()
{
    return asFJ_Label_HUD_SplashVita() != NULL;
}

bool FJ_FuiNode::isFJ_List_MenuButton()
{
    return asFJ_List_MenuButton() != NULL;
}

FJ_UpsellButton* FJ_FuiNode::asFJ_UpsellButton()
{
    return dynamic_cast<FJ_UpsellButton*>(this);
}

bool FJ_FuiNode::isFJ_UpsellButton()
{
    return asFJ_UpsellButton() != NULL;
}

bool FJ_FuiNode::isFJ_List_ListButtonAchievement()
{
    return asFJ_List_ListButtonAchievement() != NULL;
}

FJ_List_ListButtonAchievement* FJ_FuiNode::asFJ_List_ListButtonAchievement()
{
    return dynamic_cast<FJ_List_ListButtonAchievement*>(this);
}

bool FJ_FuiNode::isFJ_List_ListButtonPlayerList()
{
    return asFJ_List_ListButtonPlayerList() != NULL;
}

bool FJ_FuiNode::isFJ_List_ListButtonScoreBoard()
{
    return asFJ_List_ListButtonScoreBoard() != NULL;
}

FJ_DragonHealth* FJ_FuiNode::asFJ_DragonHealth()
{
    return dynamic_cast<FJ_DragonHealth*>(this);
}

bool FJ_FuiNode::isFJ_DragonHealth()
{
    return asFJ_DragonHealth() != NULL;
}

bool FJ_FuiNode::isFJ_HorseJumpBar()
{
    return asFJ_HorseJumpBar() != NULL;
}

bool FJ_FuiNode::isFJ_ExperienceBar()
{
    return asFJ_ExperienceBar() != NULL;
}

bool FJ_FuiNode::isFJ_SlotList()
{
    return asFJ_SlotList() != NULL;
}

bool FJ_FuiNode::isFJ_ScrollArrow()
{
    return asFJ_ScrollArrow() != NULL;
}

bool FJ_FuiNode::isFJ_MobEffect()
{
    return asFJ_MobEffect() != NULL;
}

FJ_MobEffect_Tile* FJ_FuiNode::asFJ_MobEffect_Tile()
{
    return dynamic_cast<FJ_MobEffect_Tile*>(this);
}

bool FJ_FuiNode::isFJ_MobEffect_Tile()
{
    return asFJ_MobEffect_Tile() != NULL;
}

bool FJ_FuiNode::isFJ_ProgressBar()
{
    return asFJ_ProgressBar() != NULL;
}

bool FJ_FuiNode::isFJ_BitmapIcon()
{
    return asFJ_BitmapIcon() != NULL;
}

bool FJ_FuiNode::isFJ_IconSlot()
{
    return asFJ_IconSlot() != NULL;
}

bool FJ_FuiNode::isFJ_List_MultiList()
{
    return asFJ_List_MultiList() != NULL;
}

void FJ_FuiNode::stop()
{
    fuiRenderNodeTimeline* pTimeline = m_pRenderNode->asTimeline();
    if (pTimeline != NULL)
        pTimeline->setPlaying(false);
}

int FJ_FuiNode::getFrameByLabel(const std::string& sLabel)
{
    fuiRenderNodeTimeline* pTimeline = m_pRenderNode->asTimeline();
    if (pTimeline == NULL)
        return 0;
    return pTimeline->getFrame(sLabel.c_str());
}

void FJ_FuiNode::decWidth(float fAmount)
{
    setWidth(getWidth() - fAmount);
}

void FJ_FuiNode::decAlpha(float fAmount)
{
    setAlpha(getAlpha() - fAmount);
}

FJ_FuiNode* FJ_FuiNode::getRoot()
{
    return m_pRenderNode->getRoot()->getFuiNode();
}

unsigned int FJ_FuiNode::getType()
{
    return 0;
}

void FJ_FuiNode::setCallbackScene(UIScene* pScene)
{
    m_pCallbackScene = pScene;
}
