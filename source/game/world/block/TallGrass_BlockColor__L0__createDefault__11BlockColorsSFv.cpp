#include "game/world/block/TallGrass_BlockColor__L0__createDefault__11BlockColorsSFv.hpp"

#include "game/core/GrassColor.hpp"
#include "game/core/Boxed.hpp"
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/TallGrassBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

int TallGrass_BlockColor__L0__createDefault__11BlockColorsSFv::getColor(
    const BlockState* state, LevelSource* levelSource, const BlockPos* pos,
    int)
{
    if (levelSource == NULL || pos == NULL)
    {
        Boxed* boxed = state->getBoxedValue(TallGrassBlock_VARIANT);
        const bool typed = boxed != NULL && boxed->isA(typeid(TallGrassBlock::Variant*));
        if (typed)
        {
            const TallGrassBlock::Variant* variant =
                static_cast<TypedBoxed<TallGrassBlock::Variant*>*>(boxed)->getValue();
            if (variant != NULL)
                return GrassColor::get(0.5, 1.0);
        }

        return typed;
    }

    return BiomeColors::getAverageGrassColor(levelSource, *pos);
}
