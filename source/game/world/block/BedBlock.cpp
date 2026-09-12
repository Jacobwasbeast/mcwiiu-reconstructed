#include "game/world/block/BedBlock.hpp"

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/BedBlockEntity.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/Player.hpp"

#include <typeinfo>

BedBlock::Part gBedHead(0, L"head");
static BedBlock::Part gBedFoot(1, L"foot");
static const AABB* gBedShape = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5625, 1.0);

AbstractProperty<const BedBlock::Part*>* BedBlock::PART = NULL;
AbstractProperty<bool>* BedBlock::OCCUPIED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"occupied"));

BedBlock::Part::Part(int index, const std::wstring& name)
    : mName(name), mIndex(index)
{
}

std::wstring BedBlock::Part::toString() const
{
    return mName;
}

std::wstring BedBlock::Part::getSerializedName() const
{
    return mName;
}

BedBlock::BedBlock()
    : HorizontalDirectionalBlock(Material::WOOL), EntityBlock()
{
    Block::DerivedInit();
    const BlockState* state = mStateDefinition->any();
    TypedBoxed<const Part*> part(&gBedFoot);
    state = state->setBoxedValue(PART, &part);
    TypedBoxed<bool> occupied(false);
    registerDefaultState(state->setBoxedValue(OCCUPIED, &occupied));
    mBlockEntity = true;
}

BedBlock::~BedBlock()
{
}

boost::shared_ptr<BlockEntity> BedBlock::newBlockEntity(Level* level, int data)
{
    return boost::shared_ptr<BlockEntity>(new BedBlockEntity());
}

void BedBlock::blockStaticCtor()
{
    boost::unordered_set<const Part*> parts;
    parts.insert(&gBedHead);
    parts.insert(&gBedFoot);
    PART = EnumProperty<const Part*>::create(L"part", typeid(const Part*), parts,
                                             Predicates<const Part*>::alwaysTrue);
}

const MaterialColor* BedBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    if (state->getValue(PART) == &gBedFoot)
    {
        boost::shared_ptr<BlockEntity> blockEntity = levelSource->getBlockEntity(pos);
        if (blockEntity.get() != NULL && blockEntity->GetType() == 0x01000210ULL)
        {
            boost::shared_ptr<BedBlockEntity> bedEntity = boost::static_pointer_cast<BedBlockEntity>(blockEntity);
            return MaterialColor::getDyeMaterial(bedEntity->getColor());
        }
    }
    return MaterialColor::getDyeMaterial(DyeColor::WHITE);
}

bool BedBlock::TestUse(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player)
{
    if (level == NULL || !player)
        return false;
    if (state->getValue(OCCUPIED))
    {
        boost::shared_ptr<Player> sleeper = getSleepingPlayerAt(level, pos);
        if (sleeper)
            return true;
    }
    return player->startSleepInBed(pos, false) == 0;
}

boost::shared_ptr<Player> BedBlock::getSleepingPlayerAt(Level* level, const BlockPos& pos)
{
    if (level == NULL)
        return boost::shared_ptr<Player>();
    for (std::vector<boost::shared_ptr<Player> >::iterator it = level->mPlayers.begin();
         it != level->mPlayers.end(); ++it)
    {
        boost::shared_ptr<Player> player = *it;
        if (player && player->isSleeping())
        {
            BlockPos playerPos(player);
            if (playerPos.equals(pos))
                return player;
        }
    }
    return boost::shared_ptr<Player>();
}

bool BedBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY, float hitZ, bool clientSide)
{
    return TestUse(level, pos, state, player);
}

bool BedBlock::isCubeShaped(const BlockState* state)
{
    return false;
}

bool BedBlock::isSolidRender(const BlockState* state) const
{
    return false;
}

void BedBlock::fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance)
{
    Block::fallOn(level, pos, entity, fallDistance);
}

void BedBlock::updateEntityAfterFallOn(Level* level, boost::shared_ptr<Entity> entity)
{
    Block::updateEntityAfterFallOn(level, entity);
}

void BedBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
    Block::neighborChanged(state, level, pos, block, fromPos);
}

Item* BedBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Block::getResource(state, random, lootBonus);
}

const AABB* BedBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return gBedShape;
}

bool BedBlock::hasCustomBreakingProgress(const BlockState* state)
{
    return true;
}

bool BedBlock::BlockIsDoor(const Block* block)
{
    return block == NULL;
}

bool BedBlock::hasSolidGroundAndTwoBlocksSpaceAt(Level* level, const BlockPos& pos)
{
    const BlockState* below = level->getBlockState(pos.below());
    if (below->getBlock() == NULL || !below->getBlock()->isSolidRender(below))
        return false;

    const BlockState* state = level->getBlockState(pos);
    if (!BlockIsDoor(state->getBlock()))
    {
        if (state->getMaterial()->blocksMotion())
            return false;
        if (level->getBlockState(pos.above())->getMaterial()->blocksMotion())
            return false;
    }
    return true;
}

BlockPos* BedBlock::findStandUpPosition(Level* level, const BlockPos& pos, int data)
{
    const Direction* facing = level->getBlockState(pos)->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    Vec3i normal = facing->getNormal();

    for (int offset = 0; offset < 2; ++offset)
    {
        int centerX = pos.x - normal.x * offset;
        int centerZ = pos.z - normal.z * offset;

        for (int x = centerX - 1; x <= centerX + 1; ++x)
        {
            for (int z = centerZ - 1; z <= centerZ + 1; ++z)
            {
                BlockPos standUpPos(x, pos.y, z);
                if (hasSolidGroundAndTwoBlocksSpaceAt(level, standUpPos))
                {
                    if (data < 1)
                    {
                        return new BlockPos(standUpPos);
                    }

                    --data;
                }
            }
        }
    }

    return new BlockPos(pos.above());
}

void BedBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance, int lootBonus)
{
    Block::spawnResources(level, pos, state, chance, lootBonus);
}

EPistonPushReaction BedBlock::getPistonPushReaction(const BlockState* state)
{
    return PISTON_PUSH_DESTROY;
}

int BedBlock::getRenderLayer()
{
    return 0;
}

EBlockRenderShape BedBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(22);
}

not_null_ptr<ItemInstance> BedBlock::getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state)
{
    BlockPos targetPos(pos);
    if (state->getValue(PART) == &gBedFoot)
    {
        targetPos = pos.relative(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY));
    }
    const DyeColor* color = DyeColor::WHITE;
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(targetPos);
    if (blockEntity.get() != NULL && blockEntity->GetType() == 0x01000210ULL)
    {
        boost::shared_ptr<BedBlockEntity> bedEntity = boost::static_pointer_cast<BedBlockEntity>(blockEntity);
        color = bedEntity->getColor();
    }
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, color->getBlockData()));
}

bool BedBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player)
{
    return Block::playerWillDestroy(level, pos, state, player);
}

void BedBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos, const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity, not_null_ptr<ItemInstance> tool)
{
    Block::playerDestroy(level, player, pos, state, blockEntity, tool);
}

void BedBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onRemove(level, pos, state);
    level->removeBlockEntity(pos);
}

const BlockState* BedBlock::getBlockState(int data)
{
    const Direction* facing = Direction::from2DDataValue(data);
    const BlockState* state = defaultBlockState();
    if ((data & 8) != 0)
    {
        TypedBoxed<const Part*> part(&gBedHead);
        state = state->setBoxedValue(PART, &part);
        TypedBoxed<const Direction*> boxedFacing(facing);
        state = state->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &boxedFacing);
        TypedBoxed<bool> occupied((data & 4) != 0);
        return state->setBoxedValue(OCCUPIED, &occupied);
    }
    TypedBoxed<const Part*> part(&gBedFoot);
    state = state->setBoxedValue(PART, &part);
    TypedBoxed<const Direction*> boxedFacing(facing);
    return state->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &boxedFacing);
}

const BlockState* BedBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    if (state->getValue(PART) == &gBedFoot)
    {
        const BlockState* head = levelSource->getBlockState(
            pos.relative(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)));
        if (head->getBlock() == this)
        {
            TypedBoxed<bool> occupied(head->getValue(OCCUPIED));
            state = state->setBoxedValue(OCCUPIED, &occupied);
        }
    }
    return state;
}

int BedBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->get2DDataValue();
    if (state->getValue(PART) == &gBedHead)
    {
        data |= 8;
        if (state->getValue(OCCUPIED))
        {
            data |= 4;
        }
    }
    return data;
}

EBlockFaceShape BedBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos, const Direction* direction)
{
    return BLOCK_FACE_UNDEFINED;
}

unsigned int BedBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_bIsInside)
    {
        if (tooltipData.m_bUseItem)
        {
            return 0x36d8e732;
        }
        if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success)
        {
            return 0x4c36cd96;
        }
    }
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* BedBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(
        rotation->rotate(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)));
    return state->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing);
}

const BlockState* BedBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(
        state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)));
}

BlockStateDefinition* BedBlock::createBlockStateDefinition()
{
    static const Property* properties[] = { FACING_PROPERTY, PART, OCCUPIED };
    return new BlockStateDefinition(this, properties, 3);
}

bool BedBlock::isHeadPiece(int data)
{
    return (data & 8) != 0;
}

void BedBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"planks_oak");
}
