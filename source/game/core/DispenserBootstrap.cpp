#include "game/core/DispenserBootstrap.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Position.hpp"
#include "game/core/PrimedTnt.hpp"
#include "game/core/Snowball.hpp"
#include "game/core/ThrownEgg.hpp"
#include "game/core/ThrownExpBottle.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/projectile/AbstractProjectileDispenseBehavior.hpp"
#include "game/entity/projectile/AbstractRotatedItemProjectile.hpp"
#include "game/entity/projectile/Projectile.hpp"
#include "game/entity/projectile/SmallFireball.hpp"
#include "game/entity/projectile/SpectralArrow.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/BucketItem.hpp"
#include "game/item/DyeItem.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DispenserBlock.hpp"
#include "game/world/block/DispenserBlockEntity.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/PumpkinBlock.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/block/ShulkerBoxBlockEntity.hpp"
#include "game/world/block/SkullBlock.hpp"
#include "game/world/block/SkullBlockEntity.hpp"
#include "game/world/block/TntBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

void DispenserBootstrap::StaticCtor()
{
    class ArrowDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~ArrowDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position, not_null_ptr<ItemInstance>)
        {
            boost::shared_ptr<TippableArrow> arrow(new TippableArrow(level, position->getX(), position->getY(), position->getZ()));
            arrow->setPickup(AbstractRotatedItemProjectile::ePickup_Allowed);
            return arrow;
        }
    };
    class TippedArrowDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~TippedArrowDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position, not_null_ptr<ItemInstance> itemInstance)
        {
            boost::shared_ptr<TippableArrow> arrow(new TippableArrow(level, position->getX(), position->getY(), position->getZ()));
            arrow->setEffectsFromItem(itemInstance);
            arrow->setPickup(AbstractRotatedItemProjectile::ePickup_Allowed);
            return arrow;
        }
    };
    class SpectralArrowDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~SpectralArrowDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position, not_null_ptr<ItemInstance>)
        {
            boost::shared_ptr<SpectralArrow> arrow(new SpectralArrow(level, position->getX(), position->getY(), position->getZ()));
            arrow->setPickup(AbstractRotatedItemProjectile::ePickup_Allowed);
            return arrow;
        }
    };
    class TridentDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~TridentDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position, not_null_ptr<ItemInstance> itemInstance)
        {
            boost::shared_ptr<ThrownTrident> trident(new ThrownTrident(level, itemInstance, position->getX(), position->getY(), position->getZ()));
            trident->setPickup(AbstractRotatedItemProjectile::ePickup_Allowed);
            return trident;
        }
    };
    class EggDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~EggDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position, not_null_ptr<ItemInstance>)
        {
            return boost::shared_ptr<Projectile>(new ThrownEgg(level, position->getX(), position->getY(), position->getZ()));
        }
    };
    class SnowballDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~SnowballDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position, not_null_ptr<ItemInstance>)
        {
            return boost::shared_ptr<Projectile>(new Snowball(level, position->getX(), position->getY(), position->getZ()));
        }
    };
    class ExpBottleDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~ExpBottleDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position, not_null_ptr<ItemInstance>)
        {
            return boost::shared_ptr<Projectile>(new ThrownExpBottle(level, position->getX(), position->getY(), position->getZ()));
        }
        virtual double getUncertainty() { return AbstractProjectileDispenseBehavior::getUncertainty() * 0.5; }
        virtual double getPower() { return AbstractProjectileDispenseBehavior::getPower() * 1.25; }
    };
    class ThrownPotionDispenseBehavior : public AbstractProjectileDispenseBehavior
    {
    public:
        virtual ~ThrownPotionDispenseBehavior() {}
        virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position, not_null_ptr<ItemInstance> itemInstance)
        {
            return boost::shared_ptr<Projectile>(new ThrownPotion(level, position->getX(), position->getY(), position->getZ(), itemInstance->copy()));
        }
        virtual double getUncertainty() { return AbstractProjectileDispenseBehavior::getUncertainty() * 0.5; }
        virtual double getPower() { return AbstractProjectileDispenseBehavior::getPower() * 1.25; }
    };
    class PotionDispenseBehavior : public DispenseItemBehavior
    {
    public:
        virtual ~PotionDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> dispense(BlockSource* source, not_null_ptr<ItemInstance> item)
        {
            return mDefaultDispenseItemBehavior.dispense(source, item);
        }

    private:
        DefaultDispenseItemBehavior mDefaultDispenseItemBehavior;
    };
    class SpawnEggDispenseBehavior : public DefaultDispenseItemBehavior
    {
    public:
        virtual ~SpawnEggDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            ResourceLocation type = SpawnEggItem::getNamedIdFrom(item);
            std::wstring name = item->hasCustomHoverName() ? item->getHoverName() : L"";
            int error = 0;
            boost::shared_ptr<Mob> mob = SpawnEggItem::getMob(level, type,
                source->getX() + direction->getStepX(), source->getY() + direction->getStepY(),
                source->getZ() + direction->getStepZ(), &error, name);
            if (!mob) { outcome = eOUTCOME_Failure; return item; }
            SpawnEggItem::updateCustomEntityTag(level, boost::shared_ptr<Player>(), item, boost::static_pointer_cast<Entity>(mob));
            item->shrink(1);
            outcome = eOUTCOME_Success;
            return item;
        }
    };
    class FireworksDispenseBehavior : public DefaultDispenseItemBehavior
    {
    public:
        virtual ~FireworksDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            if (level->countInstanceOf(0x88000005ULL, false, NULL, NULL) >= 200) { outcome = eOUTCOME_Failure; return item; }
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            Position* position = DispenserBlock::getDispensePosition(source);
            boost::shared_ptr<FireworksRocketEntity> rocket(new FireworksRocketEntity(level,
                position->getX() + direction->getStepX() * 0.2, position->getY() + direction->getStepY() * 0.2,
                position->getZ() + direction->getStepZ() * 0.2, item));
            level->addEntity(rocket);
            item->shrink(1);
            outcome = eOUTCOME_Success;
            return item;
        }
        virtual void playSound(BlockSource* source, eOUTCOME outcome)
        {
            if (outcome == eOUTCOME_Success) source->getWorld()->levelEvent(1004, source->getPos(), 0);
            else DefaultDispenseItemBehavior::playSound(source, outcome);
        }
    };
    class FireballDispenseBehavior : public DefaultDispenseItemBehavior
    {
    public:
        virtual ~FireballDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            if (level->countInstanceOf(0x8A40000400000000ULL, false, NULL, NULL) >= 200) { outcome = eOUTCOME_Failure; return item; }
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            Position* position = DispenserBlock::getDispensePosition(source);
            boost::shared_ptr<SmallFireball> fireball(new SmallFireball(level,
                position->getX() + direction->getStepX() * 0.3, position->getY() + direction->getStepY() * 0.3,
                position->getZ() + direction->getStepZ() * 0.3,
                direction->getStepX() + level->mRandom->nextGaussian() * 0.05,
                direction->getStepY() + level->mRandom->nextGaussian() * 0.05,
                direction->getStepZ() + level->mRandom->nextGaussian() * 0.05));
            level->addEntity(fireball);
            item->shrink(1);
            outcome = eOUTCOME_Success;
            return item;
        }
        virtual void playSound(BlockSource* source, eOUTCOME outcome)
        {
            if (outcome == eOUTCOME_Success) source->getWorld()->levelEvent(1018, source->getPos(), 0);
            else DefaultDispenseItemBehavior::playSound(source, outcome);
        }
    };
    class FilledBucketDispenseBehavior : public DefaultDispenseItemBehavior
    {
    public:
        virtual ~FilledBucketDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            BlockPos position = source->getPos().relative(direction);
            boost::shared_ptr<Player> player = level->getNearestPlayer(source->getX(), source->getY(), source->getZ(), 128.0, false, -1.0);
            if (!static_cast<BucketItem*>(item->getItem())->emptyBucket(player, level, position, item))
                return DefaultDispenseItemBehavior::dispense(source, item);
            outcome = eOUTCOME_Success;
            return not_null_ptr<ItemInstance>(new ItemInstance(Items::bucket));
        }
    };
    class EmptyBucketDispenseBehavior : public DefaultDispenseItemBehavior
    {
    public:
        virtual ~EmptyBucketDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            BlockPos position = source->getPos().relative(direction);
            const BlockState* state = level->getBlockState(position);
            Item* filled = NULL;
            if (state->getMaterial() == Material::WATER && state->getValue(LiquidBlock::LEVEL) == 0) filled = Items::waterBucket;
            else if (state->getMaterial() == Material::LAVA && state->getValue(LiquidBlock::LEVEL) == 0) filled = Items::lavaBucket;
            else return DefaultDispenseItemBehavior::execute(source, item, outcome);
            level->setBlock(position, Blocks::AIR->defaultBlockState(), 11, false);
            item->shrink(1);
            not_null_ptr<ItemInstance> result(new ItemInstance(filled));
            outcome = eOUTCOME_Success;
            if (item->isEmpty()) return result;
            boost::shared_ptr<DispenserBlockEntity> dispenser = boost::static_pointer_cast<DispenserBlockEntity>(source->getEntity());
            if (dispenser->addItem(result) < 0) DefaultDispenseItemBehavior::dispense(source, result);
            return item;
        }
    };
    class FlintAndSteelDispenseBehavior : public OptionalDispenseItemBehavior
    {
    public:
        virtual ~FlintAndSteelDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            BlockPos position = source->getPos().relative(direction);
            if (level->isEmptyBlock(position)) {
                level->setBlockAndUpdate(position, Blocks::FIRE->defaultBlockState());
                if (item->hurt(1, level->mRandom)) item->setCount(0);
            } else if (level->getBlockState(position)->getBlock() == Blocks::tnt) {
                TypedBoxed<bool> explode(true);
                const BlockState* state = Blocks::tnt->defaultBlockState()->setBoxedValue(TntBlock::EXPLODE, &explode);
                Blocks::tnt->onPlace(level, position, state);
                level->removeBlock(position);
            } else outcome = eOUTCOME_Failure;
            return item;
        }
    };
    class DyeDispenseBehavior : public OptionalDispenseItemBehavior
    {
    public:
        virtual ~DyeDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
        {
            outcome = eOUTCOME_Success;
            if (DyeColor::byItemData(item->getAuxValue()) != DyeColor::WHITE)
                return DefaultDispenseItemBehavior::execute(source, item, outcome);
            Level* level = source->getWorld();
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            BlockPos position = source->getPos().relative(direction);
            if (DyeItem::growCrop(item, level, position, false)) {
                if (!level->mIsClientSide) level->levelEvent(2005, position, 0);
            } else outcome = eOUTCOME_Failure;
            return item;
        }
    };
    class TntDispenseBehavior : public DefaultDispenseItemBehavior
    {
    public:
        virtual ~TntDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            if (!level->newPrimedTntAllowed() || !g_pApp->GetGameHostOption(eGameHostOption_13)) { outcome = eOUTCOME_Failure; return item; }
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            BlockPos position = source->getPos().relative(direction);
            boost::shared_ptr<PrimedTnt> tnt(new PrimedTnt(level, position.x + 0.5, position.y, position.z + 0.5, boost::shared_ptr<LivingEntity>()));
            level->addEntity(tnt);
            item->shrink(1);
            outcome = eOUTCOME_Success;
            return item;
        }
    };
    class SkullDispenseBehavior : public OptionalDispenseItemBehavior
    {
    public:
        virtual ~SkullDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            BlockPos position = source->getPos().relative(direction);
            SkullBlock* block = static_cast<SkullBlock*>(Blocks::skull);
            if (level->isEmptyBlock(position) && block->canSpawnMob(level, position, item)) {
                TypedBoxed<const Direction*> facing(direction);
                level->setBlock(position, block->defaultBlockState()->setBoxedValue(SkullBlock::FACING, &facing), 3, false);
                boost::shared_ptr<SkullBlockEntity> skull = boost::static_pointer_cast<SkullBlockEntity>(level->getBlockEntity(position));
                skull->setSkullType(item->getAuxValue(), L"");
                skull->setRotation(direction->getOpposite()->get2DDataValue() * 4);
                block->checkMobSpawn(level, position, skull, boost::shared_ptr<Player>());
                item->shrink(1);
                outcome = eOUTCOME_Success;
                return item;
            }
            return ArmorItem::dispenseArmor(source, item, outcome);
        }
    };
    class PumpkinDispenseBehavior : public OptionalDispenseItemBehavior
    {
    public:
        virtual ~PumpkinDispenseBehavior() {}
        virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
        {
            Level* level = source->getWorld();
            const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
            BlockPos position = source->getPos().relative(direction);
            PumpkinBlock* block = static_cast<PumpkinBlock*>(Blocks::pumpkin);
            if (level->isEmptyBlock(position) && block->canSpawnGolem(level, position)) {
                level->setBlockAndUpdate(position, block->defaultBlockState());
                item->shrink(1);
                outcome = eOUTCOME_Success;
                return item;
            }
            return ArmorItem::dispenseArmor(source, item, outcome);
        }
        virtual void playSound(BlockSource* source, eOUTCOME outcome)
        {
            source->getWorld()->levelEvent(outcome == eOUTCOME_Success ? 1000 : 1001, source->getPos(), 0);
        }
    };

    DispenserBlock::DISPENSER_REGISTRY.add(Items::arrow, new ArrowDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::tippedArrow, new TippedArrowDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::spectralArrow, new SpectralArrowDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::trident, new TridentDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::egg, new EggDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::snowball, new SnowballDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::experienceBottle, new ExpBottleDispenseBehavior());
    ThrownPotionDispenseBehavior* thrownPotion = new ThrownPotionDispenseBehavior();
    DispenserBlock::DISPENSER_REGISTRY.add(Items::splashPotion, thrownPotion);
    DispenserBlock::DISPENSER_REGISTRY.add(Items::lingeringPotion, thrownPotion);
    DispenserBlock::DISPENSER_REGISTRY.add(Items::potion, new PotionDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::spawnEgg, new SpawnEggDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::fireworks, new FireworksDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::fireCharge, new FireballDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::boat, new BoatDispenseBehavior(Boat::OAK));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::spruceBoat, new BoatDispenseBehavior(Boat::SPRUCE));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::birchBoat, new BoatDispenseBehavior(Boat::BIRCH));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::jungleBoat, new BoatDispenseBehavior(Boat::JUNGLE));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::darkOakBoat, new BoatDispenseBehavior(Boat::DARK_OAK));
    DispenserBlock::DISPENSER_REGISTRY.add(Items::acaciaBoat, new BoatDispenseBehavior(Boat::ACACIA));
    FilledBucketDispenseBehavior* filled = new FilledBucketDispenseBehavior();
    DispenserBlock::DISPENSER_REGISTRY.add(Items::waterBucket, filled);
    DispenserBlock::DISPENSER_REGISTRY.add(Items::lavaBucket, filled);
    DispenserBlock::DISPENSER_REGISTRY.add(Items::fishBucket, filled);
    DispenserBlock::DISPENSER_REGISTRY.add(Items::salmonBucket, filled);
    DispenserBlock::DISPENSER_REGISTRY.add(Items::tropicalBucket, filled);
    DispenserBlock::DISPENSER_REGISTRY.add(Items::pufferBucket, filled);
    DispenserBlock::DISPENSER_REGISTRY.add(Items::bucket, new EmptyBucketDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::flintAndSteel, new FlintAndSteelDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::dye, new DyeDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::tnt), new TntDispenseBehavior());
    DispenserBlock::DISPENSER_REGISTRY.add(Items::skull, new SkullDispenseBehavior());
    PumpkinDispenseBehavior* pumpkin = new PumpkinDispenseBehavior();
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::pumpkin), pumpkin);
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::carved_pumpkin), pumpkin);
    DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(Blocks::lit_pumpkin), pumpkin);
    const DyeColor* colors[] = { DyeColor::WHITE, DyeColor::ORANGE, DyeColor::MAGENTA, DyeColor::LIGHT_BLUE,
        DyeColor::YELLOW, DyeColor::LIME, DyeColor::PINK, DyeColor::GRAY, DyeColor::SILVER, DyeColor::CYAN,
        DyeColor::PURPLE, DyeColor::BLUE, DyeColor::BROWN, DyeColor::GREEN, DyeColor::RED, DyeColor::BLACK };
    for (unsigned int i = 0; i < sizeof(colors) / sizeof(colors[0]); ++i)
        DispenserBlock::DISPENSER_REGISTRY.add(Item::byBlock(ShulkerBoxBlock::getBlockByColor(colors[i])), new ShulkerBoxDispenseBehavior());
}

DispenserBootstrap::BoatDispenseBehavior::BoatDispenseBehavior(Boat::Type type) : mType(type) {}
DispenserBootstrap::BoatDispenseBehavior::~BoatDispenseBehavior() {}

not_null_ptr<ItemInstance> DispenserBootstrap::BoatDispenseBehavior::execute(BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
{
    Level* level = source->getWorld();
    const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
    double x = source->getX() + direction->getStepX() * 1.125;
    double y = source->getY() + direction->getStepY() * 1.125;
    double z = source->getZ() + direction->getStepZ() * 1.125;
    BlockPos position = source->getPos().relative(direction);
    const BlockState* state = level->getBlockState(position);
    double yOffset;
    if (level->countInstanceOf(ENTITY_TYPE_BOAT, true, NULL, NULL) > 39)
        return DefaultDispenseItemBehavior::dispense(source, item);
    if (state->getMaterial() == Material::WATER) yOffset = 1.0;
    else if (state->getMaterial() == Material::AIR && level->getBlockState(position.below())->getMaterial() == Material::WATER) yOffset = 0.0;
    else return DefaultDispenseItemBehavior::dispense(source, item);
    boost::shared_ptr<Boat> boat(new Boat(level, x, y + yOffset, z));
    boat->setType(mType);
    boat->mYRot = (float)direction->getOpposite()->toYRot();
    level->addEntity(boat);
    item->shrink(1);
    outcome = eOUTCOME_Success;
    return item;
}

void DispenserBootstrap::BoatDispenseBehavior::playSound(BlockSource* source, eOUTCOME outcome)
{
    DefaultDispenseItemBehavior::playSound(source, outcome);
}

DispenserBootstrap::OptionalDispenseItemBehavior::~OptionalDispenseItemBehavior() {}

void DispenserBootstrap::OptionalDispenseItemBehavior::playSound(BlockSource* source, eOUTCOME outcome)
{
    source->getWorld()->levelEvent(outcome == eOUTCOME_Failure ? 1000 : 1001, source->getPos(), 0);
}

DispenserBootstrap::ShulkerBoxDispenseBehavior::~ShulkerBoxDispenseBehavior() {}

not_null_ptr<ItemInstance> DispenserBootstrap::ShulkerBoxDispenseBehavior::execute(BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME&)
{
    Block* block = Block::byItem(item->getItem());
    Level* level = source->getWorld();
    const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
    BlockPos position = source->getPos().relative(direction);
    if (level->mayPlace(block, position, false, Direction::DOWN, boost::shared_ptr<Entity>())) {
        const Direction* facing = level->isEmptyBlock(position.below()) ? direction : Direction::UP;
        TypedBoxed<const Direction*> boxedFacing(facing);
        const BlockState* state = block->defaultBlockState()->setBoxedValue(DirectionalBlock::FACING, &boxedFacing);
        level->setBlockAndUpdate(position, state);
        boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(position);
        not_null_ptr<ItemInstance> placed = item->split(1);
        boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::static_pointer_cast<ShulkerBoxBlockEntity>(entity);
        if (placed->hasTag()) box->loadFromTag(placed->getTag()->getCompound(L"BlockEntityTag"));
        if (placed->hasCustomHoverName())
            box->setCustomName(placed->getHoverName(), placed->getOwnerUUID(), placed->wasCreatedByRestrictedPlayer(), placed->wasCreatedOnHost());
        level->blockUpdated(position, state->getBlock(), false);
    }
    return item;
}
