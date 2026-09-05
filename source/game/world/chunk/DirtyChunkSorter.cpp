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
    if (second->mClipChunk->mVisible == false)
    {
        if (first->mClipChunk->mVisible != false)
        {
            return false;
        }
    }
    else if (first->mClipChunk->mVisible == false)
    {
        return true;
    }

    double firstDistance = first->distanceToSqr(mEntity);
    double secondDistance = second->distanceToSqr(mEntity);

    if (secondDistance <= firstDistance)
    {
        if (firstDistance <= secondDistance)
        {
            return second->mUnknown <= first->mUnknown;
        }
        return true;
    }
    return false;
}
