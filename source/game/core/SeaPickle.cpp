#include "game/core/SeaPickle.hpp"

#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CoralBlock.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<int>* SeaPickle_PICKLES = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"pickles", 1, 4));
static AbstractProperty<bool>* SEA_PICKLE_WATERLOGGED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"waterlogged"));

SeaPickle::SeaPickle()
    : Bush(Material::PLANT)
{
    DerivedInit();
    TypedBoxed<int> count(1);
    TypedBoxed<bool> waterlogged(false);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(SeaPickle_PICKLES, &count)->setBoxedValue(SEA_PICKLE_WATERLOGGED, &waterlogged));
}

SeaPickle::~SeaPickle()
{
}

bool SeaPickle::isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState*, bool isClientSide)
{
    return !isClientSide && level->isUnderWater(pos, 0x10f, 0)
        && level->getBlockState(pos.below())->getBlock() == Blocks::WATER;
}

bool SeaPickle::isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    if (!isValidBonemealTarget(level, pos, state, false))
        return false;
    BlockPos origin = pos;
    int offset = 0;
    int width = 1;
    for (int layer = 0; layer < 5; ++layer)
    {
        for (int x = 0; x < width; ++x)
            for (int y = pos.y - 1; y <= pos.y + 1; ++y)
            {
                BlockPos target(pos.x - 2 + layer, y, pos.z - offset + x);
                if (target != pos && random->nextInt(3) == 0 && level->isUnderWater(target, 0x10f, 0))
                    return true;
            }
        if (layer < 2) { ++offset; width += 2; }
        else { --offset; width -= 2; }
    }
    return true;
}

void SeaPickle::performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    performBonemeal(false, level, random, pos, state);
}

bool SeaPickle::performBonemeal(bool checkOnly, Level* level, Random* random, const BlockPos& pos, const BlockState*)
{
    if (!isValidBonemealTarget(level, pos, NULL, false))
        return false;
    int offset = 0;
    int width = 1;
    for (int layer = 0; layer < 5; ++layer)
    {
        for (int x = 0; x < width; ++x)
            for (int y = pos.y - 1; y <= pos.y + 1; ++y)
            {
                BlockPos target(pos.x - 2 + layer, y, pos.z - offset + x);
                if (target == pos || random->nextInt(3) != 0 || !level->isUnderWater(target, 0x10f, 0))
                    continue;
                if (level->getBlockState(target.below())->getBlock() != Blocks::WATER || !level->isEmptyWaterBlock(target))
                    continue;
                const BlockState* current = level->getBlockState(target);
                TypedBoxed<int> count(current->getBlock() == this ? current->getValue(SeaPickle_PICKLES) : 1);
                if (current->getBlock() == this && count.getValue() >= 3)
                    count = TypedBoxed<int>(3);
                else
                    count = TypedBoxed<int>(random->nextInt(count.getValue(), 3));
                level->setBlockAndData(target.x, target.y, target.z, Block::getId(this), count.getValue(), 0xb);
            }
        if (layer < 2) { ++offset; width += 2; }
        else { --offset; width -= 2; }
    }
    if (!checkOnly)
    {
        TypedBoxed<int> count(3);
        level->setBlockAndData(pos.x, pos.y, pos.z, Block::getId(this), 3, 0xb);
    }
    return true;
}

int SeaPickle::getColor(const BlockState* state)
{
    return getColor(NULL, BlockPos(), state);
}

int SeaPickle::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    return state->getMapColor(levelSource, pos)->calculateRGBColor(0);
}

const MaterialColor* SeaPickle::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return state->getMapColor(levelSource, pos);
}

EBlockRenderShape SeaPickle::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(53);
}

Item* SeaPickle::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::sea_pickle);
}

void SeaPickle::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float, int)
{
    int count = state->getValue(SeaPickle_PICKLES);
    Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::sea_pickle), count, 0)));
}

int SeaPickle::getLightEmission(const BlockState* state)
{
    if (state->getBlock() != this)
        return 0;
    int count = state->getValue(SeaPickle_PICKLES);
    return count < 4 && !state->getValue(SEA_PICKLE_WATERLOGGED) ? 3 * count : 0;
}

void SeaPickle::playerDestroy(Level* level, boost::shared_ptr<Player>, const BlockPos& pos, const BlockState* state, boost::shared_ptr<BlockEntity>, not_null_ptr<ItemInstance>)
{
    spawnResources(level, pos, state, 1.0f, 0);
    level->setBlockAndData(pos.x, pos.y, pos.z, 0, 0, 3);
}

bool SeaPickle::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    return mayPlaceOn(level, pos.below(), level->getBlockState(pos.below()))
        && level->isUnderWater(pos, 0x10f, 0);
}

const AABB* SeaPickle::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

bool SeaPickle::mayReplaceWithPlace(LevelSource*, const BlockPos&)
{
    return true;
}

bool SeaPickle::canContainLiquid(const BlockState*) const
{
    return true;
}

bool SeaPickle::waterSpreadCausesSpawn()
{
    return true;
}

std::wstring SeaPickle::getTileItemIconName()
{
    return L"sea_pickle";
}

int SeaPickle::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(SeaPickle_PICKLES) - 1 | (state->getValue(SEA_PICKLE_WATERLOGGED) ? 8 : 0);
}

unsigned int SeaPickle::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == 351)
    {
        int pickles = tooltipData.m_pBlockState->getValue(SeaPickle_PICKLES);
        const BlockState* belowState = tooltipData.m_pLevel->getBlockState(tooltipData.m_blockPos.below());
        if (belowState->getBlock() == Blocks::WATER
            && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
            && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0
            && !belowState->getValue(CoralBlock_PROPERTY_2)
            && pickles < 3)
        {
            return 0x9f5fdc40u;
        }
    }
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* SeaPickle::getBlockState(int data)
{
    TypedBoxed<int> count((data & 7) + 1);
    const BlockState* state = defaultBlockState()->setBoxedValue(SeaPickle_PICKLES, &count);
    TypedBoxed<bool> waterlogged((data >> 3) & 1);
    return state->setBoxedValue(SEA_PICKLE_WATERLOGGED, &waterlogged);
}

BlockStateDefinition* SeaPickle::createBlockStateDefinition()
{
    const Property* properties[2] = { SeaPickle_PICKLES, SEA_PICKLE_WATERLOGGED };
    return new BlockStateDefinition(this, properties, 2);
}

bool SeaPickle::isSilkTouchable()
{
    return false;
}

bool SeaPickle::mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    int material = level->getBlockId(pos.x, pos.y, pos.z);
    if (material == 0x14 || material == 0x5f)
        return true;
    if (level->getBlockState(pos)->getBlock()->isSolidRender(state))
        return true;
    if (HalfSlabBlock::isHalfSlab(state) && state->getValue(HalfSlabBlock::TYPE) != HalfSlabBlock::BOTTOM)
        return false;
    return state->isSolid();
}

void SeaPickle::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (state->getValue(SEA_PICKLE_WATERLOGGED) == false && level->getBlockState(pos)->getLightEmission() == mLightEmission)
        level->setExtraBlock(pos, Blocks::WATER->defaultBlockState(), 3, false);
}

void SeaPickle::checkAlive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (canSurvive(level, pos, state))
        return;
    spawnResources(level, pos, state, 0.0f, 0);
    level->setBlockAndData(pos.x, pos.y, pos.z, 0, 0, 3);
}

void SeaPickle::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!level->mIsClientSide && !canSurvive(level, pos, state))
        level->blockUpdated(pos, this, true);
}

void SeaPickle::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (level->mIsClientSide)
        return;

    checkAlive(level, pos, state);

    if (level->getBlockState(pos)->getBlock() != this)
        return;

    const BlockState* currentState = level->getBlockState(pos);
    int pickles = currentState->getValue(SeaPickle_PICKLES);
    bool waterlogged = currentState->getValue(SEA_PICKLE_WATERLOGGED);
    bool underwater = level->isUnderWater(pos, 0x10f, 0);

    if (waterlogged && underwater)
    {
        TypedBoxed<int> count(pickles);
        TypedBoxed<bool> logged(true);
        const BlockState* newState = defaultBlockState()->setBoxedValue(SeaPickle_PICKLES, &count);
        newState = newState->setBoxedValue(SEA_PICKLE_WATERLOGGED, &logged);
        level->setBlock(pos, newState, 3, false);
    }
    else if (!waterlogged && !underwater)
    {
        TypedBoxed<int> count(pickles);
        TypedBoxed<bool> logged(false);
        const BlockState* newState = defaultBlockState()->setBoxedValue(SeaPickle_PICKLES, &count);
        newState = newState->setBoxedValue(SEA_PICKLE_WATERLOGGED, &logged);
        level->setBlock(pos, newState, 3, false);
    }
}

bool SeaPickle::canProvideSupport(const BlockState*, const Direction*, EBlockSupportType)
{
    return false;
}
