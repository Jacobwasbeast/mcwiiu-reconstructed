#include "game/ui/UIScene_HowToPlay.hpp"
#include "game/core/StringIds.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <new>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/Font.hpp"
#include "game/ui/UIString.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/Potions.hpp"
#include "game/core/DyeColor.hpp"
#include "game/item/Items.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/BowItem.hpp"
#include "game/item/FishingRodItem.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/ShearsItem.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_HowToPlay.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/GrassBlock.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/StoneBlock.hpp"
#include "game/world/block/SandStoneBlock.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/StringUtils.hpp"

extern ConsoleUIController gConsoleUIController;

void replaceAll(std::wstring& result, const std::wstring& source,
                const std::wstring& search, const std::wstring& replacement);

template <class T>
const UIScene_HowToPlay::sCustomItems SLOT(T item, int count, int auxValue)
{
    return UIScene_HowToPlay::sCustomItems(item, count, auxValue);
}

template <class T, class P>
const UIScene_HowToPlay::sCustomItems SLOT_POTION(T item, P potion, int count, int auxValue)
{
    return UIScene_HowToPlay::sCustomItems(item, potion, count, auxValue);
}

UIScene_HowToPlay::sCustomItems::sCustomItems(Item* item, int count, int auxValue)
    : m_block(NULL), m_item(item), m_potion(NULL), m_count(count), m_auxValue(auxValue)
{
}

UIScene_HowToPlay::sCustomItems::sCustomItems(Block* block, int count, int auxValue)
    : m_block(block), m_item(NULL), m_potion(NULL), m_count(count), m_auxValue(auxValue)
{
}

UIScene_HowToPlay::sCustomItems::sCustomItems(int blockId, int count, int auxValue)
    : m_block(Block::byId(blockId)), m_item(NULL), m_potion(NULL), m_count(count), m_auxValue(auxValue)
{
}

UIScene_HowToPlay::sCustomItems::sCustomItems(Item* item, Potion* potion, int count, int auxValue)
    : m_block(NULL), m_item(item), m_potion(potion), m_count(count), m_auxValue(auxValue)
{
}

not_null_ptr<ItemInstance> UIScene_HowToPlay::sCustomItems::GetItemInstance() const
{
    not_null_ptr<ItemInstance> instance;
    if (m_item == NULL)
        instance = not_null_ptr<ItemInstance>(new ItemInstance(m_block, m_count, m_auxValue));
    else
        instance = not_null_ptr<ItemInstance>(new ItemInstance(m_item, m_count, m_auxValue));
    if (m_potion != NULL)
        instance = PotionUtils::setPotion(instance, m_potion);
    return instance;
}

UIScene_HowToPlay::UIScene_HowToPlay(int playerId, void* data, UILayer* layer)
    : UIScene(playerId, layer), m_page(0), m_pageCount(0), m_container(), m_dynamicHtmlText(), m_labels(),
      m_enchantmentBook(), m_enchantmentBooks()
{
    initialiseMovie();

    std::wstring repeated(L10N::GetString(IDS_INVENTORY));

    m_labels[0].init(UIString(repeated));
    m_labels[1].init(UIString(repeated));
    m_labels[2].init(UIString(L10N::GetString(IDS_CHEST)));
    m_labels[3].init(UIString(repeated));
    m_labels[4].init(UIString(L10N::GetString(IDS_CHEST_LARGE)));
    m_labels[5].init(UIString(L10N::GetString(IDS_FUEL)));
    m_labels[6].init(UIString(repeated));
    m_labels[7].init(UIString(L10N::GetString(IDS_INGREDIENT)));
    m_labels[8].init(UIString(L10N::GetString(IDS_FURNACE)));
    m_labels[9].init(UIString(L10N::GetString(IDS_DISPENSER)));
    m_labels[10].init(UIString(repeated));
    m_labels[11].init(UIString(L10N::GetString(0xfbc6e08)));
    m_labels[12].init(UIString(L10N::GetString(IDS_ENCHANT)));
    m_labels[13].init(UIString(repeated));
    m_labels[14].init(UIString(L10N::GetString(IDS_BREWING_STAND)));
    m_labels[15].init(UIString(repeated));
    m_labels[16].init(UIString(repeated));

    {
        std::wstring text(L10N::GetString(0xf3a668fd));
        std::wstring::size_type pos = text.find(L"%d");
        if (pos != std::wstring::npos)
            text.replace(pos, 2, L"8");
        m_labels[17].init(UIString(text));
    }

    m_labels[18].init(UIString(L10N::GetString(0xc60bb915)));
    m_labels[19].init(UIString(repeated));
    m_labels[20].init(UIString(L10N::GetString(IDS_ITEM_EMERALD)));
    m_labels[21].init(UIString(L10N::GetString(IDS_REQUIRED_ITEMS_FOR_TRADE)));

    {
        std::wstring text(L10N::GetString(0x8c04d144));
        std::wstring result;
        replaceAll(result, text, L"_VILLAGER_TYPE_", L10N::GetString(IDS_VILLAGER_CLERIC));
        text = result;
        std::wstring::size_type pos = text.find(L"%s");
        if (pos != std::wstring::npos)
            text.replace(pos, 2, L10N::GetString(0x5ae2b631));
        m_labels[22].init(UIString(text));
    }

    m_labels[23].init(UIString(L10N::GetString(0xb3c82373)));
    m_labels[24].init(UIString(L10N::GetString(0x7b8c1d5d)));
    m_labels[25].init(UIString(L10N::GetString(IDS_TILE_HOPPER)));
    m_labels[26].init(UIString(repeated));
    m_labels[27].init(UIString(L10N::GetString(IDS_TILE_DROPPER)));
    m_labels[28].init(UIString(repeated));

    m_pageCount = 0;
    StartPage(static_cast<EHowToPlayPage>((reinterpret_cast<unsigned int>(data) >> 0x10) & 0xfff));
}

UIScene_HowToPlay::~UIScene_HowToPlay()
{
}

namespace
{
    static const unsigned int kHowToPlayTitleHash[EHowToPlayPage_Count] = {
        0x018d8ea9, IDS_HOW_TO_PLAY_BASICS, IDS_HOW_TO_PLAY_MULTIPLAYER, IDS_HOW_TO_PLAY_HUD, 0xcef2334f, 0x5ceac414,
        0x82ed3e7d, IDS_HOW_TO_PLAY_CREATIVE, IDS_HOW_TO_PLAY_INVENTORY, IDS_HOW_TO_PLAY_CHEST, 0xbc353d00, IDS_HOW_TO_PLAY_ENDERCHEST,
        IDS_HOW_TO_PLAY_CRAFTING, IDS_HOW_TO_PLAY_CRAFT_TABLE, IDS_HOW_TO_PLAY_FURNACE, IDS_HOW_TO_PLAY_DISPENSER, IDS_HOW_TO_PLAY_BREWING, 0x5ece0eed,
        IDS_HOW_TO_PLAY_ANVIL, 0x59f9a3a9, 0x730761ed, IDS_HOW_TO_PLAY_TRADING, IDS_HOW_TO_PLAY_HORSES, IDS_HOW_TO_PLAY_BEACONS,
        IDS_HOW_TO_PLAY_FIREWORKS, IDS_HOW_TO_PLAY_BANNERS, IDS_HOW_TO_PLAY_HOPPERS, IDS_HOW_TO_PLAY_DROPPERS, 0xad1e1a90, 0xc81b96aa,
        0x93eac97a, IDS_HOW_TO_PLAY_ELYTRA, IDS_HOW_TO_PLAY_OCEANS, IDS_HOW_TO_PLAY_OCEAN_MOBS, IDS_HOW_TO_PLAY_CONDUIT, 0x8c55bd04
    };

    static const int kHowToPlayContentPage[EHowToPlayPage_Count] = {
        0x00, 0x01, 0x02, 0x03, 0x01, 0x02, 0x01, 0x04, 0x05, 0x06,
        0x07, 0x17, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x15, 0x0e,
        0x0f, 0x16, 0x18, 0x19, 0x01, 0x02, 0x1b, 0x1c, 0x10, 0x11,
        0x01, 0x02, 0x01, 0x02, 0x01, 0x14
    };
}

void UIScene_HowToPlay::StartPage(EHowToPlayPage page)
{
    m_page = static_cast<int>(page);

    const wchar_t* title = L10N::GetString(static_cast<int>(kHowToPlayTitleHash[m_page]));
    std::wstring text(title == NULL ? L"" : title);
    text = gApp.FormatHTMLString(m_playerId, text, -1);

    replaceAll(text, text, L"_KICK_PLAYER_DESCRIPTION_", L10N::GetString(IDS_KICK_PLAYER_DESCRIPTION));
    replaceAll(text, text, L"_BACK_BUTTON_", L10N::GetString(0x3d497286));
    replaceAll(text, text, L"_DISABLES_ACHIEVEMENTS_", L10N::GetString(IDS_HOST_OPTION_DISABLES_ACHIEVEMENTS));
    replaceAll(text, text, L"_PLATFORM_NAME_", L10N::GetString(0xed3c4bfb));

    if (!gConsoleUIController.UsingBitmapFont())
    {
        replaceAll(text, text, L"(C)", L"©");
        replaceAll(text, text, L"®", L"(R)");
        replaceAll(text, text, L"–", L"-");
    }

    stripWhitespaceForHtml(text, true);

    wchar_t colorTag[64];
    std::swprintf(colorTag, 0x40, L"<font color=#%08x>",
                  gApp.GetHTMLColour(eMinecraftColour_HTMLColor_White));
    std::wstring coloured(colorTag);
    coloured += text;

    std::vector<std::wstring> pages;
    std::wstring::size_type start = 0;
    std::wstring::size_type pos = coloured.find(L"\r\n");
    while (pos != std::wstring::npos)
    {
        pages.push_back(coloured.substr(start, pos - start) + L" ");
        start = pos + 2;
        pos = coloured.find(L"\r\n", start);
    }
    pages.push_back(coloured.substr(start, coloured.length() - start));

    if (m_movie != NULL && m_movie->getRootNode() != NULL)
    {
        FJ_HowToPlay* node = dynamic_cast<FJ_HowToPlay*>(m_movie->getRootNode()->getFuiNode());
        if (node != NULL)
            LoadHowToPlayPage(node, kHowToPlayContentPage[m_page], pages);
    }

    updateTooltips();
    gConsoleUIController.TouchBoxRebuild(this);
}

unsigned int UIScene_HowToPlay::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    static const char* const names[30] = {
        "Label1_5", "Label2_6", "Label1_6", "Label2_7", "Label1_7", "Label3_10",
        "Label4_10", "Label2_10", "Label1_10", "Label1_11", "Label2_11", "Label1_4",
        "Label1_13", "Label2_13", "Label1_12", "Label2_12", "Label3_21", "Label2_21",
        "Label1_21", "Label3_22", NULL, "Label4_22", "Label2_22", "Label1_22",
        "Label1_25", "Label2_25", "Label1_27", "Label2_27", "Label1_28", "Label2_28"
    };
    m_dynamicHtmlText.setupControl(this, root, "DynamicHtmlText");
    m_container.setupControl(this, root, "HowToPlayContainer_13");
    m_enchantmentBook.setupControl(this, m_container.getFuiRenderNode(), "iggy_EnchantmentBook");
    m_enchantmentBooks[0].setupControl(this, m_container.getFuiRenderNode(), "iggy_slot_Button_1");
    m_enchantmentBooks[1].setupControl(this, m_container.getFuiRenderNode(), "iggy_slot_Button_2");
    m_enchantmentBooks[2].setupControl(this, m_container.getFuiRenderNode(), "iggy_slot_Button_3");
    int i;
    for (i = 0; i < 30; ++i)
    {
        if (names[i] == NULL)
            continue;
        m_labels[i].setupControl(this, root, names[i]);
    }
    m_controls.push_back(&m_dynamicHtmlText);
    m_controls.push_back(&m_container);
    m_controls.push_back(&m_enchantmentBook);
    m_controls.push_back(&m_enchantmentBooks[0]);
    m_controls.push_back(&m_enchantmentBooks[1]);
    m_controls.push_back(&m_enchantmentBooks[2]);
    m_controls.push_back(&m_labels[8]);
    m_controls.push_back(&m_labels[7]);
    m_controls.push_back(&m_labels[5]);
    m_controls.push_back(&m_labels[6]);
    m_controls.push_back(&m_labels[4]);
    m_controls.push_back(&m_labels[3]);
    m_controls.push_back(&m_labels[11]);
    m_controls.push_back(&m_labels[2]);
    m_controls.push_back(&m_labels[1]);
    m_controls.push_back(&m_labels[0]);
    m_controls.push_back(&m_labels[9]);
    m_controls.push_back(&m_labels[10]);
    m_controls.push_back(&m_labels[12]);
    m_controls.push_back(&m_labels[13]);
    m_controls.push_back(&m_labels[14]);
    m_controls.push_back(&m_labels[15]);
    m_controls.push_back(&m_labels[23]);
    m_controls.push_back(&m_labels[22]);
    m_controls.push_back(&m_labels[19]);
    m_controls.push_back(&m_labels[21]);
    m_controls.push_back(&m_labels[18]);
    m_controls.push_back(&m_labels[17]);
    m_controls.push_back(&m_labels[16]);
    m_controls.push_back(&m_labels[24]);
    m_controls.push_back(&m_labels[25]);
    m_controls.push_back(&m_labels[26]);
    m_controls.push_back(&m_labels[27]);
    m_controls.push_back(&m_labels[28]);
    m_controls.push_back(&m_labels[29]);
    return 1;
}

void UIScene_HowToPlay::updateTooltips()
{
    int first = -1;
    if (m_page == EHowToPlayPage_WhatsNew)
        gConsoleUIController.SetTooltips(m_playerId, IDS_HOW_TO_PLAY_NEXT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
    else
    {
        if (m_page != EHowToPlayPage_HostOptions)
            first = IDS_HOW_TO_PLAY_NEXT;
        gConsoleUIController.SetTooltips(m_playerId, first, IDS_TOOLTIPS_BACK, 0x403e4e5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
    }
}

std::wstring UIScene_HowToPlay::getMoviePath()
{
    return gApp.GetLocalPlayerCount() < 2 ? L"HowToPlay" : L"HowToPlaySplit";
}

void UIScene_HowToPlay::handleReload()
{
    if (m_movie != NULL && m_movie->getRootNode() != NULL)
    {
        FJ_HowToPlay* node = dynamic_cast<FJ_HowToPlay*>(m_movie->getRootNode()->getFuiNode());
        if (node != NULL)
            node->InitScene();
    }
    StartPage(static_cast<EHowToPlayPage>(m_page));
}

void UIScene_HowToPlay::handleInput(int, int action, bool repeat, bool released, bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (action == EControllerActions_MenuA && released)
    {
        if (m_page != EHowToPlayPage_HostOptions)
            StartPage(static_cast<EHowToPlayPage>(m_page + 1));
        handled = true;
    }
    else if (action == EControllerActions_MenuX && released)
    {
        if (m_pageCount <= m_page - 1)
            StartPage(static_cast<EHowToPlayPage>(m_page - 1));
        handled = true;
    }
    else if (action == EControllerActions_MenuCancel && released)
    {
        navigateBack();
        handled = true;
    }
    else if (action == EControllerActions_MenuUp || action == EControllerActions_MenuDown || action == EControllerActions_MenuPageUp || action == EControllerActions_MenuPageDown)
        sendInputToMovie(action, repeat, released, pressed);
}

void UIScene_HowToPlay::customDrawEnchantmentFont(fuiRect* rect, int variant)
{
    if (rect == NULL)
        return;
    const float width = static_cast<float>(m_enchantmentBooks[0].getWidth());
    const float height = static_cast<float>(m_enchantmentBooks[0].getHeight());
    GlStateManager::scalef((rect->xMax - rect->xMin) / width, (rect->yMax - rect->yMin) / height, 1.0F);
    GlStateManager::enableAlphaTest();
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != NULL && minecraft->mFont != NULL)
    {
        std::wstring text = variant == 2 ? L"elemental spirit animal" : (variant == 1 ? L"klaatu berata niktu" : L"the elder scrolls");
        minecraft->mFont->drawWordWrap(text, 0, 0, static_cast<int>(width),
                                       gApp.GetHTMLColour(eMinecraftColour_HTMLColor_White) | 0xff000000);
    }
    GlStateManager::disableAlphaTest();
    GlStateManager::disableRescaleNormal();
}

void UIScene_HowToPlay::customDraw(const char* name, fuiRect* rect)
{
    int slot = -1;
    if (name != NULL)
        std::sscanf(name, "slot_%d", &slot);
    if (m_page == EHowToPlayPage_Enchanting && name != NULL && std::strncmp(name, "px_", 3) == 0)
        m_enchantmentBook.render(rect);
    if (slot >= 0 && slot < 3)
        customDrawEnchantmentFont(rect, slot);
}

int UIScene_HowToPlay::getSceneType()
{
    return EUIScene_HowToPlay;
}

unsigned int UIScene_HowToPlay::getSubSceneType() const
{
    return static_cast<unsigned int>(m_page);
}

const UIScene_HowToPlay::sCustomItems* SlotListHotBar()
{
    static const UIScene_HowToPlay::sCustomItems slots[10] = {
        SLOT(Items::ironSword, 1, 0),
        SLOT(Items::bow, 1, 0),
        SLOT(Items::arrow, 64, 0),
        SLOT(Items::flintAndSteel, 1, 30),
        SLOT(Items::clock, 1, 0),
        SLOT(Items::shears, 1, 0),
        SLOT(Items::ironAxe, 1, 50),
        SLOT(Items::fishingRod, 1, 0),
        SLOT(Items::compass, 1, 0),
        SLOT(Items::filledMap, 1, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListInventory()
{
    static const UIScene_HowToPlay::sCustomItems slots[27] = {
        SLOT(Blocks::stone, 32, 0),
        SLOT(Blocks::planks, 17, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Blocks::fence_gate, 1, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::arrow, 64, 0),
        SLOT(Items::stick, 10, 0),
        SLOT(Blocks::bookshelf, 3, 0),
        SLOT(0, 0, 0),
        SLOT(Items::darkOakDoor, 1, 0),
        SLOT(0, 0, 0),
        SLOT(Items::emerald, 1, 0),
        SLOT(0, 0, 0),
        SLOT(Blocks::cobblestone_wall, 7, 0),
        SLOT(Items::sign, 1, 0),
        SLOT(0, 0, 0),
        SLOT(Blocks::stone_brick_stairs, 24, 0),
        SLOT(Blocks::stone_stairs, 16, 0),
        SLOT(0, 0, 0),
        SLOT(Blocks::log, 7, 0),
        SLOT(Blocks::acacia_stairs, 15, 0),
        SLOT(0, 0, 0),
        SLOT(Items::diamondPickaxe, 1, 0),
        SLOT(Items::paper, 21, 0),
        SLOT(Items::gunpowder, 15, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListChestSmall()
{
    static const UIScene_HowToPlay::sCustomItems slots[27] = {
        SLOT(Items::experienceBottle, 64, 0),
        SLOT(Blocks::wooden_slab, 5, 0),
        SLOT(0, 0, 0),
        SLOT(Items::glassBottle, 15, 0),
        SLOT(Blocks::enchanting_table, 1, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::wheat, 7, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Blocks::anvil, 1, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Blocks::cobblestone, 12, 0),
        SLOT(0, 0, 0),
        SLOT(Items::goldenApple, 1, 0),
        SLOT(0, 0, 0),
        SLOT(Items::stick, 12, 0),
        SLOT(0, 0, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListChestLarge()
{
    static const UIScene_HowToPlay::sCustomItems slots[54] = {
        SLOT(Blocks::glass_pane, 12, 0),
        SLOT(Items::ironDoor, 1, 0),
        SLOT(Items::pumpkinSeeds, 3, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::ironHoe, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Blocks::glass, 2, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::stoneAxe, 1, 175),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Blocks::cobblestone, 54, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::bone, 2, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::ironHelmet, 1, 0),
        SLOT(Items::ironChestplate, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::string, 3, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::woodenShovel, 1, 50),
        SLOT(Items::ironIngot, 0, 0),
        SLOT(Blocks::dirt, 6, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::leather, 2, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::birchDoor, 6, 0),
        SLOT(0, 0, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListAnvil()
{
    static const UIScene_HowToPlay::sCustomItems slots[3] = {
        SLOT(Items::ironSword, 1, 150),
        SLOT(Items::ironSword, 1, 0),
        SLOT(Items::ironSword, 1, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListBeacon()
{
    static const UIScene_HowToPlay::sCustomItems slots[5] = {
        SLOT(Items::emerald, 1, 0),
        SLOT(Items::diamond, 1, 0),
        SLOT(Items::goldIngot, 1, 0),
        SLOT(Items::ironIngot, 1, 0),
        SLOT(Items::ironIngot, 1, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListBrewing()
{
    static const UIScene_HowToPlay::sCustomItems slots[5] = {
        SLOT(Items::fermentedSpiderEye, 1, 0),
        SLOT_POTION(Items::potion, Potions::WATER, 1, 0),
        SLOT_POTION(Items::potion, Potions::WATER, 1, 0),
        SLOT_POTION(Items::potion, Potions::WATER, 1, 0),
        SLOT(Items::blazePowder, 3, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListDispenser()
{
    static const UIScene_HowToPlay::sCustomItems slots[9] = {
        SLOT(Items::arrow, 64, 0),
        SLOT(Items::arrow, 64, 0),
        SLOT(Items::arrow, 64, 0),
        SLOT(Items::arrow, 64, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListEntchantment()
{
    static const UIScene_HowToPlay::sCustomItems slots[2] = {
        SLOT(Items::bow, 1, 0),
        SLOT(Items::dye, 7, 4)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListFurnace()
{
    static const UIScene_HowToPlay::sCustomItems slots[3] = {
        SLOT(Blocks::iron_ore, 61, 0),
        SLOT(Items::coal, 63, 0),
        SLOT(Items::ironIngot, 3, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListHopper()
{
    static const UIScene_HowToPlay::sCustomItems slots[5] = {
        SLOT(Blocks::torch, 3, 0),
        SLOT(Blocks::gold_block, 1, 0),
        SLOT(Items::redstone, 32, 0),
        SLOT(Items::arrow, 32, 0),
        SLOT(Items::glowstoneDust, 12, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListArmor()
{
    static const UIScene_HowToPlay::sCustomItems slots[4] = {
        SLOT(Items::ironHelmet, 1, 0),
        SLOT(Items::leatherChestplate, 1, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListTrading()
{
    static const UIScene_HowToPlay::sCustomItems slots[9] = {
        SLOT(Items::emerald, 1, 0),
        SLOT(Items::emerald, 1, 0),
        SLOT(Items::redstone, 2, 0),
        SLOT(Items::dye, 1, DyeColor::BLUE->getItemData()),
        SLOT(Items::enderEye, 1, 0),
        SLOT(Blocks::glowstone, 3, 0),
        SLOT(0, 0, 0),
        SLOT(Items::emerald, 1, 0),
        SLOT(0, 0, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListCreative()
{
    static const UIScene_HowToPlay::sCustomItems slots[50] = {
        SLOT(Blocks::stone, 1, 0),
        SLOT(Blocks::grass, 1, 0),
        SLOT(Blocks::grass_path, 1, 0),
        SLOT(Blocks::dirt, 1, 0),
        SLOT(Blocks::cobblestone, 1, 0),
        SLOT(Blocks::sand, 1, 0),
        SLOT(Blocks::sandstone, 1, SandStoneBlock::SANDSTONE->getData()),
        SLOT(Blocks::sandstone, 1, SandStoneBlock::SMOOTH->getData()),
        SLOT(Blocks::sandstone, 1, SandStoneBlock::CHISELED->getData()),
        SLOT(Blocks::sand, 1, SandBlock::Variant::RED_SAND->getData()),
        SLOT(Blocks::red_sandstone, 1, SandStoneBlock::SANDSTONE->getData()),
        SLOT(Blocks::red_sandstone, 1, SandStoneBlock::SMOOTH->getData()),
        SLOT(Blocks::red_sandstone, 1, SandStoneBlock::CHISELED->getData()),
        SLOT(Blocks::stone, 1, StoneBlock::Variant::GRANITE->getData()),
        SLOT(Blocks::stone, 1, StoneBlock::Variant::SMOOTH_GRANITE->getData()),
        SLOT(Blocks::stone, 1, StoneBlock::Variant::ANDESITE->getData()),
        SLOT(Blocks::stone, 1, StoneBlock::Variant::SMOOTH_ANDESITE->getData()),
        SLOT(Blocks::stone, 1, StoneBlock::Variant::DIORITE->getData()),
        SLOT(Blocks::stone, 1, StoneBlock::Variant::SMOOTH_DIORITE->getData()),
        SLOT(Blocks::coal_block, 1, 0),
        SLOT(Blocks::gold_block, 1, 0),
        SLOT(Blocks::iron_block, 1, 0),
        SLOT(Blocks::lapis_block, 1, 0),
        SLOT(Blocks::diamond_block, 1, 0),
        SLOT(Blocks::emerald_block, 1, 0),
        SLOT(Blocks::quartz_block, 1, 0),
        SLOT(Blocks::coal_ore, 1, 0),
        SLOT(Blocks::lapis_ore, 1, 0),
        SLOT(Blocks::diamond_ore, 1, 0),
        SLOT(Blocks::redstone_ore, 1, 0),
        SLOT(Blocks::iron_ore, 1, 0),
        SLOT(Blocks::gold_ore, 1, 0),
        SLOT(Blocks::emerald_ore, 1, 0),
        SLOT(Blocks::quartz_ore, 1, 0),
        SLOT(Blocks::nether_wart_block, 1, 0),
        SLOT(Blocks::bedrock, 1, 0),
        SLOT(Blocks::planks, 1, PlanksBlock::Variant::OAK->getData()),
        SLOT(Blocks::planks, 1, PlanksBlock::Variant::SPRUCE->getData()),
        SLOT(Blocks::planks, 1, PlanksBlock::Variant::BIRCH->getData()),
        SLOT(Blocks::planks, 1, PlanksBlock::Variant::JUNGLE->getData()),
        SLOT(Blocks::planks, 1, PlanksBlock::Variant::ACACIA->getData()),
        SLOT(Blocks::planks, 1, PlanksBlock::Variant::DARK_OAK->getData()),
        SLOT(Blocks::log, 1, PlanksBlock::Variant::OAK->getData()),
        SLOT(Blocks::log, 1, PlanksBlock::Variant::SPRUCE->getData()),
        SLOT(Blocks::log, 1, PlanksBlock::Variant::BIRCH->getData()),
        SLOT(Blocks::log, 1, PlanksBlock::Variant::JUNGLE->getData()),
        SLOT(Blocks::log2, 1, PlanksBlock::Variant::ACACIA->getData() - 4),
        SLOT(Blocks::log2, 1, PlanksBlock::Variant::DARK_OAK->getData() - 4),
        SLOT(Blocks::gravel, 1, 0),
        SLOT(Blocks::brick_block, 1, 0)
    };
    return slots;
}
