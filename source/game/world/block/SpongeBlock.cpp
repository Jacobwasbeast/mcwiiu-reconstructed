#include "game/world/block/SpongeBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

#include <deque>
#include <utility>

AbstractProperty<bool>* SpongeBlock::WET =
    static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"wet"));

SpongeBlock::SpongeBlock()
    : Block(Material::SPONGE), mWetIcon(NULL)
{
    DerivedInit();
    TypedBoxed<bool> wet(false);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(WET, &wet));
}

std::wstring SpongeBlock::getName()
{
    return std::wstring(L10N::GetString(IDS_TILE_SPONGE));
}

int SpongeBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(WET) ? 1 : 0;
}

int SpongeBlock::removeWaterBreadthFirstSearch(Level* level, const BlockPos& pos)
{
    std::deque<std::pair<BlockPos, int> > queue;
    queue.push_back(std::make_pair(pos, 0));
    int removed = 0;
    const BlockState* air = Blocks::air->defaultBlockState();
    while (!queue.empty())
    {
        std::pair<BlockPos, int> current = queue.front();
        queue.pop_front();
        for (unsigned int i = 0; i < Direction::VALUES.size(); ++i)
        {
            BlockPos adjacent = current.first.relative(Direction::VALUES[i]);
            const BlockState* adjacentState = level->getBlockState(adjacent);
            Block* extraBlock = level->getExtraBlock(adjacent);
            if (adjacentState->getMaterial() == Material::WATER)
            {
                level->setBlock(adjacent, air, Block::UPDATE_ALL, false);
            }
            else if (adjacentState->getBlock() == Blocks::bubble_column_segment)
            {
                level->setExtraBlock(adjacent, air, Block::UPDATE_ALL, false);
                level->setBlock(adjacent, air, Block::UPDATE_ALL, false);
            }
            else if (extraBlock != Blocks::air && level->getExtraBlockState(adjacent)->getMaterial() == Material::WATER)
            {
                level->setExtraBlock(adjacent, air, Block::UPDATE_ALL, false);
            }
            else
            {
                continue;
            }
            ++removed;
            if (current.second < 6)
                queue.push_back(std::make_pair(adjacent, current.second + 1));
        }
        if (removed > 64)
            break;
    }

    return removed > 0;
}

void SpongeBlock::tryAbsorbWater(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!state->getValue(WET) && removeWaterBreadthFirstSearch(level, pos))
    {
        TypedBoxed<bool> wet(true);
        level->setBlock(pos, state->setBoxedValue(WET, &wet), Block::UPDATE_ALL, false);
        level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, pos, Block::getId(Blocks::water));
    }
}

void SpongeBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    tryAbsorbWater(level, pos, state);
}

void SpongeBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                  Block* block, const BlockPos& fromPos)
{
    tryAbsorbWater(level, pos, state);
    Block::neighborChanged(state, level, pos, block, fromPos);
}

const BlockState* SpongeBlock::getBlockState(int data)
{
    TypedBoxed<bool> wet((data & 1) != 0);
    return defaultBlockState()->setBoxedValue(WET, &wet);
}

int SpongeBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(WET) ? 1 : 0;
}

BlockStateDefinition* SpongeBlock::createBlockStateDefinition()
{
    static const Property* properties[] = { WET };
    return new BlockStateDefinition(this, properties, 1);
}

void SpongeBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    if (!state->getValue(WET))
        return;

    const Direction* direction = Direction::getRandomDirection(*random);
    if (direction == Direction::UP)
        return;

    BlockPos adjacent = pos.relative(direction);
    if (level->getBlockState(adjacent)->isTopSolidBlocking())
        return;

    double x = pos.x;
    double y = pos.y;
    double z = pos.z;
    if (direction == Direction::DOWN)
    {
        y -= 0.05;
        x += random->nextDouble();
        z += random->nextDouble();
    }
    else
    {
        y += random->nextDouble() * 0.8;
        if (direction->getAxis() == Direction::Axis::X)
        {
            z += random->nextDouble();
            x += direction == Direction::EAST ? 1.1 : 0.05;
        }
        else
        {
            x += random->nextDouble();
            z += direction == Direction::SOUTH ? 1.1 : 0.05;
        }
    }

    level->addParticle(PARTICLE_DRIP_WATER, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
}

void SpongeBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"sponge");
    mWetIcon = iconRegister->registerIcon(L"sponge_wet");
}

TextureAtlasSprite* SpongeBlock::getTexture(const Direction*, const BlockState* state)
{
    return state->getValue(WET) ? mWetIcon : mIcon;
}
