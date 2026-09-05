#include "game/world/block/SpongeBlock.hpp"

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

SpongeBlock::~SpongeBlock()
{
}

std::wstring SpongeBlock::getName()
{
    return std::wstring(L10N::GetString(0xba98e3d7));
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

    while (!queue.empty() && removed <= 64)
    {
        std::pair<BlockPos, int> current = queue.front();
        queue.pop_front();

        for (unsigned int i = 0; i < Direction::VALUES.size(); ++i)
        {
            const Direction* direction = Direction::VALUES[i];
            BlockPos adjacent = current.first.relative(direction);
            const BlockState* adjacentState = level->getBlockState(adjacent);

            if (adjacentState->getMaterial() == Material::WATER)
            {
                level->setBlock(adjacent, Blocks::AIR->defaultBlockState(), 3, false);
                ++removed;
                if (current.second < 6)
                    queue.push_back(std::make_pair(adjacent, current.second + 1));
            }
            else if (adjacentState->getBlock() == Blocks::FLOWING_WATER)
            {
                level->setBlockAndUpdate(adjacent, Blocks::AIR->defaultBlockState());
                level->setBlock(adjacent, Blocks::AIR->defaultBlockState(), 3, false);
                ++removed;
                if (current.second < 6)
                    queue.push_back(std::make_pair(adjacent, current.second + 1));
            }
            else if (adjacentState->getBlock() == Blocks::WATER)
            {
                level->setBlockAndUpdate(adjacent, Blocks::AIR->defaultBlockState());
                if (current.second < 6)
                    queue.push_back(std::make_pair(adjacent, current.second + 1));
            }
        }
    }

    return removed > 0;
}

void SpongeBlock::tryAbsorbWater(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!state->getValue(WET) && removeWaterBreadthFirstSearch(level, pos))
    {
        TypedBoxed<bool> wet(true);
        level->setBlock(pos, state->setBoxedValue(WET, &wet), 3, false);
        level->levelEvent(2001, pos, Block::getId(Blocks::WATER));
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
    if (direction == Direction::DOWN)
        return;

    BlockPos adjacent = pos.relative(direction);
    if (level->getBlockState(adjacent)->isSolidBlockingBlock())
        return;

    double x = pos.x + 0.5;
    double y = pos.y + 0.5;
    double z = pos.z + 0.5;
    double offset = random->nextDouble() * 0.8 + 0.1;
    if (direction == Direction::UP)
    {
        y += 0.5;
        x += random->nextDouble() * 0.8 - 0.4;
        z += random->nextDouble() * 0.8 - 0.4;
    }
    else if (direction->getAxis() == Direction::Axis::Y)
    {
        y -= 0.5;
        x += random->nextDouble() * 0.8 - 0.4;
        z += random->nextDouble() * 0.8 - 0.4;
    }
    else if (direction == Direction::NORTH || direction == Direction::SOUTH)
    {
        x += random->nextDouble() * 0.8 - 0.4;
        z += direction == Direction::NORTH ? -0.5 : 0.5;
        y += random->nextDouble() * 0.8 - 0.4;
    }
    else
    {
        x += direction == Direction::WEST ? -0.5 : 0.5;
        y += random->nextDouble() * 0.8 - 0.4;
        z += random->nextDouble() * 0.8 - 0.4;
    }

    level->addParticle(PARTICLE_DRIP_WATER, false, x, y, z, 0.0, 0.0, 0.0,
                       arrayWithLength<int>());
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
