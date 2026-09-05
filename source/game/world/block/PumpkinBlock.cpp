#include "game/world/block/PumpkinBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/core/GenericStats.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockInWorld.hpp"
#include "game/world/block/BlockMaterialPredicate.hpp"
#include "game/world/block/BlockPattern.hpp"
#include "game/world/block/BlockPatternBuilder.hpp"
#include "game/world/block/BlockPredicate.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"

static Predicate<BlockInWorld*>* pumpkinPredicate(Block* block)
{
    BlockPredicate predicate = BlockPredicate::forBlock(block);
    return BlockInWorld::hasState(&predicate);
}

PumpkinBlock::PumpkinBlock()
    : HorizontalDirectionalBlock(Material::WOOD),
      mTopIcon(NULL),
      mFaceIcon(NULL),
      mSnowGolemFullPattern(NULL),
      mSnowGolemBasePattern(NULL),
      mIronGolemFullPattern(NULL),
      mIronGolemBasePattern(NULL)
{
    DerivedInit();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(HorizontalDirectionalBlock::FACING, &facing));
    setTicking(true);
}

PumpkinBlock::~PumpkinBlock()
{
}

void PumpkinBlock::getOrCreateSnowGolemFull()
{
    if (mSnowGolemFullPattern == NULL)
    {
        std::wstring rows[3] = { L"^", L"#", L"#" };
        mSnowGolemFullPattern = BlockPatternBuilder::start()->aisle(rows)
            ->where(L'^', pumpkinPredicate(Blocks::carved_pumpkin))
            ->where(L'#', pumpkinPredicate(Blocks::snow))
            ->build(Direction::NORTH, Direction::UP);
    }
}

void PumpkinBlock::fjAwardCraftedGolemStat(unsigned long long craftedEntity)
{
    GenericStats::param_craftedEntity(craftedEntity);
}

void PumpkinBlock::getOrCreateIronGolemFull()
{
    if (mIronGolemFullPattern == NULL)
    {
        std::wstring rows[3] = { L"~^~", L"###", L"~#~" };
        mIronGolemFullPattern = BlockPatternBuilder::start()->aisle(rows)
            ->where(L'^', pumpkinPredicate(Blocks::carved_pumpkin))
            ->where(L'#', pumpkinPredicate(Blocks::iron_block))
            ->where(L'~', BlockInWorld::hasState(BlockMaterialPredicate::forMaterial(Material::METAL)))
            ->build(Direction::NORTH, Direction::UP);
    }
}

void PumpkinBlock::getOrCreateSnowGolemBase()
{
    if (mSnowGolemBasePattern == NULL)
    {
        std::wstring rows[3] = { L" ", L"#", L"#" };
        mSnowGolemBasePattern = BlockPatternBuilder::start()->aisle(rows)
            ->where(L'#', pumpkinPredicate(Blocks::snow))
            ->build(Direction::NORTH, Direction::UP);
    }
}

void PumpkinBlock::getOrCreateIronGolemBase()
{
    if (mIronGolemBasePattern == NULL)
    {
        std::wstring rows[3] = { L"~ ~", L"###", L"~#~" };
        mIronGolemBasePattern = BlockPatternBuilder::start()->aisle(rows)
            ->where(L'#', pumpkinPredicate(Blocks::iron_block))
            ->where(L'~', BlockInWorld::hasState(BlockMaterialPredicate::forMaterial(Material::METAL)))
            ->build(Direction::NORTH, Direction::UP);
    }
}

bool PumpkinBlock::canSpawnGolem(Level* level, const BlockPos& pos)
{
    getOrCreateSnowGolemBase();
    if (mSnowGolemBasePattern->find(level, pos) != NULL)
        return true;
    getOrCreateIronGolemBase();
    return mIronGolemBasePattern->find(level, pos) != NULL;
}

void PumpkinBlock::trySpawnGolem(Level* level, const BlockPos& pos)
{
    getOrCreateSnowGolemFull();
    BlockPattern::BlockPatternMatch* snow = mSnowGolemFullPattern->find(level, pos);
    if (snow != NULL)
    {
        for (int y = 0; y < snow->getHeight(); ++y)
            level->setBlock(snow->getBlock(0, y, 0)->getPos(), defaultBlockState(), 2, false);
        return;
    }

    getOrCreateIronGolemFull();
    BlockPattern::BlockPatternMatch* iron = mIronGolemFullPattern->find(level, pos);
    if (iron != NULL)
    {
        for (int x = 0; x < iron->getWidth(); ++x)
            for (int y = 0; y < iron->getHeight(); ++y)
                level->setBlock(iron->getBlock(x, y, 0)->getPos(), defaultBlockState(), 2, false);
    }
}

void PumpkinBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onPlace(level, pos, state);
    trySpawnGolem(level, pos);
}

bool PumpkinBlock::mayPlace(Level* level, const BlockPos& pos)
{
    const BlockState* below = level->getBlockState(pos.below());
    const BlockState* current = level->getBlockState(pos);
    if (!current->isSolidBlockingBlock() && current->getMaterial() != Material::AIR)
        return false;
    return below->canProvideSupport(Direction::UP, BLOCK_SUPPORT_CENTER);
}

const BlockState* PumpkinBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(HorizontalDirectionalBlock::FACING)));
    return state->setBoxedValue(HorizontalDirectionalBlock::FACING, &facing);
}

const BlockState* PumpkinBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(HorizontalDirectionalBlock::FACING)));
}

const BlockState* PumpkinBlock::getStateForPlacement(Level*, const BlockPos&, const Direction*, float, float, float, int, boost::shared_ptr<LivingEntity> placer)
{
    TypedBoxed<const Direction*> facing(placer->getDirection()->getOpposite());
    return defaultBlockState()->setBoxedValue(HorizontalDirectionalBlock::FACING, &facing);
}

const BlockState* PumpkinBlock::getBlockState(int data)
{
    TypedBoxed<const Direction*> facing(Direction::from2DDataValue(data));
    return defaultBlockState()->setBoxedValue(HorizontalDirectionalBlock::FACING, &facing);
}

int PumpkinBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(HorizontalDirectionalBlock::FACING)->get2DDataValue();
}

BlockStateDefinition* PumpkinBlock::createBlockStateDefinition()
{
    const Property* properties[] = { HorizontalDirectionalBlock::FACING };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* PumpkinBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP || direction == Direction::DOWN)
        return mTopIcon;
    if ((this == Blocks::carved_pumpkin || this == Blocks::lit_pumpkin)
        && direction == state->getValue(HorizontalDirectionalBlock::FACING))
        return mFaceIcon;
    return mIcon;
}

void PumpkinBlock::registerIcons(IconRegister* iconRegister)
{
    std::wstring suffix = (this == Blocks::carved_pumpkin) ? L"on" : L"off";
    mFaceIcon = iconRegister->registerIcon(getIconName() + L"_face_" + suffix);
    mTopIcon = iconRegister->registerIcon(getIconName() + L"_top");
    mIcon = iconRegister->registerIcon(getIconName() + L"_side");
}

unsigned int PumpkinBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (this == Blocks::carved_pumpkin && Item::getId(tooltipData.m_pItemInstance->getItem()) == 0x167
        && !tooltipData.m_bUseItem)
        return 0x95D5A56Eu;
    return Block::GetInteractTooltip(tooltipData);
}
