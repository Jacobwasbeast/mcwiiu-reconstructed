#include "game/world/block/BlockStateMapper.hpp"

#include "game/core/DefaultStateMapper.hpp"
#include "game/core/StateMapper.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Block.hpp"

boost::unordered_map<const BlockState*, ModelResourceLocation*>* BlockStateMapper::getMapping()
{
    boost::unordered_map<const BlockState*, ModelResourceLocation*>* mapping =
        new boost::unordered_map<const BlockState*, ModelResourceLocation*>();

    DefaultedMappedRegistry<ResourceLocation, Block*>::iterator it;

    for (it = Block::blockRegistry->begin(); it != Block::blockRegistry->end(); ++it)
    {
        Block* block = it->first;

        if (ignoredBlocks.find(block) != ignoredBlocks.end())
        {
            continue;
        }

        bool temporary = false;
        StateMapper* stateMapper;

        if (stateMappers.find(block) == stateMappers.end())
        {
            temporary = true;
            stateMapper = new DefaultStateMapper();
        }
        else
        {
            stateMapper = stateMappers[block];
        }

        boost::unordered_map<const BlockState*, ModelResourceLocation*>* states = stateMapper->mapStates(block);
        mapping->insert(states->begin(), states->end());

        if (temporary)
        {
            delete stateMapper;
        }
    }

    return mapping;
}
