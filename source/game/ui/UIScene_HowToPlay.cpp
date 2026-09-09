#include "game/ui/UIScene_HowToPlay.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>
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

UIScene_HowToPlay::UIScene_HowToPlay(int playerId, void*, UILayer* layer)
    : UIScene(playerId, layer), m_page(0), m_pageCount(0), m_container(), m_dynamicHtmlText(), m_labels(),
      m_enchantmentBook(), m_enchantmentBooks()
{
    initialiseMovie();
    m_labels[0].init(UIString(L10N::GetString(0x9f31c0ee)));
    m_labels[1].init(UIString(std::wstring(L10N::GetString(0x14630b9e))));
    m_labels[2].init(UIString(L10N::GetString(0x7d7816a1)));
    m_labels[3].init(UIString(L10N::GetString(0x1b49a944)));
    m_labels[4].init(UIString(std::wstring(L10N::GetString(0xfbc6e08))));
    m_labels[5].init(UIString(L10N::GetString(0x68521aaf)));
}

UIScene_HowToPlay::~UIScene_HowToPlay()
{
}

void UIScene_HowToPlay::StartPage(EHowToPlayPage page)
{
    m_page = static_cast<int>(page);
    const wchar_t* title = L10N::GetString(0x18d8ea9);
    std::wstring text(title == NULL ? L"" : title);
    gApp.FormatHTMLString(m_resolution, text, m_controlFocus);
    stripWhitespaceForHtml(text, true);
    m_dynamicHtmlText.addText(text, true);
    if (m_movie != NULL && m_movie->getRootNode() != NULL)
    {
        FJ_HowToPlay* node = dynamic_cast<FJ_HowToPlay*>(m_movie->getRootNode()->getFuiNode());
        if (node != NULL)
        {
            node->InitScene();
            std::vector<std::wstring> pages;
            pages.push_back(L"");
            LoadHowToPlayPage(node, m_page, pages);
            m_pageCount = static_cast<int>(pages.size());
        }
    }
    gConsoleUIController.TouchBoxRebuild(this);
}

unsigned int UIScene_HowToPlay::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie == NULL ? NULL : m_movie->getRootNode();
    if (root == NULL)
        return 0;
    static const char* const names[30] = {
        "Label1_10", "Label2_10", "Label3_10", "Label4_10", "Label1_7", "Label2_7",
        "Label1_4", "Label1_6", "Label2_6", "Label1_5", "Label1_11", "Label2_11",
        "Label1_13", "Label2_13", "Label1_12", "Label2_12", "Label1_22", "Label2_22",
        "Label3_22", "Label4_22", "Label1_21", "Label2_21", "Label3_21", "Label1_25",
        "Label2_25", "Label1_27", "Label2_27", "Label1_28", "Label2_28"
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
