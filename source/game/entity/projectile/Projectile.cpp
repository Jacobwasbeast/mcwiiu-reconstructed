#include "game/entity/projectile/Projectile.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/GameMode.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/TntBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

Projectile::Projectile(Level* level, const boost::shared_ptr<LivingEntity>& owner)
    : mOwner(owner), mLevel(level)
{
}

Projectile::~Projectile()
{
}

bool Projectile::onHit(const HitResult* result)
{
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureProjectilesBreakBlocks()
        && !mLevel->mIsClientSide
        && result->mType == HitResult::BLOCK
        && mOwner
        && (mOwner->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
    {
        BlockPos pos = result->getBlockPos();
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(mOwner);
        GameMode* gameMode = player->GetGameMode();
        if (gameMode->MayDestroy(pos))
        {
            Block* block = mLevel->getBlock(pos);
            if (block != NULL)
            {
                TntBlock* tntBlock = dynamic_cast<TntBlock*>(block);
                if (tntBlock != NULL)
                {
                    TypedBoxed<bool> explode(true);
                    const BlockState* state =
                        mLevel->getBlockState(pos)->setBoxedValue(TntBlock::EXPLODE, &explode);
                    tntBlock->destroy(mLevel, pos, state, mOwner);
                    mLevel->removeBlock(pos);
                }
                else
                {
                    mLevel->destroyBlock(pos, false);
                }
                gameMode->RecordBlockDestroyed(pos, Block::getId(block));
                return true;
            }
        }
    }
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureInvertedExplodingGrenades())
    {
        BlockPos pos = result->getBlockPos();
        mLevel->explode(boost::shared_ptr<Entity>(), pos.x - 0.5, pos.y - 0.5, pos.z - 0.5, 2.0f,
            false, true, false, false, false, 1.0f);
    }
    return false;
}

