#include "game/world/generation/CoralFeature.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CoralBlock.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

extern AbstractProperty<int>* CoralFanHang_PROPERTY_1;
extern AbstractProperty<int>* CoralFanHang_PROPERTY_2;

CoralFeature::CoralFeature() : Feature(false) {}
CoralFeature::~CoralFeature() {}

const Direction* CoralFeature::_randomDirectionExcept(Random& random, const Direction* except)
{
    const Direction* all[6] = {Direction::DOWN, Direction::UP, Direction::NORTH, Direction::SOUTH, Direction::WEST, Direction::EAST};
    const Direction* choices[5]; int n = 0; int i;
    for (i = 0; i < 6; ++i) if (all[i] != except) choices[n++] = all[i];
    return choices[random.nextInt(n)];
}

const Direction* CoralFeature::_randomDirectionExcept(Random& random, const Direction* a, const Direction* b)
{
    const Direction* all[6] = {Direction::DOWN, Direction::UP, Direction::NORTH, Direction::SOUTH, Direction::WEST, Direction::EAST};
    const Direction* choices[4]; int n = 0; int i;
    for (i = 0; i < 6; ++i) if (all[i] != a && all[i] != b) choices[n++] = all[i];
    return choices[random.nextInt(n)];
}

bool CoralFeature::_canOverwrite(Level* level, const BlockPos& pos, int data)
{
    const BlockState* state = level->getBlockState(pos);
    return state == NULL || level->isEmptyWaterBlock(pos) || Block::getElementId(state) == data;
}

bool CoralFeature::_setBlock(Level* level, const BlockPos& pos, const BlockState* state, int data)
{
    if (!_canOverwrite(level, pos, data)) return false;
    Feature::setBlock(level, pos, state, 0); return true;
}

bool CoralFeature::_setBlockOnSolid(Level* level, const BlockPos& pos, const BlockState* state, int data)
{
    if (!_canOverwrite(level, pos, data) || !level->isSolidBlockingBlock(pos.below())) return false;
    Feature::setBlock(level, pos, state, 0); return true;
}

void CoralFeature::_starFormation(Level* level, Random& random, const BlockPos& pos,
                                  const BlockState* state, float chance, int distance,
                                  bool inverted)
{
    const int data =
        state->getValue(static_cast<AbstractProperty<int>*>(CoralBlock_PROPERTY_1));
    if (!_setBlock(level, pos, state, data))
    {
        return;
    }

    const BlockPos arms[4] = { BlockPos(pos.x + distance, pos.y, pos.z),
                               BlockPos(pos.x - distance, pos.y, pos.z),
                               BlockPos(pos.x, pos.y, pos.z + distance),
                               BlockPos(pos.x, pos.y, pos.z - distance) };
    for (int i = 0; i < 4; ++i)
    {
        const float roll = random.nextFloat();
        if (inverted ? roll < chance : chance < roll)
        {
            if (!_setBlockOnSolid(level, arms[i], state, data))
            {
                return;
            }
        }
    }
}

void CoralFeature::_starCorners(Level* level, Random& random, const BlockPos& pos,
                                const BlockState* state, float chance, int distance,
                                bool inverted)
{
    const int data =
        state->getValue(static_cast<AbstractProperty<int>*>(CoralBlock_PROPERTY_1));
    const BlockPos corners[4] = { BlockPos(pos.x + distance, pos.y, pos.z + distance),
                                  BlockPos(pos.x - distance, pos.y, pos.z + distance),
                                  BlockPos(pos.x - distance, pos.y, pos.z + distance),
                                  BlockPos(pos.x + distance, pos.y, pos.z - distance) };
    for (int i = 0; i < 4; ++i)
    {
        const float roll = random.nextFloat();
        if (inverted ? roll < chance : chance < roll)
        {
            if (!_setBlockOnSolid(level, corners[i], state, data))
            {
                return;
            }
        }
    }
}

void CoralFeature::_buildPlantArm(Level* level, Random& random, const BlockPos& pos, const BlockState* state, std::vector<BlockPos>& positions, std::vector<std::pair<BlockPos, const Direction*> >& sides, const Direction* direction, bool inverted)
{
    BlockPos p = pos;
    int i;
    for (i = 0; i < 1 + random.nextInt(4); ++i)
    {
        p = p.relative(direction);
        if (!_setBlock(level, p, state, state->mElementId))
        {
            break;
        }
        positions.push_back(p);
        if (random.nextInt(3) == 0)
        {
            sides.push_back(std::make_pair(p, direction));
        }
    }
}

void CoralFeature::_buildPlantLike(Level* level, const BlockPos& pos, Random& random, std::vector<BlockPos>& positions, std::vector<std::pair<BlockPos, const Direction*> >& sides, const BlockState* state)
{
    _setBlock(level, pos, state, state->mElementId); int i; for (i = 0; i < 5; ++i) _buildPlantArm(level, random, pos, state, positions, sides, _randomDirectionExcept(random, Direction::DOWN), false);
    _starFormation(level, random, pos, state, .7f, 1, false); _starCorners(level, random, pos, state, .35f, 1, false);
}

void CoralFeature::_buildSpire(Level* level, const BlockPos& pos, Random& random, std::vector<BlockPos>& positions, std::vector<std::pair<BlockPos, const Direction*> >& sides, const BlockState* state)
{
    BlockPos p = pos; int i; for (i = 0; i < 2 + random.nextInt(4); ++i) { if (!_setBlockOnSolid(level, p, state, state->mElementId)) break; positions.push_back(p); p = p.above(); }
}

void CoralFeature::_buildHand(Level* level, const BlockPos& pos, Random& random, std::vector<BlockPos>& positions, std::vector<std::pair<BlockPos, const Direction*> >& sides, const BlockState* state)
{
    const Direction* d = _randomDirectionExcept(random, Direction::UP, Direction::DOWN); _buildPlantArm(level, random, pos, state, positions, sides, d, false); _buildPlantArm(level, random, pos, state, positions, sides, _randomDirectionExcept(random, d), false);
}

void CoralFeature::_buildSmallClump(Level* level, const BlockPos& pos, Random& random, std::vector<BlockPos>& positions, std::vector<std::pair<BlockPos, const Direction*> >& sides, const BlockState* state)
{
    int x, z; for (x = -1; x <= 1; ++x) for (z = -1; z <= 1; ++z) { BlockPos p = pos.offset(x, 0, z); if (random.nextInt(3) != 0 && _setBlock(level, p, state, state->mElementId)) positions.push_back(p); }
}

void CoralFeature::_placeCoral(Level* level, const BlockPos& pos, Random& random, std::vector<BlockPos>& positions, std::vector<std::pair<BlockPos, const Direction*> >& sides, int type)
{
    const BlockState* state = Blocks::coral->defaultBlockState();
    if (type == 0) _buildPlantLike(level, pos, random, positions, sides, state); else if (type == 1) _buildSpire(level, pos, random, positions, sides, state); else if (type == 2) _buildHand(level, pos, random, positions, sides, state); else _buildSmallClump(level, pos, random, positions, sides, state);
}

void CoralFeature::_placeTopDecorations(Level* level, const BlockPos& pos, Random& random)
{
    BlockPos p = pos.above(); if (level->isEmptyWaterBlock(p) && random.nextInt(5) == 0) Feature::setBlock(level, p, Blocks::coral_fan->defaultBlockState(), 0);
}

bool CoralFeature::_canPlaceHangOn(Level* level, const BlockPos& pos)
{
    Block* block = level->getBlock(pos);
    return block == Blocks::coral_block || block == Blocks::stone;
}
int CoralFeature::_turnFacingIntoint(const Direction* d) { if (d == Direction::NORTH) return 2; if (d == Direction::EAST) return 1; if (d == Direction::SOUTH) return 3; return 0; }
const BlockState* CoralFeature::_setCoralHangData(int facing, int variant, int type)
{
    if (type != 0 && type != 1)
    {
        TypedBoxed<int> facingValue(facing);
        return Blocks::coral_fan_hang3->defaultBlockState()->setBoxedValue(
            CoralFanHang_PROPERTY_2, &facingValue);
    }

    const BlockState* state = type == 0 ? Blocks::coral_fan_hang->defaultBlockState()
                                        : Blocks::coral_fan_hang2->defaultBlockState();
    TypedBoxed<int> variantValue(variant);
    state = state->setBoxedValue(CoralFanHang_PROPERTY_1, &variantValue);
    TypedBoxed<int> facingValue(facing);
    return state->setBoxedValue(CoralFanHang_PROPERTY_2, &facingValue);
}

void CoralFeature::_placeSideDecorations(Level* level, const BlockPos& pos, Random& random, const Direction* direction)
{
    BlockPos p = pos.relative(direction); if (_canPlaceHangOn(level, p) && level->isEmptyWaterBlock(p)) Feature::setBlock(level, p, _setCoralHangData(_turnFacingIntoint(direction), random.nextInt(2), random.nextInt(3)), 0);
}

bool CoralFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (level->mIsClientSide) return true; BlockPos pos = origin; while (!level->isSolidBlockingBlock(pos) && pos.y > 58) pos = pos.below(); if (!level->isSolidBlockingBlock(pos)) return false;
    std::vector<BlockPos> positions; std::vector<std::pair<BlockPos, const Direction*> > sides; _placeCoral(level, pos, random, positions, sides, random.nextInt(5));
    std::vector<BlockPos>::iterator i; for (i = positions.begin(); i != positions.end(); ++i) _placeTopDecorations(level, *i, random);
    std::vector<std::pair<BlockPos, const Direction*> >::iterator j; for (j = sides.begin(); j != sides.end(); ++j) _placeSideDecorations(level, j->first, random, j->second); return true;
}
