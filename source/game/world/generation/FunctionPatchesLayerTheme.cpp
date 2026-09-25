#include "game/world/generation/FunctionPatchesLayerTheme.hpp"

#include "game/core/DyeColor.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/SnowLayerBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/WoolCarpetBlock.hpp"

FunctionPatchesLayerTheme::FunctionPatchesLayerTheme(unsigned int seed, double noiseScale, BlockFunction blockFunction)
    : RandomPatchesLayerTheme(seed, noiseScale)
    , mBlockFunction(blockFunction)
{
}

const BlockState* FunctionPatchesLayerTheme::GRASS_PATCHES(Random&, float noise, int heightAboveSurface, bool)
{
    if (noise < 0.3f)
    {
        return NULL;
    }

    if (heightAboveSurface == ABOVE_SURFACE(1) || heightAboveSurface == ABOVE_SURFACE(2))
    {
        TypedBoxed<DoublePlantBlock::Variant*> variant(DoublePlantBlock::VARIANTS[2]);
        DoublePlantBlock::Half* plantHalf = heightAboveSurface == ABOVE_SURFACE(2)
            ? DoublePlantBlock::HALVES[DoublePlantBlock::Half::UPPER]
            : DoublePlantBlock::HALVES[DoublePlantBlock::Half::LOWER];
        TypedBoxed<DoublePlantBlock::Half*> half(plantHalf);
        const BlockState* state = Blocks::double_plant->defaultBlockState();
        state = state->setBoxedValue(DoublePlantBlock::VARIANT, &variant);
        return state->setBoxedValue(DoublePlantBlock::HALF, &half);
    }

    if (heightAboveSurface == 0)
    {
        const BlockState* state = Blocks::dirt->defaultBlockState();
        TypedBoxed<DirtBlock::Variant*> variant(DirtBlock::Variant::PODZOL);
        return state->setBoxedValue(DirtBlock::VARIANT, &variant);
    }

    return NULL;
}

const BlockState* FunctionPatchesLayerTheme::HELLFIRE_PATCHES(Random&, float noise, int heightAboveSurface, bool isAir)
{
    if (noise >= 0.5f)
    {
        if (!isAir && heightAboveSurface == 0)
        {
            return Blocks::netherrack->defaultBlockState();
        }

        if (isAir && heightAboveSurface == ABOVE_SURFACE(1))
        {
            return Blocks::FIRE->defaultBlockState();
        }
    }

    return NULL;
}

const BlockState* FunctionPatchesLayerTheme::SNOWLAYER_PATCHES(Random&, float noise, int heightAboveSurface, bool)
{
    if (noise > 0.4f && heightAboveSurface == ABOVE_SURFACE(1))
    {
        int layers = static_cast<int>((5.0 * (static_cast<double>(noise) - 0.4)) / 0.6) + 1;
        const BlockState* state = Blocks::snow_layer->defaultBlockState();
        TypedBoxed<int> value(layers);
        return state->setBoxedValue(SnowLayerBlock::LAYERS, &value);
    }

    return NULL;
}

const BlockState* FunctionPatchesLayerTheme::TNT_TRAPS(Random&, float noise, int heightAboveSurface, bool)
{
    if (noise >= 0.6f)
    {
        int aboveSurface = ABOVE_SURFACE(1);
        if (heightAboveSurface < aboveSurface)
        {
            if (heightAboveSurface == 0)
            {
                return Blocks::gravel->defaultBlockState();
            }

            if (heightAboveSurface == BELOW_SURFACE(1))
            {
                return Blocks::tnt->defaultBlockState();
            }

            return Blocks::AIR->defaultBlockState();
        }

        if (heightAboveSurface <= aboveSurface)
        {
            return Blocks::stone_pressure_plate->defaultBlockState();
        }
    }

    return NULL;
}

const BlockState* FunctionPatchesLayerTheme::CARPET_TRAP(Random&, float noise, int heightAboveSurface, bool)
{
    if (noise <= 0.11f)
    {
        if (heightAboveSurface == ABOVE_SURFACE(1))
        {
            const BlockState* state = Blocks::carpet->defaultBlockState();
            TypedBoxed<const DyeColor*> color(DyeColor::byBlockData(14));
            return state->setBoxedValue(gWoolCarpetColorProperty, &color);
        }

        if (noise < -0.43f && heightAboveSurface < 1)
        {
            return Blocks::AIR->defaultBlockState();
        }
    }

    return NULL;
}

const BlockState* FunctionPatchesLayerTheme::BlockAt(Random& random, float noise, int heightAboveSurface, bool isAir) const
{
    return mBlockFunction(random, noise, heightAboveSurface, isAir);
}
