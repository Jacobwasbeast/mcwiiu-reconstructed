#include "game/world/block/SlimeBlock.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

#include <cmath>
#include <boost/pointer_cast.hpp>

SlimeBlock::SlimeBlock()
    : HalfTransparentBlock(L"slime", Material::CLAY, false, MaterialColor::GRASS)
{
    DerivedInit();
    mParticleGravity = 0.8f;
}

SlimeBlock::~SlimeBlock()
{
}

int SlimeBlock::getRenderLayer()
{
    return 1;
}

void SlimeBlock::fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance)
{
    if (entity->mOnGround)
    {
        Block::fallOn(level, pos, entity, fallDistance);
    }
    else
    {
        entity->causeFallDamage(fallDistance, 0.0f);
    }
}

void SlimeBlock::updateEntityAfterFallOn(Level* level, boost::shared_ptr<Entity> entity)
{
    if (entity->mYd > -0.08 && entity->mYd < 0.0)
    {
        if ((entity->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer =
                boost::dynamic_pointer_cast<MultiplayerLocalPlayer>(entity);
            if (localPlayer)
            {
                localPlayer->onSlimeBlockBounce();
            }
        }

        entity->mYd = -entity->mYd;
        if ((entity->GetType() & EntityType::LIVING_ENTITY) != EntityType::LIVING_ENTITY)
        {
            entity->mYd *= 1.1;
        }
    }
    else
    {
        Block::updateEntityAfterFallOn(level, entity);
    }
}

void SlimeBlock::stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity)
{
    if (std::abs(entity->mYd) < 0.1)
    {
        double speed = std::abs(entity->mYd) * 0.2 + 0.4;
        entity->mXd *= speed;
        entity->mZd *= speed;
    }

    Block::stepOn(level, pos, entity);
}

EBlockRenderShape SlimeBlock::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_SLIME_BLOCK;
}
