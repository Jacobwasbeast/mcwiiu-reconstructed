#include "game/ui/UIScene_TradingMenu.hpp"

#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/core/Merchant.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StringUtils.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/MerchantRecipe.hpp"
#include "game/item/MerchantRecipeList.hpp"
#include "game/core/Slot.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerboundCustomPayloadPacket.hpp"
#include "game/network/TradeItemPacket.hpp"
#include "game/ui/FJ_TradingMenu.hpp"
#include "game/ui/MerchantMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

#include <cstdio>

extern CMinecraftApp* g_pApp;
extern ConsoleUIController gConsoleUIController;

struct TradingMenuData
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Merchant> merchant;
    Level* level;
};

UIScene_TradingMenu::UIScene_TradingMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer),
      m_scrollRightShown(true),
      m_scrollLeftShown(true),
      m_merchantMenuPtr(NULL),
      m_merchantMenu(),
      m_trades(),
      m_tradeCount(0),
      m_tradeRow(0),
      m_tradeOffset(0),
      m_tradeFocusSet(false),
      m_tutorialHintValue(0),
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
    m_requiredLabel.init(UIString(L"HotBar"));
    m_villagerText.init(UIString(L10N::GetString(0x14630b9e)));
    m_request1Label.init(UIString(L10N::GetString(0x3dbed919)));
    m_request2Label.init(UIString(L"HotBar"));
    m_inventoryLabel.init(UIString(L"HotBar"));

    Minecraft* minecraft = Minecraft::GetInstance();
    MultiPlayerGameMode* gameMode = minecraft->mGameModes[m_playerId];
    if (gameMode != 0)
    {
        Tutorial* tutorial = gameMode->getTutorial();
        if (tutorial != 0)
        {
            m_tutorialHintValue = tutorial->currentState;
            tutorial->changeTutorialState(static_cast<eTutorial_State>(0x19), this);
        }
    }

    TradingMenuData* sceneData = static_cast<TradingMenuData*>(data);
    m_merchantMenu = boost::shared_ptr<MerchantMenu>(new MerchantMenu(
        sceneData->inventory, sceneData->merchant, sceneData->level));
    m_merchantMenuPtr = m_merchantMenu.get();
    minecraft->mLocalPlayers[m_playerId]->mContainerMenu = m_merchantMenuPtr;

    m_request1.addSlots(0x27, 1);
    m_request2.addSlots(0x28, 1);
    m_tradingBar.addSlots(0x29, 7);
    m_inventory.addSlots(3, 0x1b);
    m_hotBar.addSlots(0x1e, 9);

    gConsoleUIController.OverrideSFX(m_playerId, 0, true);
    gConsoleUIController.OverrideSFX(m_playerId, 0x14, true);
    gConsoleUIController.OverrideSFX(m_playerId, 0xb, true);
    gConsoleUIController.OverrideSFX(m_playerId, 10, true);
    gConsoleUIController.OverrideSFX(m_playerId, 7, true);
    gConsoleUIController.OverrideSFX(m_playerId, 6, true);
    gConsoleUIController.OverrideSFX(m_playerId, 4, true);
    gConsoleUIController.OverrideSFX(m_playerId, 5, true);
    g_pApp->SetRichPresenceContext(m_playerId, 0xd);
}

UIScene_TradingMenu::~UIScene_TradingMenu()
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[m_playerId];
    if (m_merchantMenuPtr != NULL)
    {
        m_merchantMenuPtr->removed(player);
        if (player && player->mContainerMenu == m_merchantMenuPtr)
            player->closeContainer();
        m_merchantMenuPtr = NULL;
    }
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
    Minecraft* minecraft = Minecraft::GetInstance();
    MultiPlayerGameMode* gameMode = minecraft->mGameModes[m_playerId];
    if (gameMode != 0)
    {
        Tutorial* tutorial = gameMode->getTutorial();
        if (tutorial != 0)
            tutorial->changeTutorialState(static_cast<eTutorial_State>(m_tutorialHintValue), 0);
    }

    if (minecraft->mLocalPlayers[m_playerId])
        minecraft->mLocalPlayers[m_playerId]->closeContainer();

    gConsoleUIController.OverrideSFX(m_playerId, 0, false);
    gConsoleUIController.OverrideSFX(m_playerId, 0x14, false);
    gConsoleUIController.OverrideSFX(m_playerId, 0xb, false);
    gConsoleUIController.OverrideSFX(m_playerId, 10, false);
    gConsoleUIController.OverrideSFX(m_playerId, 7, false);
    gConsoleUIController.OverrideSFX(m_playerId, 6, false);
    gConsoleUIController.OverrideSFX(m_playerId, 4, false);
    gConsoleUIController.OverrideSFX(m_playerId, 5, false);
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
    not_null_ptr<ItemInstance> buyB = recipe->getBuyBItem();

    boost::shared_ptr<Player> player = Minecraft::GetInstance()->GetPlayerByPlayerIndex(m_playerId);

    const int matchesA = player->mInventory->countMatches(buyA);
    const bool haveEnoughA = (matchesA < 1) ? buyA->isEmpty() : (buyA->getCount() <= matchesA);

    const int matchesB = player->mInventory->countMatches(buyB);
    if (matchesB < 1)
        return haveEnoughA && buyB->isEmpty();
    return haveEnoughA && (buyB->getCount() <= matchesB);
}

void UIScene_TradingMenu::setTradeRedBox(int trade, bool show)
{
    m_tradingBar.showSlotRedBox(trade, show);
}

void UIScene_TradingMenu::setTitle(const std::wstring& title)
{
    m_villagerText.setLabel(UIString(title), false, false);
}

void UIScene_TradingMenu::setOfferDescription(std::vector<HtmlString>* description)
{
    fuiRenderNode* root = m_movie->getRootNode();
    FJ_TradingMenu* document = static_cast<FJ_TradingMenu*>(
        root->getFuiNode()->asFJ_Document());
    document->SetOfferDescription(HtmlString::Compose(description));
}

void UIScene_TradingMenu::setRequest1Name(const std::wstring& name)
{
    m_request1Label.setLabel(UIString(name), false, false);
}

void UIScene_TradingMenu::setRequest2Name(const std::wstring& name)
{
    m_request2Label.setLabel(UIString(name), false, false);
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
    int tooltipHint = -1;

    boost::shared_ptr<Player> player = Minecraft::GetInstance()->GetPlayerByPlayerIndex(m_playerId);
    boost::shared_ptr<Merchant> merchant = m_merchantMenu->getMerchant();
    MerchantRecipeList* offers = merchant->getOffers(player);

    if (offers != 0)
    {
        m_trades.clear();
        int bestIndex = 0x7fffffff;
        int index = 0;
        for (MerchantRecipeList::iterator it = offers->begin(); it != offers->end(); ++it)
        {
            MerchantRecipe* recipe = *it;
            if (!recipe->isDeprecated())
            {
                m_trades.push_back(std::make_pair(recipe, index));
                if (index < bestIndex)
                    bestIndex = index;
            }
            ++index;
        }

        if (!m_tradeFocusSet && bestIndex != 0 && static_cast<unsigned int>(bestIndex) < offers->size())
        {
            m_merchantMenuPtr->setSelectionHint(bestIndex);

            ByteArrayOutputStream byteStream;
            DataOutputStream dataStream(&byteStream);
            dataStream.writeInt(bestIndex);

            ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
            connection->send(boost::shared_ptr<Packet>(new ServerboundCustomPayloadPacket(
                std::wstring(L"MC|TrSel"), byteStream.toByteArray())));
        }

        const int tradeCount = static_cast<int>(m_trades.size());
        if (tradeCount < m_tradeOffset + 7)
        {
            const int newOffset = tradeCount - 7;
            m_tradeOffset = newOffset < 0 ? 0 : newOffset;
        }

        for (int slot = 0; slot < 7; ++slot)
        {
            bool redBox = false;
            const int tradeIndex = m_tradeOffset + slot;
            if (!m_trades.empty() && static_cast<unsigned int>(tradeIndex) < m_trades.size())
                redBox = !canMake(m_trades[tradeIndex].first);
            setTradeRedBox(slot, redBox);
        }

        if (m_trades.empty() || static_cast<unsigned int>(m_tradeRow + m_tradeOffset) >= m_trades.size())
        {
            setTitle(merchant->getDisplayName());
            setRequest1Name(std::wstring(L"HotBar"));
            setRequest2Name(std::wstring(L"HotBar"));
            setRequest1RedBox(false);
            setRequest2RedBox(false);
            std::vector<HtmlString> emptyDescription;
            setOfferDescription(&emptyDescription);
            m_tradeFocusSet = true;
        }
        else
        {
            MerchantRecipe* recipe = m_trades[m_tradeRow + m_tradeOffset].first;

            std::wstring villagerText = L10N::GetString(0x8c04d144);
            villagerText = replaceAll(villagerText, std::wstring(L"{*VILLAGER_TYPE*}"), merchant->getDisplayName());
            not_null_ptr<ItemInstance> sellItem = recipe->getSellItem();
            villagerText.replace(villagerText.find(L"%s"), 2, sellItem->getHoverName());
            setTitle(villagerText);

            not_null_ptr<ItemInstance> buyA = recipe->getBuyAItem();
            not_null_ptr<ItemInstance> buyB = recipe->getBuyBItem();
            if (!buyA->isEmpty())
            {
                setRequest1Name(buyA->getHoverName());
                setRequest2Name(buyB->isEmpty() ? std::wstring(L"HotBar") : buyB->getHoverName());
            }
            else
            {
                setRequest1Name(std::wstring(L"HotBar"));
                setRequest2Name(buyB->isEmpty() ? std::wstring(L"HotBar") : buyB->getHoverName());
            }

            const int matchesA = player->mInventory->countMatches(buyA);
            bool haveEnoughA;
            if (matchesA < 1)
            {
                setRequest1RedBox(true);
                haveEnoughA = false;
            }
            else
            {
                setRequest1RedBox(buyA->getCount() > matchesA);
                haveEnoughA = buyA->getCount() <= matchesA;
            }

            const int matchesB = player->mInventory->countMatches(buyB);
            if (matchesB < 1)
            {
                setRequest2RedBox(!buyB->isEmpty());
                if (haveEnoughA && buyB->isEmpty())
                    tooltipHint = 0x80cf40eb;
            }
            else
            {
                setRequest2RedBox(buyB->getCount() > matchesB);
                if (haveEnoughA && buyB->getCount() <= matchesB)
                    tooltipHint = 0x80cf40eb;
            }
        }
    }

    gConsoleUIController.SetTooltips(m_playerId, tooltipHint, 0xded2ef45, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
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
    if (m_scrollRightShown == show)
        return;

    fuiRenderNode* root = m_movie->getRootNode();
    FJ_TradingMenu* document = static_cast<FJ_TradingMenu*>(
        root->getFuiNode()->asFJ_Document());
    document->ShowScrollRightArrow(show);
    m_scrollRightShown = show;
}

void UIScene_TradingMenu::showScrollLeftArrow(bool show)
{
    if (m_scrollLeftShown == show)
        return;

    fuiRenderNode* root = m_movie->getRootNode();
    FJ_TradingMenu* document = static_cast<FJ_TradingMenu*>(
        root->getFuiNode()->asFJ_Document());
    document->ShowScrollLeftArrow(show);
    m_scrollLeftShown = show;
}

void UIScene_TradingMenu::handleTick()
{
    boost::shared_ptr<Player> player = Minecraft::GetInstance()->GetPlayerByPlayerIndex(m_playerId);
    boost::shared_ptr<Merchant> merchant = m_merchantMenu->getMerchant();
    MerchantRecipeList* offers = merchant->getOffers(player);

    if (offers != 0 && !m_tradeFocusSet)
        updateDisplay();

    const unsigned int tradeCount = static_cast<unsigned int>(m_trades.size());
    showScrollRightArrow(static_cast<unsigned int>(m_tradeOffset + 7) < tradeCount);
    showScrollLeftArrow(m_tradeOffset != 0);

    if (!merchant->isAvailableToTrade())
    {
        gConsoleUIController.ShowTooltip(m_playerId, 2, false);
        gConsoleUIController.ShowTooltip(m_playerId, 1, false);
        gConsoleUIController.ShowTooltip(m_playerId, 0, false);
        gConsoleUIController.ShowTooltip(m_playerId, 7, false);
        gConsoleUIController.CloseUIScenes(m_playerId, false);
    }
}

void UIScene_TradingMenu::tick()
{
    UIScene::tick();
    handleTick();
}

void UIScene_TradingMenu::moveSelector(bool right)
{
    if (m_movie != 0)
    {
        fuiRenderNode* root = m_movie->getRootNode();
        FJ_TradingMenu* document = root == 0 ? 0 :
            static_cast<FJ_TradingMenu*>(root->getFuiNode()->asFJ_Document());
        if (document != 0)
            document->MoveSelector(right);
    }
}

int UIScene_TradingMenu::handleKeyDown(int, int action, bool)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    MultiPlayerGameMode* gameMode = minecraft->mGameModes[m_playerId];
    if (gameMode != 0)
    {
        Tutorial* tutorial = gameMode->getTutorial();
        if (tutorial != 0)
        {
            tutorial->handleUIInput(action);
            if (gConsoleUIController.IsTutorialVisible(m_playerId) && !tutorial->completed)
            {
                GameModeConstraints* constraints = gameMode->getGameModeConstraints();
                if (!constraints->isInputAllowed(action))
                    return 0;
            }
        }
    }

    if (action == 0)
    {
        if (m_trades.empty())
            return 1;

        const int selectedIndex = m_tradeRow + m_tradeOffset;
        if (static_cast<unsigned int>(selectedIndex) >= m_trades.size())
            return 1;

        MerchantRecipe* recipe = m_trades[selectedIndex].first;
        const int originalIndex = m_trades[selectedIndex].second;
        if (recipe->isDeprecated())
            return 1;

        not_null_ptr<ItemInstance> buyA = recipe->getBuyAItem();
        not_null_ptr<ItemInstance> buyB = recipe->getBuyBItem();
        boost::shared_ptr<Player> player = minecraft->GetPlayerByPlayerIndex(m_playerId);

        const int matchesA = player->mInventory->countMatches(buyA);
        const int matchesB = player->mInventory->countMatches(buyB);

        if ((!buyA->isEmpty() && matchesA < buyA->getCount()) ||
            (!buyB->isEmpty() && matchesB < buyB->getCount()))
            return 1;

    boost::shared_ptr<Merchant> merchant = m_merchantMenu->getMerchant();
        merchant->notifyTrade(recipe);

        player->mInventory->removeResources(buyA);
        player->mInventory->removeResources(buyB);

        not_null_ptr<ItemInstance> sellItemCopy = recipe->getSellItem()->copy();
        if (!player->mInventory->add(sellItemCopy))
            player->drop(sellItemCopy, false);

        const int containerId = *reinterpret_cast<const int*>(
            reinterpret_cast<const unsigned char*>(m_merchantMenuPtr) + 0x20);
        ClientPacketListener* connection = minecraft->getConnection(m_playerId);
        connection->send(boost::shared_ptr<Packet>(
            new TradeItemPacket(containerId, originalIndex)));

        updateDisplay();
        return 1;
    }

    if (action == 1)
    {
        gConsoleUIController.ShowTooltip(m_playerId, 2, false);
        gConsoleUIController.ShowTooltip(m_playerId, 1, false);
        gConsoleUIController.ShowTooltip(m_playerId, 0, false);
        gConsoleUIController.ShowTooltip(m_playerId, 7, false);
        gConsoleUIController.CloseUIScenes(m_playerId, false);
        return 1;
    }

    int selectedIndex;
    if (action == 6)
    {
        if (m_tradeRow == 6)
        {
            if (m_trades.empty())
                return 1;
            if (static_cast<unsigned int>(m_tradeOffset + 7) >= m_trades.size())
                return 1;
            ++m_tradeOffset;
        }
        else
        {
            ++m_tradeRow;
            moveSelector(true);
        }
        updateDisplay();
        selectedIndex = m_tradeRow + m_tradeOffset;
    }
    else if (action == 7)
    {
        if (m_tradeRow == 0)
        {
            if (m_tradeOffset < 1)
                return 1;
            --m_tradeOffset;
        }
        else
        {
            --m_tradeRow;
            moveSelector(false);
        }
        updateDisplay();
        selectedIndex = m_tradeRow + m_tradeOffset;
    }
    else
    {
        return 0;
    }

    if (m_trades.empty() || static_cast<unsigned int>(selectedIndex) >= m_trades.size())
        return 1;

    const int originalIndex = m_trades[selectedIndex].second;
    m_merchantMenuPtr->setSelectionHint(originalIndex);

    ByteArrayOutputStream byteStream;
    DataOutputStream dataStream(&byteStream);
    dataStream.writeInt(originalIndex);

    ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
    connection->send(boost::shared_ptr<Packet>(new ServerboundCustomPayloadPacket(
        std::wstring(L"MC|TrSel"), byteStream.toByteArray())));

    return 1;
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
    if (!minecraft->mLocalPlayers[m_playerId])
        return;
    if (minecraft->mGameModes[m_playerId] == 0)
        return;

    int slotId = -1;
    std::sscanf(name, "slot_%d", &slotId);

    not_null_ptr<ItemInstance> item;
    const int tradeCount = static_cast<int>(m_trades.size());

    if (slotId < 0x27)
    {
        Slot* slot = m_merchantMenuPtr->getSlot(slotId);
        item = slot->getItem();
    }
    else if (slotId < 0x29)
    {
        if (tradeCount != 0 && static_cast<unsigned int>(m_tradeRow + m_tradeOffset) < static_cast<unsigned int>(tradeCount))
        {
            MerchantRecipe* recipe = m_trades[m_tradeRow + m_tradeOffset].first;
            item = (slotId != 0x28) ? recipe->getBuyAItem() : recipe->getBuyBItem();
        }
    }
    else
    {
        const int tradeIndex = slotId - 0x29 + m_tradeOffset;
        if (tradeCount != 0 && static_cast<unsigned int>(tradeIndex) < static_cast<unsigned int>(tradeCount))
        {
            MerchantRecipe* recipe = m_trades[tradeIndex].first;
            item = recipe->getSellItem();
        }
    }

    if (!item->isEmpty())
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
