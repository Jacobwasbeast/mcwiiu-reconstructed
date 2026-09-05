#include "game/world/chunk/DirtyChunkSorter.hpp"

#include "game/world/chunk/Chunk.hpp"
#include "game/entity/mob/LivingEntity.hpp"

DirtyChunkSorter::DirtyChunkSorter(boost::shared_ptr<LivingEntity> entity, int renderDistance)
{
    mEntity = entity;
    mRenderDistance = renderDistance;
}

bool DirtyChunkSorter::operator()(const Chunk* first, const Chunk* second) const
{
    if (second->mChunkVisibility->mVisible == false)
    {
        if (first->mChunkVisibility->mVisible != false)
        {
            return false;
        }
    }
    else if (first->mChunkVisibility->mVisible == false)
    {
        return true;
    }

    double firstDistance = first->distanceToSqr(mEntity);
    double secondDistance = second->distanceToSqr(mEntity);

    if (secondDistance <= firstDistance)
    {
        if (firstDistance <= secondDistance)
        {
            return second->mIndex <= first->mIndex;
        }
        return true;
    }
    return false;
}
