#include "game/ui/FJ_MultilistMenu.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/FJ_List_MultiList.hpp"
#include "game/util/Math.hpp"

FJ_MultilistMenu::FJ_MultilistMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);
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

    float listHeight = static_cast<float>(multiList->asFJ_List_MultiList()->GetListHeight());
    if (listHeight < multiList->getHeight())
    {
        float oldChildHeight = backgroundList->getChildAt(0)->getHeight();
        backgroundList->getChildAt(0)->setHeight(
            listHeight + 2.0f * (multiList->getY() - backgroundList->getY()));

        float newChildHeight = backgroundList->getChildAt(0)->getHeight();
        backgroundText->getChildAt(0)->setHeight(
            newChildHeight - 2.0f * (backgroundText->getY() - backgroundList->getY()));

        if (getStage()->getStageHeight() != 360.0f && getStage()->getStageHeight() != 540.0f)
        {
            float thirdHeight = oldChildHeight / 3.0f;
            float latestChildHeight = backgroundList->getChildAt(0)->getHeight();
            int rounded = Math::round(thirdHeight - latestChildHeight * 0.5f);
            int safeDelta = rounded;
            int safeBottom = getISafeZoneBottom();
            if (safeBottom < safeDelta)
                safeDelta -= safeBottom;
            float delta = static_cast<float>(safeDelta);
            if (delta > 0.0f)
            {
                multiList->incY(delta);
                backgroundList->incY(delta);
                description->incY(delta);
                backgroundText->incY(delta);

                float bgTextHeight = backgroundText->getHeight();
                description->setHeight(
                    bgTextHeight - 2.0f * (description->getY() - backgroundText->getY()));
            }
        }
    }

    if (getStage()->getStageHeight() == 360.0f || getStage()->getStageHeight() == 540.0f)
    {
        float stageHeight = getStage()->getStageHeight();
        float childHeight = backgroundList->getChildAt(0)->getHeight();
        int rounded = Math::round((stageHeight - childHeight) * 0.5f);
        int safeDelta = rounded;
        int safeBottom = getISafeZoneBottom();
        if (safeBottom < safeDelta)
            safeDelta -= safeBottom;
        float delta = static_cast<float>(safeDelta);

        multiList->incY(delta);
        backgroundList->incY(delta);
        description->incY(delta);
        backgroundText->incY(delta);

        float bgTextHeight = backgroundText->getHeight();
        description->setHeight(
            bgTextHeight - 2.0f * (description->getY() - backgroundText->getY()));
    }
}

void FJ_MultilistMenu::DoHorizontalResizeCheck()
{
    FJ_FuiNode* multiList = getChildByName("MultiList");
    FJ_FuiNode* backgroundList = getChildByName("BackgroundPanel.List");
    FJ_FuiNode* backgroundText = getChildByName("BackgroundPanel.Text");
    FJ_FuiNode* description = getChildByName("Description");

    FJ_List_MultiList* list = multiList->asFJ_List_MultiList();
    float listWidth = static_cast<float>(list->GetListWidth());
    if (multiList->getWidth() < listWidth)
    {
        listWidth = static_cast<float>(list->GetListWidth());
        float widthDelta = listWidth - multiList->getWidth();

        backgroundList->getChildAt(0)->incWidth(widthDelta);
        multiList->incWidth(widthDelta);

        int itemIndex = 0;
        int itemCount = list->getINumButtons();
        while (itemIndex < itemCount)
        {
            std::wstring savedLabel = list->GetItemLabel(itemIndex);
            list->ReturnItem(itemIndex)->asFJ_Base()->SetLabel(std::wstring());
            float itemWidth = list->ReturnItem(itemIndex)->getWidth();
            list->ReturnItem(itemIndex)->asFJ_Base()->SetNewObjectWidth(
                static_cast<int>(itemWidth + widthDelta));
            list->ReturnItem(itemIndex)->asFJ_Base()->SetLabel(savedLabel);
            ++itemIndex;
            itemCount = list->getINumButtons();
        }

        float offset = static_cast<float>(Math::round(widthDelta * 0.5f));
        multiList->decX(offset);
        backgroundList->decX(offset);
        if (description->isVisible())
        {
            description->incX(offset);
            backgroundText->incX(offset);
        }
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
