#include "game/ui/FJ_HowToPlay.hpp"

#include <cmath>
#include <cstdio>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_DynamicHtmlText.hpp"
#include "game/ui/FJ_ExperienceBar.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_LabelOutline.hpp"
#include "game/ui/FJ_ProgressBar.hpp"
#include "game/ui/FJ_SlotList.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"

namespace
{
    FJ_FuiNode* child(FJ_FuiNode* node, const char* format, int index)
    {
        char name[64];
        std::sprintf(name, format, index);
        return node->getChildByName(name);
    }

    FJ_FuiNode* child(FJ_FuiNode* node, const char* format, int first, int second)
    {
        char name[64];
        std::sprintf(name, format, first, second);
        return node->getChildByName(name);
    }

    void setVisible(FJ_FuiNode* node, const char* format, int index, bool visible)
    {
        FJ_FuiNode* item = child(node, format, index);
        if (item != NULL)
            item->setVisible(visible);
    }

    void setVisible(FJ_FuiNode* node, const char* format, int first, int second, bool visible)
    {
        FJ_FuiNode* item = child(node, format, first, second);
        if (item != NULL)
            item->setVisible(visible);
    }

    bool setLabelVisible(FJ_FuiNode* node, int label, int page, bool visible)
    {
        FJ_FuiNode* item = child(node, "Label%d_%d", label, page);
        FJ_Label* labelNode = item == NULL ? NULL : item->asFJ_Label();
        if (labelNode == NULL)
            return false;
        labelNode->setVisible(visible);
        return true;
    }

    void setEditText(FJ_FuiNode* node, const char* name, const std::wstring& text)
    {
        FJ_FuiNode* item = node->getChildByName(name);
        if (item == NULL)
            return;
        FJ_FuiNode* childNode = item->getChildAt(0);
        if (childNode == NULL)
            return;
        fuiRenderNode* renderNode = childNode->getRenderNodeChildAt(0);
        if (renderNode == NULL)
            return;
        fuiRenderNodeEditText* editText = static_cast<fuiRenderNodeEditText*>(renderNode->asEditText());
        if (editText != NULL)
            editText->setText(text);
    }
}

bool LoadHowToPlayPage(FJ_HowToPlay* pNode, int iPage, std::vector<std::wstring>& pages)
{
    if (iPage > 0x1c || (pNode->m_page == iPage && !pNode->m_initialized))
        return false;

    pNode->m_initialized = false;
    if (pNode->m_page == -1)
    {
        for (int i = 0; i < 0x1d; ++i)
        {
            setVisible(pNode, "HowToPlayText_%d", i, false);
            setVisible(pNode, "HowToPlayImage_%d", i, false);
            setVisible(pNode, "HowToPlayContainer_%d", i, false);

            for (int label = 1; label <= 4; ++label)
            {
                if (!setLabelVisible(pNode, label, i, false))
                    break;
            }

            FJ_FuiNode* pageContainer = child(pNode, "HowToPlayContainer_%d", i);
            FJ_FuiNode* slotParent = pageContainer == NULL ? pNode : pageContainer;
            for (int list = 0; list < 7; ++list)
            {
                FJ_FuiNode* slotNode = child(slotParent, "HowToPlaySlotList_%d_%d", list, i);
                FJ_SlotList* slots = slotNode == NULL ? NULL : slotNode->asFJ_SlotList();
                if (slots == NULL)
                    break;
                slots->setVisible(false);
            }
        }
    }
    else
    {
        setVisible(pNode, "HowToPlayImage_%d", pNode->m_page, false);
        setVisible(pNode, "HowToPlayContainer_%d", pNode->m_page, false);
        for (int label = 1; label <= 4; ++label)
        {
            if (!setLabelVisible(pNode, label, pNode->m_page, false))
                break;
        }

        FJ_FuiNode* oldContainer = child(pNode, "HowToPlayContainer_%d", pNode->m_page);
        FJ_FuiNode* oldSlotParent = oldContainer == NULL ? pNode : oldContainer;
        for (int list = 0; list < 7; ++list)
        {
            FJ_FuiNode* slotNode = child(oldSlotParent, "HowToPlaySlotList_%d_%d", list, pNode->m_page);
            FJ_SlotList* slots = slotNode == NULL ? NULL : slotNode->asFJ_SlotList();
            if (slots == NULL)
                break;
            slots->Clear();
            slots->setVisible(false);
        }
    }

    FJ_FuiNode* textNode = child(pNode, "HowToPlayText_%d", iPage);
    FJ_DynamicHtmlText* dynamicText = pNode->m_dynamicHtmlText == NULL
        ? NULL : pNode->m_dynamicHtmlText->asFJ_DynamicHtmlText();
    if (textNode != NULL && dynamicText != NULL)
    {
        dynamicText->setX(textNode->getX());
        dynamicText->setY(textNode->getY());
        dynamicText->setWidth(textNode->getWidth());
        dynamicText->setHeight(textNode->getHeight());
        dynamicText->Reload(static_cast<float>(std::round(textNode->getWidth())),
                            static_cast<float>(std::round(textNode->getHeight())));
        for (std::vector<std::wstring>::const_iterator it = pages.begin(); it != pages.end(); ++it)
            dynamicText->AddText(*it, it + 1 == pages.end());
        FJ_FuiNodeStage* stage = pNode->getStage();
        if (stage != NULL)
            stage->setFocus(dynamicText);
    }

    FJ_FuiNode* image = child(pNode, "HowToPlayImage_%d", iPage);
    if (image != NULL)
        image->setVisible(true);
    FJ_FuiNode* container = child(pNode, "HowToPlayContainer_%d", iPage);
    if (container != NULL)
        container->setVisible(true);
    FJ_FuiNode* pageParent = container == NULL ? pNode : container;

    for (int label = 1; label <= 4; ++label)
    {
        if (!setLabelVisible(pNode, label, iPage, true))
            break;
    }

    int slot = 0;
    for (int list = 0; list < 7; ++list)
    {
        FJ_FuiNode* slotNode = child(pageParent, "HowToPlaySlotList_%d_%d", list, iPage);
        FJ_SlotList* slots = slotNode == NULL ? NULL : slotNode->asFJ_SlotList();
        if (slots == NULL)
            break;
        slots->setVisible(true);
        slots->Init();
        int count = slots->getRows() * slots->getColumns();
        for (int i = 0; i < count; ++i)
            slots->addSlot(slot++, false, false);
    }

    if (iPage == 3)
    {
        FJ_FuiNode* level = pageParent->getChildByName("PlayerLevel");
        FJ_LabelOutline* outline = level == NULL ? NULL : level->asFJ_LabelOutline();
        if (outline != NULL)
            outline->SetLabel(L"3");
        FJ_FuiNode* experienceNode = pageParent->getChildByName("ExpBar");
        FJ_ExperienceBar* experience = experienceNode == NULL ? NULL : experienceNode->asFJ_ExperienceBar();
        if (experience != NULL)
            experience->setProgress(0.75f);
        pNode->m_page = iPage;
        return true;
    }
    if (iPage == 10)
    {
        FJ_FuiNode* fireNode = pageParent->getChildByName("FurnaceFire");
        FJ_ProgressBar* fire = fireNode == NULL ? NULL : fireNode->asFJ_ProgressBar();
        if (fire != NULL)
        {
            fire->setProgress(0.5f);
            fire->setProgress(0.5f);
        }
        pNode->m_page = iPage;
        return true;
    }
    if (iPage == 12)
    {
        FJ_FuiNode* bubblesNode = pageParent->getChildByName("BrewingBubbles");
        FJ_FuiNode* arrowNode = pageParent->getChildByName("BrewingArrow");
        FJ_FuiNode* plateNode = pageParent->getChildByName("BrewingPlateBar");
        FJ_ProgressBar* bubbles = bubblesNode == NULL ? NULL : bubblesNode->asFJ_ProgressBar();
        FJ_ProgressBar* arrow = arrowNode == NULL ? NULL : arrowNode->asFJ_ProgressBar();
        FJ_ProgressBar* plate = plateNode == NULL ? NULL : plateNode->asFJ_ProgressBar();
        if (bubbles != NULL)
            bubbles->setProgress(0.5f);
        if (arrow != NULL)
            arrow->setProgress(0.5f);
        if (plate != NULL)
            plate->setProgress(0.7f);
        pNode->m_page = iPage;
        return true;
    }
    if (iPage == 13)
    {
        setEditText(pageParent, "EnchantmentLabel01", L"3");
        setEditText(pageParent, "EnchantmentLabel02", L"5");
        setEditText(pageParent, "EnchantmentLabel03", L"8");
    }
    else if (iPage == 0x15)
        setEditText(pageParent, "AnvilTextInput", L"Excalibur");
    else if (iPage == 0x16)
    {
        FJ_FuiNode* item = child(pageParent, "HowToPlaySlotList_%d_%d", 2, 22);
        FJ_SlotList* slots = item == NULL ? NULL : item->asFJ_SlotList();
        if (slots != NULL)
        {
            slots->SetSlotRedBox(0, true);
            slots->SetSlotRedBox(1, true);
            slots->SetSlotRedBox(4, true);
        }
    }

    pNode->m_page = iPage;
    return true;
}

FJ_HowToPlay::FJ_HowToPlay(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode), m_page(0), m_initialized(false), m_dynamicHtmlText(NULL)
{
    m_pages.clear();
    InitScene();
}

FJ_HowToPlay::~FJ_HowToPlay()
{
    m_dynamicHtmlText = NULL;
    m_initialized = false;
    m_page = 0;
}

void FJ_HowToPlay::_init(FJ_FuiNode* pNode, FJ_Event* pEvent)
{
}

void FJ_HowToPlay::init(FJ_Event* pEvent)
{
}

void FJ_HowToPlay::InitScene()
{
    addEventListener(eFJEventType_Init, this, _init, false);
    m_page = -1;
    m_dynamicHtmlTextName.assign("DynamicHtmlText");
    m_dynamicHtmlText = getChildByName(m_dynamicHtmlTextName);
    m_dynamicHtmlText->setX(0.0f);
    m_pages.clear();
    m_pages.push_back(L"");
    LoadHowToPlayPage(this, 0, m_pages);
    m_initialized = true;
}

FJ_HowToPlay* FJ_HowToPlay::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_HowToPlay(pRenderNode);
}

unsigned int FJ_HowToPlay::getType()
{
    return 0x58;
}

void FJ_HowToPlay::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_FuiNodeStage* stage = getStage();
    double stageHeight = stage->getStageHeight();
    if (stageHeight != 360.0 && stageHeight != 540.0)
        return;
    setX(left);
    setY(top);
}
