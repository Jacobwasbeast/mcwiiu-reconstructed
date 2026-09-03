#include "game/core/DefaultStateMapper.hpp"

#include "game/client/render/ModelResourceLocation.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"
#include "game/world/block/BlockState.hpp"

class Block;

extern DefaultedMappedRegistry<ResourceLocation, Block*>* blockRegistry
    __asm__("_ZN5Block12blockRegistryE");

DefaultStateMapper::~DefaultStateMapper()
{
}

ModelResourceLocation* DefaultStateMapper::getLocation(const BlockState* state)
{
    return ModelResourceLocation::Make(blockRegistry->getKey(state->getBlock()),
                                       getValuesString(state->getValues()));
}
