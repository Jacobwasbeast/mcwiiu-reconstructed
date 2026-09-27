#include "game/item/Item.hpp"
#include "game/core/StringIds.hpp"

#include <cstddef>
#include <math.h>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/core/AttributeModifier.hpp"
#include "game/core/Boat.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Integer.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/Rarity.hpp"
#include "game/core/Stats.hpp"
#include "game/entity/mob/HangingEntityItem.hpp"
#include "game/entity/mob/HorseArmorItem.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/projectile/ArrowItem.hpp"
#include "game/entity/projectile/SpectralArrowItem.hpp"
#include "game/item/AirItem.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/ArmorStandItem.hpp"
#include "game/item/AxeItem.hpp"
#include "game/item/BannerItem.hpp"
#include "game/item/BedItem.hpp"
#include "game/item/BoatItem.hpp"
#include "game/item/BookItem.hpp"
#include "game/item/BottleItem.hpp"
#include "game/item/BowItem.hpp"
#include "game/item/BowlFoodItem.hpp"
#include "game/item/BucketItem.hpp"
#include "game/item/CarrotOnAStickItem.hpp"
#include "game/item/ChorusFruitItem.hpp"
#include "game/item/ClockItem.hpp"
#include "game/item/CoalItem.hpp"
#include "game/item/CompassItem.hpp"
#include "game/item/DebugFourjItem.hpp"
#include "game/item/DoorItem.hpp"
#include "game/item/DyeItem.hpp"
#include "game/item/EggItem.hpp"
#include "game/item/ElytraItem.hpp"
#include "game/item/EmptyMapItem.hpp"
#include "game/item/EnchantedBookItem.hpp"
#include "game/item/EndCrystalItem.hpp"
#include "game/item/EnderEyeItem.hpp"
#include "game/item/EnderpearlItem.hpp"
#include "game/item/ExperienceBottleItem.hpp"
#include "game/item/FireChargeItem.hpp"
#include "game/item/FireworkChargeItem.hpp"
#include "game/item/FireworksItem.hpp"
#include "game/item/FishFoodItem.hpp"
#include "game/item/FishingRodItem.hpp"
#include "game/item/FlintAndSteelItem.hpp"
#include "game/item/FoodItem.hpp"
#include "game/item/GoldenAppleItem.hpp"
#include "game/item/HoeItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/LeadItem.hpp"
#include "game/item/MapItem.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/MinecartItem.hpp"
#include "game/item/NameTagItem.hpp"
#include "game/item/PickaxeItem.hpp"
#include "game/item/PotionItem.hpp"
#include "game/item/LingeringPotionItem.hpp"
#include "game/item/SplashPotionItem.hpp"
#include "game/item/RecordItem.hpp"
#include "game/item/SaddleItem.hpp"
#include "game/item/SeedFoodItem.hpp"
#include "game/item/SeedItem.hpp"
#include "game/item/ShearsItem.hpp"
#include "game/item/ShovelItem.hpp"
#include "game/item/SignItem.hpp"
#include "game/item/SimpleFoiledItem.hpp"
#include "game/item/SkullItem.hpp"
#include "game/item/SnowItem.hpp"
#include "game/item/SnowballItem.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/item/SwordItem.hpp"
#include "game/item/TridentItem.hpp"
#include "game/item/WritableBookItem.hpp"
#include "game/item/WrittenBookItem.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/AnvilBlockItem.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/world/block/BlockPlanterItem.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CoralFanBlockItem.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/FlowerBlock.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/HalfNewStoneSlabBlock.hpp"
#include "game/world/block/HalfWoodSlabBlock.hpp"
#include "game/world/block/ItemPropertyFunction_Cooldown.hpp"
#include "game/world/block/ItemPropertyFunction_Damage.hpp"
#include "game/world/block/ItemPropertyFunction_Damaged.hpp"
#include "game/world/block/ItemPropertyFunction_LeftHanded.hpp"
#include "game/world/block/LeafBlockItem.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MonsterEggBlock.hpp"
#include "game/world/block/MultiBlockItem.hpp"
#include "game/world/block/NewLogBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PistonBlockItem.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/PrismarineBlock.hpp"
#include "game/world/block/RedSandStoneBlock.hpp"
#include "game/world/block/RedStoneDustItem.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/SandStoneBlock.hpp"
#include "game/world/block/SeaPickleBlockItem.hpp"
#include "game/world/block/ShulkerBoxBlockItem.hpp"
#include "game/world/block/SlabBlockItem.hpp"
#include "game/world/block/SpongeBlock.hpp"
#include "game/world/block/StoneBlock.hpp"
#include "game/world/block/StoneBrickBlock.hpp"
#include "game/world/block/TintedBlockItem.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/WallBlock.hpp"
#include "game/world/block/WaterLilyBlockItem.hpp"
#include "game/world/generation/Player.hpp"
#include "game/util/Random.hpp"

MappedRegistry<ResourceLocation,
               Item*,
               boost::hash<ResourceLocation>,
               std::equal_to<ResourceLocation>,
               boost::hash<Item*>,
               std::equal_to<Item*> > Item::itemRegistry;

boost::unordered_map<Block*, Item*> Item::BLOCK_TO_ITEM;

static const std::wstring LEATHER_HORSE_ARMOR_ICON_NAME = L"_brown";

static const ItemPropertyFunction* DAMAGED_PROPERTY = new ItemPropertyFunction_Damaged();
static const ItemPropertyFunction* DAMAGE_PROPERTY = new ItemPropertyFunction_Damage();
static const ItemPropertyFunction* LEFT_HANDED_PROPERTY = new ItemPropertyFunction_LeftHanded();
static const ItemPropertyFunction* COOLDOWN_PROPERTY = new ItemPropertyFunction_Cooldown();

const Item::Tier* Item::Tier::WOOD = new Item::Tier(0, 59, 2.0f, 0.0f, 15, 0);
const Item::Tier* Item::Tier::STONE = new Item::Tier(1, 131, 4.0f, 1.0f, 5, 1);
const Item::Tier* Item::Tier::IRON = new Item::Tier(2, 250, 6.0f, 2.0f, 14, 2);
const Item::Tier* Item::Tier::DIAMOND = new Item::Tier(3, 1561, 8.0f, 3.0f, 10, 3);
const Item::Tier* Item::Tier::GOLD = new Item::Tier(0, 32, 12.0f, 0.0f, 22, 4);

Random* Item::random = new Random();

static const int LOG_BLOCK_NAME_IDS[] = {
    0x0B851C01, 0x7D86FA86, 0xBB86EB26, 0x635D7533, 0xF11C45A4, 0x98793762
};

static const int WOOL_BLOCK_NAME_IDS[] = {
    0x4EE0C673, 0x6333B0A7, 0xBC7213BF, 0x48794C3A, 0x603AD112, 0xD2B9E2CA, 0x1E9ABF39, 0x261A069B,
    0xE335FA61, 0xB5D7B8D8, 0x8536CAD9, 0xAE40AC60, 0x26CC1F13, 0x61BD1619, 0xF8F21E7C, 0x61FFACAB
};

static const int QUARTZ_BLOCK_NAME_IDS[] = {
    IDS_TILE_QUARTZ_BLOCK, IDS_TILE_QUARTZ_BLOCK_CHISELED, 0x33529C72
};

static const int STAINED_CLAY_BLOCK_NAME_IDS[] = {
    0x5366E34F, 0x2767F1BB, 0xD7228303, 0x2BA187F6, 0xFEE55BFE, 0x8CFEA0D6, 0x401911F5, 0x2D51BCB7,
    0x4FEE910D, 0x883B0C24, 0xB1E87A05, 0x57114E5C, 0x63D1E61F, 0xACB66EF5, 0xD5D0D890, 0xCEB2BF07
};

static const int WOOL_CARPET_BLOCK_NAME_IDS[] = {
    IDS_TILE_CARPET_BLACK, IDS_TILE_CARPET_RED, IDS_TILE_CARPET_GREEN, IDS_TILE_CARPET_BROWN, IDS_TILE_CARPET_BLUE, IDS_TILE_CARPET_PURPLE, IDS_TILE_CARPET_CYAN, 0xF7A53C3E,
    IDS_TILE_CARPET_GRAY, IDS_TILE_CARPET_PINK, IDS_TILE_CARPET_LIME, IDS_TILE_CARPET_YELLOW, IDS_TILE_CARPET_LIGHT_BLUE, IDS_TILE_CARPET_MAGENTA, IDS_TILE_CARPET_ORANGE, IDS_TILE_CARPET_WHITE
};

static const int STAINED_GLASS_BLOCK_NAME_IDS[] = {
    IDS_TILE_STAINED_GLASS_BLACK, IDS_TILE_STAINED_GLASS_RED, IDS_TILE_STAINED_GLASS_GREEN, IDS_TILE_STAINED_GLASS_BROWN, IDS_TILE_STAINED_GLASS_BLUE, IDS_TILE_STAINED_GLASS_PURPLE, IDS_TILE_STAINED_GLASS_CYAN, 0xD9C1E722,
    IDS_TILE_STAINED_GLASS_GRAY, IDS_TILE_STAINED_GLASS_PINK, IDS_TILE_STAINED_GLASS_LIME, IDS_TILE_STAINED_GLASS_YELLOW, IDS_TILE_STAINED_GLASS_LIGHT_BLUE, IDS_TILE_STAINED_GLASS_MAGENTA, IDS_TILE_STAINED_GLASS_ORANGE, IDS_TILE_STAINED_GLASS_WHITE
};

static const int STAINED_GLASS_PANE_BLOCK_NAME_IDS[] = {
    0xAACD630F, 0xFD174B7B, 0x31036BC3, 0x07B97AB6, 0x3A9A663E, 0x61ADDA16, 0x7E488535, 0xB5F2CFF7,
    0x8E1E044D, 0x70728664, 0x1C56FE45, 0x60E0979C, 0xD965D95F, 0x9E0869B5, 0x02F273D0, 0xA85048C7
};

static const int CONCRETE_BLOCK_NAME_IDS[] = {
    IDS_TILE_CONCRETE_BLACK, IDS_TILE_CONCRETE_RED, IDS_TILE_CONCRETE_GREEN, IDS_TILE_CONCRETE_BROWN, IDS_TILE_CONCRETE_BLUE, IDS_TILE_CONCRETE_PURPLE, IDS_TILE_CONCRETE_CYAN, 0x5D78FF60,
    IDS_TILE_CONCRETE_GRAY, IDS_TILE_CONCRETE_PINK, IDS_TILE_CONCRETE_LIME, IDS_TILE_CONCRETE_YELLOW, IDS_TILE_CONCRETE_LIGHT_BLUE, IDS_TILE_CONCRETE_MAGENTA, IDS_TILE_CONCRETE_ORANGE, IDS_TILE_CONCRETE_WHITE
};

static const int CONCRETE_POWDER_BLOCK_NAME_IDS[] = {
    IDS_TILE_CONCRETE_POWDER_BLACK, IDS_TILE_CONCRETE_POWDER_RED, IDS_TILE_CONCRETE_POWDER_GREEN, IDS_TILE_CONCRETE_POWDER_BROWN, IDS_TILE_CONCRETE_POWDER_BLUE, IDS_TILE_CONCRETE_POWDER_PURPLE, IDS_TILE_CONCRETE_POWDER_CYAN, 0xF2E161B4,
    IDS_TILE_CONCRETE_POWDER_GRAY, IDS_TILE_CONCRETE_POWDER_PINK, IDS_TILE_CONCRETE_POWDER_LIME, IDS_TILE_CONCRETE_POWDER_YELLOW, 0x89B703CC, IDS_TILE_CONCRETE_POWDER_MAGENTA, IDS_TILE_CONCRETE_POWDER_ORANGE, IDS_TILE_CONCRETE_POWDER_WHITE
};

Item::Tier::Tier(int level, int uses, float speed, float attackDamageBonus, int enchantmentValue, int ordinalValue)
{
    mLevel = level;
    mUses = uses;
    mSpeed = speed;
    mAttackDamageBonus = attackDamageBonus;
    mEnchantmentValue = enchantmentValue;
    mOrdinal = ordinalValue;
}

int Item::Tier::getUses() const
{
    return mUses;
}

float Item::Tier::getAttackDamageBonus() const
{
    return mAttackDamageBonus;
}

float Item::Tier::getSpeed() const
{
    return mSpeed;
}

int Item::Tier::getLevel() const
{
    return mLevel;
}

int Item::Tier::getEnchantmentValue() const
{
    return mEnchantmentValue;
}

Item* Item::Tier::getTierItem() const
{
    if (this == WOOD)
    {
        return byBlock(Blocks::planks);
    }

    if (this == STONE)
    {
        return byBlock(Blocks::cobblestone);
    }

    if (this == GOLD)
    {
        return Items::goldIngot;
    }

    if (this == IRON)
    {
        return Items::ironIngot;
    }

    if (this != DIAMOND)
    {
        return NULL;
    }

    return Items::diamond;
}

int Item::Tier::ordinal() const
{
    return mOrdinal;
}

Material* Item::getMaterial()
{
    return mMaterial;
}

bool Item::IsShulkerBox(int id)
{
    return id >= 219 && id <= 234;
}

bool Item::IsShulkerBox(Item* item)
{
    return item != NULL && IsShulkerBox(getId(item));
}

Item::Item()
{
    mMaxStackSize = 64;
    mMaxDamage = 0;
    mIcon = NULL;
    mBaseItemType = 0;
    mBaseItemMaterial = 0;
    mHandEquipped = false;
    mStackedByData = false;
    mAllowOffhand = false;
    mCraftingRemainingItem = NULL;
    mName = L"";
    mIconName = L"";
    mPreferInventory = false;
    mProperties = new SimpleRegistry<ResourceLocation,
                                     const ItemPropertyFunction*,
                                     boost::hash<ResourceLocation>,
                                     std::equal_to<ResourceLocation> >();

    addProperty(ResourceLocation(L"lefthanded"), LEFT_HANDED_PROPERTY);
    addProperty(ResourceLocation(L"cooldown"), COOLDOWN_PROPERTY);
}

Item::~Item()
{
}

void Item::addProperty(ResourceLocation location, const ItemPropertyFunction* function)
{
    mProperties->registerKey(location, function);
}

const ItemPropertyFunction* Item::getProperty(ResourceLocation location)
{
    return mProperties->get(location);
}

bool Item::hasProperties()
{
    return !mProperties->keySet().empty();
}

void Item::staticInit()
{
    Stats::buildItemStats();
}

Item* Item::byBlock(Block* block)
{
    boost::unordered_map<Block*, Item*>::iterator found = BLOCK_TO_ITEM.find(block);
    if (found == BLOCK_TO_ITEM.end())
    {
        return Items::air;
    }

    return found->second;
}

Item* Item::byId(int id)
{
    if (id < 0)
    {
        return NULL;
    }
    return itemRegistry.byId(id);
}

Item* Item::byString(const std::wstring& name)
{
    if (name.size() == 0 || name.compare(0, name.size(), L"") == 0)
    {
        return NULL;
    }

    ResourceLocation location(name);

    if (!itemRegistry.containsKey(ResourceLocation(location)))
    {
        return byId(Integer::parseInt(name, 10));
    }

    return itemRegistry.get(ResourceLocation(location));
}

int Item::getId(Item* item)
{
    if (item == NULL)
    {
        return 0;
    }

    return itemRegistry.getId(item);
}

int Item::VALIDgetId(Item* item)
{
    if (item == NULL)
    {
        return 0;
    }

    return itemRegistry.getId(item);
}

bool Item::isItemABlock(int id)
{
    return (unsigned int)(id - 256) > 767 && id < 2255;
}

int Item::getBlockId(Item* item)
{
    int id = getId(item);

    if (!isItemABlock(id))
    {
        return 0;
    }

    if (id > 1023)
    {
        id -= 768;
    }

    return id;
}

bool Item::existsWithId(int id)
{
    return itemRegistry.containsId(id);
}

void Item::registerItem(int id, const ResourceLocation& location, Item* item)
{
    itemRegistry.registerMapping(id, location, item);
}

void Item::registerItem(int id, const std::wstring& name, Item* item)
{
    registerItem(id, ResourceLocation(name), item);
}

void Item::registerBlock(Block* block, Item* item)
{
    int id = Block::getId(block);

    if (id > 255)
    {
        id += 768;
    }

    registerItem(id, Block::blockRegistry->getKey(block), item);

    BLOCK_TO_ITEM[block] = item;
}

void Item::registerBlock(Block* block)
{
    registerBlock(block, new BlockItem(block));
}

int Item::StoneBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return StoneBlock::Variant::byData(itemInstance->getAuxValue())->getNameId();
}

int Item::StoneBlockDescResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return StoneBlock::Variant::byData(itemInstance->getAuxValue())->getDescriptionId();
}

int Item::DirtBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return DirtBlock::Variant::byData(itemInstance->getAuxValue())->getNameId();
}

int Item::DirtBlockDescResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return DirtBlock::Variant::byData(itemInstance->getAuxValue())->getDescriptionId();
}

int Item::PlanksBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return PlanksBlock::Variant::byData(itemInstance->getAuxValue())->getTranslationKey();
}

int Item::SaplingBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return Blocks::sapling->getDescriptionId(itemInstance->getAuxValue());
}

int Item::SandBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return SandBlock::Variant::byData(itemInstance->getAuxValue())->getNameId();
}

int Item::SandBlockDescResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return SandBlock::Variant::byData(itemInstance->getAuxValue())->getDescriptionId();
}

int Item::Log1BlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    PlanksBlock::Variant* variant =
        Blocks::log->getBlockState(itemInstance->getAuxValue())->getValue(OldLogBlock::VARIANT);

    return LOG_BLOCK_NAME_IDS[variant->getData()];
}

int Item::Log2BlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    PlanksBlock::Variant* variant =
        Blocks::log2->getBlockState(itemInstance->getAuxValue())->getValue(NewLogBlock::VARIANT);

    return LOG_BLOCK_NAME_IDS[variant->getData()];
}

int Item::SpongeBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    if (Blocks::sponge->getBlockState(itemInstance->getAuxValue())->getValue(SpongeBlock::WET))
    {
        return -256870266;
    }

    return -1164450345;
}

int Item::SpongeBlockDescResolver(not_null_ptr<ItemInstance> itemInstance)
{
    if (Blocks::sponge->getBlockState(itemInstance->getAuxValue())->getValue(SpongeBlock::WET))
    {
        return -118885941;
    }

    return 1450677978;
}

int Item::SandstoneBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return SandStoneBlock::Variant::byData(itemInstance->getAuxValue())->getTranslationKey();
}

int Item::WoolBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return WOOL_BLOCK_NAME_IDS[DyeColor::byItemData(itemInstance->getAuxValue())->getBlockData()];
}

int Item::FlowerBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    FlowerBlock* block = (FlowerBlock*)Block::byId(getId(itemInstance->getItem()));

    return block->getBlockState(itemInstance->getAuxValue())->getValue(block->mTypeProperty)->getNameId();
}

int Item::FlowerBlockDescResolver(not_null_ptr<ItemInstance> itemInstance)
{
    FlowerBlock* block = (FlowerBlock*)Block::byId(getId(itemInstance->getItem()));

    return block->getBlockState(itemInstance->getAuxValue())->getValue(block->mTypeProperty)->getDescriptionId();
}

int Item::MonsterEggBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return MonsterEggBlock::Variant::byData(itemInstance->getAuxValue())->getTranslationKey();
}

int Item::StoneBrickBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return StoneBrickBlock::Variant::byData(itemInstance->getAuxValue())->getTranslationKey();
}

int Item::CobblestoneWallBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return WallBlock::Variant::byData(itemInstance->getAuxValue())->getTranslationKey();
}

int Item::QuartzBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    unsigned int auxValue = itemInstance->getAuxValue();

    if (auxValue > 2)
    {
        auxValue = 0;
    }

    return QUARTZ_BLOCK_NAME_IDS[auxValue];
}

int Item::ClayBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return STAINED_CLAY_BLOCK_NAME_IDS[DyeColor::byItemData(itemInstance->getAuxValue())->getBlockData()];
}

int Item::WoolCarpetBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return WOOL_CARPET_BLOCK_NAME_IDS[DyeColor::byItemData(itemInstance->getAuxValue())->getBlockData()];
}

int Item::DoublePlantBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return DoublePlantBlock::Variant::byData(itemInstance->getAuxValue())->getNameId();
}

int Item::DoublePlantBlockDescResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return DoublePlantBlock::Variant::byData(itemInstance->getAuxValue())->getDescriptionId();
}

int Item::GlassBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return STAINED_GLASS_BLOCK_NAME_IDS[DyeColor::byItemData(itemInstance->getAuxValue())->getBlockData()];
}

int Item::GlassPaneBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return STAINED_GLASS_PANE_BLOCK_NAME_IDS[DyeColor::byItemData(itemInstance->getAuxValue())->getBlockData()];
}

int Item::PrismarineBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return PrismarineBlock::Variant::byData(itemInstance->getAuxValue())->getNameId();
}

int Item::PrismarineBlockDescResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return PrismarineBlock::Variant::byData(itemInstance->getAuxValue())->getDescriptionId();
}

int Item::RedSandstoneBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return RedSandStoneBlock::Variant::byData(itemInstance->getAuxValue())->getTranslationKey();
}

int Item::ConcreteBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return CONCRETE_BLOCK_NAME_IDS[DyeColor::byItemData(itemInstance->getAuxValue())->getBlockData()];
}

int Item::ConcretePowderBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return CONCRETE_POWDER_BLOCK_NAME_IDS[DyeColor::byItemData(itemInstance->getAuxValue())->getBlockData()];
}

int Item::CoralBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return Blocks::coral_block->getDescriptionId(itemInstance->getAuxValue());
}

int Item::CoralBlockDescResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return Blocks::coral_block->getUseDescriptionId(itemInstance->getAuxValue());
}

int Item::CoralBushNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return Blocks::coral->getDescriptionId(itemInstance->getAuxValue());
}

int Item::CoralBushDescResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return Blocks::coral->getUseDescriptionId(itemInstance->getAuxValue());
}

Item* Item::setDescriptionId(int descriptionId)
{
    mDescriptionId = descriptionId;

    return this;
}

Item* Item::setUseDescriptionId(int useDescriptionId)
{
    mUseDescriptionId = useDescriptionId;

    return this;
}

Item* Item::setDescriptionId(int descriptionId, int useDescriptionId)
{
    setDescriptionId(descriptionId);

    return setUseDescriptionId(useDescriptionId);
}

Item* Item::setBaseItemTypeAndMaterial(int baseItemType, int baseItemMaterial)
{
    mBaseItemType = baseItemType;
    mBaseItemMaterial = baseItemMaterial;

    return this;
}

Item* Item::setIconName(const std::wstring& iconName)
{
    mIconName = iconName;

    return this;
}

Item* Item::setMaxStackSize(int maxStackSize)
{
    mMaxStackSize = maxStackSize;

    return this;
}

Item* Item::setMaxDamage(int maxDamage)
{
    mMaxDamage = maxDamage;

    if (maxDamage > 0)
    {
        addProperty(ResourceLocation(L"damaged"), DAMAGED_PROPERTY);
        addProperty(ResourceLocation(L"damage"), DAMAGE_PROPERTY);
    }

    return this;
}

Item* Item::setAllowOffhand(bool allowOffhand)
{
    mAllowOffhand = allowOffhand;

    return this;
}

Item* Item::SetPreferInventory(bool preferInventory)
{
    mPreferInventory = preferInventory;

    return this;
}

Item* Item::setCraftingRemainingItem(Item* craftingRemainingItem)
{
    mCraftingRemainingItem = craftingRemainingItem;

    return this;
}

Item* Item::setStackedByData(bool stackedByData)
{
    mStackedByData = stackedByData;

    return this;
}

Item* Item::handEquipped()
{
    mHandEquipped = true;

    return this;
}

int Item::getBaseItemType()
{
    return mBaseItemType;
}

int Item::getMaxDamage()
{
    return mMaxDamage;
}

bool Item::isStackedByData()
{
    return mStackedByData;
}

bool Item::canBeDepleted()
{
    return mMaxDamage > 0 && (!mStackedByData || mMaxStackSize == 1);
}

bool Item::allowOffhand()
{
    return mAllowOffhand;
}

bool Item::PreferInventory()
{
    return mPreferInventory && Minecraft::InMiniGame(eMiniGameId_Any, true);
}

bool Item::IsArmor()
{
    return GetArmorType() != 0;
}

Item* Item::getCraftingRemainingItem()
{
    return mCraftingRemainingItem;
}

bool Item::hasCraftingRemainingItem()
{
    return mCraftingRemainingItem != NULL;
}

std::wstring Item::getIconName()
{
    std::wstring iconName;
    iconName.assign(mIconName);
    return iconName;
}

const wchar_t* Item::getDescription()
{
    return L10N::GetString(getDescriptionId(-1));
}

const wchar_t* Item::getDescription(not_null_ptr<ItemInstance> itemInstance)
{
    return L10N::GetString(getDescriptionId(itemInstance));
}

HitResult* Item::getPlayerPOVHitResult(Level* level, boost::shared_ptr<Player> player, bool useLiquids)
{
    float xRot = player->mXRot;
    float yRot = player->mYRot;

    Vec3* from = Vec3::newTemp(player->mX, player->mY + player->getEyeHeight(), player->mZ);

    float yaw = -yRot * 0.017453292f - 3.1415927f;
    float cosYaw = cosf(yaw);
    float sinYaw = sinf(yaw);

    float pitch = -(xRot * 0.017453292f);
    float cosPitch = -cosf(pitch);
    float sinPitch = sinf(pitch);

    double dx = sinYaw * cosPitch;
    double dy = sinPitch;
    double dz = cosYaw * cosPitch;

    Vec3* to = from->add(dx * 5.0, dy * 5.0, dz * 5.0);

    if (useLiquids)
    {
        from = from->add(dx * 1.3, dy * 1.3, dz * 1.3);
        from->normalize();
    }

    return level->clip(from, to, useLiquids, !useLiquids, false, false, NULL, false);
}

not_null_ptr<ItemInstance> Item::getDefaultInstance()
{
    not_null_ptr<ItemInstance> result(new ItemInstance(this));
    return result;
}

bool Item::verifyTagAfterLoad(CompoundTag* tag)
{
    return false;
}

unsigned int Item::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return IDS_NONE;
}

InteractionResult::EInteractionResult Item::useOn(boost::shared_ptr<Player> player,
                 Level* level,
                 const BlockPos& pos,
                 InteractionHand::EInteractionHand hand,
                 const Direction* direction,
                 float hitX,
                 float hitY,
                 float hitZ,
                 bool simulate)
{
    return InteractionResult::eInteractionResult_Pass;
}

float Item::getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state)
{
    (void)itemInstance;
    (void)state;
    const float defaultSpeed = 1.0f;
    return defaultSpeed;
}

bool Item::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    return false;
}

InteractionResultHolder Item::use(Level* level,
                                  boost::shared_ptr<Player> player,
                                  InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
}

not_null_ptr<ItemInstance> Item::finishUsingItem(not_null_ptr<ItemInstance> itemInstance,
                                                 Level* level,
                                                 boost::shared_ptr<LivingEntity> entity)
{
    not_null_ptr<ItemInstance> result = itemInstance;
    return result;
}

int Item::getMaxStackSize()
{
    return mMaxStackSize;
}

int Item::getLevelDataForAuxValue(int auxValue)
{
    return 0;
}

bool Item::hurtEnemy(not_null_ptr<ItemInstance> itemInstance,
                     boost::shared_ptr<LivingEntity> target,
                     boost::shared_ptr<LivingEntity> attacker)
{
    return false;
}

bool Item::mineBlock(not_null_ptr<ItemInstance> itemInstance,
                     Level* level,
                     const BlockState* state,
                     const BlockPos& pos,
                     boost::shared_ptr<LivingEntity> entity)
{
    return false;
}

int Item::getAttackDamage(boost::shared_ptr<Entity> entity)
{
    return 1;
}

bool Item::canDestroySpecial(const BlockState* state)
{
    return false;
}

bool Item::interactEnemy(not_null_ptr<ItemInstance> itemInstance,
                         boost::shared_ptr<Player> player,
                         boost::shared_ptr<LivingEntity> target,
                         InteractionHand::EInteractionHand hand)
{
    return false;
}

bool Item::isHandEquipped()
{
    return mHandEquipped;
}

bool Item::isMirroredArt()
{
    return false;
}

int Item::getDescriptionId(int auxValue)
{
    return mDescriptionId;
}

int Item::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    return mDescriptionId;
}

int Item::getUseDescriptionId()
{
    return mUseDescriptionId;
}

int Item::getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    return mUseDescriptionId;
}

bool Item::shouldOverrideMultiplayerNBT()
{
    return true;
}

int Item::getColor(not_null_ptr<ItemInstance> itemInstance, int layer)
{
    return 0xFFFFFF;
}

void Item::inventoryTick(not_null_ptr<ItemInstance> itemInstance,
                         Level* level,
                         boost::shared_ptr<Entity> entity,
                         int slot,
                         bool selected)
{
}

void Item::onCraftedBy(not_null_ptr<ItemInstance> itemInstance, Level* level, boost::shared_ptr<Player> player)
{
}

bool Item::isComplex()
{
    return false;
}

int Item::getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const
{
    return 0;
}

int Item::getUseDuration(not_null_ptr<ItemInstance> itemInstance)
{
    return 0;
}

void Item::releaseUsing(not_null_ptr<ItemInstance> itemInstance,
                        Level* level,
                        boost::shared_ptr<LivingEntity> entity,
                        int timeLeft)
{
}

void Item::appendHoverText(not_null_ptr<ItemInstance> itemInstance,
                           boost::shared_ptr<Player> player,
                           std::vector<HtmlString>* lines,
                           bool advanced)
{
}

std::wstring Item::getName(not_null_ptr<ItemInstance> itemInstance)
{
    const int descriptionId = getDescriptionId(itemInstance);
    return std::wstring(L10N::GetString(descriptionId));
}

bool Item::isFoil(not_null_ptr<ItemInstance> itemInstance)
{
    return itemInstance->isEnchanted();
}

const Rarity* Item::getRarity(not_null_ptr<ItemInstance> itemInstance)
{
    if (itemInstance->isEnchanted())
    {
        return Rarity::RARE;
    }

    return Rarity::COMMON;
}

bool Item::isEnchantable(not_null_ptr<ItemInstance> itemInstance)
{
    return getMaxStackSize() == 1 && canBeDepleted();
}

int Item::getEnchantmentValue()
{
    return 0;
}

bool Item::mayBePlacedInAdventureMode()
{
    return true;
}

bool Item::isValidRepairItem(not_null_ptr<ItemInstance> itemInstance, not_null_ptr<ItemInstance> repairItem)
{
    return false;
}

boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* Item::getDefaultAttributeModifiers(
    const EquipmentSlot* slot)
{
    return new boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>();
}

void Item::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(mIconName);
}

bool Item::hasMultipleSpriteLayers()
{
    return false;
}

TextureAtlasSprite* Item::getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance)
{
    return getIcon(auxValue);
}

int Item::getIconType()
{
    return 1;
}

TextureAtlasSprite* Item::getIcon(int auxValue)
{
    return mIcon;
}

TextureAtlasSprite* Item::getIcon(not_null_ptr<ItemInstance> itemInstance)
{
    return getIcon(itemInstance->getAuxValue());
}

int Item::GetArmorType()
{
    return 0;
}

int Item::GetOverrideCount()
{
    return -1;
}

int Item::GetOverrideCountColour()
{
    return -1;
}

TextureAtlasSprite* Item::GetOverrideCountIcon(not_null_ptr<ItemInstance> itemInstance)
{
    return NULL;
}

void Item::staticCtor()
{
    registerBlock(Blocks::water);
    registerBlock(Blocks::lava);
    registerBlock(Blocks::cocoa);
    registerBlock(Blocks::reeds);
    registerBlock(Blocks::lit_redstone_lamp);
    registerBlock(Blocks::nether_wart);
    registerBlock(Blocks::potatoes);
    registerBlock(Blocks::carrots);
    registerBlock(Blocks::cake);
    registerBlock(Blocks::wheat);
    registerBlock(Blocks::double_wooden_slab,
        (new SlabBlockItem(Blocks::double_wooden_slab,
                           static_cast<HalfSlabBlock*>(Blocks::wooden_slab),
                           static_cast<HalfSlabBlock*>(Blocks::double_wooden_slab)))->setDescriptionId(Blocks::double_wooden_slab->getDescriptionId(-1)));

    registerBlock(Blocks::double_stone_slab,
        (new SlabBlockItem(Blocks::double_stone_slab,
                           static_cast<HalfSlabBlock*>(Blocks::stone_slab),
                           static_cast<HalfSlabBlock*>(Blocks::double_stone_slab)))->setDescriptionId(Blocks::double_stone_slab->getDescriptionId(-1)));

    registerBlock(Blocks::double_stone_slab2,
        (new SlabBlockItem(Blocks::double_stone_slab2,
                           static_cast<HalfSlabBlock*>(Blocks::stone_slab2),
                           static_cast<HalfSlabBlock*>(Blocks::double_stone_slab2)))->setDescriptionId(Blocks::double_stone_slab2->getDescriptionId(-1)));

    registerBlock(Blocks::bed);
    registerBlock(Blocks::piston_head);
    registerBlock(Blocks::redstone_wire);
    registerBlock(Blocks::standing_sign);
    registerBlock(Blocks::wall_sign);
    registerBlock(Blocks::lit_redstone_ore);
    registerBlock(Blocks::unlit_redstone_torch);
    registerBlock(Blocks::unpowered_repeater);
    registerBlock(Blocks::powered_repeater);
    registerBlock(Blocks::pumpkin_stem);
    registerBlock(Blocks::melon_stem);
    registerBlock(Blocks::brewing_stand);
    registerBlock(Blocks::cauldron);
    registerBlock(Blocks::tripwire);
    registerBlock(Blocks::flower_pot);
    registerBlock(Blocks::skull);
    registerBlock(Blocks::unpowered_comparator);
    registerBlock(Blocks::powered_comparator);
    registerBlock(Blocks::air,
        new AirItem(Blocks::air));

    registerBlock(Blocks::stone,
        (new MultiBlockItem(Blocks::stone, Blocks::stone, StoneBlockNameResolver, StoneBlockDescResolver))->setDescriptionId(Blocks::stone->getDescriptionId(-1)));

    registerBlock(Blocks::grass,
        new TintedBlockItem(Blocks::grass, false));

    registerBlock(Blocks::dirt,
        new MultiBlockItem(Blocks::dirt, Blocks::dirt, DirtBlockNameResolver, DirtBlockDescResolver));

    registerBlock(Blocks::cobblestone);
    registerBlock(Blocks::planks,
        (new MultiBlockItem(Blocks::planks, Blocks::planks, PlanksBlockNameResolver, NULL))->setDescriptionId(Blocks::planks->getDescriptionId(-1)));

    registerBlock(Blocks::sapling,
        (new MultiBlockItem(Blocks::sapling, Blocks::sapling, SaplingBlockNameResolver, NULL))->setDescriptionId(Blocks::sapling->getDescriptionId(-1)));

    registerBlock(Blocks::bedrock);
    registerBlock(Blocks::sand,
        new MultiBlockItem(Blocks::sand, Blocks::sand, SandBlockNameResolver, SandBlockDescResolver));

    registerBlock(Blocks::gravel);
    registerBlock(Blocks::gold_ore);
    registerBlock(Blocks::iron_ore);
    registerBlock(Blocks::coal_ore);
    registerBlock(Blocks::log,
        (new MultiBlockItem(Blocks::log, Blocks::log, Log1BlockNameResolver, NULL))->setDescriptionId(Blocks::log->getDescriptionId(-1)));

    registerBlock(Blocks::log2,
        (new MultiBlockItem(Blocks::log2, Blocks::log2, Log2BlockNameResolver, NULL))->setDescriptionId(Blocks::log2->getDescriptionId(-1)));

    registerBlock(Blocks::leaves,
        (new LeafBlockItem(static_cast<LeafBlock*>(Blocks::leaves)))->setDescriptionId(Blocks::leaves->getDescriptionId(-1)));

    registerBlock(Blocks::leaves2,
        (new LeafBlockItem(static_cast<LeafBlock*>(Blocks::leaves2)))->setDescriptionId(Blocks::leaves2->getDescriptionId(-1)));

    registerBlock(Blocks::sponge,
        (new MultiBlockItem(Blocks::sponge, Blocks::sponge, SpongeBlockNameResolver, SpongeBlockDescResolver))->setDescriptionId(Blocks::sponge->getDescriptionId(-1)));

    registerBlock(Blocks::glass);
    registerBlock(Blocks::lapis_ore);
    registerBlock(Blocks::lapis_block);
    registerBlock(Blocks::dispenser);
    registerBlock(Blocks::sandstone,
        (new MultiBlockItem(Blocks::sandstone, Blocks::sandstone, SandstoneBlockNameResolver, NULL))->setDescriptionId(Blocks::sandstone->getDescriptionId(-1)));

    registerBlock(Blocks::noteblock);
    registerBlock(Blocks::golden_rail);
    registerBlock(Blocks::detector_rail);
    registerBlock(Blocks::sticky_piston,
        new PistonBlockItem(Blocks::sticky_piston));

    registerBlock(Blocks::web);
    int tallGrassDescriptionPostfixes[3];
    tallGrassDescriptionPostfixes[0] = IDS_TILE_SHRUB;
    tallGrassDescriptionPostfixes[1] = IDS_TILE_TALL_GRASS;
    tallGrassDescriptionPostfixes[2] = IDS_TILE_FERN;

    arrayWithLength<int> tallGrassPostfixes;
    tallGrassPostfixes.data = tallGrassDescriptionPostfixes;
    tallGrassPostfixes.length = 3;

    registerBlock(Blocks::tallgrass,
        (new TintedBlockItem(Blocks::tallgrass, true))->setDescriptionPostfixes(tallGrassPostfixes));

    registerBlock(Blocks::deadbush);
    registerBlock(Blocks::piston,
        new PistonBlockItem(Blocks::piston));

    registerBlock(Blocks::wool,
        (new MultiBlockItem(Blocks::wool, Blocks::wool, WoolBlockNameResolver, NULL))->setDescriptionId(Blocks::wool->getDescriptionId(-1)));

    registerBlock(Blocks::yellow_flower,
        (new MultiBlockItem(Blocks::yellow_flower, Blocks::yellow_flower, FlowerBlockNameResolver, FlowerBlockDescResolver))->setDescriptionId(Blocks::yellow_flower->getDescriptionId(-1)));

    registerBlock(Blocks::red_flower,
        (new MultiBlockItem(Blocks::red_flower, Blocks::red_flower, FlowerBlockNameResolver, FlowerBlockDescResolver))->setDescriptionId(Blocks::red_flower->getDescriptionId(-1)));

    registerBlock(Blocks::brown_mushroom);
    registerBlock(Blocks::red_mushroom);
    registerBlock(Blocks::gold_block);
    registerBlock(Blocks::iron_block);
    registerBlock(Blocks::stone_slab,
        (new SlabBlockItem(Blocks::stone_slab,
                           static_cast<HalfSlabBlock*>(Blocks::stone_slab),
                           static_cast<HalfSlabBlock*>(Blocks::double_stone_slab)))->setDescriptionId(Blocks::stone_slab->getDescriptionId(-1)));

    registerBlock(Blocks::brick_block);
    registerBlock(Blocks::tnt);
    registerBlock(Blocks::bookshelf);
    registerBlock(Blocks::mossy_cobblestone);
    registerBlock(Blocks::obsidian);
    registerBlock(Blocks::torch);
    registerBlock(Blocks::mob_spawner);
    registerBlock(Blocks::oak_stairs);
    registerBlock(Blocks::chest);
    registerBlock(Blocks::diamond_ore);
    registerBlock(Blocks::diamond_block);
    registerBlock(Blocks::crafting_table);
    registerBlock(Blocks::farmland);
    registerBlock(Blocks::furnace);
    registerBlock(Blocks::lit_furnace);
    registerBlock(Blocks::ladder);
    registerBlock(Blocks::rail);
    registerBlock(Blocks::stone_stairs);
    registerBlock(Blocks::lever);
    registerBlock(Blocks::stone_pressure_plate);
    registerBlock(Blocks::wooden_pressure_plate);
    registerBlock(Blocks::redstone_ore);
    registerBlock(Blocks::redstone_torch);
    registerBlock(Blocks::stone_button);
    registerBlock(Blocks::snow_layer,
        new SnowItem(Blocks::snow_layer));

    registerBlock(Blocks::ice);
    registerBlock(Blocks::snow);
    registerBlock(Blocks::cactus);
    registerBlock(Blocks::clay);
    registerBlock(Blocks::jukebox);
    registerBlock(Blocks::fence);
    registerBlock(Blocks::spruce_fence);
    registerBlock(Blocks::birch_fence);
    registerBlock(Blocks::jungle_fence);
    registerBlock(Blocks::dark_oak_fence);
    registerBlock(Blocks::acacia_fence);
    registerBlock(Blocks::pumpkin);
    registerBlock(Blocks::carved_pumpkin);
    registerBlock(Blocks::netherrack);
    registerBlock(Blocks::soul_sand);
    registerBlock(Blocks::glowstone);
    registerBlock(Blocks::lit_pumpkin);
    registerBlock(Blocks::trapdoor);
    registerBlock(Blocks::spruce_trapdoor);
    registerBlock(Blocks::birch_trapdoor);
    registerBlock(Blocks::jungle_trapdoor);
    registerBlock(Blocks::acacia_trapdoor);
    registerBlock(Blocks::dark_oak_trapdoor);
    registerBlock(Blocks::monster_egg,
        (new MultiBlockItem(Blocks::monster_egg, Blocks::monster_egg, MonsterEggBlockNameResolver, NULL))->setDescriptionId(Blocks::monster_egg->getDescriptionId(-1)));

    registerBlock(Blocks::stonebrick,
        (new MultiBlockItem(Blocks::stonebrick, Blocks::stonebrick, StoneBrickBlockNameResolver, NULL))->setDescriptionId(Blocks::stonebrick->getDescriptionId(-1)));

    registerBlock(Blocks::brown_mushroom_block,
        (new MultiBlockItem(Blocks::brown_mushroom_block, Blocks::brown_mushroom_block, NULL, NULL))->setDescriptionId(Blocks::brown_mushroom_block->getDescriptionId(-1)));

    registerBlock(Blocks::red_mushroom_block,
        (new MultiBlockItem(Blocks::red_mushroom_block, Blocks::red_mushroom_block, NULL, NULL))->setDescriptionId(Blocks::red_mushroom_block->getDescriptionId(-1)));

    registerBlock(Blocks::iron_bars);
    registerBlock(Blocks::glass_pane);
    registerBlock(Blocks::melon_block);
    registerBlock(Blocks::vine,
        new TintedBlockItem(Blocks::vine, false));

    registerBlock(Blocks::fence_gate);
    registerBlock(Blocks::spruce_fence_gate);
    registerBlock(Blocks::birch_fence_gate);
    registerBlock(Blocks::jungle_fence_gate);
    registerBlock(Blocks::dark_oak_fence_gate);
    registerBlock(Blocks::acacia_fence_gate);
    registerBlock(Blocks::brick_stairs);
    registerBlock(Blocks::stone_brick_stairs);
    registerBlock(Blocks::mycelium);
    registerBlock(Blocks::waterlily,
        new WaterLilyBlockItem(Blocks::waterlily));

    registerBlock(Blocks::nether_brick);
    registerBlock(Blocks::nether_brick_fence);
    registerBlock(Blocks::nether_brick_stairs);
    registerBlock(Blocks::enchanting_table);
    registerBlock(Blocks::end_portal_frame);
    registerBlock(Blocks::end_stone);
    registerBlock(Blocks::dragon_egg);
    registerBlock(Blocks::redstone_lamp);
    registerBlock(Blocks::wooden_slab,
        (new SlabBlockItem(Blocks::wooden_slab,
                           static_cast<HalfSlabBlock*>(Blocks::wooden_slab),
                           static_cast<HalfSlabBlock*>(Blocks::double_wooden_slab)))->setDescriptionId(Blocks::wooden_slab->getDescriptionId(-1)));

    registerBlock(Blocks::sandstone_stairs);
    registerBlock(Blocks::emerald_ore);
    registerBlock(Blocks::ender_chest);
    registerBlock(Blocks::tripwire_hook);
    registerBlock(Blocks::emerald_block);
    registerBlock(Blocks::spruce_stairs);
    registerBlock(Blocks::birch_stairs);
    registerBlock(Blocks::jungle_stairs);
    registerBlock(Blocks::command_block);
    registerBlock(Blocks::beacon);
    registerBlock(Blocks::cobblestone_wall,
        (new MultiBlockItem(Blocks::cobblestone_wall, Blocks::cobblestone_wall, CobblestoneWallBlockNameResolver, NULL))->setDescriptionId(Blocks::cobblestone_wall->getDescriptionId(-1)));

    registerBlock(Blocks::wooden_button);
    registerBlock(Blocks::anvil,
        (new AnvilBlockItem(Blocks::anvil))->setDescriptionId(Blocks::anvil->getDescriptionId(-1)));

    registerBlock(Blocks::trapped_chest);
    registerBlock(Blocks::light_weighted_pressure_plate);
    registerBlock(Blocks::heavy_weighted_pressure_plate);
    registerBlock(Blocks::daylight_detector);
    registerBlock(Blocks::redstone_block);
    registerBlock(Blocks::quartz_ore);
    registerBlock(Blocks::hopper);
    registerBlock(Blocks::quartz_block,
        (new MultiBlockItem(Blocks::quartz_block, Blocks::quartz_block, QuartzBlockNameResolver, NULL))->setDescriptionId(Blocks::quartz_block->getDescriptionId(-1)));

    registerBlock(Blocks::quartz_stairs);
    registerBlock(Blocks::activator_rail);
    registerBlock(Blocks::dropper);
    registerBlock(Blocks::stained_hardened_clay,
        (new MultiBlockItem(Blocks::stained_hardened_clay, Blocks::stained_hardened_clay, ClayBlockNameResolver, NULL))->setDescriptionId(Blocks::stained_hardened_clay->getDescriptionId(-1)));

    registerBlock(Blocks::barrier);
    registerBlock(Blocks::iron_trapdoor);
    registerBlock(Blocks::hay_block);
    registerBlock(Blocks::carpet,
        (new MultiBlockItem(Blocks::carpet, Blocks::carpet, WoolCarpetBlockNameResolver, NULL))->setDescriptionId(Blocks::carpet->getDescriptionId(-1)));

    registerBlock(Blocks::hardened_clay);
    registerBlock(Blocks::coal_block);
    registerBlock(Blocks::packed_ice);
    registerBlock(Blocks::acacia_stairs);
    registerBlock(Blocks::dark_oak_stairs);
    registerBlock(Blocks::slime);
    registerBlock(Blocks::double_plant,
        (new MultiBlockItem(Blocks::double_plant, Blocks::double_plant, DoublePlantBlockNameResolver, DoublePlantBlockDescResolver))->setDescriptionId(Blocks::double_plant->getDescriptionId(-1)));

    registerBlock(Blocks::stained_glass,
        (new MultiBlockItem(Blocks::stained_glass, Blocks::stained_glass, GlassBlockNameResolver, NULL))->setDescriptionId(Blocks::stained_glass->getDescriptionId(-1)));

    registerBlock(Blocks::stained_glass_pane,
        (new MultiBlockItem(Blocks::stained_glass_pane, Blocks::stained_glass_pane, GlassPaneBlockNameResolver, NULL))->setDescriptionId(Blocks::stained_glass_pane->getDescriptionId(-1)));

    registerBlock(Blocks::prismarine,
        (new MultiBlockItem(Blocks::prismarine, Blocks::prismarine, PrismarineBlockNameResolver, PrismarineBlockDescResolver))->setDescriptionId(Blocks::prismarine->getDescriptionId(-1))->setBaseItemTypeAndMaterial(20, 54));

    registerBlock(Blocks::sea_lantern);
    registerBlock(Blocks::red_sandstone,
        (new MultiBlockItem(Blocks::red_sandstone, Blocks::red_sandstone, RedSandstoneBlockNameResolver, NULL))->setDescriptionId(Blocks::red_sandstone->getDescriptionId(-1)));

    registerBlock(Blocks::red_sandstone_stairs);
    registerBlock(Blocks::stone_slab2,
        (new SlabBlockItem(Blocks::stone_slab2,
                           static_cast<HalfSlabBlock*>(Blocks::stone_slab2),
                           static_cast<HalfSlabBlock*>(Blocks::double_stone_slab2)))->setDescriptionId(Blocks::stone_slab2->getDescriptionId(-1)));

    registerBlock(Blocks::repeating_command_block);
    registerBlock(Blocks::chain_command_block);
    registerBlock(Blocks::end_bricks);
    registerBlock(Blocks::beetroots);
    registerBlock(Blocks::grass_path);
    registerBlock(Blocks::frosted_ice);
    registerBlock(Blocks::magma);
    registerBlock(Blocks::nether_wart_block);
    registerBlock(Blocks::red_nether_brick);
    registerBlock(Blocks::bone_block);
    registerBlock(Blocks::end_rod);
    registerBlock(Blocks::chorus_plant);
    registerBlock(Blocks::chorus_flower);
    registerBlock(Blocks::purpur_block);
    registerBlock(Blocks::purpur_pillar);
    registerBlock(Blocks::purpur_stairs);
    registerBlock(Blocks::purpur_slab,
        (new SlabBlockItem(Blocks::purpur_slab,
                           static_cast<HalfSlabBlock*>(Blocks::purpur_slab),
                           static_cast<HalfSlabBlock*>(Blocks::purpur_double_slab)))->setDescriptionId(Blocks::purpur_slab->getDescriptionId(-1)));

    registerBlock(Blocks::observer);
    registerBlock(Blocks::white_shulker_box,
        new ShulkerBoxBlockItem(Blocks::white_shulker_box));

    registerBlock(Blocks::orange_shulker_box,
        new ShulkerBoxBlockItem(Blocks::orange_shulker_box));

    registerBlock(Blocks::magenta_shulker_box,
        new ShulkerBoxBlockItem(Blocks::magenta_shulker_box));

    registerBlock(Blocks::light_blue_shulker_box,
        new ShulkerBoxBlockItem(Blocks::light_blue_shulker_box));

    registerBlock(Blocks::yellow_shulker_box,
        new ShulkerBoxBlockItem(Blocks::yellow_shulker_box));

    registerBlock(Blocks::lime_shulker_box,
        new ShulkerBoxBlockItem(Blocks::lime_shulker_box));

    registerBlock(Blocks::pink_shulker_box,
        new ShulkerBoxBlockItem(Blocks::pink_shulker_box));

    registerBlock(Blocks::gray_shulker_box,
        new ShulkerBoxBlockItem(Blocks::gray_shulker_box));

    registerBlock(Blocks::silver_shulker_box,
        new ShulkerBoxBlockItem(Blocks::silver_shulker_box));

    registerBlock(Blocks::cyan_shulker_box,
        new ShulkerBoxBlockItem(Blocks::cyan_shulker_box));

    registerBlock(Blocks::purple_shulker_box,
        new ShulkerBoxBlockItem(Blocks::purple_shulker_box));

    registerBlock(Blocks::blue_shulker_box,
        new ShulkerBoxBlockItem(Blocks::blue_shulker_box));

    registerBlock(Blocks::brown_shulker_box,
        new ShulkerBoxBlockItem(Blocks::brown_shulker_box));

    registerBlock(Blocks::green_shulker_box,
        new ShulkerBoxBlockItem(Blocks::green_shulker_box));

    registerBlock(Blocks::red_shulker_box,
        new ShulkerBoxBlockItem(Blocks::red_shulker_box));

    registerBlock(Blocks::black_shulker_box,
        new ShulkerBoxBlockItem(Blocks::black_shulker_box));

    registerBlock(Blocks::white_glazed_terracotta);
    registerBlock(Blocks::orange_glazed_terracotta);
    registerBlock(Blocks::magenta_glazed_terracotta);
    registerBlock(Blocks::light_blue_glazed_terracotta);
    registerBlock(Blocks::yellow_glazed_terracotta);
    registerBlock(Blocks::lime_glazed_terracotta);
    registerBlock(Blocks::pink_glazed_terracotta);
    registerBlock(Blocks::gray_glazed_terracotta);
    registerBlock(Blocks::silver_glazed_terracotta);
    registerBlock(Blocks::cyan_glazed_terracotta);
    registerBlock(Blocks::purple_glazed_terracotta);
    registerBlock(Blocks::blue_glazed_terracotta);
    registerBlock(Blocks::brown_glazed_terracotta);
    registerBlock(Blocks::green_glazed_terracotta);
    registerBlock(Blocks::red_glazed_terracotta);
    registerBlock(Blocks::black_glazed_terracotta);
    registerBlock(Blocks::concrete,
        (new MultiBlockItem(Blocks::concrete, Blocks::concrete, ConcreteBlockNameResolver, NULL))->setDescriptionId(Blocks::concrete->getDescriptionId(-1)));

    registerBlock(Blocks::concrete_powder,
        (new MultiBlockItem(Blocks::concrete_powder, Blocks::concrete_powder, ConcretePowderBlockNameResolver, NULL))->setDescriptionId(Blocks::concrete_powder->getDescriptionId(-1)));

    registerBlock(Blocks::turtle_egg);
    registerBlock(Blocks::conduit);
    registerBlock(Blocks::kelp);
    registerBlock(Blocks::coral_block,
        new MultiBlockItem(Blocks::coral_block, Blocks::coral_block, CoralBlockNameResolver, CoralBlockDescResolver));

    registerBlock(Blocks::coral,
        new MultiBlockItem(Blocks::coral, Blocks::coral, CoralBushNameResolver, CoralBushDescResolver));

    registerBlock(Blocks::coral_fan,
        new CoralFanBlockItem(Blocks::coral_fan));

    registerBlock(Blocks::coral_fan_dead,
        new CoralFanBlockItem(Blocks::coral_fan_dead));

    registerBlock(Blocks::dried_kelp_block);
    registerBlock(Blocks::sea_grass);
    registerBlock(Blocks::sea_pickle,
        new SeaPickleBlockItem(Blocks::sea_pickle));

    registerBlock(Blocks::blue_ice);
    registerBlock(Blocks::prismarine_stairs);
    registerBlock(Blocks::prismarine_bricks_stairs);
    registerBlock(Blocks::dark_prismarine_stairs);
    registerBlock(Blocks::prismarine_double_slab,
        (new SlabBlockItem(Blocks::prismarine_double_slab,
                           static_cast<HalfSlabBlock*>(Blocks::prismarine_slab),
                           static_cast<HalfSlabBlock*>(Blocks::prismarine_double_slab)))->setDescriptionId(Blocks::prismarine_double_slab->getDescriptionId(-1)));

    registerBlock(Blocks::prismarine_slab,
        (new SlabBlockItem(Blocks::prismarine_slab,
                           static_cast<HalfSlabBlock*>(Blocks::prismarine_slab),
                           static_cast<HalfSlabBlock*>(Blocks::prismarine_double_slab)))->setDescriptionId(Blocks::prismarine_slab->getDescriptionId(-1)));

    registerBlock(Blocks::stripped_log_spruce);
    registerBlock(Blocks::stripped_log_birch);
    registerBlock(Blocks::stripped_log_jungle);
    registerBlock(Blocks::stripped_log_acacia);
    registerBlock(Blocks::stripped_log_dark_oak);
    registerBlock(Blocks::stripped_log_oak);
    registerBlock(Blocks::acacia_pressure_plate);
    registerBlock(Blocks::birch_pressure_plate);
    registerBlock(Blocks::dark_oak_pressure_plate);
    registerBlock(Blocks::jungle_pressure_plate);
    registerBlock(Blocks::spruce_pressure_plate);
    registerBlock(Blocks::acacia_button);
    registerBlock(Blocks::birch_button);
    registerBlock(Blocks::dark_oak_button);
    registerBlock(Blocks::jungle_button);
    registerBlock(Blocks::spruce_button);
    registerBlock(Blocks::bark_spruce);
    registerBlock(Blocks::bark_birch);
    registerBlock(Blocks::bark_acacia);
    registerBlock(Blocks::bark_jungle);
    registerBlock(Blocks::bark_dark_oak);
    registerBlock(Blocks::bark_oak);
    registerItem(256, L"iron_shovel",
        (new ShovelItem(Tier::IRON))->setIconName(L"shovelIron")->setBaseItemTypeAndMaterial(2, 3)->setDescriptionId(IDS_ITEM_SHOVEL_IRON, IDS_DESC_SHOVEL));

    registerItem(257, L"iron_pickaxe",
        (new PickaxeItem(Tier::IRON))->setIconName(L"pickaxeIron")->setBaseItemTypeAndMaterial(3, 3)->setDescriptionId(IDS_ITEM_PICKAXE_IRON, 0x8161B947));

    registerItem(258, L"iron_axe",
        (new AxeItem(Tier::IRON))->setIconName(L"hatchetIron")->setBaseItemTypeAndMaterial(4, 3)->setDescriptionId(0xC55D95AC, 0x4D38CA25));

    registerItem(259, L"flint_and_steel",
        (new FlintAndSteelItem())->setIconName(L"flintAndSteel")->setBaseItemTypeAndMaterial(28, 26)->setDescriptionId(IDS_ITEM_FLINT_AND_STEEL, 0x46397603));

    registerItem(260, L"apple",
        (new FoodItem(4, 0.3f, false))->setIconName(L"apple")->setDescriptionId(IDS_ITEM_APPLE, IDS_DESC_APPLE));

    registerItem(261, L"bow",
        (new BowItem())->setIconName(L"bow")->setBaseItemTypeAndMaterial(24, 17)->setDescriptionId(IDS_ITEM_BOW, 0x07341480));

    registerItem(262, L"arrow",
        (new ArrowItem())->setIconName(L"arrow")->setBaseItemTypeAndMaterial(24, 18)->setDescriptionId(IDS_ITEM_ARROW, 0x57C4CE0B)->SetPreferInventory(true)->setAllowOffhand(true));

    registerItem(263, L"coal",
        (new CoalItem())->setIconName(L"coal")->setBaseItemTypeAndMaterial(31, 49)->setDescriptionId(IDS_ITEM_COAL, 0xE13DA78B));

    registerItem(264, L"diamond",
        (new Item())->setIconName(L"diamond")->setBaseItemTypeAndMaterial(31, 5)->setDescriptionId(IDS_ITEM_DIAMOND, 0x1FCFAFCD));

    registerItem(265, L"iron_ingot",
        (new Item())->setIconName(L"ingotIron")->setBaseItemTypeAndMaterial(31, 3)->setDescriptionId(IDS_ITEM_INGOT_IRON, IDS_DESC_INGOT));

    registerItem(266, L"gold_ingot",
        (new Item())->setIconName(L"ingotGold")->setBaseItemTypeAndMaterial(31, 4)->setDescriptionId(IDS_ITEM_INGOT_GOLD, IDS_DESC_INGOT));

    registerItem(267, L"iron_sword",
        (new SwordItem(Tier::IRON))->setIconName(L"swordIron")->setBaseItemTypeAndMaterial(1, 3)->setDescriptionId(IDS_ITEM_SWORD_IRON, 0x5DE5CBCF));

    registerItem(268, L"wooden_sword",
        (new SwordItem(Tier::WOOD))->setIconName(L"swordWood")->setBaseItemTypeAndMaterial(1, 1)->setDescriptionId(IDS_ITEM_SWORD_WOOD, 0x5DE5CBCF));

    registerItem(269, L"wooden_shovel",
        (new ShovelItem(Tier::WOOD))->setIconName(L"shovelWood")->setBaseItemTypeAndMaterial(2, 1)->setDescriptionId(IDS_ITEM_SHOVEL_WOOD, IDS_DESC_SHOVEL));

    registerItem(270, L"wooden_pickaxe",
        (new PickaxeItem(Tier::WOOD))->setIconName(L"pickaxeWood")->setBaseItemTypeAndMaterial(3, 1)->setDescriptionId(IDS_ITEM_PICKAXE_WOOD, 0x8161B947));

    registerItem(271, L"wooden_axe",
        (new AxeItem(Tier::WOOD))->setIconName(L"hatchetWood")->setBaseItemTypeAndMaterial(4, 1)->setDescriptionId(0x27EAFCDD, 0x4D38CA25));

    registerItem(272, L"stone_sword",
        (new SwordItem(Tier::STONE))->setIconName(L"swordStone")->setBaseItemTypeAndMaterial(1, 2)->setDescriptionId(IDS_ITEM_SWORD_STONE, 0x5DE5CBCF));

    registerItem(273, L"stone_shovel",
        (new ShovelItem(Tier::STONE))->setIconName(L"shovelStone")->setBaseItemTypeAndMaterial(2, 2)->setDescriptionId(IDS_ITEM_SHOVEL_STONE, IDS_DESC_SHOVEL));

    registerItem(274, L"stone_pickaxe",
        (new PickaxeItem(Tier::STONE))->setIconName(L"pickaxeStone")->setBaseItemTypeAndMaterial(3, 2)->setDescriptionId(IDS_ITEM_PICKAXE_STONE, 0x8161B947));

    registerItem(275, L"stone_axe",
        (new AxeItem(Tier::STONE))->setIconName(L"hatchetStone")->setBaseItemTypeAndMaterial(4, 2)->setDescriptionId(0x38C2FBED, 0x4D38CA25));

    registerItem(276, L"diamond_sword",
        (new SwordItem(Tier::DIAMOND))->setIconName(L"swordDiamond")->setBaseItemTypeAndMaterial(1, 5)->setDescriptionId(IDS_ITEM_SWORD_DIAMOND, 0x5DE5CBCF));

    registerItem(277, L"diamond_shovel",
        (new ShovelItem(Tier::DIAMOND))->setIconName(L"shovelDiamond")->setBaseItemTypeAndMaterial(2, 5)->setDescriptionId(IDS_ITEM_SHOVEL_DIAMOND, IDS_DESC_SHOVEL));

    registerItem(278, L"diamond_pickaxe",
        (new PickaxeItem(Tier::DIAMOND))->setIconName(L"pickaxeDiamond")->setBaseItemTypeAndMaterial(3, 5)->setDescriptionId(IDS_ITEM_PICKAXE_DIAMOND, 0x8161B947));

    registerItem(279, L"diamond_axe",
        (new AxeItem(Tier::DIAMOND))->setIconName(L"hatchetDiamond")->setBaseItemTypeAndMaterial(4, 5)->setDescriptionId(0x962A37C2, 0x4D38CA25));

    registerItem(280, L"stick",
        (new Item())->setIconName(L"stick")->handEquipped()->setDescriptionId(IDS_ITEM_STICK, 0x3F88E88C)->setBaseItemTypeAndMaterial(48, 1));

    registerItem(281, L"bowl",
        (new Item())->setIconName(L"bowl")->setBaseItemTypeAndMaterial(26, 1)->setDescriptionId(IDS_ITEM_BOWL, IDS_DESC_BOWL)->setMaxStackSize(64));

    registerItem(282, L"mushroom_stew",
        (new BowlFoodItem(6))->setIconName(L"mushroomStew")->setBaseItemTypeAndMaterial(39, 0)->setDescriptionId(IDS_ITEM_MUSHROOM_STEW, 0xF87FB06B));

    registerItem(283, L"golden_sword",
        (new SwordItem(Tier::GOLD))->setIconName(L"swordGold")->setBaseItemTypeAndMaterial(1, 4)->setDescriptionId(IDS_ITEM_SWORD_GOLD, 0x5DE5CBCF));

    registerItem(284, L"golden_shovel",
        (new ShovelItem(Tier::GOLD))->setIconName(L"shovelGold")->setBaseItemTypeAndMaterial(2, 4)->setDescriptionId(IDS_ITEM_SHOVEL_GOLD, IDS_DESC_SHOVEL));

    registerItem(285, L"golden_pickaxe",
        (new PickaxeItem(Tier::GOLD))->setIconName(L"pickaxeGold")->setBaseItemTypeAndMaterial(3, 4)->setDescriptionId(IDS_ITEM_PICKAXE_GOLD, 0x8161B947));

    registerItem(286, L"golden_axe",
        (new AxeItem(Tier::GOLD))->setIconName(L"hatchetGold")->setBaseItemTypeAndMaterial(4, 4)->setDescriptionId(0xA0C4155E, 0x4D38CA25));

    registerItem(287, L"string",
        (new BlockPlanterItem(Blocks::tripwire))->setIconName(L"string")->setDescriptionId(IDS_ITEM_STRING, 0xC6844E97));

    registerItem(288, L"feather",
        (new Item())->setIconName(L"feather")->setDescriptionId(IDS_ITEM_FEATHER, 0xF3341313));

    registerItem(289, L"gunpowder",
        (new Item())->setIconName(L"sulphur")->setDescriptionId(0x4BF44373, 0xF7E6C72F));

    registerItem(290, L"wooden_hoe",
        (new HoeItem(Tier::WOOD))->setIconName(L"hoeWood")->setBaseItemTypeAndMaterial(5, 1)->setDescriptionId(IDS_ITEM_HOE_WOOD, 0x6166FAC8));

    registerItem(291, L"stone_hoe",
        (new HoeItem(Tier::STONE))->setIconName(L"hoeStone")->setBaseItemTypeAndMaterial(5, 2)->setDescriptionId(IDS_ITEM_HOE_STONE, 0x6166FAC8));

    registerItem(292, L"iron_hoe",
        (new HoeItem(Tier::IRON))->setIconName(L"hoeIron")->setBaseItemTypeAndMaterial(5, 3)->setDescriptionId(IDS_ITEM_HOE_IRON, 0x6166FAC8));

    registerItem(293, L"diamond_hoe",
        (new HoeItem(Tier::DIAMOND))->setIconName(L"hoeDiamond")->setBaseItemTypeAndMaterial(5, 5)->setDescriptionId(IDS_ITEM_HOE_DIAMOND, 0x6166FAC8));

    registerItem(294, L"golden_hoe",
        (new HoeItem(Tier::GOLD))->setIconName(L"hoeGold")->setBaseItemTypeAndMaterial(5, 4)->setDescriptionId(IDS_ITEM_HOE_GOLD, 0x6166FAC8));

    registerItem(295, L"wheat_seeds",
        (new SeedItem(Blocks::wheat, Blocks::farmland))->setIconName(L"seeds")->setDescriptionId(0x95C975B2, 0x999D8B6E));

    registerItem(296, L"wheat",
        (new Item())->setIconName(L"wheat")->setBaseItemTypeAndMaterial(31, 53)->setDescriptionId(IDS_ITEM_WHEAT, 0x9CB06D4D));

    registerItem(297, L"bread",
        (new FoodItem(5, 0.6f, false))->setIconName(L"bread")->setDescriptionId(IDS_ITEM_BREAD, 0x791B364A));

    registerItem(298, L"leather_helmet",
        (new ArmorItem(ArmorItem::ArmorMaterial::LEATHER, 0, EquipmentSlot::HEAD))->setIconName(L"helmetCloth")->setBaseItemTypeAndMaterial(7, 6)->setDescriptionId(0xF761EBF0, 0xB114599B));

    registerItem(299, L"leather_chestplate",
        (new ArmorItem(ArmorItem::ArmorMaterial::LEATHER, 0, EquipmentSlot::CHEST))->setIconName(L"chestplateCloth")->setBaseItemTypeAndMaterial(8, 6)->setDescriptionId(0xEABFA580, 0x05E7B53B));

    registerItem(300, L"leather_leggings",
        (new ArmorItem(ArmorItem::ArmorMaterial::LEATHER, 0, EquipmentSlot::LEGS))->setIconName(L"leggingsCloth")->setBaseItemTypeAndMaterial(9, 6)->setDescriptionId(0x25CD26DF, 0x0F1B63F4));

    registerItem(301, L"leather_boots",
        (new ArmorItem(ArmorItem::ArmorMaterial::LEATHER, 0, EquipmentSlot::FEET))->setIconName(L"bootsCloth")->setBaseItemTypeAndMaterial(10, 6)->setDescriptionId(0x92747ED8, 0xCF27E723));

    registerItem(302, L"chainmail_helmet",
        (new ArmorItem(ArmorItem::ArmorMaterial::CHAIN, 1, EquipmentSlot::HEAD))->setIconName(L"helmetChain")->setBaseItemTypeAndMaterial(7, 7)->setDescriptionId(IDS_ITEM_HELMET_CHAIN, 0xCEB3986D));

    registerItem(303, L"chainmail_chestplate",
        (new ArmorItem(ArmorItem::ArmorMaterial::CHAIN, 1, EquipmentSlot::CHEST))->setIconName(L"chestplateChain")->setBaseItemTypeAndMaterial(8, 7)->setDescriptionId(IDS_ITEM_CHESTPLATE_CHAIN, 0x4028FB8D));

    registerItem(304, L"chainmail_leggings",
        (new ArmorItem(ArmorItem::ArmorMaterial::CHAIN, 1, EquipmentSlot::LEGS))->setIconName(L"leggingsChain")->setBaseItemTypeAndMaterial(9, 7)->setDescriptionId(IDS_ITEM_LEGGINGS_CHAIN, 0x36108302));

    registerItem(305, L"chainmail_boots",
        (new ArmorItem(ArmorItem::ArmorMaterial::CHAIN, 1, EquipmentSlot::FEET))->setIconName(L"bootsChain")->setBaseItemTypeAndMaterial(10, 7)->setDescriptionId(IDS_ITEM_BOOTS_CHAIN, 0x8F8739A5));

    registerItem(306, L"iron_helmet",
        (new ArmorItem(ArmorItem::ArmorMaterial::IRON, 2, EquipmentSlot::HEAD))->setIconName(L"helmetIron")->setBaseItemTypeAndMaterial(7, 3)->setDescriptionId(IDS_ITEM_HELMET_IRON, 0xACDE6D0A));

    registerItem(307, L"iron_chestplate",
        (new ArmorItem(ArmorItem::ArmorMaterial::IRON, 2, EquipmentSlot::CHEST))->setIconName(L"chestplateIron")->setBaseItemTypeAndMaterial(8, 3)->setDescriptionId(IDS_ITEM_CHESTPLATE_IRON, 0x09958B2A));

    registerItem(308, L"iron_leggings",
        (new ArmorItem(ArmorItem::ArmorMaterial::IRON, 2, EquipmentSlot::LEGS))->setIconName(L"leggingsIron")->setBaseItemTypeAndMaterial(9, 3)->setDescriptionId(IDS_ITEM_LEGGINGS_IRON, 0x0FA8857D));

    registerItem(309, L"iron_boots",
        (new ArmorItem(ArmorItem::ArmorMaterial::IRON, 2, EquipmentSlot::FEET))->setIconName(L"bootsIron")->setBaseItemTypeAndMaterial(10, 3)->setDescriptionId(IDS_ITEM_BOOTS_IRON, 0xD0506AE2));

    registerItem(310, L"diamond_helmet",
        (new ArmorItem(ArmorItem::ArmorMaterial::DIAMOND, 3, EquipmentSlot::HEAD))->setIconName(L"helmetDiamond")->setBaseItemTypeAndMaterial(7, 5)->setDescriptionId(IDS_ITEM_HELMET_DIAMOND, 0x1348B454));

    registerItem(311, L"diamond_chestplate",
        (new ArmorItem(ArmorItem::ArmorMaterial::DIAMOND, 3, EquipmentSlot::CHEST))->setIconName(L"chestplateDiamond")->setBaseItemTypeAndMaterial(8, 5)->setDescriptionId(IDS_ITEM_CHESTPLATE_DIAMOND, 0x4FC26274));

    registerItem(312, L"diamond_leggings",
        (new ArmorItem(ArmorItem::ArmorMaterial::DIAMOND, 3, EquipmentSlot::LEGS))->setIconName(L"leggingsDiamond")->setBaseItemTypeAndMaterial(9, 5)->setDescriptionId(IDS_ITEM_LEGGINGS_DIAMOND, 0x475471AB));

    registerItem(313, L"diamond_boots",
        (new ArmorItem(ArmorItem::ArmorMaterial::DIAMOND, 3, EquipmentSlot::FEET))->setIconName(L"bootsDiamond")->setBaseItemTypeAndMaterial(10, 5)->setDescriptionId(IDS_ITEM_BOOTS_DIAMOND, 0xB58229BC));

    registerItem(314, L"golden_helmet",
        (new ArmorItem(ArmorItem::ArmorMaterial::GOLD, 4, EquipmentSlot::HEAD))->setIconName(L"helmetGold")->setBaseItemTypeAndMaterial(7, 4)->setDescriptionId(IDS_ITEM_HELMET_GOLD, 0x96B464F8));

    registerItem(315, L"golden_chestplate",
        (new ArmorItem(ArmorItem::ArmorMaterial::GOLD, 4, EquipmentSlot::CHEST))->setIconName(L"chestplateGold")->setBaseItemTypeAndMaterial(8, 4)->setDescriptionId(IDS_ITEM_CHESTPLATE_GOLD, 0xCAE9EF98));

    registerItem(316, L"golden_leggings",
        (new ArmorItem(ArmorItem::ArmorMaterial::GOLD, 4, EquipmentSlot::LEGS))->setIconName(L"leggingsGold")->setBaseItemTypeAndMaterial(9, 4)->setDescriptionId(IDS_ITEM_LEGGINGS_GOLD, 0x09C244BF));

    registerItem(317, L"golden_boots",
        (new ArmorItem(ArmorItem::ArmorMaterial::GOLD, 4, EquipmentSlot::FEET))->setIconName(L"bootsGold")->setBaseItemTypeAndMaterial(10, 4)->setDescriptionId(IDS_ITEM_BOOTS_GOLD, 0x27F702C0));

    registerItem(318, L"flint",
        (new Item())->setIconName(L"flint")->setDescriptionId(IDS_ITEM_FLINT, IDS_DESC_FLINT));

    registerItem(319, L"porkchop",
        (new FoodItem(3, 0.3f, true))->setIconName(L"porkchopRaw")->setDescriptionId(IDS_ITEM_PORKCHOP_RAW, 0x3A857AE3));

    registerItem(320, L"cooked_porkchop",
        (new FoodItem(8, 0.8f, true))->setIconName(L"porkchopCooked")->setDescriptionId(IDS_ITEM_PORKCHOP_COOKED, 0x3351245E));

    registerItem(321, L"painting",
        (new HangingEntityItem(168296449))->setIconName(L"painting")->setBaseItemTypeAndMaterial(34, 6)->setDescriptionId(IDS_ITEM_PAINTING, 0xE2BB6816));

    GoldenAppleItem* goldenApple = new GoldenAppleItem(4, 1.2f, false);
    goldenApple->setCanAlwaysEat();
    registerItem(322, L"golden_apple",
        goldenApple->setIconName(L"appleGold")->setBaseItemTypeAndMaterial(38, 46)->setDescriptionId(IDS_ITEM_APPLE_GOLD));

    registerItem(323, L"sign",
        (new SignItem())->setIconName(L"sign")->setBaseItemTypeAndMaterial(34, 1)->setDescriptionId(IDS_ITEM_SIGN, 0xB6286B39));

    registerItem(324, L"wooden_door",
        (new DoorItem(Blocks::wooden_door))->setIconName(L"doorWood")->setBaseItemTypeAndMaterial(6, 1)->setDescriptionId(0x3C9F77B1, IDS_DESC_DOOR_WOOD));

    Item* bucket = (new BucketItem(BucketItem::EMPTY))->setMaxStackSize(16)->setIconName(L"bucket")->setBaseItemTypeAndMaterial(26, 24)->setDescriptionId(IDS_ITEM_BUCKET, 0xD5C581EC);
    registerItem(325, L"bucket",
        bucket);

    registerItem(326, L"water_bucket",
        (new BucketItem(BucketItem::WATER))->setIconName(L"bucketWater")->setDescriptionId(IDS_ITEM_BUCKET_WATER, IDS_DESC_BUCKET_WATER)->setCraftingRemainingItem(bucket));

    registerItem(327, L"lava_bucket",
        (new BucketItem(BucketItem::LAVA))->setIconName(L"bucketLava")->setDescriptionId(IDS_ITEM_BUCKET_LAVA, IDS_DESC_BUCKET_LAVA)->setCraftingRemainingItem(bucket));

    registerItem(328, L"minecart",
        (new MinecartItem(new Minecart::Type(0)))->setIconName(L"minecart")->setDescriptionId(IDS_ITEM_MINECART, 0xDF071B81));

    registerItem(329, L"saddle",
        (new SaddleItem())->setIconName(L"saddle")->setDescriptionId(IDS_ITEM_SADDLE, 0xB6415569));

    registerItem(330, L"iron_door",
        (new DoorItem(Blocks::iron_door))->setIconName(L"doorIron")->setBaseItemTypeAndMaterial(6, 3)->setDescriptionId(IDS_ITEM_DOOR_IRON, IDS_DESC_DOOR_IRON));

    registerItem(331, L"redstone",
        (new RedStoneDustItem())->setIconName(L"redstone")->setBaseItemTypeAndMaterial(31, 48)->setDescriptionId(IDS_ITEM_REDSTONE, 0x83B8379B));

    registerItem(332, L"snowball",
        (new SnowballItem())->setIconName(L"snowball")->setDescriptionId(IDS_ITEM_SNOWBALL, IDS_DESC_SNOWBALL));

    registerItem(333, L"boat",
        (new BoatItem(Boat::OAK))->setIconName(L"boat")->setDescriptionId(IDS_ITEM_BOAT_OAK, 0x1BAB2B22)->setBaseItemTypeAndMaterial(55, 63));

    registerItem(334, L"leather",
        (new Item())->setIconName(L"leather")->setDescriptionId(IDS_ITEM_LEATHER, 0xCB787D79)->setBaseItemTypeAndMaterial(53, 0));

    registerItem(335, L"milk_bucket",
        (new BucketItem(BucketItem::MILK))->setIconName(L"milk")->setDescriptionId(IDS_ITEM_BUCKET_MILK, IDS_DESC_BUCKET_MILK)->setCraftingRemainingItem(bucket));

    registerItem(336, L"brick",
        (new Item())->setIconName(L"brick")->setDescriptionId(IDS_ITEM_BRICK, 0x043FC433));

    registerItem(337, L"clay_ball",
        (new Item())->setIconName(L"clay")->setDescriptionId(IDS_ITEM_CLAY, 0xA273EEED));

    registerItem(338, L"reeds",
        (new BlockPlanterItem(Blocks::reeds))->setIconName(L"reeds")->setDescriptionId(0x15F282BB, 0x298FE4B7));

    registerItem(339, L"paper",
        (new Item())->setIconName(L"paper")->setBaseItemTypeAndMaterial(46, 50)->setDescriptionId(IDS_ITEM_PAPER, 0xA5BBD614));

    registerItem(340, L"book",
        (new BookItem())->setIconName(L"book")->setBaseItemTypeAndMaterial(46, 51)->setDescriptionId(IDS_ITEM_BOOK, IDS_DESC_BOOK));

    registerItem(341, L"slime_ball",
        (new Item())->setBaseItemTypeAndMaterial(31, 57)->setIconName(L"slimeball")->setDescriptionId(IDS_ITEM_SLIMEBALL, IDS_DESC_SLIMEBALL));

    registerItem(342, L"chest_minecart",
        (new MinecartItem(new Minecart::Type(1)))->setIconName(L"minecart_chest")->setDescriptionId(IDS_ITEM_MINECART_CHEST, 0x3ED319BA));

    registerItem(343, L"furnace_minecart",
        (new MinecartItem(new Minecart::Type(2)))->setIconName(L"minecart_furnace")->setDescriptionId(IDS_ITEM_MINECART_FURNACE, 0xBB5D88CB));

    registerItem(344, L"egg",
        (new EggItem())->setIconName(L"egg")->setDescriptionId(IDS_ITEM_EGG, 0x7E0AC927));

    registerItem(345, L"compass",
        (new CompassItem())->setIconName(L"compass")->setBaseItemTypeAndMaterial(25, 19)->setDescriptionId(IDS_ITEM_COMPASS, 0xDA63BBF2));

    registerItem(346, L"fishing_rod",
        (new FishingRodItem())->setIconName(L"fishingRod")->setBaseItemTypeAndMaterial(37, 1)->setDescriptionId(IDS_ITEM_FISHING_ROD, IDS_DESC_FISHINGROD));

    registerItem(347, L"clock",
        (new ClockItem())->setIconName(L"clock")->setBaseItemTypeAndMaterial(25, 20)->setDescriptionId(IDS_ITEM_CLOCK, 0x6B05B9D2));

    registerItem(348, L"glowstone_dust",
        (new Item())->setIconName(L"yellowDust")->setDescriptionId(0xD8D0E583, 0x2C633DEF));

    registerItem(349, L"fish",
        (new FishFoodItem(false))->setIconName(L"fishRaw")->setDescriptionId(0xFB51769E, 0x224B0B42)->setStackedByData(true));

    registerItem(350, L"cooked_fish",
        (new FishFoodItem(true))->setIconName(L"fishCooked")->setDescriptionId(0x692511AB, 0x51994487)->setStackedByData(true));

    registerItem(351, L"dye",
        (new DyeItem())->setIconName(L"dyePowder")->setBaseItemTypeAndMaterial(18, 12)->setDescriptionId(IDS_ITEM_DYE_POWDER, -1));

    registerItem(352, L"bone",
        (new Item())->setIconName(L"bone")->setDescriptionId(IDS_ITEM_BONE, IDS_DESC_BONE)->handEquipped());

    registerItem(353, L"sugar",
        (new Item())->setIconName(L"sugar")->setDescriptionId(IDS_ITEM_SUGAR, 0x14DD1B10));

    registerItem(354, L"cake",
        (new BlockPlanterItem(Blocks::cake))->setIconName(L"cake")->setDescriptionId(IDS_ITEM_CAKE, 0x1BA88C3E));

    registerItem(355, L"bed",
        (new BedItem())->setMaxStackSize(1)->setIconName(L"bed")->setBaseItemTypeAndMaterial(58, 71)->setDescriptionId(IDS_ITEM_BED, IDS_DESC_BED));

    registerItem(356, L"repeater",
        (new BlockPlanterItem(Blocks::unpowered_repeater))->setIconName(L"diode")->setDescriptionId(0xB97B9D5D, IDS_DESC_REDSTONEREPEATER)->setBaseItemTypeAndMaterial(59, 0));

    registerItem(357, L"cookie",
        (new FoodItem(2, 0.1f, false))->setIconName(L"cookie")->setDescriptionId(IDS_ITEM_COOKIE, 0x566BFA2E));

    registerItem(358, L"filled_map",
        (new MapItem())->setIconName(L"map")->setBaseItemTypeAndMaterial(25, 21)->setDescriptionId(IDS_ITEM_MAP, 0xADF9D986)->setAllowOffhand(true));

    registerItem(359, L"shears",
        (new ShearsItem())->setIconName(L"shears")->setBaseItemTypeAndMaterial(28, 27)->setDescriptionId(IDS_ITEM_SHEARS, IDS_DESC_SHEEP_HARVEST_COBWEB_LEAVE_SEAGRAS_BLOCK_DESCRIPTION));

    registerItem(360, L"melon",
        (new FoodItem(2, 0.3f, false))->setIconName(L"melon")->setDescriptionId(IDS_ITEM_MELON_SLICE, 0x2F0A0A78));

    registerItem(361, L"pumpkin_seeds",
        (new SeedItem(Blocks::pumpkin_stem, Blocks::farmland))->setIconName(L"seeds_pumpkin")->setBaseItemTypeAndMaterial(33, 22)->setDescriptionId(IDS_ITEM_PUMPKIN_SEEDS, IDS_DESC_PUMPKIN_SEEDS));

    registerItem(362, L"melon_seeds",
        (new SeedItem(Blocks::melon_stem, Blocks::farmland))->setIconName(L"seeds_melon")->setBaseItemTypeAndMaterial(33, 37)->setDescriptionId(IDS_ITEM_MELON_SEEDS, IDS_DESC_MELON_SEEDS));

    registerItem(363, L"beef",
        (new FoodItem(3, 0.3f, true))->setIconName(L"beefRaw")->setDescriptionId(IDS_ITEM_BEEF_RAW, 0x93F6A0CD));

    registerItem(364, L"cooked_beef",
        (new FoodItem(8, 0.8f, true))->setIconName(L"beefCooked")->setDescriptionId(0x19A67FAC, 0xBAF03780));

    registerItem(365, L"chicken",
        (new FoodItem(2, 0.3f, true))->setEatEffect(new MobEffectInstance(MobEffects::HUNGER, 600, 0), 0.3f)->setIconName(L"chickenRaw")->setDescriptionId(IDS_ITEM_CHICKEN_RAW, 0xFDE89328));

    registerItem(366, L"cooked_chicken",
        (new FoodItem(6, 0.6f, true))->setIconName(L"chickenCooked")->setDescriptionId(IDS_ITEM_CHICKEN_COOKED, 0x7AB879ED));

    registerItem(367, L"rotten_flesh",
        (new FoodItem(4, 0.1f, true))->setEatEffect(new MobEffectInstance(MobEffects::HUNGER, 600, 0), 0.8f)->setIconName(L"rottenFlesh")->setDescriptionId(IDS_ITEM_ROTTEN_FLESH, 0x75420D7F));

    registerItem(368, L"ender_pearl",
        (new EnderpearlItem())->setIconName(L"enderPearl")->setDescriptionId(IDS_ITEM_ENDER_PEARL, IDS_DESC_ENDER_PEARL));

    registerItem(369, L"blaze_rod",
        (new Item())->setIconName(L"blazeRod")->setDescriptionId(IDS_ITEM_BLAZE_ROD, IDS_DESC_BLAZE_ROD)->handEquipped());

    registerItem(370, L"ghast_tear",
        (new Item())->setIconName(L"ghastTear")->setDescriptionId(IDS_ITEM_GHAST_TEAR, IDS_DESC_GHAST_TEAR));

    registerItem(371, L"gold_nugget",
        (new Item())->setIconName(L"goldNugget")->setBaseItemTypeAndMaterial(31, 4)->setDescriptionId(IDS_ITEM_GOLD_NUGGET, 0xD366D7E7));

    registerItem(372, L"nether_wart",
        (new SeedItem(Blocks::nether_wart, Blocks::soul_sand))->setIconName(L"netherStalkSeeds")->setDescriptionId(0xFB353413, 0x5B73F84F));

    registerItem(373, L"potion",
        (new PotionItem())->setIconName(L"potion")->setDescriptionId(IDS_ITEM_POTION, 0xBA18E901));

    Item* glassBottle = (new BottleItem())->setIconName(L"glassBottle")->setBaseItemTypeAndMaterial(26, 34)->setDescriptionId(IDS_ITEM_GLASS_BOTTLE, 0x5B174183);
    registerItem(374, L"glass_bottle",
        glassBottle);

    registerItem(375, L"spider_eye",
        (new FoodItem(2, 0.8f, false))->setEatEffect(new MobEffectInstance(MobEffects::POISON, 100, 0), 1.0f)->setIconName(L"spiderEye")->setDescriptionId(IDS_ITEM_SPIDER_EYE, IDS_DESC_SPIDER_EYE));

    registerItem(376, L"fermented_spider_eye",
        (new Item())->setIconName(L"fermentedSpiderEye")->setDescriptionId(IDS_ITEM_FERMENTED_SPIDER_EYE, 0x83378EC0));

    registerItem(377, L"blaze_powder",
        (new Item())->setIconName(L"blazePowder")->setDescriptionId(IDS_ITEM_BLAZE_POWDER, 0xDF506406));

    registerItem(378, L"magma_cream",
        (new Item())->setIconName(L"magmaCream")->setDescriptionId(IDS_ITEM_MAGMA_CREAM, 0x5BB12FE2));

    registerItem(379, L"brewing_stand",
        (new BlockPlanterItem(Blocks::brewing_stand))->setIconName(L"brewingStand")->setBaseItemTypeAndMaterial(30, 35)->setDescriptionId(IDS_ITEM_BREWING_STAND, 0x8DAD714B));

    registerItem(380, L"cauldron",
        (new BlockPlanterItem(Blocks::cauldron))->setIconName(L"cauldron")->setBaseItemTypeAndMaterial(26, 3)->setDescriptionId(IDS_ITEM_CAULDRON, 0x92B04126));

    registerItem(381, L"ender_eye",
        (new EnderEyeItem())->setIconName(L"eyeOfEnder")->setBaseItemTypeAndMaterial(25, 33)->setDescriptionId(IDS_ITEM_EYE_OF_ENDER, IDS_DESC_EYE_OF_ENDER));

    registerItem(382, L"speckled_melon",
        (new Item())->setIconName(L"speckledMelon")->setBaseItemTypeAndMaterial(38, 37)->setDescriptionId(0x83DF6357, 0xE080A7FB));

    registerItem(383, L"spawn_egg",
        (new SpawnEggItem())->setIconName(L"monsterPlacer")->setDescriptionId(0x9FA6F429, 0x16B8B295));

    registerItem(384, L"experience_bottle",
        (new ExperienceBottleItem())->setIconName(L"expBottle")->setDescriptionId(0x8EAA3E30, 0xC511BB2C));

    registerItem(385, L"fire_charge",
        (new FireChargeItem())->setIconName(L"fireball")->setBaseItemTypeAndMaterial(23, 38)->setDescriptionId(IDS_ITEM_FIREBALL, IDS_DESC_FIREBALL));

    registerItem(386, L"writable_book",
        (new WritableBookItem())->setIconName(L"writingBook")->setBaseItemTypeAndMaterial(46, 51)->setDescriptionId(IDS_ITEM_BOOK_AND_QUILL, IDS_DESC_WRITINGBOOK));

    registerItem(387, L"written_book",
        (new WrittenBookItem())->setIconName(L"writtenBook")->setDescriptionId(IDS_ITEM_WRITTEN_BOOK, IDS_DESC_WRITTENBOOK)->setMaxStackSize(16));

    registerItem(388, L"emerald",
        (new Item())->setIconName(L"emerald")->setBaseItemTypeAndMaterial(31, 44)->setDescriptionId(IDS_ITEM_EMERALD, 0x25A6420C));

    registerItem(389, L"item_frame",
        (new HangingEntityItem(168296450))->setIconName(L"frame")->setBaseItemTypeAndMaterial(34, 34)->setDescriptionId(IDS_ITEM_ITEMFRAME, IDS_DESC_ITEMFRAME));

    registerItem(390, L"flower_pot",
        (new BlockPlanterItem(Blocks::flower_pot))->setIconName(L"flowerPot")->setDescriptionId(IDS_FLOWERPOT, IDS_DESC_FLOWERPOT)->setBaseItemTypeAndMaterial(53, 0));

    registerItem(391, L"carrot",
        (new SeedFoodItem(3, 0.6f, Blocks::carrots, Blocks::farmland))->setIconName(L"carrots")->setDescriptionId(IDS_CARROTS, IDS_DESC_CARROTS));

    registerItem(392, L"potato",
        (new SeedFoodItem(1, 0.3f, Blocks::potatoes, Blocks::farmland))->setIconName(L"potato")->setDescriptionId(IDS_POTATO, 0x6FE336FB));

    registerItem(393, L"baked_potato",
        (new FoodItem(5, 0.6f, false))->setIconName(L"potatoBaked")->setDescriptionId(IDS_ITEM_POTATO_BAKED, 0xCF1F0015));

    registerItem(394, L"poisonous_potato",
        (new FoodItem(2, 0.3f, false))->setEatEffect(new MobEffectInstance(MobEffects::POISON, 100, 0), 0.6f)->setIconName(L"potatoPoisonous")->setDescriptionId(IDS_ITEM_POTATO_POISONOUS, 0xC8C7EDE9));

    registerItem(395, L"map",
        (new EmptyMapItem())->setIconName(L"map_empty")->setBaseItemTypeAndMaterial(25, 21)->setDescriptionId(IDS_ITEM_MAP_EMPTY, 0x8842D8BC));

    registerItem(396, L"golden_carrot",
        (new FoodItem(6, 1.2f, false))->setIconName(L"carrotGolden")->setBaseItemTypeAndMaterial(38, 47)->setDescriptionId(IDS_ITEM_CARROT_GOLDEN, IDS_DESC_CARROT_GOLDEN));

    registerItem(397, L"skull",
        (new SkullItem())->setIconName(L"")->setDescriptionId(IDS_ITEM_SKULL, 0x896AE85F));

    registerItem(398, L"carrot_on_a_stick",
        (new CarrotOnAStickItem())->setIconName(L"carrotOnAStick")->setBaseItemTypeAndMaterial(37, 47)->setDescriptionId(IDS_ITEM_CARROT_ON_A_STICK, IDS_DESC_CARROT_ON_A_STICK));

    registerItem(399, L"nether_star",
        (new SimpleFoiledItem())->setIconName(L"nether_star")->setDescriptionId(IDS_NETHER_STAR, 0xC21DEE99));

    registerItem(400, L"pumpkin_pie",
        (new FoodItem(8, 0.3f, false))->setIconName(L"pumpkinPie")->setDescriptionId(IDS_ITEM_PUMPKIN_PIE, 0x857DE8BD));

    registerItem(401, L"fireworks",
        (new FireworksItem())->setIconName(L"fireworks")->setBaseItemTypeAndMaterial(44, 0)->setDescriptionId(IDS_FIREWORKS, IDS_DESC_FIREWORKS));

    registerItem(402, L"firework_charge",
        (new FireworkChargeItem())->setIconName(L"fireworks_charge")->setBaseItemTypeAndMaterial(44, 0)->setDescriptionId(0x12DAE0F3, 0x4C040C0D));

    registerItem(403, L"enchanted_book",
        (new EnchantedBookItem())->setIconName(L"enchantedBook")->setMaxStackSize(1)->setDescriptionId(IDS_ITEM_ENCHANTED_BOOK, IDS_DESC_ENCHANTED_BOOK));

    registerItem(404, L"comparator",
        (new BlockPlanterItem(Blocks::unpowered_comparator))->setIconName(L"comparator")->setDescriptionId(IDS_ITEM_COMPARATOR, 0xF7057FE0)->setBaseItemTypeAndMaterial(59, 0));

    registerItem(405, L"netherbrick",
        (new Item())->setIconName(L"netherbrick")->setDescriptionId(IDS_ITEM_NETHERBRICK, IDS_DESC_ITEM_NETHERBRICK));

    registerItem(406, L"quartz",
        (new Item())->setIconName(L"netherquartz")->setDescriptionId(IDS_ITEM_NETHER_QUARTZ, IDS_DESC_NETHER_QUARTZ));

    registerItem(407, L"tnt_minecart",
        (new MinecartItem(new Minecart::Type(3)))->setIconName(L"minecart_tnt")->setDescriptionId(IDS_ITEM_MINECART_TNT, IDS_DESC_MINECART_TNT));

    registerItem(408, L"hopper_minecart",
        (new MinecartItem(new Minecart::Type(5)))->setIconName(L"minecart_hopper")->setDescriptionId(IDS_ITEM_MINECART_HOPPER, IDS_DESC_MINECART_HOPPER)->setBaseItemTypeAndMaterial(51, 2));

    registerItem(409, L"prismarine_shard",
        (new Item())->setIconName(L"prismarine_shard")->setDescriptionId(IDS_ITEM_PRISMARINE_SHARD, IDS_DESC_PRISMARINE_SHARD)->setBaseItemTypeAndMaterial(13, 0));

    registerItem(410, L"prismarine_crystals",
        (new Item())->setIconName(L"prismarine_crystals")->setDescriptionId(IDS_ITEM_PRISMARINE_CRYSTALS, 0x47DB5008)->setBaseItemTypeAndMaterial(13, 0));

    registerItem(411, L"rabbit",
        (new FoodItem(3, 0.3f, true))->setIconName(L"rabbit_raw")->setDescriptionId(IDS_ITEM_RABBIT_RAW, 0x2E4841C7)->setBaseItemTypeAndMaterial(53, 0));

    registerItem(412, L"cooked_rabbit",
        (new FoodItem(5, 0.6f, true))->setIconName(L"rabbit_cooked")->setDescriptionId(IDS_ITEM_RABBIT_COOKED, IDS_DESC_RABBIT_COOKED));

    registerItem(413, L"rabbit_stew",
        (new BowlFoodItem(10))->setBaseItemTypeAndMaterial(39, 0)->setIconName(L"rabbit_stew")->setDescriptionId(IDS_ITEM_RABBIT_STEW, IDS_DESC_RABBIT_STEW));

    registerItem(414, L"rabbit_foot",
        (new Item())->setIconName(L"rabbit_foot")->setDescriptionId(IDS_ITEM_RABBIT_FOOT, 0x09EA74D9));

    registerItem(415, L"rabbit_hide",
        (new Item())->setIconName(L"rabbit_hide")->setDescriptionId(IDS_ITEM_RABBIT_HIDE, 0x75867C83));

    registerItem(416, L"armor_stand",
        (new ArmorStandItem())->setBaseItemTypeAndMaterial(34, 1)->setDescriptionId(IDS_ITEM_ARMOR_STAND, IDS_DESC_ARMORSTAND)->setIconName(L"wooden_armorstand")->setMaxStackSize(16));

    registerItem(417, L"iron_horse_armor",
        (new HorseArmorItem(2, 5, 105, L"meo"))->setIconName(L"iron_horse_armor")->setMaxStackSize(1)->setDescriptionId(IDS_ITEM_IRON_HORSE_ARMOR, 0x37AF95D0));

    registerItem(418, L"golden_horse_armor",
        (new HorseArmorItem(3, 7, 104, L"goo"))->setIconName(L"gold_horse_armor")->setMaxStackSize(1)->setDescriptionId(IDS_ITEM_GOLD_HORSE_ARMOR, 0x3E6173E2));

    registerItem(419, L"diamond_horse_armor",
        (new HorseArmorItem(4, 11, 103, L"dio"))->setIconName(L"diamond_horse_armor")->setMaxStackSize(1)->setDescriptionId(IDS_ITEM_DIAMOND_HORSE_ARMOR, 0xD59E460E));

    registerItem(420, L"lead",
        (new LeadItem())->setIconName(L"lead")->setBaseItemTypeAndMaterial(25, 0)->setDescriptionId(IDS_ITEM_LEAD, 0x729DE426));

    registerItem(421, L"name_tag",
        (new NameTagItem())->setIconName(L"name_tag")->setDescriptionId(IDS_ITEM_NAME_TAG, 0xBB06F9B0));

    registerItem(422, L"command_block_minecart",
        (new MinecartItem(new Minecart::Type(6)))->setIconName(L"minecart_command_block")->setDescriptionId(IDS_ITEM_MINECART_COMMAND));

    registerItem(423, L"mutton",
        (new FoodItem(2, 0.3f, true))->setIconName(L"mutton_raw")->setDescriptionId(IDS_ITEM_MUTTON_RAW, 0x432B18A0));

    registerItem(424, L"cooked_mutton",
        (new FoodItem(6, 0.8f, true))->setIconName(L"mutton_cooked")->setDescriptionId(IDS_ITEM_MUTTON_COOKED, 0xCB2E65B5));

    registerItem(425, L"banner",
        (new BannerItem())->setIconName(L"cookie")->setBaseItemTypeAndMaterial(54, 0)->setDescriptionId(IDS_ITEM_BANNER));

    registerItem(426, L"end_crystal",
        (new EndCrystalItem())->setDescriptionId(IDS_ITEM_END_CRYSTAL, 0xD396A19A));

    registerItem(427, L"spruce_door",
        (new DoorItem(Blocks::spruce_door))->setIconName(L"door_spruce")->setBaseItemTypeAndMaterial(6, 39)->setDescriptionId(0xB9F7283E, IDS_DESC_DOOR_WOOD));

    registerItem(428, L"birch_door",
        (new DoorItem(Blocks::birch_door))->setIconName(L"door_birch")->setBaseItemTypeAndMaterial(6, 40)->setDescriptionId(0x915B72F6, IDS_DESC_DOOR_WOOD));

    registerItem(429, L"jungle_door",
        (new DoorItem(Blocks::jungle_door))->setIconName(L"door_jungle")->setBaseItemTypeAndMaterial(6, 41)->setDescriptionId(0x52AF79BB, IDS_DESC_DOOR_WOOD));

    registerItem(430, L"acacia_door",
        (new DoorItem(Blocks::acacia_door))->setIconName(L"door_acacia")->setBaseItemTypeAndMaterial(6, 42)->setDescriptionId(0xF59508EC, IDS_DESC_DOOR_WOOD));

    registerItem(431, L"dark_oak_door",
        (new DoorItem(Blocks::dark_oak_door))->setIconName(L"door_dark_oak")->setBaseItemTypeAndMaterial(6, 43)->setDescriptionId(0x7BF6657D, IDS_DESC_DOOR_WOOD));

    ChorusFruitItem* chorusFruit = new ChorusFruitItem(4, 0.3f);
    chorusFruit->setCanAlwaysEat();
    registerItem(432, L"chorus_fruit",
        chorusFruit->setIconName(L"chorus_fruit")->setDescriptionId(IDS_ITEM_CHORUS_FRUIT, 0x75B20FC3));

    registerItem(433, L"chorus_fruit_popped",
        (new Item())->setIconName(L"chorus_fruit_popped")->setDescriptionId(IDS_ITEM_CHORUS_FRUIT_POPPED, 0xFD12A3EA));

    registerItem(434, L"beetroot",
        (new FoodItem(1, 0.6f, false))->setIconName(L"beetroot")->setDescriptionId(IDS_ITEM_BEETROOT, IDS_DESC_ITEM_BEETROOT));

    registerItem(435, L"beetroot_seeds",
        (new SeedItem(Blocks::beetroots, Blocks::farmland))->setIconName(L"beetroot_seeds")->setBaseItemTypeAndMaterial(33, 0)->setDescriptionId(IDS_ITEM_BEETROOT_SEEDS, IDS_DESC_ITEM_BEETROOT_SEEDS));

    registerItem(436, L"beetroot_soup",
        (new BowlFoodItem(6))->setIconName(L"beetroot_soup")->setBaseItemTypeAndMaterial(39, 0)->setDescriptionId(IDS_ITEM_BEETROOT_SOUP, 0x2ADE8F66));

    registerItem(437, L"dragon_breath",
        (new Item())->setIconName(L"dragon_breath")->setCraftingRemainingItem(glassBottle)->setDescriptionId(IDS_ITEM_DRAGON_BREATH, IDS_DESC_ITEM_DRAGON_BREATH));

    registerItem(438, L"splash_potion",
        (new SplashPotionItem())->setIconName(L"splash_potion")->setDescriptionId(IDS_ITEM_SPLASH_POTION, 0xA22445C1));

    registerItem(439, L"spectral_arrow",
        (new SpectralArrowItem())->setIconName(L"spectral_arrow")->setBaseItemTypeAndMaterial(24, 18)->setDescriptionId(IDS_ITEM_SPECTRAL_ARROW, 0xFF7B7200)->setAllowOffhand(true));

    registerItem(440, L"tipped_arrow",
        (new ArrowItem())->setIconName(L"tipped_arrow_base")->setBaseItemTypeAndMaterial(24, 18)->setDescriptionId(IDS_ITEM_TIPPED_ARROW, IDS_DESC_ITEM_TIPPED_ARROW_GENERIC)->setAllowOffhand(true));

    registerItem(441, L"lingering_potion",
        (new LingeringPotionItem())->setIconName(L"lingering_potion")->setDescriptionId(IDS_ITEM_LINGERING_POTION, 0xC459779F));

    registerItem(443, L"elytra",
        (new ElytraItem())->setIconName(L"elytra")->setDescriptionId(IDS_ITEM_ELYTRA, 0xB80B655F));

    registerItem(444, L"spruce_boat",
        (new BoatItem(Boat::SPRUCE))->setIconName(L"spruce_boat")->setBaseItemTypeAndMaterial(55, 64)->setDescriptionId(IDS_ITEM_BOAT_SPRUCE, 0x5061F0B5));

    registerItem(445, L"birch_boat",
        (new BoatItem(Boat::BIRCH))->setIconName(L"birch_boat")->setBaseItemTypeAndMaterial(55, 65)->setDescriptionId(IDS_ITEM_BOAT_BIRCH, 0xC2D7D60D));

    registerItem(446, L"jungle_boat",
        (new BoatItem(Boat::JUNGLE))->setIconName(L"jungle_boat")->setBaseItemTypeAndMaterial(55, 66)->setDescriptionId(IDS_ITEM_BOAT_JUNGLE, 0x8D796B10));

    registerItem(447, L"acacia_boat",
        (new BoatItem(Boat::ACACIA))->setIconName(L"acacia_boat")->setBaseItemTypeAndMaterial(55, 67)->setDescriptionId(IDS_ITEM_BOAT_ACACIA, 0x1B9311E7));

    registerItem(448, L"dark_oak_boat",
        (new BoatItem(Boat::DARK_OAK))->setIconName(L"dark_oak_boat")->setBaseItemTypeAndMaterial(55, 68)->setDescriptionId(IDS_ITEM_BOAT_DARK_OAK, 0x8C104A11));

    registerItem(449, L"totem_of_undying",
        (new Item())->setIconName(L"totem")->setDescriptionId(IDS_ITEM_TOTEM_OF_UNDYING, IDS_DESC_TOTEM_OF_UNDYING)->setMaxStackSize(1)->setAllowOffhand(true));

    registerItem(450, L"shulker_shell",
        (new Item())->setIconName(L"shulker_shell")->setDescriptionId(IDS_ITEM_SHULKER_SHELL, 0x541139C5));

    registerItem(452, L"iron_nugget",
        (new Item())->setIconName(L"iron_nugget")->setBaseItemTypeAndMaterial(31, 3)->setDescriptionId(IDS_ITEM_IRON_NUGGET, IDS_DESC_IRON_NUGGET));

    int leatherHorseArmorDefense[2];
    leatherHorseArmorDefense[0] = 209;
    leatherHorseArmorDefense[1] = 210;

    registerItem(453, L"leather_horse_armor",
        (new HorseArmorItem(1, 3, leatherHorseArmorDefense, 2, L"lee"))->setIconName(LEATHER_HORSE_ARMOR_ICON_NAME)->setMaxStackSize(1)->setBaseItemTypeAndMaterial(60, 6)->setDescriptionId(IDS_ITEM_LEATHER_HORSE_ARMOR, 0x41A31661));

    registerItem(454, L"trident",
        (new TridentItem())->setIconName(L"trident")->setDescriptionId(IDS_ITEM_TRIDENT, 0x18914E64));

    registerItem(455, L"turtle_shell_piece",
        (new Item())->setIconName(L"turtle_shell_piece")->setDescriptionId(0x999CE804, 0xBA1D8B28));

    registerItem(456, L"turtle_helmet",
        (new ArmorItem(ArmorItem::ArmorMaterial::TURTLE, 5, EquipmentSlot::HEAD))->setIconName(L"turtle_helmet")->setBaseItemTypeAndMaterial(7, 72)->setDescriptionId(0x1552F892, 0x7340A73C));

    registerItem(457, L"kelp",
        (new BlockPlanterItem(Blocks::kelp))->setIconName(L"kelp")->setDescriptionId(IDS_ITEM_KELP, 0xEBBD31AA));

    FoodItem* driedKelp = new FoodItem(1, 0.1f, false);
    driedKelp->setIsFastFood();
    registerItem(458, L"dried_kelp",
        driedKelp->setIconName(L"dried_kelp")->setBaseItemTypeAndMaterial(31, 73)->setDescriptionId(IDS_ITEM_DRIED_KELP, 0x240F411B));

    registerItem(459, L"fish_bucket",
        (new BucketItem(BucketItem::FISH))->setIconName(L"bucketFish")->setDescriptionId(IDS_ITEM_BUCKET_COD, IDS_DESC_BUCKET_COD)->setCraftingRemainingItem(bucket));

    registerItem(460, L"salmon_bucket",
        (new BucketItem(BucketItem::SALMON))->setIconName(L"bucketSalmon")->setDescriptionId(IDS_ITEM_BUCKET_SALMON, IDS_DESC_BUCKET_SALMON)->setCraftingRemainingItem(bucket));

    registerItem(461, L"puffer_bucket",
        (new BucketItem(BucketItem::PUFFER))->setIconName(L"bucketPuffer")->setDescriptionId(IDS_ITEM_BUCKET_PUFFER, 0xF9C79911)->setCraftingRemainingItem(bucket));

    registerItem(462, L"tropical_bucket",
        (new BucketItem(BucketItem::TROPICAL))->setIconName(L"bucketTropical")->setDescriptionId(IDS_ITEM_BUCKET_TROPICAL, IDS_DESC_BUCKET_TROPICAL)->setCraftingRemainingItem(bucket));

    registerItem(463, L"nautilus_core",
        (new Item())->setIconName(L"nautilus_core")->setDescriptionId(0x3982DB07, 0x5988048B));

    registerItem(464, L"nautilus",
        (new Item())->setIconName(L"nautilus")->setDescriptionId(IDS_ITEM_NAUTILUS, 0x3AFE27FF)->setAllowOffhand(true));

    registerItem(465, L"phantom_membrane",
        (new Item())->setIconName(L"phantom_membrane")->setDescriptionId(IDS_ITEM_PHANTOM_MEMBRANE, 0x82C2ECDB));

    registerItem(2255, L"debug_fourj_item",
        (new DebugFourjItem())->setIconName(L"barrier"));

    registerItem(2256, L"record_13",
        (new RecordItem(L"13", 0xC04578A7, SoundEvents::RECORD_13))->setDescriptionId(0xD4B0C693, 0xD8DB7EFF));

    registerItem(2257, L"record_cat",
        (new RecordItem(L"cat", 0xB4B22E0C, SoundEvents::RECORD_CAT))->setDescriptionId(0xD4B0C693, 0xD8DB7EFF));

    registerItem(2258, L"record_blocks",
        (new RecordItem(L"blocks", 0x92253055, SoundEvents::RECORD_BLOCKS))->setDescriptionId(0xD4B0C693, 0xD8DB7EFF));

    registerItem(2259, L"record_chirp",
        (new RecordItem(L"chirp", 0xB4B7037A, SoundEvents::RECORD_CHIRP))->setDescriptionId(0xD4B0C693, 0xD8DB7EFF));

    registerItem(2260, L"record_far",
        (new RecordItem(L"far", 0x088245C3, SoundEvents::RECORD_FAR))->setDescriptionId(0xD4B0C693, 0xD8DB7EFF));

    registerItem(2261, L"record_mall",
        (new RecordItem(L"mall", 0xAF7708A8, SoundEvents::RECORD_MALL))->setDescriptionId(0xD4B0C693, 0xD8DB7EFF));

    registerItem(2262, L"record_mellohi",
        (new RecordItem(L"mellohi", 0x7DE96171, SoundEvents::RECORD_MELLOHI))->setDescriptionId(0xD4B0C693, 0xD8DB7EFF));

    registerItem(2263, L"record_stal",
        (new RecordItem(L"stal", 0xD6E3ADD6, SoundEvents::RECORD_STAL))->setDescriptionId(0xD4B0C693, 0xD8DB7EFF));

    registerItem(2264, L"record_strad",
        (new RecordItem(L"strad", 0xDF75129F, SoundEvents::RECORD_STRAD))->setDescriptionId(0xD4B0C693, 0xD8DB7EFF));

    registerItem(2265, L"record_ward",
        (new RecordItem(L"ward", 0x7A19C56F, SoundEvents::RECORD_WARD))->setDescriptionId(0xD4B0C693, 0xD8DB7EFF));

    registerItem(2266, L"record_11",
        (new RecordItem(L"11", 0x9F3EF366, SoundEvents::RECORD_11))->setDescriptionId(0xD4B0C693, 0xD8DB7EFF));

    registerItem(2267, L"record_wait",
        (new RecordItem(L"where are we now", 0x51B5A3DD, SoundEvents::RECORD_WAIT))->setDescriptionId(0xD4B0C693, 0xD8DB7EFF));

    Items::staticCtor();
}
