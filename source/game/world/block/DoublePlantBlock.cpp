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
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"

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
DoublePlantBlock::Variant::~Variant() { delete [] mTextures; }
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

DoublePlantBlock::DoublePlantBlock() : Bush()
{
    DerivedInit();
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
    if (variant != NULL) placeAt(level, pos, variant, 2);
}
void DoublePlantBlock::blockStaticCtor()
{
    if (HALVES[0] == NULL)
    {
        HALVES[0] = new Half(Half::LOWER);
        HALVES[1] = new Half(Half::UPPER);
        VARIANTS[0] = new Variant(0, L"sunflower", 175, 175, NULL);
        VARIANTS[1] = new Variant(1, L"syringa", 176, 176, NULL);
        VARIANTS[2] = new Variant(2, L"grass", 177, 177, NULL);
        VARIANTS[3] = new Variant(3, L"fern", 178, 178, NULL);
        VARIANTS[4] = new Variant(4, L"rose", 179, 179, NULL);
        VARIANTS[5] = new Variant(5, L"paeonia", 180, 180, NULL);
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
    if (!canSurvive(level, pos, state)) level->blockUpdated(pos, this, true);
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
int DoublePlantBlock::getSpawnResourcesAuxValue(const BlockState* state) { return state->getValue(VARIANT)->getData(); }
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
int DoublePlantBlock::popGrassResources(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player>)
{
    if (state->getValue(HALF) == HALVES[Half::UPPER])
    {
        level->setBlock(pos, defaultBlockState(), 2, false);
        return 1;
    }
    return 0;
}
void DoublePlantBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos, const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity, not_null_ptr<ItemInstance> tool)
{
    popGrassResources(level, pos, state, player);
    Block::playerDestroy(level, player, pos, state, blockEntity, tool);
}
bool DoublePlantBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player)
{
    popGrassResources(level, pos, state, player);
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
    int result = state->getValue(VARIANT)->getData();
    if (state->getValue(HALF) == HALVES[Half::UPPER]) result |= 8;
    return result;
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
TextureAtlasSprite* DoublePlantBlock::getIconTexture(const Direction*, const BlockState* state) { return getTexture(state, Variant::BOTTOM); }
TextureAtlasSprite* DoublePlantBlock::getTexture(const Direction*, const BlockState* state) { return getTexture(state, Variant::BOTTOM); }
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
