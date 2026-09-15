#include "game/world/block/EndGatewayBlock.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/client/render/BlockEntityRenderDispatcher.hpp"
#include "game/entity/mob/TheEndGatewayEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

static const AABB* END_GATEWAY_CLIP_SHAPE = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
static Property* END_GATEWAY_DATA = IntegerProperty::create(L"age", 0, 3);

EndGatewayBlock::EndGatewayBlock(Material* material)
    : BaseEntityBlock(material), EntityBlock()
{
    DerivedInit();
    setLightEmission(1.0f);
}

EndGatewayBlock::~EndGatewayBlock()
{
}

boost::shared_ptr<BlockEntity> EndGatewayBlock::newBlockEntity(Level*, int data)
{
    boost::shared_ptr<TheEndGatewayEntity> entity(new TheEndGatewayEntity());
    entity->setData(data);
    return entity;
}

bool EndGatewayBlock::shouldRenderFace(const BlockState*, LevelSource* levelSource,
                                       const BlockPos& pos, const Direction* direction)
{
    const BlockPos neighbor = pos.relative(direction);
    const BlockState* state = levelSource->getBlockState(neighbor);
    return state->getBlock()->isSolidRender(state) == false
        && state->getBlock()->getRenderShape(state) != 0;
}

const AABB* EndGatewayBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return END_GATEWAY_CLIP_SHAPE;
}

bool EndGatewayBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool EndGatewayBlock::isCubeShaped(const BlockState*)
{
    return false;
}

int EndGatewayBlock::getResourceCount(Random*)
{
    return 0;
}

void EndGatewayBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity.get() == NULL
        || blockEntity->GetType() != BlockEntityRenderDispatcher::kEndGatewayBlockEntityType)
        return;

    boost::shared_ptr<TheEndGatewayEntity> gateway = boost::static_pointer_cast<TheEndGatewayEntity>(blockEntity);
    int amount = gateway->getParticleAmount();
    for (int i = 0; i < amount; ++i)
    {
        double x = static_cast<double>(pos.x) + random->nextFloat();
        double y = static_cast<double>(pos.y) + random->nextFloat();
        double z = static_cast<double>(pos.z) + random->nextFloat();
        double velocityX = (random->nextFloat() - 0.5) * 0.5;
        double velocityY = (random->nextFloat() - 0.5) * 0.5;
        double velocityZ = (random->nextFloat() - 0.5) * 0.5;
        int sign = random->nextInt(2) * 2 - 1;
        if (random->nextBoolean())
        {
            z = static_cast<double>(pos.z) + 0.5 + 0.25 * sign;
            velocityZ = static_cast<double>(random->nextFloat() + random->nextFloat()) * sign;
        }
        else
        {
            x = static_cast<double>(pos.x) + 0.5 + 0.25 * sign;
            velocityX = static_cast<double>(random->nextFloat() + random->nextFloat()) * sign;
        }
        level->addParticle(PARTICLE_END_PORTAL, x, y, z, velocityX, velocityY, velocityZ,
                           arrayWithLength<int>());
    }
}

not_null_ptr<ItemInstance> EndGatewayBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>();
}

const MaterialColor* EndGatewayBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&)
{
    return MaterialColor::NONE;
}

EBlockFaceShape EndGatewayBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

bool EndGatewayBlock::isWaterBlocking() const
{
    return true;
}

bool EndGatewayBlock::waterSpreadCausesSpawn()
{
    return false;
}

bool EndGatewayBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

BlockStateDefinition* EndGatewayBlock::createBlockStateDefinition()
{
    const Property* properties[] = { END_GATEWAY_DATA };
    return new BlockStateDefinition(this, properties, 1);
}

const BlockState* EndGatewayBlock::getBlockState(int data)
{
    TypedBoxed<int> value(data);
    return defaultBlockState()->setBoxedValue(END_GATEWAY_DATA, &value);
}

int EndGatewayBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    Boxed* value = state->getBoxedValue(END_GATEWAY_DATA);
    if (value == NULL)
        return 0;
    return static_cast<const TypedBoxed<int>*>(value)->getValue();
}
