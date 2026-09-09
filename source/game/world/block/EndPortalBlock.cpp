#include "game/world/block/EndPortalBlock.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TheEndPortalBlockEntity.hpp"
#include "game/core/IconRegister.hpp"

EndPortalBlock::EndPortalBlock(Material* material)
    : BaseEntityBlock(material), EntityBlock()
{
    DerivedInit();
    setLightEmission(1.0f);
}

EndPortalBlock::~EndPortalBlock()
{
}

boost::shared_ptr<BlockEntity> EndPortalBlock::newBlockEntity(Level*, int)
{
    return boost::shared_ptr<TheEndPortalBlockEntity>(new TheEndPortalBlockEntity());
}

const AABB* EndPortalBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

bool EndPortalBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource,
                                      const BlockPos& pos, const Direction* direction)
{
    if (direction == NULL)
        return Block::shouldRenderFace(state, levelSource, pos, direction);
    return false;
}

void EndPortalBlock::addCollisionAABBs(const BlockState*, Level*, const BlockPos&, const AABB*,
                                       std::vector<AABB*>*, boost::shared_ptr<Entity>, bool)
{
}

bool EndPortalBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool EndPortalBlock::isCubeShaped(const BlockState*)
{
    return false;
}

int EndPortalBlock::getResourceCount(Random*)
{
    return 0;
}

void EndPortalBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                                  const boost::shared_ptr<Entity>& entity)
{
    if (entity->GetType() == 0x08000003ULL)
        return;

    if (Minecraft::InMiniGame((EMiniGameId)15, true)
        && Minecraft::GetInstance()->GetMiniGame()->ArePortalsDisabled())
        return;

    if (entity->isPassenger() || entity->isVehicle() || !entity->canChangeDimensions())
        return;

    AABB* portalBox;
    if ((entity->GetType() & 0x8A020000ULL) == 0x8A020000ULL)
    {
        portalBox = AABB::newTemp(pos.x, pos.y, pos.z, pos.x + 1.0, pos.y + 1.0, pos.z + 1.0);
    }
    else
    {
        portalBox = state->getShape(level, pos)->move(pos);
    }

    if (level->mIsClientSide)
        return;

    if (!entity->mBoundingBox->intersects(portalBox->mMinX, portalBox->mMinY, portalBox->mMinZ,
                                          portalBox->mMaxX, portalBox->mMaxY, portalBox->mMaxZ))
        return;

    if ((entity->GetType() & 0x0C800000ULL) == 0x0C800000ULL)
    {
        int x = 0;
        int z = 0;
        if (level->mDimension == NULL && !level->getLevelData()->getHasStrongholdEndPortal()
            && g_pApp->GetTerrainFeaturePosition((_eTerrainFeatureType)6, &x, &z))
        {
            level->getLevelData()->setXStrongholdEndPortal(x);
            level->getLevelData()->setZStrongholdEndPortal(z);
            level->getLevelData()->setHasStrongholdEndPortal();
        }
    }

    if (!entity->mIgnorePortals)
        entity->changeDimension(1);
}

void EndPortalBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    double x = static_cast<double>(pos.x) + random->nextFloat();
    double y = static_cast<double>(pos.y) + 0.8;
    double z = static_cast<double>(pos.z) + random->nextFloat();
    level->addParticle(PARTICLE_END_PORTAL, x, y, z, 0.0, 0.0, 0.0,
                       arrayWithLength<int>());
}

not_null_ptr<ItemInstance> EndPortalBlock::getCloneItemInstance(Level*, const BlockPos&,
                                                                  const BlockState*)
{
    return not_null_ptr<ItemInstance>();
}

const MaterialColor* EndPortalBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&)
{
    return MaterialColor::NONE;
}

EBlockFaceShape EndPortalBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&,
                                                   const Direction*)
{
    return (EBlockFaceShape)8;
}

bool EndPortalBlock::isWaterBlocking() const
{
    return true;
}

bool EndPortalBlock::waterSpreadCausesSpawn()
{
    return false;
}

bool EndPortalBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

void EndPortalBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"portal");
}

EBlockRenderShape EndPortalBlock::getRenderShape(const BlockState*)
{
    return (EBlockRenderShape)-1;
}
