#include "game/world/block/TntBlock.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Explosion.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PrimedTnt.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/item/Items.hpp"

AbstractProperty<bool>* TntBlock::EXPLODE = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"explode"));

TntBlock::TntBlock()
    : Block(Material::EXPLOSIVE), mTopIcon(NULL), mBottomIcon(NULL)
{
    DerivedInit();
    TypedBoxed<bool> explode(false);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(EXPLODE, &explode));
}

void TntBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onPlace(level, pos, state);
    if (level->hasNeighborSignal(pos) && gApp.GetGameHostOption(eGameHostOption_TNT))
    {
        TypedBoxed<bool> explode(true);
        destroy(level, pos, state->setBoxedValue(EXPLODE, &explode));
        level->removeBlock(pos);
    }
}

void TntBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (level->hasNeighborSignal(pos) && gApp.GetGameHostOption(eGameHostOption_TNT))
    {
        TypedBoxed<bool> explode(true);
        destroy(level, pos, state->setBoxedValue(EXPLODE, &explode));
        level->removeBlock(pos);
    }
}

void TntBlock::wasExploded(Level* level, const BlockPos& pos, Explosion* explosion)
{
    if (level->mIsClientSide || !level->newPrimedTntAllowed() || !gApp.GetGameHostOption(eGameHostOption_TNT))
        return;

    boost::shared_ptr<PrimedTnt> primedTnt(new PrimedTnt(level, pos.x + 0.5, pos.y, pos.z + 0.5,
        explosion->getSourceMob()));
    primedTnt->setFuse(level->mRandom->nextInt(primedTnt->getLife() / 4) + primedTnt->getLife() / 8);
    level->addEntity(primedTnt);
}

void TntBlock::destroy(Level* level, const BlockPos& pos, const BlockState* state)
{
    destroy(level, pos, state, boost::shared_ptr<LivingEntity>());
}

void TntBlock::destroy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<LivingEntity> entity)
{
    if (level->mIsClientSide)
        return;

    if ((state->getValue(EXPLODE) || Minecraft::InMiniGame(eMiniGameId_Tumble, true))
        && level->newPrimedTntAllowed() && gApp.GetGameHostOption(eGameHostOption_TNT))
    {
        boost::shared_ptr<PrimedTnt> primedTnt(new PrimedTnt(level, pos.x + 0.5, pos.y, pos.z + 0.5,
            entity));
        level->addEntity(primedTnt);
        level->playSound(boost::shared_ptr<Player>(), primedTnt->mX, primedTnt->mY, primedTnt->mZ,
            SoundEvents::TNT_PRIMED, SoundSource::eSoundSource_Blocks, 1.0f, 1.0f, 16.0f);
    }
}

bool TntBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY, float hitZ, bool clientSide)
{
    if (clientSide)
        return false;

    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (!item->isEmpty())
    {
        Item* held = item->getItem();
        if (held == Items::flintAndSteel || held == Items::fireCharge)
        {
            TypedBoxed<bool> explode(true);
            destroy(level, pos, state->setBoxedValue(EXPLODE, &explode), player);
            level->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
            if (held == Items::flintAndSteel)
                item->hurtAndBreak(1, player);
            else if (!player->isCreative())
                item->shrink(1);
            return true;
        }
    }
    return Block::use(level, pos, state, player, hand, direction, hitX, hitY, hitZ, clientSide);
}

void TntBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state, const boost::shared_ptr<Entity>& entity)
{
    if (!Minecraft::InMiniGame(eMiniGameId_Battle, true) && !level->mIsClientSide && entity != NULL)
    {
        if ((entity->GetType() & EntityType::ARROW) == EntityType::ARROW && entity->isOnFire())
        {
            boost::shared_ptr<Arrow> arrow = boost::dynamic_pointer_cast<Arrow>(entity);
            TypedBoxed<bool> explode(true);
            const BlockState* primedState = level->getBlockState(pos)->setBoxedValue(EXPLODE, &explode);
            boost::shared_ptr<LivingEntity> owner;
            if (arrow->mOwner != NULL
                && (arrow->mOwner->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
                owner = boost::dynamic_pointer_cast<LivingEntity>(arrow->mOwner);
            destroy(level, pos, primedState, owner);
            level->removeBlock(pos);
        }
    }
}

bool TntBlock::dropFromExplosion(Explosion*)
{
    return false;
}

void TntBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance)
{
    Block::setPlacedBy(level, pos, state, placer, itemInstance);
    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && gApp.GetGameHostOption(eGameHostOption_TNT))
    {
        TypedBoxed<bool> explode(true);
        destroy(level, pos, state->setBoxedValue(EXPLODE, &explode), placer);
        level->removeBlock(pos);
    }
}

const BlockState* TntBlock::getBlockState(int data)
{
    TypedBoxed<bool> explode((data & 1) != 0);
    return defaultBlockState()->setBoxedValue(EXPLODE, &explode);
}

int TntBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(EXPLODE) ? 1 : 0;
}

BlockStateDefinition* TntBlock::createBlockStateDefinition()
{
    const Property* properties[] = { EXPLODE };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* TntBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction == Direction::DOWN)
        return mBottomIcon;
    if (direction == Direction::UP)
        return mTopIcon;
    return mIcon;
}

void TntBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"tnt_side");
    mTopIcon = iconRegister->registerIcon(L"tnt_top");
    mBottomIcon = iconRegister->registerIcon(L"tnt_bottom");
}
