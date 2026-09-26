#include "game/world/block/ComparatorBlock.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/ComparatorBlockEntity.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<ComparatorBlock::ComparatorMode*>* ComparatorBlock::MODE = NULL;
ComparatorBlock::ComparatorMode* ComparatorBlock::ComparatorMode::COMPARE = NULL;
ComparatorBlock::ComparatorMode* ComparatorBlock::ComparatorMode::SUBTRACT = NULL;
static AbstractProperty<bool>* sPoweredProperty =
    static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"powered"));

class ComparatorModePredicate : public Predicate<ComparatorBlock::ComparatorMode*>
{
public:
    virtual bool apply(ComparatorBlock::ComparatorMode*) const { return true; }
};

ComparatorBlock::ComparatorMode::ComparatorMode(const std::wstring& name) : mName(name) {}
ComparatorBlock::ComparatorMode::~ComparatorMode() {}
std::wstring ComparatorBlock::ComparatorMode::toString() const { return mName; }
std::wstring ComparatorBlock::ComparatorMode::getSerializedName() const { return mName; }

ComparatorBlock::EntitySelector::EntitySelector(const Direction* direction) : mDirection(direction) {}
ComparatorBlock::EntitySelector::~EntitySelector() {}
bool ComparatorBlock::EntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    return entity && entity->getDirection() == mDirection;
}

ComparatorBlock::ComparatorBlock(bool on) : DiodeBlock(on)
{
    DerivedInit();

    TypedBoxed<const Direction*> facing(Direction::NORTH);
    TypedBoxed<bool> powered(false);
    TypedBoxed<ComparatorMode*> mode(ComparatorMode::COMPARE);

    registerDefaultState(mStateDefinition->any()
                             ->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing)
                             ->setBoxedValue(sPoweredProperty, &powered)
                             ->setBoxedValue(MODE, &mode));

    mBlockEntity = true;
}
ComparatorBlock::~ComparatorBlock() {}

boost::shared_ptr<BlockEntity> ComparatorBlock::newBlockEntity(Level*, int)
{
    return boost::shared_ptr<BlockEntity>(new ComparatorBlockEntity());
}

void ComparatorBlock::blockStaticCtor()
{
    ComparatorMode::COMPARE = new ComparatorMode(L"compare");
    ComparatorMode::SUBTRACT = new ComparatorMode(L"subtract");
    boost::unordered_set<ComparatorMode*> values;
    values.insert(ComparatorMode::COMPARE);
    values.insert(ComparatorMode::SUBTRACT);
    ComparatorModePredicate predicate;
    MODE = EnumProperty<ComparatorMode*>::create(L"mode", typeid(ComparatorMode*), values, &predicate);
}

std::wstring ComparatorBlock::getName() { return L10N::GetString(IDS_ITEM_COMPARATOR); }
Item* ComparatorBlock::getResource(const BlockState*, Random*, int) { return Items::comparator; }
not_null_ptr<ItemInstance> ComparatorBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*) { return not_null_ptr<ItemInstance>(new ItemInstance(Items::comparator)); }
int ComparatorBlock::getTurnOnDelay(const BlockState*) { return 2; }

const BlockState* ComparatorBlock::getOnBlock(const BlockState* state)
{
    TypedBoxed<ComparatorMode*> mode(ComparatorMode::COMPARE);
    return Blocks::powered_comparator->defaultBlockState()->setBoxedValue(MODE, &mode)->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, state->getBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY));
}

const BlockState* ComparatorBlock::getOffBlock(const BlockState* state)
{
    TypedBoxed<ComparatorMode*> mode(state->getValue(MODE));
    return Blocks::unpowered_comparator->defaultBlockState()->setBoxedValue(MODE, &mode)->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, state->getBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY));
}

bool ComparatorBlock::isOn(const BlockState* state)
{
    return mOn && state->getValue(MODE) != NULL;
}

int ComparatorBlock::getOutputSignal(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    boost::shared_ptr<BlockEntity> blockEntity = levelSource->getBlockEntity(pos);
    boost::shared_ptr<ComparatorBlockEntity> comparator = boost::dynamic_pointer_cast<ComparatorBlockEntity>(blockEntity);
    if (comparator) return comparator->getOutputSignal();
    return 0;
}

int ComparatorBlock::calculateOutputSignal(Level* level, const BlockPos& pos, const BlockState* state)
{
    int input = getInputSignal(level, pos, state);
    if (state->getValue(MODE) == ComparatorMode::SUBTRACT)
    {
        int side = getAlternateSignal(level, pos, state);
        input -= side;
        if (input < 0) input = 0;
    }
    return input;
}

bool ComparatorBlock::shouldTurnOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    int input = getInputSignal(level, pos, state);
    if (input == 0) return false;
    if (state->getValue(MODE) == ComparatorMode::SUBTRACT) return input > getAlternateSignal(level, pos, state);
    return true;
}

boost::shared_ptr<ItemFrame> ComparatorBlock::getItemFrame(Level* level, const Direction* direction, const BlockPos& pos)
{
    AABB* box = AABB::newTemp(pos);
    EntitySelector selector(direction);
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(ItemFrame), box, &selector, false);
    if (entities->size() != 1)
    {
        delete entities;
        return boost::shared_ptr<ItemFrame>();
    }
    boost::shared_ptr<ItemFrame> frame = boost::dynamic_pointer_cast<ItemFrame>(entities->at(0));
    delete entities;
    return frame;
}

int ComparatorBlock::getInputSignal(Level* level, const BlockPos& pos, const BlockState* state)
{
    int signal = DiodeBlock::getInputSignal(level, pos, state);
    const Direction* direction = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    BlockPos inputPos = pos.relative(direction);
    const BlockState* inputState = level->getBlockState(inputPos);
    if (inputState->hasAnalogOutputSignal())
    {
        signal = inputState->getAnalogOutputSignal(level, inputPos);
    }
    else if (signal < 15 && inputState->isSolidBlockingCubeAndNotSignalSource())
    {
        inputPos = inputPos.relative(direction);
        inputState = level->getBlockState(inputPos);
        if (inputState->hasAnalogOutputSignal())
        {
            signal = inputState->getAnalogOutputSignal(level, inputPos);
        }
        else if (inputState->getMaterial() == Material::AIR)
        {
            boost::shared_ptr<ItemFrame> frame = getItemFrame(level, direction, inputPos);
            if (frame)
            {
                signal = frame->getAnalogOutput();
            }
        }
    }
    return signal;
}

bool ComparatorBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player>, InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide) return true;
    if (state->getValue(MODE) == ComparatorMode::COMPARE)
    {
        TypedBoxed<ComparatorMode*> mode(ComparatorMode::SUBTRACT);
        level->setBlock(pos, state->setBoxedValue(MODE, &mode), Block::UPDATE_CLIENTS, false);
        return true;
    }
    TypedBoxed<ComparatorMode*> mode(ComparatorMode::COMPARE);
    level->setBlock(pos, state->setBoxedValue(MODE, &mode), Block::UPDATE_CLIENTS, false);
    return true;
}

void ComparatorBlock::checkTickOnNeighbor(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (level->isBlockToBeTickedAt(pos, this)) return;
    int newOutput = calculateOutputSignal(level, pos, state);
    int oldOutput = getOutputSignal(level, pos, state);
    if (newOutput == oldOutput && isOn(state) == shouldTurnOn(level, pos, state)) return;
    level->addToTickNextTick(pos, this, getTurnOnDelay(state), shouldPrioritize(level, pos, state) ? -1 : 0);
}

void ComparatorBlock::refreshOutputState(Level* level, const BlockPos& pos, const BlockState* state)
{
    int oldOutput = getOutputSignal(level, pos, state);
    int output = calculateOutputSignal(level, pos, state);
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    boost::shared_ptr<ComparatorBlockEntity> comparator = boost::dynamic_pointer_cast<ComparatorBlockEntity>(blockEntity);
    if (comparator) comparator->setOutputSignal(output);
    if (oldOutput == output && state->getValue(MODE) != ComparatorMode::SUBTRACT) return;

    bool shouldOn = shouldTurnOn(level, pos, state);
    bool currentOn = isOn(state);
    int flags = g_pApp->GetGameHostOption(eGameHostOption_MiniGame) ? 0x24 : 2;
    if (!currentOn)
    {
        if (shouldOn)
        {
            TypedBoxed<bool> powered(true);
            level->setBlock(pos, state->setBoxedValue(sPoweredProperty, &powered), flags, false);
        }
    }
    else if (!shouldOn)
    {
        TypedBoxed<bool> powered(false);
        level->setBlock(pos, state->setBoxedValue(sPoweredProperty, &powered), flags, false);
    }
    updateNeighborsInFront(level, pos, state);
}

void ComparatorBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (mOn) refreshOutputState(level, pos, state);
    DiodeBlock::tick(level, pos, state, NULL);
}

void ComparatorBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    DiodeBlock::onPlace(level, pos, state);
    level->setBlockEntity(pos, newBlockEntity(level, 0));
}

void ComparatorBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onRemove(level, pos, state);
    level->removeBlockEntity(pos);
}

bool ComparatorBlock::triggerEvent(const BlockState* state, Level* level, const BlockPos& pos, int eventId, int eventParam)
{
    Block::triggerEvent(state, level, pos, eventId, eventParam);
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (!blockEntity)
    {
        return false;
    }
    return blockEntity->triggerEvent(eventId, eventParam);
}

const BlockState* ComparatorBlock::getBlockState(int data)
{
    TypedBoxed<const Direction*> facing(Direction::from2DDataValue(data));
    TypedBoxed<bool> powered((data & 8) != 0);
    TypedBoxed<ComparatorMode*> mode((data & 4) ? ComparatorMode::SUBTRACT : ComparatorMode::COMPARE);
    return defaultBlockState()
        ->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing)
        ->setBoxedValue(sPoweredProperty, &powered)
        ->setBoxedValue(MODE, &mode);
}

int ComparatorBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->get2DDataValue();
    if (state->getValue(sPoweredProperty)) data |= 8;
    if (state->getValue(MODE) == ComparatorMode::SUBTRACT) data |= 4;
    return data;
}

unsigned int ComparatorBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_bUseItem && tooltipData.m_bCanPlace) return IDS_TOOLTIPS_USE;
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* ComparatorBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)));
    return state->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing);
}

const BlockState* ComparatorBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)));
}

BlockStateDefinition* ComparatorBlock::createBlockStateDefinition()
{
    const Property* properties[] = { HorizontalDirectionalBlock::FACING_PROPERTY, MODE,
                                     sPoweredProperty };
    return new BlockStateDefinition(this, properties, 3);
}

const BlockState* ComparatorBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction, float hitX, float hitY, float hitZ, int auxValue, boost::shared_ptr<LivingEntity> placer)
{
    TypedBoxed<ComparatorMode*> mode(ComparatorMode::COMPARE);
    return DiodeBlock::getStateForPlacement(level, pos, direction, hitX, hitY, hitZ, auxValue, placer)->setBoxedValue(MODE, &mode);
}

EBlockRenderShape ComparatorBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_COMPARATOR;
}

bool ComparatorBlock::TestUse()
{
    return true;
}
TextureAtlasSprite* ComparatorBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction != Direction::DOWN)
    {
        if (direction == Direction::UP)
        {
            return mIcon;
        }
        return Blocks::stone_slab->getTexture(direction);
    }
    if (state->getValue(sPoweredProperty))
    {
        return Blocks::redstone_torch->getTexture(direction);
    }
    return Blocks::unlit_redstone_torch->getTexture(direction);
}
