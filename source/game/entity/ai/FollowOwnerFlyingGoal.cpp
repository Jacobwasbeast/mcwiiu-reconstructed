#include "game/entity/ai/FollowOwnerFlyingGoal.hpp"

#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

FollowOwnerFlyingGoal::FollowOwnerFlyingGoal(TamableAnimal* tamable, double speedModifier,
                                             float startDistance, float stopDistance)
    : FollowOwnerGoal(tamable, speedModifier, startDistance, stopDistance)
{
}

bool FollowOwnerFlyingGoal::isTeleportFriendlyBlock(int x, int z, int y, int dx, int dz)
{
    BlockPos blockPos(x + dx, y - 1, z + dz);
    const BlockState* blockState = mLevel->getBlockState(blockPos);
    return (blockState->isTopSolidBlocking() || blockState->getMaterial() == Material::LEAVES)
        && mLevel->isEmptyBlock(BlockPos(x + dx, y, z + dz))
        && mLevel->isEmptyBlock(BlockPos(x + dx, y + 1, z + dz));
}

std::wstring FollowOwnerFlyingGoal::getName()
{
    return std::wstring(L"FollowOwnerFlyingGoal");
}
