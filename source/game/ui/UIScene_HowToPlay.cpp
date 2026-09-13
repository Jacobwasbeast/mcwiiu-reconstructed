#include "game/ui/UIScene_HowToPlay.hpp"

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

extern ConsoleUIController gConsoleUIController;

void replaceAll(std::wstring& result, const std::wstring& source,
                const std::wstring& search, const std::wstring& replacement);
void stripWhitespaceForHtml(std::wstring& text, bool collapse);

namespace
{
    typedef UIScene_HowToPlay::sCustomItems CustomItem;

    static CustomItem makeItem(Item* item, int count, int aux)
    {
        return CustomItem(item, aux, count);
    }

    static CustomItem makeBlock(Block* block, int count, int aux)
    {
        return CustomItem(block, aux, count);
    }

    static CustomItem makePotion(Item* item, Potion* potion, int count, int aux)
    {
        return CustomItem(item, potion, aux, count);
    }
}

template <typename T>
void SLOT(UIScene_HowToPlay::sCustomItems* destination, T item, int count, int aux)
{
    new (destination) UIScene_HowToPlay::sCustomItems(reinterpret_cast<Item*>(item), aux, count);
}

template <>
void SLOT<int>(UIScene_HowToPlay::sCustomItems* destination, int item, int count, int aux)
{
    new (destination) UIScene_HowToPlay::sCustomItems(item, aux, count);
}

template <>
void SLOT<Block*>(UIScene_HowToPlay::sCustomItems* destination, Block* item, int count, int aux)
{
    new (destination) UIScene_HowToPlay::sCustomItems(item, aux, count);
}

template <>
void SLOT<Potion*>(UIScene_HowToPlay::sCustomItems* destination, Potion* item, int count, int aux)
{
    new (destination) UIScene_HowToPlay::sCustomItems(reinterpret_cast<Item*>(item), aux, count);
}

template <typename T>
void SLOT(UIScene_HowToPlay::sCustomItems* destination, Item* item, Potion* potion,
          int count, int aux)
{
    (void)sizeof(T);
    new (destination) UIScene_HowToPlay::sCustomItems(item, potion, aux, count);
}

UIScene_HowToPlay::sCustomItems::sCustomItems(Item* item, int auxValue, int count)
    : m_block(NULL), m_item(item), m_potion(NULL), m_auxValue(auxValue), m_count(count)
{
}

UIScene_HowToPlay::sCustomItems::sCustomItems(Block* block, int auxValue, int count)
    : m_block(block), m_item(NULL), m_potion(NULL), m_auxValue(auxValue), m_count(count)
{
}

UIScene_HowToPlay::sCustomItems::sCustomItems(int blockId, int auxValue, int count)
    : m_block(Block::byId(blockId)), m_item(NULL), m_potion(NULL), m_auxValue(auxValue), m_count(count)
{
}

UIScene_HowToPlay::sCustomItems::sCustomItems(Item* item, Potion* potion, int auxValue, int count)
    : m_block(NULL), m_item(item), m_potion(potion), m_auxValue(auxValue), m_count(count)
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

    std::wstring repeated(L10N::GetString(0x14630b9e));

    m_labels[0].init(UIString(repeated));
    m_labels[1].init(UIString(repeated));
    m_labels[2].init(UIString(L10N::GetString(0x24fee045)));
    m_labels[3].init(UIString(repeated));
    m_labels[4].init(UIString(L10N::GetString(0x68521aaf)));
    m_labels[5].init(UIString(L10N::GetString(0x9f31c0ee)));
    m_labels[6].init(UIString(repeated));
    m_labels[7].init(UIString(L10N::GetString(0x7d7816a1)));
    m_labels[8].init(UIString(L10N::GetString(0x1b49a944)));
    m_labels[9].init(UIString(L10N::GetString(0xe398f425)));
    m_labels[10].init(UIString(repeated));
    m_labels[11].init(UIString(L10N::GetString(0xfbc6e08)));
    m_labels[12].init(UIString(L10N::GetString(0xf4bd500f)));
    m_labels[13].init(UIString(repeated));
    m_labels[14].init(UIString(L10N::GetString(0xdf516f15)));
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
    m_labels[20].init(UIString(L10N::GetString(0xf681ddf0)));
    m_labels[21].init(UIString(L10N::GetString(0x3dbed919)));

    {
        std::wstring text(L10N::GetString(0x8c04d144));
        std::wstring result;
        replaceAll(result, text, L"_VILLAGER_TYPE_", L10N::GetString(0x869e763f));
        text = result;
        std::wstring::size_type pos = text.find(L"%s");
        if (pos != std::wstring::npos)
            text.replace(pos, 2, L10N::GetString(0x5ae2b631));
        m_labels[22].init(UIString(text));
    }

    m_labels[23].init(UIString(L10N::GetString(0xb3c82373)));
    m_labels[24].init(UIString(L10N::GetString(0x7b8c1d5d)));
    m_labels[25].init(UIString(L10N::GetString(0x7c89fe37)));
    m_labels[26].init(UIString(repeated));
    m_labels[27].init(UIString(L10N::GetString(0x25cba5e1)));
    m_labels[28].init(UIString(repeated));

    m_pageCount = 0;
    StartPage(static_cast<EHowToPlayPage>((reinterpret_cast<unsigned int>(data) >> 0x10) & 0xfff));
}

UIScene_HowToPlay::~UIScene_HowToPlay()
{
}

namespace
{
    static const unsigned int kHowToPlayTitleHash[36] = {
        0x018d8ea9, 0x8758a585, 0x0433e4c6, 0x8941a6ad, 0xcef2334f, 0x5ceac414,
        0x82ed3e7d, 0xddf4faf7, 0x8e8b1966, 0x2913d7cd, 0xbc353d00, 0xe160391d,
        0x58ef7c86, 0xb6ab3c0f, 0xc6ebf0ec, 0xc6f80b3d, 0x186eb58e, 0x5ece0eed,
        0xfcf1d650, 0x59f9a3a9, 0x730761ed, 0xdd493e2f, 0x2ff78ffc, 0x23733dbb,
        0xdd5bbbbe, 0xe146e1c3, 0xd3c04577, 0xaa5bd119, 0xad1e1a90, 0xc81b96aa,
        0x93eac97a, 0xe91b67db, 0xbe472149, 0x580332f6, 0x24c276da, 0x8c55bd04
    };

    static const int kHowToPlayContentPage[36] = {
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

    replaceAll(text, text, L"_KICK_PLAYER_DESCRIPTION_", L10N::GetString(0x1350b613));
    replaceAll(text, text, L"_BACK_BUTTON_", L10N::GetString(0x3d497286));
    replaceAll(text, text, L"_DISABLES_ACHIEVEMENTS_", L10N::GetString(0xbf244113));
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
                  gApp.GetHTMLColour(static_cast<eMinecraftColour>(0x1b5)));
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
    fuiRenderNode* root = m_movie == NULL ? NULL : m_movie->getRootNode();
    if (root == NULL)
        return 0;
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
        m_controls.push_back(&m_labels[i]);
    }
    m_controls.push_back(&m_dynamicHtmlText);
    m_controls.push_back(&m_container);
    m_controls.push_back(&m_enchantmentBook);
    m_controls.push_back(&m_enchantmentBooks[0]);
    m_controls.push_back(&m_enchantmentBooks[1]);
    m_controls.push_back(&m_enchantmentBooks[2]);
    return 1;
}

void UIScene_HowToPlay::updateTooltips()
{
    int first = -1;
    if (m_page == 0)
        gConsoleUIController.SetTooltips(m_controlFocus, 0xa3ab7b4b, 0x4f6bcf5e, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
    else
    {
        if (m_page != 0x23)
            first = 0xa3ab7b4b;
        gConsoleUIController.SetTooltips(m_controlFocus, first, 0x4f6bcf5e, 0x403e4e5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
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

void UIScene_HowToPlay::handleInput(int, int action, bool, bool released, bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_controlFocus, action, false, released, pressed);
    if (action == 0 && pressed && m_page != 0x23)
    {
        StartPage(static_cast<EHowToPlayPage>(m_page + 1));
        handled = true;
    }
    else if (action == 2 && pressed && m_pageCount <= m_page - 1)
    {
        StartPage(static_cast<EHowToPlayPage>(m_page - 1));
        handled = true;
    }
    else if (action == 0x15 && pressed)
    {
        navigateBack();
        handled = true;
    }
    else if (action < 4 || action > 5 && action < 8 || action > 9)
        sendInputToMovie(action, false, released, pressed);
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
                                       gApp.GetHTMLColour(static_cast<eMinecraftColour>(0x1b5)) | 0xff000000);
    }
    GlStateManager::disableAlphaTest();
    GlStateManager::disableRescaleNormal();
}

void UIScene_HowToPlay::customDraw(const char* name, fuiRect* rect)
{
    int slot = -1;
    if (name != NULL)
        std::sscanf(name, "slot_%d", &slot);
    if (m_page == 0x11 && name != NULL && std::strncmp(name, "px_", 3) == 0)
        m_enchantmentBook.render(rect);
    if (slot >= 0 && slot < 3)
        customDrawEnchantmentFont(rect, slot);
}

int UIScene_HowToPlay::getSceneType()
{
    return 0x10;
}

unsigned int UIScene_HowToPlay::getSubSceneType() const
{
    return static_cast<unsigned int>(m_page);
}

const UIScene_HowToPlay::sCustomItems* SlotListAnvil__Fv()
{
    static CustomItem slots[3] = {
        makeItem(Items::ironIngot, 1, 150), makeItem(Items::ironIngot, 1, 0),
        makeItem(Items::ironIngot, 1, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListArmor__Fv()
{
    static CustomItem slots[4] = {
        makeItem(Items::ironHelmet, 1, 0), makeItem(Items::ironChestplate, 1, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListBeacon__Fv()
{
    static CustomItem slots[5] = {
        makeItem(Items::emerald, 1, 0), makeItem(Items::ironIngot, 1, 0),
        makeItem(Items::goldIngot, 1, 0), makeItem(Items::diamond, 1, 0),
        makeItem(Items::diamond, 1, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListBrewing__Fv()
{
    static CustomItem slots[5] = {
        makeItem(Items::glassBottle, 1, 0),
        makePotion(Items::potion, Potion::byName(L"healing"), 1, 0),
        makePotion(Items::potion, Potion::byName(L"healing"), 1, 0),
        makePotion(Items::potion, Potion::byName(L"healing"), 1, 0),
        makeItem(Items::blazePowder, 3, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListChestLarge__Fv()
{
    static CustomItem slots[54] = {
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0)
    };
    slots[0] = makeBlock(Blocks::chest, 12, 0);
    slots[1] = makeItem(Items::diamond, 1, 0);
    slots[2] = makeItem(Items::emerald, 3, 0);
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListChestSmall__Fv()
{
    static CustomItem slots[27] = {
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0)
    };
    slots[0] = makeBlock(Blocks::chest, 1, 0);
    slots[1] = makeItem(Items::diamond, 1, 0);
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListCreative__Fv()
{
    static CustomItem slots[50] = {
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0)
    };
    slots[0] = makeBlock(Blocks::stone, 1, 0);
    slots[1] = makeBlock(Blocks::grass, 1, 0);
    slots[2] = makeBlock(Blocks::dirt, 1, 0);
    slots[3] = makeBlock(Blocks::cobblestone, 1, 0);
    slots[4] = makeBlock(Blocks::planks, 1, 0);
    slots[5] = makeBlock(Blocks::sand, 1, 0);
    slots[6] = makeBlock(Blocks::glass, 1, 0);
    slots[7] = makeBlock(Blocks::chest, 1, 0);
    slots[8] = makeItem(Items::ironIngot, 1, 0);
    slots[9] = makeItem(Items::goldIngot, 1, 0);
    slots[10] = makeItem(Items::diamond, 1, 0);
    slots[11] = makeItem(Items::emerald, 1, 0);
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListDispenser__Fv()
{
    static CustomItem slots[9] = {
        makeItem(Items::diamond, 64, 0), makeItem(Items::diamond, 64, 0),
        makeItem(Items::diamond, 64, 0), makeItem(Items::diamond, 64, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListEntchantment__Fv()
{
    static CustomItem slots[2] = {
        makeItem(Items::bow, 1, 0), makeItem(Items::enchantedBook, 7, 4)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListFurnace__Fv()
{
    static CustomItem slots[3] = {
        makeBlock(Blocks::coal_ore, 61, 0), makeItem(Items::coal, 63, 0),
        makeItem(Items::goldIngot, 3, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListHopper__Fv()
{
    static CustomItem slots[5] = {
        makeBlock(Blocks::iron_block, 3, 0), makeBlock(Blocks::iron_block, 1, 0),
        makeItem(Items::diamond, 32, 0), makeItem(Items::diamond, 32, 0),
        makeItem(Items::ironIngot, 12, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListHotBar__Fv()
{
    static CustomItem slots[10] = {
        makeItem(Items::diamond, 1, 0), makeItem(Items::bow, 1, 0),
        makeItem(Items::diamond, 64, 0), makeItem(Items::diamond, 1, 30),
        makeItem(Items::diamond, 1, 0), makeItem(Items::shears, 1, 0),
        makeItem(Items::diamond, 1, 50), makeItem(Items::fishingRod, 1, 0),
        makeItem(Items::diamond, 1, 0), makeItem(Items::map, 1, 0)
    };
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListInventory__Fv()
{
    static CustomItem slots[27] = {
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0),
        makeItem(NULL, 0, 0), makeItem(NULL, 0, 0), makeItem(NULL, 0, 0)
    };
    slots[0] = makeBlock(Blocks::stone, 32, 0);
    slots[1] = makeBlock(Blocks::grass, 17, 0);
    slots[4] = makeBlock(Blocks::cobblestone, 1, 0);
    slots[7] = makeItem(Items::diamond, 64, 0);
    slots[8] = makeItem(Items::ironIngot, 10, 0);
    slots[9] = makeBlock(Blocks::brick_block, 3, 0);
    slots[11] = makeItem(Items::ironSword, 1, 0);
    slots[13] = makeItem(Items::diamond, 1, 0);
    slots[15] = makeBlock(Blocks::wool, 7, 0);
    slots[16] = makeItem(Items::ironPickaxe, 1, 0);
    slots[19] = makeBlock(Blocks::glass, 24, 0);
    slots[20] = makeBlock(Blocks::planks, 16, 0);
    slots[23] = makeBlock(Blocks::stonebrick, 7, 0);
    slots[24] = makeBlock(Blocks::iron_bars, 15, 0);
    slots[26] = makeItem(Items::diamond, 1, 0);
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListTrading__Fv()
{
    static CustomItem slots[9] = {
        makeItem(Items::emerald, 1, 0), makeItem(Items::emerald, 1, 0),
        makeItem(Items::diamond, 2, 0), makeItem(Items::enchantedBook, 1, 0),
        makeItem(Items::wheat, 1, 0), makeBlock(Blocks::wool, 3, 0),
        makeItem(NULL, 0, 0), makeItem(Items::emerald, 1, 0),
        makeItem(NULL, 0, 0)
    };
    return slots;
}
