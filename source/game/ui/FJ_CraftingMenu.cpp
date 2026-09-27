#include "game/ui/FJ_CraftingMenu.hpp"

#include <cstdio>
#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

namespace {

FJ_FuiNode* child(FJ_FuiNode* node, const char* name) {
    if (node == 0) {
        return 0;
    }
    return node->getChildByName(std::string(name));
}

void setChildVisible(FJ_FuiNode* node, const char* name, bool visible) {
    FJ_FuiNode* item = child(node, name);
    if (item != 0) {
        item->setVisible(visible);
    }
}

std::string hTabName(int index) {
    char name[16];
    std::sprintf(name, "Tab%d", index + 1);
    return std::string(name);
}

std::string hTabIconName(int index) {
    char name[16];
    std::sprintf(name, "TabIcon%d", index + 1);
    return std::string(name);
}

std::string vTabName(int index) {
    char name[16];
    std::sprintf(name, "Tab%d", index + 1);
    return std::string(name);
}

std::string vTabIconName(int index) {
    char name[16];
    std::sprintf(name, "TabIcon%d", index + 1);
    return std::string(name);
}

const int kNormalIconFrames[7] = { 1, 2, 3, 5, 6, 7, 1 };
const int kSpecialIconFrames[7] = { 1, 2, 3, 4, 5, 6, 7 };

}

FJ_CraftingMenu::FJ_CraftingMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode),
      m_craftingType(0),
      m_horizontalTabCount(0),
      m_verticalTabCount(0),
      m_panelMode(0),
      m_panelType(0),
      m_activeTab(0),
      m_activeSubtab(0),
      m_selectorStep(0),
      m_descriptionBaseY(0) {
    int i;
    for (i = 0; i < 2; ++i) {
        m_slot2Items[i] = 0;
    }
    for (i = 0; i < 3; ++i) {
        m_slot3Items[i] = 0;
    }
    for (i = 0; i < 5; ++i) {
        m_ingredients[i] = false;
    }
}

FJ_CraftingMenu::~FJ_CraftingMenu() {
}

void FJ_CraftingMenu::handleSlideComplete(FJ_Event* event) {
    if (event != 0 && event->m_pTarget != 0) {
        event->m_pTarget->dispatchEvent(event);
    }
}

void FJ_CraftingMenu::_handleSlideComplete(FJ_FuiNode* node, FJ_Event* event) {
    if (node != 0) {
        FJ_CraftingMenu* menu = static_cast<FJ_CraftingMenu*>(node);
        menu->handleSlideComplete(event);
    }
}

FJ_Document* FJ_CraftingMenu::Create(fuiRenderNode* renderNode) {
    return new FJ_CraftingMenu(renderNode);
}

unsigned int FJ_CraftingMenu::getType() {
    return 11;
}

void FJ_CraftingMenu::showPanelDisplay(int panel) {
    setChildVisible(this, "MainPanel.Ingredient1", panel == 2 && m_ingredients[0]);
    setChildVisible(this, "MainPanel.Ingredient2", panel == 2 && m_ingredients[1]);
    setChildVisible(this, "MainPanel.Ingredient3", panel == 2 && m_ingredients[2]);
    setChildVisible(this, "MainPanel.Ingredient4", panel == 2 && m_ingredients[3]);
    FJ_FuiNode* ingredient5 = child(this, "MainPanel.Ingredient5");
    if (ingredient5 != 0) {
        ingredient5->setVisible(panel == 2 && m_ingredients[4]);
    }
    setChildVisible(this, "MainPanel.Ingredient1Desc", panel == 2 && m_ingredients[0]);
    setChildVisible(this, "MainPanel.Ingredient2Desc", panel == 2 && m_ingredients[1]);
    setChildVisible(this, "MainPanel.Ingredient3Desc", panel == 2 && m_ingredients[2]);
    setChildVisible(this, "MainPanel.Ingredient4Desc", panel == 2 && m_ingredients[3]);
    FJ_FuiNode* ingredient5Desc = child(this, "MainPanel.Ingredient5Desc");
    if (ingredient5Desc != 0) {
        ingredient5Desc->setVisible(panel == 2 && m_ingredients[4]);
    }
    if (panel != m_panelType) {
        bool showInventory = panel == 0;
        setChildVisible(this, "MainPanel.Inventory", showInventory);
        setChildVisible(this, "MainPanel.HotBar", showInventory);
        setChildVisible(this, "MainPanel.InventoryLabel", showInventory);
        setChildVisible(this, "MainPanel.DescriptionText", panel == 1);
        setChildVisible(this, "MainPanel.IngredientsLabel", panel == 2);
        setChildVisible(this, "MainPanel.PreviewLabel", panel == 3);
        setChildVisible(this, "MainPanel.PreviewSlot", panel == 3);
        m_panelType = panel;
    }
}

void FJ_CraftingMenu::ShowDescriptionPanel(bool show) {
    setChildVisible(this, "MainPanel.DescriptionPanel", show);
    setChildVisible(this, "MainPanel.DescriptionTextTop", show);
}

void FJ_CraftingMenu::ShowSuperCraftingSlots(int count) {
    bool showThree = count == 3;
    m_slot3Items[0]->setVisible(showThree);
    m_slot3Items[1]->setVisible(showThree);
    m_slot3Items[2]->setVisible(showThree);
    bool showTwo = count == 2;
    m_slot2Items[0]->setVisible(showTwo);
    m_slot2Items[1]->setVisible(showTwo);
    m_panelMode = count;
    bool active = count != 0;
    child(this, "MainPanel.SpinnerTitle")->setVisible(active);
    child(this, "MainPanel.CraftingHSlots")->setVisible(count == 0);
    child(this, "MainPanel.GroupName")->setVisible(count == 0);
    child(this, "MainPanel.CraftingSelector.ScrollArrows")->setVisible(active);
}

void FJ_CraftingMenu::MoveSelector(int horizontal, int vertical) {
    FJ_FuiNode* craftingSelector = child(this, "MainPanel.CraftingSelector");
    FJ_FuiNode* scrollArrows = child(craftingSelector, "ScrollArrows");
    FJ_FuiNode* craftingHSlots = child(this, "MainPanel.CraftingHSlots");
    FJ_FuiNode* slotSelector2 = child(this, "MainPanel.SlotSelector2");
    FJ_FuiNode* slot2VSlot = child(slotSelector2, "Crafting2VSlot2");
    FJ_FuiNode* slotSelector3 = child(this, "MainPanel.SlotSelector3");
    FJ_FuiNode* slot3VSlot = child(slotSelector3, "Crafting3VSlot3");

    if (m_panelMode == 0) {
        craftingSelector->setX(craftingHSlots->getX() -
                                static_cast<float>(Math::round((craftingSelector->getWidth() - m_selectorStep) * 0.5f)) +
                                static_cast<float>(horizontal * m_selectorStep));
        craftingSelector->setY(craftingHSlots->getY() -
                                static_cast<float>(Math::round((craftingSelector->getHeight() - m_selectorStep) * 0.5f)));
        if (slot2VSlot == 0 || slot3VSlot == 0) {
            slotSelector2->setVisible(vertical == 1);
            slotSelector3->setVisible(vertical == 2);
            if (slotSelector2->isVisible()) {
                slotSelector2->setX(craftingHSlots->getX() -
                                     static_cast<float>(Math::round((slotSelector2->getWidth() - m_selectorStep) * 0.5f)) +
                                     static_cast<float>(vertical * m_selectorStep));
                slotSelector2->setY(craftingHSlots->getY() -
                                     static_cast<float>(Math::round((slotSelector2->getHeight() - m_selectorStep) * 0.5f)));
            }
            if (slotSelector3->isVisible()) {
                slotSelector3->setX(craftingHSlots->getX() -
                                     static_cast<float>(Math::round((slotSelector3->getWidth() - m_selectorStep) * 0.5f)) +
                                     static_cast<float>(vertical * m_selectorStep));
                slotSelector3->setY(craftingHSlots->getY() -
                                     static_cast<float>(Math::round((slotSelector3->getHeight() - m_selectorStep) * 0.5f)));
                scrollArrows->setVisible(vertical != 0);
                return;
            }
        } else {
            slotSelector2->setVisible(vertical == 1);
            slotSelector3->setVisible(vertical == 2);
            if (slotSelector2->isVisible()) {
                slotSelector2->setX(craftingHSlots->getX() -
                                     static_cast<float>(Math::round((slotSelector2->getWidth() - m_selectorStep) * 0.5f)) +
                                     static_cast<float>(vertical * m_selectorStep));
                slotSelector2->setY(craftingHSlots->getY() -
                                     static_cast<float>(Math::round((slotSelector2->getHeight() - m_selectorStep) * 0.5f)) +
                                     static_cast<float>(m_selectorStep / 2));
            }
            if (slotSelector3->isVisible()) {
                slotSelector3->setX(craftingHSlots->getX() -
                                     static_cast<float>(Math::round((slotSelector3->getWidth() - m_selectorStep) * 0.5f)) +
                                     static_cast<float>(vertical * m_selectorStep));
                slotSelector3->setY(craftingHSlots->getY() -
                                     static_cast<float>(Math::round((slotSelector3->getHeight() - m_selectorStep) * 0.5f)));
                scrollArrows->setVisible(vertical != 0);
                return;
            }
        }
    } else {
        slotSelector2->setVisible(false);
        slotSelector3->setVisible(false);
        if (m_panelMode == 2) {
            if (m_slot2Items[0] != 0 && m_slot2Items[1] != 0) {
                int index = vertical > 1 ? 1 : vertical;
                FJ_FuiNode* target = m_slot2Items[index];
                craftingSelector->setX(target->getX() -
                                        static_cast<float>(Math::round((craftingSelector->getWidth() - m_selectorStep) * 0.5f)));
                craftingSelector->setY(target->getY() -
                                        static_cast<float>(Math::round((craftingSelector->getHeight() - m_selectorStep) * 0.5f)));
            }
        } else if (m_panelMode == 3) {
            if (m_slot3Items[0] != 0 && m_slot3Items[1] != 0 && m_slot3Items[2] != 0) {
                int index = vertical > 2 ? 2 : vertical;
                FJ_FuiNode* target = m_slot3Items[index];
                craftingSelector->setX(target->getX() -
                                        static_cast<float>(Math::round((craftingSelector->getWidth() - m_selectorStep) * 0.5f)));
                craftingSelector->setY(target->getY() -
                                        static_cast<float>(Math::round((craftingSelector->getHeight() - m_selectorStep) * 0.5f)));
            }
        }
    }
    scrollArrows->setVisible(vertical != 0);
}

void FJ_CraftingMenu::init(FJ_Event* event) {
    m_slot2Items[0] = child(this, "MainPanel.Slot2_1");
    m_slot2Items[1] = child(this, "MainPanel.Slot2_2");
    m_slot3Items[0] = child(this, "MainPanel.Slot3_1");
    m_slot3Items[1] = child(this, "MainPanel.Slot3_2");
    m_slot3Items[2] = child(this, "MainPanel.Slot3_3");
    m_horizontalTabCount = 0;
    m_verticalTabCount = 0;
    m_panelType = -1;
    m_activeTab = -1;
    m_activeSubtab = -1;
    for (int i = 0; i < 5; ++i) {
        m_ingredients[i] = false;
    }
    m_descriptionBaseY = 0;
    showPanelDisplay(1);
    ShowDescriptionPanel(false);
    ShowSuperCraftingSlots(0);
    MoveSelector(0, 0);
}

void FJ_CraftingMenu::InitCraftingMenu(int type) {
    FJ_FuiNode* ingredientsLayout = child(this, "MainPanel.IngredientsLayout");
    FJ_FuiNode* ingredientsLayoutSmall = child(this, "MainPanel.IngredientsLayout_Small");
    FJ_FuiNode* craftingOutput = child(this, "MainPanel.CraftingOutput");
    FJ_FuiNode* craftingArrow = child(this, "MainPanel.CraftingArrow");
    FJ_Label* itemName = child(this, "MainPanel.ItemName")->asFJ_Label();
    FJ_Label* itemNameSmall = child(this, "MainPanel.ItemNameSmall")->asFJ_Label();
    FJ_Label* ingredient1Desc = child(this, "MainPanel.Ingredient1Desc")->asFJ_Label();
    FJ_Label* ingredient2Desc = child(this, "MainPanel.Ingredient2Desc")->asFJ_Label();
    FJ_Label* ingredient3Desc = child(this, "MainPanel.Ingredient3Desc")->asFJ_Label();
    FJ_Label* ingredient4Desc = child(this, "MainPanel.Ingredient4Desc")->asFJ_Label();
    FJ_Label* ingredient5Desc = child(this, "MainPanel.Ingredient5Desc")->asFJ_Label();

    m_craftingType = type;
    if (type == 0) {
        ingredientsLayout->setVisible(false);
        ingredientsLayoutSmall->setVisible(true);
        float widthDiff = (ingredientsLayout->getWidth() - ingredientsLayoutSmall->getWidth()) * 0.5f;
        ingredientsLayout->decX(widthDiff);
        craftingOutput->decX(widthDiff);
        float rightEdge = ingredientsLayoutSmall->getX() + ingredientsLayoutSmall->getWidth();
        craftingArrow->setX(-(craftingArrow->getWidth() * 0.5f -
                               ((craftingOutput->getX() - rightEdge) * 0.5f + rightEdge)));
        itemName->setUseHtmlText(true);
    } else {
        ingredientsLayout->setVisible(true);
        ingredientsLayoutSmall->setVisible(false);
        itemName->setUseHtmlText(true);
    }
    if (itemNameSmall != 0) {
        itemNameSmall->setUseHtmlText(true);
    }
    ingredient1Desc->setUseHtmlText(true);
    ingredient2Desc->setUseHtmlText(true);
    ingredient3Desc->setUseHtmlText(true);
    ingredient4Desc->setUseHtmlText(true);
    if (ingredient5Desc != 0) {
        ingredient5Desc->setUseHtmlText(true);
    }
    if (itemNameSmall != 0) {
        itemNameSmall->SetLabel(std::wstring());
    }
    m_descriptionBaseY = static_cast<int>(ingredient2Desc->getY());
}

void FJ_CraftingMenu::SetItemName(const std::wstring& name) {
    FJ_Label* itemName = child(this, "MainPanel.ItemName")->asFJ_Label();
    FJ_Label* itemNameSmall = child(this, "MainPanel.ItemNameSmall")->asFJ_Label();
    itemName->SetLabel(name);
    if (itemNameSmall != 0) {
        itemNameSmall->SetLabel(std::wstring());
    }
    if (itemName->GetTextField()->getNumLines() > 2) {
        itemName->SetLabel(std::wstring());
        if (itemNameSmall != 0) {
            itemNameSmall->SetLabel(name);
        }
    }
}

void FJ_CraftingMenu::SetItemDescription(int index, const std::wstring& description) {
    FJ_Label* ingredient1Desc = child(this, "MainPanel.Ingredient1Desc")->asFJ_Label();
    FJ_Label* ingredient2Desc = child(this, "MainPanel.Ingredient2Desc")->asFJ_Label();
    FJ_Label* ingredient3Desc = child(this, "MainPanel.Ingredient3Desc")->asFJ_Label();
    FJ_Label* ingredient4Desc = child(this, "MainPanel.Ingredient4Desc")->asFJ_Label();
    FJ_Label* ingredient5Desc = child(this, "MainPanel.Ingredient5Desc")->asFJ_Label();

    if (index == 0) {
        ingredient1Desc->SetLabel(description);
        return;
    }
    if (index == 1) {
        ingredient2Desc->SetLabel(description);
        ingredient2Desc->setY(static_cast<float>(m_descriptionBaseY));
        if (ingredient2Desc->GetTextField()->getNumLines() >= 3) {
            ingredient2Desc->incY(static_cast<float>(ingredient2Desc->GetAverageCharHeight()) * 0.5f);
        }
        return;
    }
    if (index == 2) {
        ingredient3Desc->SetLabel(description);
        return;
    }
    if (index == 3) {
        ingredient4Desc->SetLabel(description);
        return;
    }
    if (index == 4) {
        if (ingredient5Desc != 0) {
            ingredient5Desc->SetLabel(description);
        }
    }
}

void FJ_CraftingMenu::EnableTab(int index, bool enabled) {
    FJ_FuiNode* mainPanel = child(this, "MainPanel")->asMovieClip();
    FJ_FuiNode* tab = mainPanel->getChildByName(hTabName(index))->asMovieClip();
    FJ_FuiNode* tabOff = mainPanel->getChildByName(std::string(tab->getName()) + "_Off")->asMovieClip();
    FJ_FuiNode* staticIconHolder = mainPanel->getChildByName(hTabIconName(index))->asMovieClip();
    if (enabled) {
        tabOff->gotoAndStop(1);
        tab->setVisible(true);
        if (staticIconHolder != 0) {
            staticIconHolder->setVisible(true);
        }
    } else {
        tabOff->gotoAndStop(2);
        tab->setVisible(false);
        if (staticIconHolder != 0) {
            staticIconHolder->setVisible(false);
        }
    }
}

FJ_FuiNode* FJ_CraftingMenu::SetupIggySlot(FJ_FuiNode* node) {
    FJ_FuiNode* movieClip = node->getChildByName("iggy")->asMovieClip();
    if (movieClip != 0) {
        fuiRenderNode* sizeSource = movieClip->getRenderNodeChildAt(0);
        fuiRect rect;
        rect.xMin = 0.0f;
        rect.xMax = sizeSource->getWidth();
        rect.yMin = 0.0f;
        rect.yMax = sizeSource->getHeight();
        char name[64];
        std::sprintf(name, "iggy_%s", node->getName());
        fuiRenderNodeCodeGenRect* rectNode =
            new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, name);
        rectNode->setAlpha(0.25f);
        FJ_FuiNode* newNode = new FJ_FuiNode(rectNode);
        rectNode->setFuiNode(newNode);
        movieClip->removeChildAt(0, true);
        movieClip->addChildAt(newNode, 0);
        return newNode;
    }
    return node->getChildByName(std::string("iggy_") + node->getName());
}

void FJ_CraftingMenu::SetCraftingMenu(int horizontalCount, int verticalCount, bool enabled) {
    int i;
    for (i = 0; i < 7; ++i) {
        FJ_FuiNode* mainPanel = child(this, "MainPanel")->asMovieClip();
        FJ_FuiNode* tab = mainPanel->getChildByName(hTabName(i))->asMovieClip();
        if (tab == 0) {
            break;
        }
        FJ_FuiNode* tabOff = mainPanel->getChildByName(std::string(tab->getName()) + "_Off")->asMovieClip();
        FJ_FuiNode* staticIconHolder = mainPanel->getChildByName(hTabIconName(i))->asMovieClip();
        FJ_FuiNode* iggySlot = SetupIggySlot(tab);
        iggySlot->setVisible(enabled);
        FJ_FuiNode* staticIcon = tab->getChildByName("staticicon");
        staticIcon->setVisible(!enabled);
        if (m_craftingType == 0) {
            staticIcon->asMovieClip()->getChildAt(0)->asMovieClip()->gotoAndStop(kNormalIconFrames[i]);
        } else {
            staticIcon->asMovieClip()->getChildAt(0)->asMovieClip()->gotoAndStop(kSpecialIconFrames[i]);
        }
        int frame = i == 0 ? 1 : (i == 6 ? 3 : 2);
        tab->getChildByName("HorTabOn")->asMovieClip()->gotoAndStop(frame);
        bool visible = i < horizontalCount;
        if (staticIconHolder != 0) {
            staticIconHolder->setVisible(visible);
        }
        tab->setVisible(visible);
        tabOff->setVisible(visible);
        m_horizontalTabCount = horizontalCount;
    }
    for (i = 0; i < 5; ++i) {
        FJ_FuiNode* mainPanel = child(this, "MainPanel")->asMovieClip();
        FJ_FuiNode* tab = mainPanel->getChildByName(vTabName(i))->asMovieClip();
        if (tab == 0) {
            break;
        }
        FJ_FuiNode* tabOff = mainPanel->getChildByName(std::string(tab->getName()) + "_Off")->asMovieClip();
        FJ_FuiNode* staticIconHolder = mainPanel->getChildByName(vTabIconName(i))->asMovieClip();
        SetupIggySlot(tab);
        bool visible = i < verticalCount;
        if (staticIconHolder != 0) {
            staticIconHolder->setVisible(visible);
        }
        tab->setVisible(visible);
        tabOff->setVisible(visible);
        m_verticalTabCount = verticalCount;
    }
}

void FJ_CraftingMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    const double stageHeight = getStage()->getStageHeight();
    if (stageHeight != 360.0 && stageHeight != 540.0)
        return;

    setY(top);
}

void FJ_CraftingMenu::SelectVerticalItem(int index) {
    if (m_panelMode == 0) {
        FJ_FuiNode* craftingSelector = child(this, "MainPanel.CraftingSelector");
        FJ_FuiNode* craftingHSlots = child(this, "MainPanel.CraftingHSlots");
        FJ_FuiNode* slotSelector2 = child(this, "MainPanel.SlotSelector2");
        FJ_FuiNode* slot2VSlot = child(slotSelector2, "Crafting2VSlot2");
        FJ_FuiNode* slotSelector3 = child(this, "MainPanel.SlotSelector3");
        FJ_FuiNode* slot3VSlot = child(slotSelector3, "Crafting3VSlot3");
        int vertical = index - 1;
        if (slot2VSlot != 0 && slot3VSlot != 0) {
            if (slotSelector2->isVisible() && vertical == -1) {
                vertical = 0;
            }
            craftingSelector->setY(craftingHSlots->getY() -
                                    static_cast<float>(Math::round((craftingSelector->getHeight() - m_selectorStep) * 0.5f)) +
                                    static_cast<float>(vertical * m_selectorStep));
        }
    }
}

void FJ_CraftingMenu::UpdateTabs(FJ_Event* event) {
    int i;
    if (m_horizontalTabCount > 0) {
        FJ_FuiNode* mainPanel = child(this, "MainPanel")->asMovieClip();
        for (i = 0; i < m_horizontalTabCount; ++i) {
            FJ_FuiNode* tab = mainPanel->getChildByName(hTabName(i))->asMovieClip();
            FJ_FuiNode* tabOff = mainPanel->getChildByName(std::string(tab->getName()) + "_Off")->asMovieClip();
            if (i == m_activeTab) {
                tab->gotoAndStop(2);
                tabOff->setVisible(false);
            } else {
                tab->gotoAndStop(1);
                tabOff->setVisible(true);
            }
        }
    }
    if (m_verticalTabCount > 0) {
        FJ_FuiNode* mainPanel = child(this, "MainPanel")->asMovieClip();
        for (i = 0; i < m_verticalTabCount; ++i) {
            FJ_FuiNode* tab = mainPanel->getChildByName(vTabName(i))->asMovieClip();
            FJ_FuiNode* tabOff = mainPanel->getChildByName(std::string(tab->getName()) + "_Off")->asMovieClip();
            if (i == m_activeSubtab) {
                tab->gotoAndStop(2);
                tabOff->setVisible(false);
            } else {
                tab->gotoAndStop(1);
                tabOff->setVisible(true);
            }
        }
    }
}

void FJ_CraftingMenu::SetActiveTab(int tab, int subtab) {
    m_activeTab = tab;
    m_activeSubtab = subtab;
    EnableTab(tab, true);
}

void FJ_CraftingMenu::ShowIngredient(int index, bool show) {
    m_ingredients[index] = show;
}

void FJ_CraftingMenu::_UpdateTabs(FJ_FuiNode* node, FJ_Event* event) {
    if (node != 0 && event != 0) {
        node->eventListen(event, false);
    }
}
