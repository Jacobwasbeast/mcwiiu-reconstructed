#include "game/ui/FJ_CraftingMenu.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"

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

}

FJ_CraftingMenu::FJ_CraftingMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode),
      m_root(0),
      m_panel(0),
      m_description(0),
      m_selector(0),
      m_panelType(0),
      m_activeTab(0),
      m_activeSubtab(0),
      m_verticalItem(0),
      m_selectorStep(0) {
    int i;
    for (i = 0; i < 5; ++i) {
        m_ingredientNodes[i] = 0;
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
    m_panelType = panel;
    setChildVisible(this, "MainPanel_Inventory", panel == 0);
    setChildVisible(this, "MainPanel_Crafting", panel == 1);
    setChildVisible(this, "MainPanel_Ingredient1", panel == 2 && m_ingredients[0]);
    setChildVisible(this, "MainPanel_Ingredient2", panel == 2 && m_ingredients[1]);
    setChildVisible(this, "MainPanel_Ingredient3", panel == 2 && m_ingredients[2]);
    setChildVisible(this, "MainPanel_Ingredient4", panel == 2 && m_ingredients[3]);
    setChildVisible(this, "MainPanel_Ingredient5", panel == 2 && m_ingredients[4]);
}

void FJ_CraftingMenu::ShowDescriptionPanel(bool show) {
    if (m_description == 0) {
        m_description = child(this, "MainPanel_Description");
    }
    if (m_description != 0) {
        m_description->setVisible(show);
    }
}

void FJ_CraftingMenu::ShowSuperCraftingSlots(int count) {
    int i;
    for (i = 0; i < 5; ++i) {
        char name[32];
        name[0] = 'S'; name[1] = 'l'; name[2] = 'o'; name[3] = 't';
        name[4] = static_cast<char>('1' + i); name[5] = 0;
        setChildVisible(this, name, i < count);
    }
}

void FJ_CraftingMenu::MoveSelector(int horizontal, int vertical) {
    m_selectorStep = horizontal;
    m_verticalItem += vertical;
    if (m_selector == 0) {
        m_selector = child(this, "MainPanel_SlotSelector2");
    }
    if (m_selector != 0) {
        m_selector->incX(static_cast<float>(horizontal));
        m_selector->incY(static_cast<float>(vertical));
    }
}

void FJ_CraftingMenu::init(FJ_Event* event) {
    FJ_Document::init(event);
    m_root = child(this, "MainPanel");
    m_panel = child(this, "MainPanel_Crafting");
    m_description = child(this, "MainPanel_Description");
    m_selector = child(this, "MainPanel_SlotSelector2");
    UpdateTabs(event);
}

void FJ_CraftingMenu::InitCraftingMenu(int type) {
    m_panelType = type;
    showPanelDisplay(type);
    ShowDescriptionPanel(false);
    ShowSuperCraftingSlots(0);
}

void FJ_CraftingMenu::SetItemName(const std::wstring& name) {
    if (m_description == 0) {
        m_description = child(this, "MainPanel_Description");
    }
    if (m_description != 0 && !name.empty()) {
        m_description->setVisible(true);
    }
}

void FJ_CraftingMenu::SetItemDescription(int index, const std::wstring& description) {
    if (index >= 0 && index < 5) {
        m_ingredients[index] = !description.empty();
        ShowIngredient(index, m_ingredients[index]);
    }
}

void FJ_CraftingMenu::EnableTab(int index, bool enabled) {
    if (index < 0 || index >= 5) {
        return;
    }
    char name[16];
    name[0] = 'T'; name[1] = 'a'; name[2] = 'b'; name[3] = static_cast<char>('1' + index); name[4] = 0;
    setChildVisible(this, name, enabled);
}

FJ_FuiNode* FJ_CraftingMenu::SetupIggySlot(FJ_FuiNode* node) {
    if (node != 0) {
        node->setVisible(true);
    }
    return node;
}

void FJ_CraftingMenu::SetCraftingMenu(int category, int item, bool enabled) {
    m_activeTab = category;
    m_activeSubtab = item;
    EnableTab(category, enabled);
    showPanelDisplay(enabled ? 1 : 0);
}

void FJ_CraftingMenu::SetSafeZone(int top, int bottom, int left, int right) {
    FJ_Document::SetSafeZone(top, bottom, left, right);
}

void FJ_CraftingMenu::SelectVerticalItem(int index) {
    m_verticalItem = index;
    MoveSelector(0, index);
}

void FJ_CraftingMenu::UpdateTabs(FJ_Event* event) {
    if (event != 0) {
        event->m_pTarget = this;
    }
    _UpdateTabs(this, event);
}

void FJ_CraftingMenu::SetActiveTab(int tab, int subtab) {
    m_activeTab = tab;
    m_activeSubtab = subtab;
    EnableTab(tab, true);
}

void FJ_CraftingMenu::ShowIngredient(int index, bool show) {
    if (index < 0 || index >= 5) {
        return;
    }
    m_ingredients[index] = show;
    const char* names[5] = {
        "MainPanel_Ingredient1", "MainPanel_Ingredient2", "MainPanel_Ingredient3",
        "MainPanel_Ingredient4", "MainPanel_Ingredient5"
    };
    setChildVisible(this, names[index], show && m_panelType == 2);
}

void FJ_CraftingMenu::_UpdateTabs(FJ_FuiNode* node, FJ_Event* event) {
    if (node != 0 && event != 0) {
        node->eventListen(event, false);
    }
}
