#include "game/ui/UIScene_TradingMenu.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/MerchantRecipe.hpp"
#include "game/core/Slot.hpp"
#include "game/ui/FJ_TradingMenu.hpp"
#include "game/ui/MerchantMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRect.hpp"

#include <cstdio>

extern CMinecraftApp* g_pApp;
extern ConsoleUIController gConsoleUIController;

UIScene_TradingMenu::UIScene_TradingMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer),
      m_scrollRightShown(false),
      m_scrollLeftShown(true),
      m_merchantMenu(),
      m_trades(),
      m_tradeCount(0),
      m_tradeRow(0),
      m_tradeOffset(0),
      m_tradeState(0),
      m_tradeFocus(0),
      m_mainPanel(),
      m_tradingBar(),
      m_request1(),
      m_request2(),
      m_hotBar(),
      m_inventory(),
      m_villagerText(),
      m_requiredLabel(),
      m_request1Label(),
      m_request2Label(),
      m_inventoryLabel()
{
    initialiseMovie();
    m_villagerText.init(UIString(L"HotBar"));
    m_requiredLabel.init(UIString(L10N::GetString(0x14630b9e)));
    m_request1Label.init(UIString(L10N::GetString(0x3dbed919)));
    m_request2Label.init(UIString(L"HotBar"));
    m_inventoryLabel.init(UIString(L"HotBar"));
    m_request1.addSlots(0x27, 1);
    m_request2.addSlots(0x28, 1);
    m_tradingBar.addSlots(0x29, 7);
    m_inventory.addSlots(3, 0x1b);
    m_hotBar.addSlots(0x1e, 9);
}

UIScene_TradingMenu::~UIScene_TradingMenu()
{
}

unsigned int UIScene_TradingMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_mainPanel.setupControl(this, root, std::string("MainPanel"));
    m_controls.push_back(&m_mainPanel);
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_tradingBar.setupControl(this, panel, std::string("TradingBar"));
    m_controls.push_back(&m_tradingBar);
    m_request1.setupControl(this, panel, std::string("Request1"));
    m_controls.push_back(&m_request1);
    m_request2.setupControl(this, panel, std::string("Request2"));
    m_controls.push_back(&m_request2);
    m_villagerText.setupControl(this, panel, std::string("VillagerText"));
    m_controls.push_back(&m_villagerText);
    m_requiredLabel.setupControl(this, panel, std::string("RequiredLabel"));
    m_controls.push_back(&m_requiredLabel);
    m_request1Label.setupControl(this, panel, std::string("Request1Label"));
    m_controls.push_back(&m_request1Label);
    m_request2Label.setupControl(this, panel, std::string("Request2Label"));
    m_controls.push_back(&m_request2Label);
    m_hotBar.setupControl(this, panel, std::string("HotBar"));
    m_controls.push_back(&m_hotBar);
    m_inventory.setupControl(this, panel, std::string("Inventory"));
    m_controls.push_back(&m_inventory);
    m_inventoryLabel.setupControl(this, panel, std::string("InventoryLabel"));
    m_controls.push_back(&m_inventoryLabel);
    return 1;
}

std::wstring UIScene_TradingMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"TradingMenu") :
        std::wstring(L"TradingMenuSplit");
}

void UIScene_TradingMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, (int)0x80cf40eb, 0x4f6bcf5e, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_TradingMenu::handleDestroy()
{
    UIScene::handleDestroy();
}

int UIScene_TradingMenu::GetItemDescription(not_null_ptr<ItemInstance> item)
{
    std::vector<HtmlString>* lines = item->getTooltipLines(boost::shared_ptr<Player>(), false);
    return lines == 0 ? 0 : static_cast<int>(lines->size());
}

bool UIScene_TradingMenu::canMake(MerchantRecipe* recipe)
{
    if (recipe == 0 || recipe->isDeprecated())
        return false;
    not_null_ptr<ItemInstance> buyA = recipe->getBuyAItem();
    if (buyA->isEmpty())
        return false;
    return true;
}

void UIScene_TradingMenu::setTradeRedBox(int trade, bool show)
{
    m_tradingBar.showSlotRedBox(trade, show);
}

void UIScene_TradingMenu::setTitle(const std::wstring& title)
{
    m_villagerText.init(UIString(title));
}

void UIScene_TradingMenu::setOfferDescription(std::vector<HtmlString>* description)
{
    if (description != 0)
        m_mainPanel.setVisible(true);
}

void UIScene_TradingMenu::setRequest1Name(const std::wstring& name)
{
    m_request1Label.init(UIString(name));
}

void UIScene_TradingMenu::setRequest2Name(const std::wstring& name)
{
    m_request2Label.init(UIString(name));
}

void UIScene_TradingMenu::setRequest1RedBox(bool show)
{
    m_request1.showSlotRedBox(0, show);
}

void UIScene_TradingMenu::setRequest2RedBox(bool show)
{
    m_request2.showSlotRedBox(0, show);
}

void UIScene_TradingMenu::updateDisplay()
{
    updateTooltips();
}

void UIScene_TradingMenu::handleReload()
{
    m_request1.addSlots(0x27, 1);
    m_request2.addSlots(0x28, 1);
    m_tradingBar.addSlots(0x29, 7);
    m_inventory.addSlots(3, 0x1b);
    m_hotBar.addSlots(0x1e, 9);
    updateDisplay();
}

void UIScene_TradingMenu::showScrollRightArrow(bool show)
{
    m_scrollRightShown = show;
}

void UIScene_TradingMenu::showScrollLeftArrow(bool show)
{
    m_scrollLeftShown = show;
}

void UIScene_TradingMenu::handleTick()
{
    if (!m_updatePending)
        updateDisplay();
}

void UIScene_TradingMenu::tick()
{
    UIScene::tick();
    handleTick();
}

void UIScene_TradingMenu::moveSelector(bool right)
{
    if (m_movie != 0)
        m_movie->setVisible("MainPanel", true);
}

int UIScene_TradingMenu::handleKeyDown(int, int action, bool)
{
    if (action == 0)
        moveSelector(false);
    else if (action == 1)
        moveSelector(true);
    else if (action == 2)
        updateDisplay();
    return 0;
}

void UIScene_TradingMenu::handleInput(int, int action, bool repeat, bool released,
                                      bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (released)
        handled = handleKeyDown(m_playerId, action, repeat) != 0;
}

void UIScene_TradingMenu::customDraw(const char* name, fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft == 0 || rect == 0 || name == 0 || !m_merchantMenu)
        return;

    int slotId = -1;
    if (std::sscanf(name, "slot_%d", &slotId) != 1)
        return;

    not_null_ptr<ItemInstance> item;
    const int tradeCount = static_cast<int>(m_trades.size());
    if (slotId < 0)
        return;

    if (slotId < 0x27)
    {
        Slot* slot = m_merchantMenu->getSlot(slotId);
        if (slot == 0)
            return;
        item = slot->getItem();
    }
    else
    {
        int tradeIndex;
        if (slotId < 0x29)
        {
            if (tradeCount == 0 || m_tradeRow + m_tradeOffset >= tradeCount)
                return;
            tradeIndex = m_tradeOffset;
        }
        else
        {
            tradeIndex = slotId - 0x29 + m_tradeOffset;
            if (tradeCount == 0 || tradeIndex >= tradeCount)
                return;
        }

        MerchantRecipe* recipe = m_trades[tradeIndex].first;
        if (recipe == 0)
            return;
        if (slotId == 0x27)
            item = recipe->getBuyAItem();
        else if (slotId == 0x28)
            item = recipe->getBuyBItem();
        else
            item = recipe->getSellItem();
    }

    if (item->isEmpty())
        return;
    customDrawSlotControlFui(rect, m_playerId, item, 1.0f, true, true, false);
}

void UIScene_TradingMenu::handleInventoryUpdated(void*)
{
    updateDisplay();
}

void UIScene_TradingMenu::HandleMessage(EUIMessage message, void*)
{
    if (message == EUIMessage_Unknown)
        updateDisplay();
}

boost::shared_ptr<MerchantMenu> UIScene_TradingMenu::getMerchant()
{
    return m_merchantMenu;
}

int UIScene_TradingMenu::getSceneType()
{
    return 0x32;
}
