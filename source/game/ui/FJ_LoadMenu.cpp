#include "game/ui/FJ_LoadMenu.hpp"

#include <string>

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_HtmlText.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/FJ_List_MultiList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

static FJ_FuiNode* child(FJ_FuiNode* node, const char* name)
{
    return node->getChildByName(std::string(name));
}

static void hideAndRemove(FJ_FuiNode* parent, const char* name)
{
    FJ_FuiNode* node = child(parent, name);
    node->setVisible(false);
    parent->removeChild(node, true);
}

FJ_LoadMenu::FJ_LoadMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode), m_listHeight(0), m_customLoadMenu(false), m_iconY(0)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);
}

FJ_LoadMenu::~FJ_LoadMenu()
{
}

void FJ_LoadMenu::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
    {
        m_pCallbackScene->handleCheckboxToggled(checkBox->getId(), checkBox->getChecked());
    }
}

void FJ_LoadMenu::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_LoadMenu*>(node)->handleCheckboxToggled(event);
}

void FJ_LoadMenu::init(FJ_Event* event)
{
    m_root = getChildByName(std::string("MainPanel"));
    FJ_Document::init(event);
}

void FJ_LoadMenu::InitCustomLoadMenu()
{
    m_customLoadMenu = true;
    FJ_FuiNode* mainPanel = child(this, "MainPanel");
    FJ_FuiNode* multiList = child(mainPanel, "MultiList");
    m_listHeight = static_cast<int>(multiList->getHeight());

    hideAndRemove(mainPanel, "Seed");
    hideAndRemove(mainPanel, "GameModeToggle");
    hideAndRemove(mainPanel, "Difficulty");
    hideAndRemove(mainPanel, "TexturePackSelector");
    hideAndRemove(mainPanel, "TexturePackSelectorBackground");
    hideAndRemove(mainPanel, "CheckboxOnline");
    hideAndRemove(mainPanel, "MoreOptions");
    hideAndRemove(mainPanel, "LoadSettings");
    FJ_FuiNode* icon = child(mainPanel, "Icon");
    icon->setVisible(false);
    m_iconY = static_cast<int>(icon->getY());
    mainPanel->removeChild(icon, true);
    hideAndRemove(mainPanel, "TexturePackName");

    multiList->setTabIndex(1);
    SetFocus(-1);
}

void FJ_LoadMenu::RescaleCustomLoadMenu(bool comparison)
{
    FJ_FuiNode* mainPanel = child(this, "MainPanel");
    FJ_FuiNode* comparisonPic = child(mainPanel, "ComparisonPic");
    FJ_FuiNode* levelIcon = child(mainPanel, "LevelIcon");
    FJ_List_MultiList* multiList = static_cast<FJ_List_MultiList*>(child(mainPanel, "MultiList")->asFJ_List_MultiList());
    FJ_FuiNode* descriptionNode = child(mainPanel, "TexturePackDescription");
    FJ_HtmlText* description = static_cast<FJ_HtmlText*>(descriptionNode->asFJ_HtmlText());
    int listHeight = multiList->GetListHeight();
    int availableHeight = m_listHeight - listHeight;
    float reclaimedHeight = static_cast<float>(availableHeight);
    float listOffset = reclaimedHeight * 0.5f;
    int halfOffset = Math::round(listOffset);
    int descriptionOffset = static_cast<int>(comparisonPic->getY() - halfOffset - (m_iconY + halfOffset) - m_iconY);
    float descriptionHeight = static_cast<float>(description->GetRealHeight());
    float descriptionY = descriptionNode->getY();
    float comparisonY = comparisonPic->getY();
    descriptionHeight = static_cast<float>(static_cast<int>(descriptionHeight + comparisonY - (descriptionY + descriptionHeight)));
    descriptionHeight += levelIcon->getHeight() - reclaimedHeight;
    descriptionNode->setHeight(descriptionHeight);

    if (description->GetNumberOfLines() < 3)
    {
        float textY = comparisonPic->getY();
        float textHeight = comparisonPic->getHeight();
        float listY = levelIcon->getY();
        int optimalHeight = description->GetOptimalHeight(-1);
        float textBottom = static_cast<float>(optimalHeight) + comparisonPic->getHeight();
        float adjustment = static_cast<float>(static_cast<int>(textY + textHeight - listY) - static_cast<int>(textBottom));
        reclaimedHeight = adjustment;
        child(mainPanel, "MultiList")->incY(static_cast<float>(m_listHeight - listHeight) - adjustment);
        listOffset = adjustment * 0.5f;
    }
    else
    {
        int optimalHeight = description->GetOptimalHeight(-1);
        if (descriptionOffset < optimalHeight)
        {
            availableHeight -= optimalHeight - descriptionOffset;
            reclaimedHeight = static_cast<float>(availableHeight);
            child(mainPanel, "MultiList")->incY(static_cast<float>(m_listHeight - listHeight) - static_cast<float>(availableHeight));
            listOffset = static_cast<float>(availableHeight) * 0.5f;
        }
    }

    child(mainPanel, "Background")->getChildAt(0)->decHeight(reclaimedHeight);
    int roundedOffset = Math::round(listOffset);
    child(mainPanel, "Background")->incY(static_cast<float>(roundedOffset));
    child(mainPanel, "GameName")->incY(static_cast<float>(roundedOffset));
    child(mainPanel, "CreatedMode")->incY(static_cast<float>(roundedOffset));
    child(mainPanel, "LevelIcon")->incY(static_cast<float>(roundedOffset));
    child(mainPanel, "LevelIconSlot")->incY(static_cast<float>(roundedOffset));
    child(mainPanel, "MultiList")->incY(static_cast<float>(roundedOffset));
    child(mainPanel, "TextPanel")->getChildAt(0)->decHeight(reclaimedHeight);
    child(mainPanel, "TextPanel")->incY(static_cast<float>(roundedOffset));
    descriptionNode->setY(static_cast<float>(m_iconY + roundedOffset));

    if (!comparison)
    {
        child(mainPanel, "ComparisonPic")->decY(static_cast<float>(roundedOffset));
        child(mainPanel, "ComparisonPicBG")->decY(static_cast<float>(roundedOffset));
        descriptionNode->setScaleY(1.0f);
        descriptionNode->setHeight(static_cast<float>(description->GetOptimalHeight(-1)));
    }
    else
    {
        descriptionNode->setScaleY(1.0f);
        FJ_FuiNode* panelItem = child(mainPanel, "TextPanel")->getChildAt(0);
        descriptionNode->setHeight(panelItem->getHeight() - static_cast<float>(m_iconY << 1));
        hideAndRemove(mainPanel, "ComparisonPic");
        hideAndRemove(mainPanel, "ComparisonPicBG");
    }
}

FJ_LoadMenu* FJ_LoadMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_LoadMenu(renderNode);
}

unsigned int FJ_LoadMenu::getType()
{
    return 2;
}
