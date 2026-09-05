#include "game/core/DispenserBootstrap.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/BehaviorRegistry.hpp"
#include "game/core/Direction.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"

class DispenserBlock
{
public:
    static BehaviorRegistry DISPENSER_REGISTRY;
    static DirectionProperty* FACING;
};

BehaviorRegistry DispenserBlock::DISPENSER_REGISTRY(NULL);
DirectionProperty* DispenserBlock::FACING = NULL;

void DispenserBootstrap::StaticCtor()
{
    DispenserBlock::DISPENSER_REGISTRY.add(Items::boat, new BoatDispenseBehavior(Boat::OAK));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::spruceBoat, new BoatDispenseBehavior(Boat::SPRUCE));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::birchBoat, new BoatDispenseBehavior(Boat::BIRCH));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::jungleBoat, new BoatDispenseBehavior(Boat::JUNGLE));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::acaciaBoat, new BoatDispenseBehavior(Boat::ACACIA));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::darkOakBoat, new BoatDispenseBehavior(Boat::DARK_OAK));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::arrow, new OptionalDispenseItemBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::spectralArrow, new OptionalDispenseItemBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::tippedArrow, new OptionalDispenseItemBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::white_shulker_box), new ShulkerBoxDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::orange_shulker_box), new ShulkerBoxDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::magenta_shulker_box), new ShulkerBoxDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::light_blue_shulker_box), new ShulkerBoxDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::yellow_shulker_box), new ShulkerBoxDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::lime_shulker_box), new ShulkerBoxDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::pink_shulker_box), new ShulkerBoxDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::gray_shulker_box), new ShulkerBoxDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::silver_shulker_box), new ShulkerBoxDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::cyan_shulker_box), new ShulkerBoxDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::purple_shulker_box), new ShulkerBoxDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::blue_shulker_box), new ShulkerBoxDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::brown_shulker_box), new ShulkerBoxDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::green_shulker_box), new ShulkerBoxDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::red_shulker_box), new ShulkerBoxDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::black_shulker_box), new ShulkerBoxDispenseBehavior());
}

DispenserBootstrap::BoatDispenseBehavior::BoatDispenseBehavior(Boat::Type type) : mType(type)
{
}

DispenserBootstrap::BoatDispenseBehavior::~BoatDispenseBehavior()
{
}

not_null_ptr<ItemInstance> DispenserBootstrap::BoatDispenseBehavior::execute(
    BlockSource* blockSource, not_null_ptr<ItemInstance> itemInstance,
    DefaultDispenseItemBehavior::eOUTCOME& outcome)
{
    Level* level = blockSource->getWorld();
    const Direction* direction = blockSource->getBlockState()->getValue(DispenserBlock::FACING);
    BlockPos position = blockSource->getPos().relative(direction);
    const BlockState* state = level->getBlockState(position);
    if (state->getMaterial() == Material::WATER)
    {
        Boat* boat = new Boat(level, position.x + 0.5, position.y + 1.0, position.z + 0.5);
        boat->setType(mType);
        boat->setRot((float)direction->getOpposite()->toYRot(), 0.0f);
        level->addEntity(boost::shared_ptr<Entity>(boat));
        itemInstance->shrink(1);
        outcome = DefaultDispenseItemBehavior::eOUTCOME_Success;
        return itemInstance;
    }
    return DefaultDispenseItemBehavior::execute(blockSource, itemInstance, outcome);
}

void DispenserBootstrap::BoatDispenseBehavior::playSound(
    BlockSource* blockSource, DefaultDispenseItemBehavior::eOUTCOME outcome)
{
    DefaultDispenseItemBehavior::playSound(blockSource, outcome);
}

DispenserBootstrap::OptionalDispenseItemBehavior::~OptionalDispenseItemBehavior()
{
}

void DispenserBootstrap::OptionalDispenseItemBehavior::playSound(
    BlockSource* blockSource, DefaultDispenseItemBehavior::eOUTCOME outcome)
{
    blockSource->getWorld()->levelEvent(outcome == DefaultDispenseItemBehavior::eOUTCOME_Failure ? 1000 : 1001,
                                        blockSource->getPos(), 0);
}

DispenserBootstrap::ShulkerBoxDispenseBehavior::~ShulkerBoxDispenseBehavior()
{
}

not_null_ptr<ItemInstance> DispenserBootstrap::ShulkerBoxDispenseBehavior::execute(
    BlockSource* blockSource, not_null_ptr<ItemInstance> itemInstance,
    DefaultDispenseItemBehavior::eOUTCOME& outcome)
{
    return itemInstance;
}
