#include "game/item/Containers.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Container.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

static Random randomGenerator;

void Containers::dropItemStack(Level* level, double x, double y, double z, not_null_ptr<ItemInstance> itemInstance)
{
    float spread = 0.8f;
    float offset = 0.1f;
    float xOffset = randomGenerator.nextFloat() * spread + offset;
    float yOffset = randomGenerator.nextFloat() * spread + offset;
    float zOffset = randomGenerator.nextFloat() * spread + offset;

    while (!itemInstance->isEmpty())
    {
        int count = randomGenerator.nextInt(21) + 10;
        not_null_ptr<ItemInstance> split = itemInstance->split(count);
        split->set4JData(itemInstance->get4JData());

        boost::shared_ptr<ItemEntity> itemEntity(new ItemEntity(level,
            x + (double)xOffset,
            y + (double)yOffset,
            z + (double)zOffset, split));
        itemEntity->mXd = randomGenerator.nextGaussian() * 0.05;
        itemEntity->mYd = randomGenerator.nextGaussian() * 0.05 + 0.2;
        itemEntity->mZd = randomGenerator.nextGaussian() * 0.05;
        level->addEntity(itemEntity);
    }
}

void Containers::dropContents(Level* level, double x, double y, double z, boost::shared_ptr<Container> container)
{
    for (unsigned int slot = 0; slot < container->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> itemInstance = container->getItem(slot);
        if (!itemInstance->isEmpty())
        {
            dropItemStack(level, x, y, z, itemInstance);
        }
    }
}

void Containers::dropContents(Level* level, const BlockPos& pos, boost::shared_ptr<Container> container)
{
    dropContents(level, (double)pos.x - 0.5,
        (double)pos.y - 0.5,
        (double)pos.z - 0.5, container);
}

void Containers::dropContents(Level* level, boost::shared_ptr<Entity> entity, boost::shared_ptr<Container> container)
{
    dropContents(level, entity->mX, entity->mY, entity->mZ, container);
}
