#include "game/world/generation/CoralCrustFeature.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Rotation.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/generation/StructureTemplate.hpp"

extern AbstractProperty<int>* CoralBlock_PROPERTY_1;
extern AbstractProperty<int>* CoralFan_PROPERTY_1;
extern AbstractProperty<int>* CoralFan_PROPERTY_2;
extern AbstractProperty<int>* CoralFanHang_PROPERTY_1;
extern AbstractProperty<int>* CoralFanHang_PROPERTY_2;
extern AbstractProperty<bool>* CoralFanHang_PROPERTY_3;

CoralCrustFeature::~CoralCrustFeature() {}

BlockPos CoralCrustFeature::getOffsetFromRot(Rotation* rotation, int type) const
{
    static const int values[6][4][3] = {
        {{6,0,6},{-7,0,6},{-7,0,-7},{6,0,-7}},
        {{6,0,6},{-6,0,6},{-7,0,-7},{6,0,-7}},
        {{4,0,4},{-4,0,4},{-4,0,-4},{4,0,-4}},
        {{4,0,4},{-4,0,4},{-4,0,-4},{4,0,-4}},
        {{3,0,6},{-7,0,3},{-2,0,-7},{7,0,-3}},
        {{3,0,6},{-7,0,3},{-2,0,-7},{7,0,-3}}
    };
    int r = rotation == Rotation::NONE ? 0 : rotation == Rotation::CLOCKWISE_90 ? 1 : rotation == Rotation::CLOCKWISE_180 ? 2 : 3;
    if (type < 0) type = 0;
    if (type > 5) type = 5;
    return BlockPos(values[type][r][0], values[type][r][1], values[type][r][2]);
}

BlockPos CoralCrustFeature::getCropOffsetFromRot(Rotation* rotation, int type) const
{
    static const int values[6][4][3] = {
        {{-5,0,-2},{3,0,-5},{5,0,2},{-2,0,5}},
        {{-7,-2,-3},{3,-2,-6},{6,-2,3},{-2,-2,5}},
        {{-4,0,-3},{3,0,-4},{4,0,3},{-3,0,4}},
        {{-3,0,-3},{3,0,-3},{3,0,3},{-3,0,3}},
        {{-2,0,-2},{2,0,-2},{2,0,2},{-2,0,2}},
        {{-5,0,-5},{6,0,-6},{5,0,5},{-5,0,5}}
    };
    int r = rotation == Rotation::NONE ? 0 : rotation == Rotation::CLOCKWISE_90 ? 1 : rotation == Rotation::CLOCKWISE_180 ? 2 : 3;
    if (type < 0) type = 0;
    if (type > 5) type = 5;
    return BlockPos(values[type][r][0], values[type][r][1], values[type][r][2]);
}

const BlockState* CoralCrustFeature::setColorData(Level*, const BlockState* state, int color) const
{
    TypedBoxed<int> value(color);
    return state->setBoxedValue(CoralBlock_PROPERTY_1, &value);
}

bool CoralCrustFeature::canOverwrite(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos) == NULL || level->isEmptyWaterBlock(pos);
}

void CoralCrustFeature::placeCoral(Level* level, const BlockPos& pos, Random& random, std::vector<BlockPos>& positions, std::vector<std::pair<BlockPos, const Direction*> >& sides, int type)
{
    const BlockState* state = setColorData(level, Blocks::coral_block->defaultBlockState(), type);
    if (random.nextInt(4) == 0)
    {
        if (!level->isUnderWater(pos, 259, 0)) return;
        const BlockPos checks[4] = {pos.east(), pos.north(), pos.west(), pos.south()};
        int i;
        for (i = 0; i < 4; ++i) if (canOverwrite(level, checks[i])) level->setBlock(checks[i], state, 0, false);
        level->setBlock(pos, state, 0, false);
        if (level->setBlock(pos.above(), state, 0, false) && random.nextInt(4) == 0) positions.push_back(pos.above(2));
        return;
    }
    int length = random.nextInt(1, 4);
    BlockPos current = pos;
    const Direction* direction = NULL;
    level->setBlock(current, state, 0, false);
    int i;
    for (i = 0; i < length; ++i)
    {
        if (direction == NULL || random.nextInt(2) == 0) direction = random.nextBoolean() ? Direction::EAST : Direction::WEST;
        else direction = random.nextBoolean() ? Direction::NORTH : Direction::SOUTH;
        current = current.relative(direction);
        if (!level->isUnderWater(current, 259, 0) || !canOverwrite(level, current) || !level->setBlock(current, state, 0, false)) return;
        if (random.nextInt(4) == 0) positions.push_back(current.above());
        if (random.nextBoolean()) sides.push_back(std::make_pair(current, direction));
        if (random.nextBoolean()) sides.push_back(std::make_pair(current, direction));
        if (random.nextInt(3) == 0) sides.push_back(std::make_pair(current, direction));
    }
}

void CoralCrustFeature::placeTopDecorations(Level* level, const BlockPos& pos, Random& random)
{
    if (level->isEmptyWaterBlock(pos) && !level->isEmptyWaterBlock(pos.below()) && random.nextInt(5) == 0)
    {
        const BlockState* state = Blocks::coral_fan->defaultBlockState();
        TypedBoxed<int> value(random.nextInt(CoralFan_PROPERTY_1->getValueCount()));
        state = state->setBoxedValue(CoralFan_PROPERTY_1, &value);
        TypedBoxed<int> secondValue(random.nextInt(CoralFan_PROPERTY_2->getValueCount()));
        state = state->setBoxedValue(CoralFan_PROPERTY_2, &secondValue);
        setBlock(level, pos, state, 0);
    }
}

bool CoralCrustFeature::canPlaceHangOn(Level* level, const BlockPos& pos)
{
    Block* block = level->getBlock(pos);
    return block == Blocks::coral_block || block == Blocks::stone;
}

const Direction* CoralCrustFeature::randomDirectionExcept(Random& random, const Direction* except) const
{
    const Direction* choices[3];
    if (except == Direction::NORTH) { choices[0] = Direction::SOUTH; choices[1] = Direction::WEST; choices[2] = Direction::EAST; }
    else if (except == Direction::SOUTH) { choices[0] = Direction::NORTH; choices[1] = Direction::WEST; choices[2] = Direction::EAST; }
    else if (except == Direction::EAST) { choices[0] = Direction::NORTH; choices[1] = Direction::SOUTH; choices[2] = Direction::WEST; }
    else { choices[0] = Direction::NORTH; choices[1] = Direction::SOUTH; choices[2] = Direction::EAST; }
    return choices[random.nextInt(3)];
}

int CoralCrustFeature::turnFacingIntoint(const Direction* direction) const
{
    if (direction == Direction::NORTH) return 2;
    if (direction == Direction::EAST) return 1;
    if (direction == Direction::SOUTH) return 3;
    return 0;
}

const BlockState* CoralCrustFeature::setCoralHangData(int facing, int variant, int type) const
{
    Block* block = type == 0 ? Blocks::coral_fan_hang : type == 1 ? Blocks::coral_fan_hang2 : Blocks::coral_fan_hang3;
    const BlockState* state = block->defaultBlockState();
    TypedBoxed<int> variantValue(variant);
    state = state->setBoxedValue(CoralFanHang_PROPERTY_1, &variantValue);
    TypedBoxed<int> facingValue(facing);
    return state->setBoxedValue(CoralFanHang_PROPERTY_2, &facingValue);
}

void CoralCrustFeature::placeSideDecorations(Level* level, const BlockPos& pos, Random& random, const Direction* direction)
{
    const Direction* side = randomDirectionExcept(random, direction);
    BlockPos target = pos.relative(side);
    if (canPlaceHangOn(level, target) && level->isEmptyWaterBlock(target) && level->isUnderWater(target, 259, 0))
        setBlock(level, target, setCoralHangData(turnFacingIntoint(side), random.nextInt(2), random.nextInt(3)), 0);
}

void CoralCrustFeature::placeCoralBase(Level* level, const BlockPos& pos, Random& random, const StructureTemplate* structure, StructurePlaceSettings& settings)
{
    StructureTemplate::MarkerMap markers = structure->getMarkers(pos, &settings);
    std::vector<BlockPos> positions;
    std::vector<std::pair<BlockPos, const Direction*> > sides;
    int type = random.nextInt(5);
    StructureTemplate::MarkerMap::iterator i;
    for (i = markers.begin(); i != markers.end(); ++i)
    {
        BlockPos marker = i->first;
        if (level->isSolidBlockingBlock(marker.below()))
        {
            if (level->isSolidBlockingBlock(marker)) marker = marker.above();
            placeCoral(level, marker, random, positions, sides, type);
        }
    }
    std::vector<BlockPos>::iterator p;
    for (p = positions.begin(); p != positions.end(); ++p) placeTopDecorations(level, *p, random);
    std::vector<std::pair<BlockPos, const Direction*> >::iterator s;
    for (s = sides.begin(); s != sides.end(); ++s) placeSideDecorations(level, s->first, random, s->second);
}

bool CoralCrustFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (level->mIsClientSide) return true;
    BlockPos position = origin;
    while (!level->isSolidBlockingBlock(position) && position.y <= 53) position = position.above();
    if (!level->isSolidBlockingBlock(position)) return false;
    std::vector<BlockPos> positions;
    std::vector<int> types;
    std::vector<Rotation*> rotations;
    StructurePlaceSettings settings;
    settings.setIgnoreEntities(true);
    int initialType = random.nextInt(5);
    Rotation* initialRotation = Rotation::getRandom(random);
    int i;
    if (random.nextInt(4) == 0)
    {
        int templateType = random.nextInt(6);
        settings.setRotation(initialRotation);
        const StructureTemplate* structure = level->getLevelStorage()->getStructureManager()->getOrCreate(static_cast<eMinecraftStructureTemplate>(templateType + 177));
        BlockPos target = position + getCropOffsetFromRot(initialRotation, templateType);
        structure->placeInWorld(level, target, &settings);
        placeCoralBase(level, target, random, structure, settings);
        return true;
    }
    int count = random.nextInt(4, 9);
    for (i = 0; i < count / 2; ++i)
    {
        int type = random.nextInt(5);
        Rotation* rotation = Rotation::getRandom(random);
        int x = random.nextInt(3, 12) * (random.nextBoolean() ? -1 : 1);
        int z = random.nextInt(3, 12) * (random.nextBoolean() ? -1 : 1);
        BlockPos candidate = position.offset(x, 0, z) + getOffsetFromRot(rotation, type);
        if (level->isSolidBlockingBlock(candidate.below()))
        {
            positions.push_back(candidate);
            types.push_back(type);
            rotations.push_back(rotation);
        }
    }
    positions.push_back(position);
    types.push_back(initialType);
    rotations.push_back(initialRotation);
    for (i = 0; i < (int)positions.size(); ++i)
    {
        settings.setRotation(rotations[i]);
        const StructureTemplate* structure = level->getLevelStorage()->getStructureManager()->getOrCreate(static_cast<eMinecraftStructureTemplate>(types[i] + 177));
        structure->placeInWorld(level, positions[i], &settings);
        placeCoralBase(level, positions[i], random, structure, settings);
    }
    return true;
}
