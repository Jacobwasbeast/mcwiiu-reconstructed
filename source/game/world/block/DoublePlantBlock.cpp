#include "game/world/block/DoublePlantBlock.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TallGrassBlock.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Stats.hpp"
#include "game/item/Items.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"

namespace
{
std::wstring sVariantTextures[6][4] = {
    {L"double_plant_sunflower_top", L"double_plant_sunflower_bottom",
     L"double_plant_sunflower_front", L"double_plant_sunflower_back"},
    {L"double_plant_syringa_top", L"double_plant_syringa_bottom", L"", L""},
    {L"double_plant_grass_top", L"double_plant_grass_bottom", L"", L""},
    {L"double_plant_fern_top", L"double_plant_fern_bottom", L"", L""},
    {L"double_plant_rose_top", L"double_plant_rose_bottom", L"", L""},
    {L"double_plant_paeonia_top", L"double_plant_paeonia_bottom", L"", L""},
};
}

AbstractProperty<DoublePlantBlock::Variant*>* DoublePlantBlock::VARIANT = NULL;
AbstractProperty<DoublePlantBlock::Half*>* DoublePlantBlock::HALF = NULL;
AbstractProperty<const Direction*>* DoublePlantBlock::FACING = NULL;
DoublePlantBlock::Variant* DoublePlantBlock::VARIANTS[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
DoublePlantBlock::Half* DoublePlantBlock::HALVES[2] = { NULL, NULL };
DoublePlantBlock::Variant* gDoublePlantGrass = NULL;
DoublePlantBlock::Variant* gDoublePlantFern = NULL;

DoublePlantBlock::Half::Half(int id) : mId(id) {}
DoublePlantBlock::Half::~Half() {}
std::wstring DoublePlantBlock::Half::toString() const { return getSerializedName(); }
std::wstring DoublePlantBlock::Half::getSerializedName() const { return mId == UPPER ? L"upper" : L"lower"; }

DoublePlantBlock::Variant::Variant(int data, const std::wstring& name, int nameId, int descriptionId,
                                   std::wstring* textures)
    : mData(data), mName(name), mNameId(nameId), mDescriptionId(descriptionId), mTextures(textures)
{
    mIcons[0] = NULL; mIcons[1] = NULL; mIcons[2] = NULL; mIcons[3] = NULL;
}
DoublePlantBlock::Variant::~Variant() {}
int DoublePlantBlock::Variant::getData() { return mData; }
DoublePlantBlock::Variant* DoublePlantBlock::Variant::byData(int data)
{
    if (data < 0 || data >= 6) return VARIANTS[0];
    return VARIANTS[data];
}
TextureAtlasSprite* DoublePlantBlock::Variant::getIcon(ETexture texture) { return mIcons[texture]; }
std::wstring* DoublePlantBlock::Variant::getTexture(int texture) { return mTextures + texture; }
void DoublePlantBlock::Variant::setIcon(ETexture texture, TextureAtlasSprite* icon) { mIcons[texture] = icon; }
std::wstring DoublePlantBlock::Variant::toString() const { return mName; }
std::wstring DoublePlantBlock::Variant::getSerializedName() const { return mName; }
int DoublePlantBlock::Variant::getNameId() { return mNameId; }
int DoublePlantBlock::Variant::getDescriptionId() { return mDescriptionId; }

DoublePlantBlock::DoublePlantBlock() : Bush(Material::REPLACEABLE_PLANT)
{
    DerivedInit();
    TypedBoxed<Variant*> variantBox(VARIANTS[0]);
    TypedBoxed<Half*> halfBox(HALVES[Half::LOWER]);
    TypedBoxed<const Direction*> facingBox(Direction::NORTH);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(VARIANT, &variantBox)->setBoxedValue(HALF, &halfBox)->setBoxedValue(FACING, &facingBox));
    setDestroyTime(0.0f)->setSoundType(SoundType::GRASS);
}
DoublePlantBlock::~DoublePlantBlock() {}

int DoublePlantBlock::getType(LevelSource*, const BlockPos&, const BlockState* state)
{
    if (state->getBlock() != this) return -1;
    Variant* variant = state->getValue(VARIANT);
    return variant == NULL ? -1 : variant->getData();
}
bool DoublePlantBlock::isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool)
{
    int type = getType(level, pos, state);
    return type != 2 && type != 3;
}
bool DoublePlantBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*) { return true; }
void DoublePlantBlock::performBonemeal(Level* level, Random*, const BlockPos& pos, const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    placeAt(level, pos, variant, 2);
}
void DoublePlantBlock::blockStaticCtor()
{
    if (HALVES[0] == NULL)
    {
        HALVES[0] = new Half(Half::LOWER);
        HALVES[1] = new Half(Half::UPPER);
        VARIANTS[0] = new Variant(0, L"sunflower", static_cast<int>(0xfb6416aa),
                                  static_cast<int>(0xb200bb7f), sVariantTextures[0]);
        VARIANTS[1] = new Variant(1, L"syringa", static_cast<int>(0xdf183664),
                                  static_cast<int>(0x6a2ec061), sVariantTextures[1]);
        VARIANTS[2] = new Variant(2, L"double_grass", static_cast<int>(0x0d46268b),
                                  static_cast<int>(0xf8386cce), sVariantTextures[2]);
        VARIANTS[3] = new Variant(3, L"double_fern", static_cast<int>(0x6fd6b722),
                                  static_cast<int>(0xf5dafd9f), sVariantTextures[3]);
        VARIANTS[4] = new Variant(4, L"double_rose", static_cast<int>(0x44477db7),
                                  static_cast<int>(0xb6fac4d2), sVariantTextures[4]);
        VARIANTS[5] = new Variant(5, L"paeonia", static_cast<int>(0x6209364a),
                                  static_cast<int>(0x6a49a77f), sVariantTextures[5]);
        gDoublePlantGrass = VARIANTS[2];
        gDoublePlantFern = VARIANTS[3];

        boost::unordered_set<Variant*> variants;
        for (int i = 0; i < 6; ++i)
        {
            variants.insert(VARIANTS[i]);
        }
        VARIANT = EnumProperty<Variant*>::create(L"variant", typeid(Variant*), variants,
                                                 Predicates<Variant*>::alwaysTrue);

        boost::unordered_set<Half*> halves;
        halves.insert(HALVES[0]);
        halves.insert(HALVES[1]);
        HALF = EnumProperty<Half*>::create(L"half", typeid(Half*), halves,
                                           Predicates<Half*>::alwaysTrue);
    }
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
}

const AABB* DoublePlantBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(HALF) == HALVES[Half::UPPER] ? AABB::newTemp(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)
                                                        : AABB::newTemp(0.0, 0.0, 0.0, 1.0, 2.0, 1.0);
}
bool DoublePlantBlock::mayPlace(Level* level, const BlockPos& pos) { return Bush::mayPlace(level, pos) && level->isEmptyBlock(pos.above()); }
bool DoublePlantBlock::mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* state = levelSource->getBlockState(pos);
    return state->getBlock() != this || (state->getValue(HALF) != HALVES[Half::UPPER] && state->getValue(HALF) != HALVES[Half::LOWER]);
}
bool DoublePlantBlock::canContainLiquid(const BlockState*) const { return false; }
void DoublePlantBlock::checkAlive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (canSurvive(level, pos, state)) return;
    if (state->getValue(HALF) != HALVES[Half::UPPER])
    {
        BlockPos abovePos = pos.above();
        if (level->getBlockState(abovePos)->getBlock() == this)
        {
            level->setBlock(abovePos, Blocks::AIR->defaultBlockState(), 2, false);
        }
        level->setBlock(pos, Blocks::AIR->defaultBlockState(), 3, false);
        spawnResources(level, pos, state, 0);
    }
    else
    {
        BlockPos belowPos = pos.below();
        bool belowIsThis = level->getBlockState(belowPos)->getBlock() == this;
        level->setBlock(pos, Blocks::AIR->defaultBlockState(), 2, false);
        if (belowIsThis)
        {
            level->setBlock(belowPos, Blocks::AIR->defaultBlockState(), 3, false);
        }
    }
}
bool DoublePlantBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (state != NULL && state->getValue(HALF) == HALVES[Half::UPPER]) return level->getBlockState(pos.below())->getBlock() == this;
    return Bush::canSurvive(level, pos, state) && level->getBlockState(pos.above())->getBlock() == this;
}
Item* DoublePlantBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    int type = getType(NULL, BlockPos(), state);
    if (type == 2 || type == 3) return Block::getResource(state, random, lootBonus);
    return Item::byBlock(this);
}
int DoublePlantBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    if (state->getValue(HALF) != HALVES[Half::UPPER] && state->getValue(VARIANT) != gDoublePlantGrass)
    {
        return state->getValue(VARIANT)->getData();
    }
    return 0;
}
int DoublePlantBlock::getColor(LevelSource* levelSource, const BlockPos& pos, int) { return getColor(levelSource, pos, levelSource->getBlockState(pos)); }
int DoublePlantBlock::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    int type = getType(levelSource, pos, state);
    return type == 2 || type == 3 ? BiomeColors::getAverageGrassColor(levelSource, pos) : 0xffffff;
}
void DoublePlantBlock::placeAt(Level* level, const BlockPos& pos, Variant* variant, int flags)
{
    TypedBoxed<Variant*> value(variant);
    TypedBoxed<Half*> lower(HALVES[Half::LOWER]);
    level->setBlock(pos, defaultBlockState()->setBoxedValue(VARIANT, &value)->setBoxedValue(HALF, &lower), flags, false);
    TypedBoxed<Half*> upper(HALVES[Half::UPPER]);
    level->setBlock(pos.above(), defaultBlockState()->setBoxedValue(VARIANT, &value)->setBoxedValue(HALF, &upper), flags, false);
}
void DoublePlantBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<LivingEntity>, not_null_ptr<ItemInstance>)
{
    TypedBoxed<Half*> upper(HALVES[Half::UPPER]);
    level->setBlock(pos.above(), state->setBoxedValue(HALF, &upper), 2, false);
}
int DoublePlantBlock::popGrassResources(Level* level, const BlockPos& pos, const BlockState* state,
                                        boost::shared_ptr<Player> player)
{
    Variant* variant = state->getValue(VARIANT);
    if (variant != gDoublePlantGrass && variant != gDoublePlantFern)
        return 0;

    player->awardStat(Stats::blockMinedStats.data[Block::getId(this)],
                      GenericStats::param_blocksMined(state, 1));
    Block::popResource(level, pos,
                       not_null_ptr<ItemInstance>(new ItemInstance(
                           Blocks::tallgrass, 2,
                           variant == gDoublePlantGrass ? TallGrassBlock::Variant::GRASS->getData()
                                                        : TallGrassBlock::Variant::FERN->getData())));
    return 1;
}
void DoublePlantBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos, const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity, not_null_ptr<ItemInstance> tool)
{
    if (!level->mIsClientSide && tool->getItem() == Items::shears && state->getValue(HALF) == HALVES[Half::LOWER])
    {
        if (popGrassResources(level, pos, state, player) != 0) return;
    }
    Block::playerDestroy(level, player, pos, state, blockEntity, tool);
}
bool DoublePlantBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player)
{
    if (state->getValue(HALF) == HALVES[Half::UPPER])
    {
        BlockPos belowPos = pos.below();
        if (level->getBlockState(belowPos)->getBlock() == this)
        {
            level->destroyBlock(belowPos, true);
        }
    }
    else
    {
        BlockPos abovePos = pos.above();
        if (level->getBlockState(abovePos)->getBlock() == this)
        {
            level->setBlock(abovePos, Blocks::AIR->defaultBlockState(), 2, false);
        }
    }
    return Block::playerWillDestroy(level, pos, state, player);
}
not_null_ptr<ItemInstance> DoublePlantBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(this, 1, state->getValue(VARIANT)->getData()));
}
const BlockState* DoublePlantBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> variant(Variant::byData(data & 7));
    TypedBoxed<Half*> half((data & 8) ? HALVES[Half::UPPER] : HALVES[Half::LOWER]);
    return defaultBlockState()->setBoxedValue(VARIANT, &variant)->setBoxedValue(HALF, &half);
}
const BlockState* DoublePlantBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    if (state->getValue(HALF) == HALVES[Half::UPPER])
    {
        const BlockState* below = levelSource->getBlockState(pos.below());
        if (below->getBlock() == this)
        {
            TypedBoxed<Variant*> variant(below->getValue(VARIANT));
            return state->setBoxedValue(VARIANT, &variant);
        }
    }
    return state;
}
int DoublePlantBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    if (state->getValue(HALF) == HALVES[Half::UPPER])
    {
        return state->getValue(FACING)->get2DDataValue() | 8;
    }
    return state->getValue(VARIANT)->getData();
}
unsigned int DoublePlantBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData) { return Block::GetInteractTooltip(tooltipData); }
BlockStateDefinition* DoublePlantBlock::createBlockStateDefinition()
{
    const Property* properties[] = { HALF, VARIANT, FACING };
    return new BlockStateDefinition(this, properties, 3);
}
EBlockOffsetType DoublePlantBlock::getOffsetType() { return BLOCK_OFFSET_XZ; }
EBlockRenderShape DoublePlantBlock::getRenderShape(const BlockState*) { return (EBlockRenderShape)41; }
TextureAtlasSprite* DoublePlantBlock::getTexture(const BlockState* state, Variant::ETexture texture) { return state->getValue(VARIANT)->getIcon(texture); }
TextureAtlasSprite* DoublePlantBlock::getIconTexture(const Direction*, const BlockState* state)
{
    if (state->getValue(VARIANT) == VARIANTS[0]) return getTexture(state, Variant::SIDE);
    return getTexture(state, Variant::BOTTOM);
}
TextureAtlasSprite* DoublePlantBlock::getTexture(const Direction*, const BlockState* state)
{
    return getTexture(state, state->getValue(HALF) != HALVES[Half::UPPER] ? Variant::TOP : Variant::BOTTOM);
}
void DoublePlantBlock::registerIcons(IconRegister* iconRegister)
{
    for (int variantIndex = 0; variantIndex < 6; ++variantIndex)
    {
        Variant* variant = VARIANTS[variantIndex];
        for (int texture = 0; texture < 4; ++texture)
        {
            std::wstring* name = variant->getTexture(texture);
            if (!name->empty())
            {
                variant->setIcon(static_cast<Variant::ETexture>(texture), iconRegister->registerIcon(*name));
            }
        }
    }
}
