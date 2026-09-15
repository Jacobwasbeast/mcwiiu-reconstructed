#include "game/world/block/CommandBlock.hpp"

#include <typeinfo>

#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/CommandBlockEntity.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<const Direction*>* CommandBlock::FACING = NULL;
AbstractProperty<bool>* CommandBlock::CONDITIONAL =
    static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"conditional"));

void CommandBlock::blockStaticCtor()
{
    FACING = DirectionalBlock::FACING;
}

CommandBlock::CommandBlock(const MaterialColor* materialColor, Mode mode)
    : BaseEntityBlock(Material::METAL, materialColor),
      mFrontIcon(NULL),
      mBackIcon(NULL),
      mConditionalIcon(NULL),
      mMode(mode)
{
    DerivedInit();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    TypedBoxed<bool> conditional(false);
    const BlockState* state = mStateDefinition->any();
    state = state->setBoxedValue(FACING, &facing);
    registerDefaultState(state->setBoxedValue(CONDITIONAL, &conditional));
}

CommandBlock::~CommandBlock()
{
}

unsigned int CommandBlock::getMode()
{
    return static_cast<unsigned int>(mMode);
}

boost::shared_ptr<BlockEntity> CommandBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> entity(new CommandBlockEntity(static_cast<CommandBlock::Mode>(mMode)));
    entity->setLevel(level);
    return entity;
}

void CommandBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                   Block* block, const BlockPos& fromPos)
{
    (void)state;
    (void)level;
    (void)pos;
    (void)block;
    (void)fromPos;
}

void CommandBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    (void)level;
    (void)pos;
    (void)state;
    (void)random;
}

int CommandBlock::getTickDelay(Level*)
{
    return 1;
}

bool CommandBlock::use(Level*, const BlockPos&, const BlockState*, boost::shared_ptr<Player>, InteractionHand::EInteractionHand, const Direction*, float, float, float, bool)
{
    return false;
}

bool CommandBlock::hasAnalogOutputSignal(const BlockState*)
{
    return true;
}

int CommandBlock::getAnalogOutputSignal(const BlockState*, Level* level, const BlockPos& pos)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity && blockEntity->GetType() == 0x1000209ULL)
    {
        boost::shared_ptr<CommandBlockEntity> commandBlockEntity = boost::dynamic_pointer_cast<CommandBlockEntity>(blockEntity);
        return static_cast<int>(commandBlockEntity->getCommandBlock()->getSuccessCount());
    }
    return 0;
}

void CommandBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<LivingEntity>, not_null_ptr<ItemInstance> itemInstance)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity->GetType() == 0x1000209ULL)
    {
        boost::shared_ptr<CommandBlockEntity> commandBlockEntity = boost::dynamic_pointer_cast<CommandBlockEntity>(blockEntity);
        if (itemInstance->hasCustomHoverName())
        {
            commandBlockEntity->getCommandBlock()->setName(itemInstance->getHoverName(), true);
        }
    }
}

int CommandBlock::getResourceCount(Random*)
{
    return 0;
}

EBlockRenderShape CommandBlock::getRenderShape(const BlockState*)
{
    return (EBlockRenderShape)55;
}

const BlockState* CommandBlock::getBlockState(int data)
{
    TypedBoxed<const Direction*> facing(Direction::from3DDataValue(data & 7));
    TypedBoxed<bool> conditional((data & 8) != 0);
    const BlockState* state = defaultBlockState();
    state = state->setBoxedValue(FACING, &facing);
    return state->setBoxedValue(CONDITIONAL, &conditional);
}

int CommandBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(FACING)->get3DDataValue();
    if (state->getValue(CONDITIONAL))
    {
        data |= 8;
    }
    return data;
}

const BlockState* CommandBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &facing);
}

const BlockState* CommandBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}

BlockStateDefinition* CommandBlock::createBlockStateDefinition()
{
    const Property* properties[] = { FACING, CONDITIONAL };
    return new BlockStateDefinition(this, properties, 2);
}

const BlockState* CommandBlock::getStateForPlacement(Level*, const BlockPos& pos, const Direction*, float, float, float, int, boost::shared_ptr<LivingEntity> placer)
{
    const Direction* facing = Direction::getDirectionFromLivingEntity(pos, placer)->getOpposite();
    TypedBoxed<const Direction*> boxedFacing(facing);
    TypedBoxed<bool> conditional(false);
    const BlockState* state = defaultBlockState();
    state = state->setBoxedValue(FACING, &boxedFacing);
    return state->setBoxedValue(CONDITIONAL, &conditional);
}

bool CommandBlock::isCommandBlock(Block* block)
{
    return block == Blocks::command_block || block == Blocks::chain_command_block
        || block == Blocks::repeating_command_block;
}

TextureAtlasSprite* CommandBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == state->getValue(FACING))
    {
        return mBackIcon;
    }
    if (direction == state->getValue(FACING)->getOpposite())
    {
        return mFrontIcon;
    }
    if (state->getValue(CONDITIONAL))
    {
        return mConditionalIcon;
    }
    return mIcon;
}

void CommandBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(getIconName() + L"_side");
    mFrontIcon = iconRegister->registerIcon(getIconName() + L"_front");
    mBackIcon = iconRegister->registerIcon(getIconName() + L"_back");
    mConditionalIcon = iconRegister->registerIcon(getIconName() + L"_conditional");
}
