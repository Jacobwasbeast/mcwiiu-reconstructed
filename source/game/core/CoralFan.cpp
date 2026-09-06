#include "game/core/CoralFan.hpp"

#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

static AbstractProperty<int>* CORAL_FAN_TYPE = NULL;
static AbstractProperty<bool>* CORAL_FAN_DEAD = NULL;
Property* CoralFan_PROPERTY_1 = NULL;
Property* CoralFan_PROPERTY_2 = NULL;

static const unsigned int CORAL_FAN_DESCRIPTION_IDS[2][5] =
{
    { 0xA1128C9DU, 0x54923487U, 0x5C092885U, 0x29A6A280U, 0x6932757FU },
    { 0xB34422EEU, 0x49DEF5D4U, 0x842794C6U, 0x4A67CEEBU, 0x93926B4CU }
};

CoralFan::CoralFan()
    : Bush(Material::CORAL)
{
    DerivedInit();
    for (int i = 0; i < 10; ++i)
        mIcons[i] = NULL;

    TypedBoxed<int> type(3);
    TypedBoxed<bool> dead(true);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(CORAL_FAN_TYPE, &type)->setBoxedValue(CORAL_FAN_DEAD, &dead));
}

CoralFan::~CoralFan()
{
}

int CoralFan::getColor(const BlockState*) { return -1; }
int CoralFan::getColor(LevelSource*, const BlockPos&, const BlockState*) { return -1; }

const MaterialColor* CoralFan::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return Blocks::coral_block->getMapColor(state, levelSource, pos);
}

Item* CoralFan::getResource(const BlockState*, Random*, int) { return NULL; }
const AABB* CoralFan::getShape(const BlockState*, LevelSource*, const BlockPos&) { return NULL; }
bool CoralFan::isSolidRender(const BlockState*) const { return false; }
bool CoralFan::isCubeShaped(const BlockState*) { return false; }
int CoralFan::getRenderLayer() { return 0; }
EBlockRenderShape CoralFan::getRenderShape(const BlockState*) { return static_cast<EBlockRenderShape>(0x33); }

void CoralFan::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!level->mIsClientSide)
        checkAlive(level, pos, state);
}

bool CoralFan::canContainLiquid(const BlockState*) const { return true; }

void CoralFan::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!level->mIsClientSide)
        level->levelEvent(level->mRandom->nextInt(40, 200), pos, convertBlockStateToLegacyData(state));
}

EBlockOffsetType CoralFan::getOffsetType() { return BLOCK_OFFSET_XZ; }

TextureAtlasSprite* CoralFan::getTexture(const Direction*, const BlockState* state)
{
    int type = state->getValue(CORAL_FAN_TYPE);
    if (type < 0 || type > 4)
        type = 0;
    if (this == Blocks::coral_fan_dead)
        type += 5;
    return mIcons[type];
}

void CoralFan::registerIcons(IconRegister* iconRegister)
{
    static const wchar_t* names[10] =
    {
        L"coral_fan_red", L"coral_fan_orange", L"coral_fan_yellow", L"coral_fan_blue", L"coral_fan_purple",
        L"dead_coral_fan_red", L"dead_coral_fan_orange", L"dead_coral_fan_yellow", L"dead_coral_fan_blue", L"dead_coral_fan_purple"
    };
    for (int i = 0; i < 10; ++i)
        mIcons[i] = iconRegister->registerIcon(names[i]);
}

int CoralFan::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(CORAL_FAN_TYPE) | (state->getValue(CORAL_FAN_DEAD) ? 8 : 0);
}

const BlockState* CoralFan::getBlockState(int data)
{
    TypedBoxed<int> type(data & 7);
    const BlockState* state = defaultBlockState()->setBoxedValue(CORAL_FAN_TYPE, &type);
    TypedBoxed<bool> dead((data >> 3) & 1);
    return state->setBoxedValue(CORAL_FAN_DEAD, &dead);
}

int CoralFan::getDescriptionId(int auxValue)
{
    bool dead = this == Blocks::coral_fan_dead;
    int type = auxValue & 7;
    if (type > 4)
        type = 0;
    return CORAL_FAN_DESCRIPTION_IDS[dead ? 1 : 0][type];
}

bool CoralFan::mayReplaceWithPlace(LevelSource*, const BlockPos&) { return true; }

not_null_ptr<ItemInstance> CoralFan::getSilkTouchItemInstance(const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(this, 1, convertBlockStateToLegacyData(state)));
}

not_null_ptr<ItemInstance> CoralFan::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(this, 1, convertBlockStateToLegacyData(state)));
}

BlockStateDefinition* CoralFan::createBlockStateDefinition()
{
    if (CORAL_FAN_TYPE == NULL)
    {
        CORAL_FAN_TYPE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"type", 0, 4));
        CORAL_FAN_DEAD = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"dead"));
        CoralFan_PROPERTY_1 = CORAL_FAN_TYPE;
        CoralFan_PROPERTY_2 = CORAL_FAN_DEAD;
    }
    const Property* properties[2] = { CORAL_FAN_TYPE, CORAL_FAN_DEAD };
    return new BlockStateDefinition(this, properties, 2);
}

bool CoralFan::isSilkTouchable() { return true; }
bool CoralFan::mayPlace(Level* level, const BlockPos& pos) { return Block::mayPlace(level, pos); }

bool CoralFan::mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    BlockState* support = const_cast<BlockState*>(level->getBlockState(pos));
    if (support->isSolidBlockingBlock())
        return true;
    if (HalfSlabBlock::isHalfSlab(support))
        return support->getValue(HalfSlabBlock::TYPE) == HalfSlabBlock::TOP;
    return false;
}

void CoralFan::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if (this != Blocks::coral_fan_dead && level->getExtraBlock(pos)->mBaseItemType == mBaseItemType)
        level->setExtraBlock(pos, Blocks::AIR->defaultBlockState(), 3, false);
}

bool CoralFan::isWaterBlocking() const { return false; }

void CoralFan::checkAlive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!level->isUnderWater(pos, 263, 0))
    {
        TypedBoxed<int> type(state->getValue(CORAL_FAN_TYPE));
        TypedBoxed<bool> dead(true);
        const BlockState* deadState = Blocks::coral_fan_dead->defaultBlockState()->setBoxedValue(CORAL_FAN_TYPE, &type)->setBoxedValue(CORAL_FAN_DEAD, &dead);
        level->setBlock(pos, deadState, 3, false);
    }
    BlockPos below = pos.below();
    if (!level->isSolidBlockingBlock(below) && !mayPlaceOn(level, below, level->getBlockState(below)))
    {
        level->levelEvent(2001, pos, Block::getIdWithData(state));
        level->setBlock(pos, Blocks::AIR->defaultBlockState(), 3, false);
    }
}
