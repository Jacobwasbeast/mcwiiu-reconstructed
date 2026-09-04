#include "game/ui/FJ_MultilistMenu.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"

FJ_MultilistMenu::FJ_MultilistMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
}

FJ_MultilistMenu::~FJ_MultilistMenu()
{
}

void FJ_MultilistMenu::handleCheckboxToggled(FJ_Event* event)
{
    if (event == NULL || event->m_pTarget == NULL)
        return;

    if (event->m_pTarget->asFJ_CheckBox() == NULL)
        return;

    FJ_FuiNode* parent = event->m_pTarget->getParent();
    if (parent != NULL)
        parent->dispatchEvent(event);
}

void FJ_MultilistMenu::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    if (node != NULL)
        static_cast<FJ_MultilistMenu*>(node)->handleCheckboxToggled(event);
}

FJ_Document* FJ_MultilistMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_MultilistMenu(renderNode);
}

unsigned int FJ_MultilistMenu::getType()
{
    return 42;
}

void FJ_MultilistMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    const double stageHeight = getStage()->getStageHeight();
    if (stageHeight != 360.0 && stageHeight != 540.0)
        return;

    setX(-static_cast<float>(right) - 176.0f);
}

void FJ_MultilistMenu::DoVerticalResizeCheck()
{
    FJ_FuiNode* multiList = getChildByName("MultiList");
    FJ_FuiNode* backgroundList = getChildByName("BackgroundPanel.List");
    FJ_FuiNode* backgroundText = getChildByName("BackgroundPanel.Text");
    FJ_FuiNode* description = getChildByName("Description");

    if (multiList == NULL || backgroundList == NULL || backgroundText == NULL || description == NULL)
        return;

    float listHeight = 0.0f;
    const int childCount = multiList->getNumChildren();
    int childIndex = 0;
    while (childIndex < childCount)
    {
        FJ_FuiNode* child = multiList->getChildAt(childIndex);
        if (child != NULL)
            listHeight += child->getHeight();
        ++childIndex;
    }
    if (listHeight <= multiList->getHeight())
        return;

    FJ_FuiNode* listTop = backgroundList->getChildAt(0);
    FJ_FuiNode* textTop = backgroundText->getChildAt(0);
    if (listTop != NULL)
        listTop->setHeight(listHeight);
    if (textTop != NULL)
        textTop->setHeight(listHeight);

    const double stageHeight = getStage()->getStageHeight();
    if (stageHeight != 360.0 && stageHeight != 540.0)
        return;

    FJ_FuiNode* panelTop = backgroundList->getChildAt(0);
    if (panelTop == NULL)
        return;

    const float bottom = panelTop->getY() + panelTop->getHeight();
    const int safeBottom = getISafeZoneBottom();
    const float offset = bottom > static_cast<float>(safeBottom)
        ? static_cast<float>(safeBottom) - bottom : 0.0f;
    if (offset == 0.0f)
        return;

    multiList->incY(offset);
    backgroundList->incY(offset);
    backgroundText->incY(offset);
    description->incY(offset);
}

void FJ_MultilistMenu::DoHorizontalResizeCheck()
{
    FJ_FuiNode* multiList = getChildByName("MultiList");
    FJ_FuiNode* backgroundList = getChildByName("BackgroundPanel.List");
    FJ_FuiNode* backgroundText = getChildByName("BackgroundPanel.Text");
    FJ_FuiNode* description = getChildByName("Description");

    if (multiList == NULL || backgroundList == NULL || backgroundText == NULL || description == NULL)
        return;

    float listWidth = 0.0f;
    const int childCount = multiList->getNumChildren();
    int childIndex = 0;
    while (childIndex < childCount)
    {
        FJ_FuiNode* child = multiList->getChildAt(childIndex);
        if (child != NULL && child->getWidth() > listWidth)
            listWidth = child->getWidth();
        ++childIndex;
    }
    if (listWidth <= multiList->getWidth())
        return;

    const float widthDelta = listWidth - multiList->getWidth();
    FJ_FuiNode* panel = backgroundList->getChildAt(0);
    if (panel != NULL)
        panel->incWidth(widthDelta);
    multiList->incWidth(widthDelta);

    const float offset = static_cast<float>(static_cast<int>(widthDelta * 0.5f));
    multiList->decX(offset);
    backgroundList->decX(offset);
    if (description->isVisible())
    {
        description->incX(offset);
        backgroundText->incX(offset);
    }
}

void FJ_MultilistMenu::HideDescription()
{
    FJ_FuiNode* multiList = getChildByName("MultiList");
    FJ_FuiNode* backgroundList = getChildByName("BackgroundPanel.List");
    FJ_FuiNode* backgroundText = getChildByName("BackgroundPanel.Text");
    FJ_FuiNode* description = getChildByName("Description");

    if (description != NULL)
        description->setVisible(false);
    if (backgroundText != NULL)
        backgroundText->setVisible(false);
    if (multiList == NULL || backgroundList == NULL)
        return;

    const double stageWidth = getStage()->getStageWidth();
    const float offset = static_cast<float>(static_cast<int>((stageWidth - multiList->getWidth()) * 0.5));
    multiList->setX(offset);
    backgroundList->setX(offset);
}
