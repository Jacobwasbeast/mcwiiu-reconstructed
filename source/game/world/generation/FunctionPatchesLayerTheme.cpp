#include "game/world/generation/FunctionPatchesLayerTheme.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Property.hpp"
#include "game/core/Boxed.hpp"
#include "game/world/block/TypedBoxed.hpp"

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
        const BlockState* state = Blocks::double_plant->defaultBlockState();
        const std::vector<const Property*>& properties = state->getProperties();
        state = state->setBoxedValue(properties[0], properties[0]->getValueAtIndex(2));
        return state->setBoxedValue(properties[1], properties[1]->getValueAtIndex(heightAboveSurface == ABOVE_SURFACE(2) ? 1 : 0));
    }

    if (heightAboveSurface == 0)
    {
        const BlockState* state = Blocks::dirt->defaultBlockState();
        const std::vector<const Property*>& properties = state->getProperties();
        return state->setBoxedValue(properties[0], properties[0]->getValueAtIndex(2));
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
        const std::vector<const Property*>& properties = state->getProperties();
        TypedBoxed<int> value(layers);
        return state->setBoxedValue(properties[0], &value);
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
            return Blocks::carpet->getBlockState(14);
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
