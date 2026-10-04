#include "game/world/block/LeafBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/FoliageColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"

AbstractProperty<bool>* LeafBlock::DECAYABLE = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"decayable"));
AbstractProperty<bool>* LeafBlock::CHECK_DECAY = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"check_decay"));

extern unsigned int THREAD_STORAGE_INDEX;

static const std::wstring LEAF_TEXTURES[12] = {
    L"leaves", L"leaves_spruce", L"leaves_birch", L"leaves_jungle", L"leaves_acacia", L"leaves_big_oak",
    L"leaves_opaque", L"leaves_spruce_opaque", L"leaves_birch_fast", L"leaves_jungle_opaque", L"leaves_acacia_fast", L"leaves_big_oak_fast"
};

LeafBlock::LeafBlock()
    : TransparentBlock(Material::LEAVES, false)
{
}

LeafBlock::~LeafBlock()
{
    delete[] mLeafDistance;
}

bool LeafBlock::isLeaves(Block* block)
{
    return block == Blocks::leaves || block == Blocks::leaves2;
}

void LeafBlock::DerivedInit()
{
    Block::DerivedInit();
    mIgnoreSimilarity = false;
    mLeafDistance = NULL;
    setTicking(true);
    setDestroyTime(0.2f);
    setLightBlock(1);
    setSoundType(SoundType::GRASS);
}

int LeafBlock::getColor() const
{
    return FoliageColor::get(0.5, 1.0);
}

int LeafBlock::getColor(const BlockState* state)
{
    return FoliageColor::getDefaultColor();
}

int LeafBlock::getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex)
{
    return getColor(levelSource, pos, levelSource->getBlockState(pos));
}

int LeafBlock::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    return BiomeColors::getAverageFoliageColor(levelSource, pos);
}

void LeafBlock::onRemove(Level* level, const BlockPos& pos, const BlockState*)
{
    int px = pos.x;
    int py = pos.y;
    int pz = pos.z;
    if (!level->hasChunksAt(BlockPos(px - 2, py - 2, pz - 2), BlockPos(px + 2, py + 2, pz + 2)))
        return;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            for (int z = -1; z <= 1; ++z)
            {
                BlockPos neighborPos = pos.offset(x, y, z);
                const BlockState* neighborState = level->getBlockState(neighborPos);
                if (neighborState->getMaterial() == Material::LEAVES && !neighborState->getValue(CHECK_DECAY))
                {
                    level->setBlock(neighborPos, neighborState->setValue(CHECK_DECAY, true), Block::UPDATE_INVISIBLE, false);
                }
            }
        }
    }
}

void LeafBlock::die(Level* level, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(pos);
    Block::spawnResources(level, pos, state, 0);
    level->removeBlock(pos);
}

void LeafBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (level->mIsClientSide || !state->getValue(CHECK_DECAY) || !state->getValue(DECAYABLE))
        return;

    int px = pos.x;
    int pz = pos.z;
    int py = pos.y;

    if (mLeafDistance == NULL)
        mLeafDistance = new int[32768];

    int x;
    int y;
    int z;
    int distance;
    const int center = 16 * 1024 + 16 * 32 + 16;

    if (level->hasChunksAt(BlockPos(px - 5, py - 5, pz - 5), BlockPos(px + 5, py + 5, pz + 5)))
    {
        BlockPos checkPos(0, 0, 0);
        for (x = -4; x <= 4; ++x)
        {
            for (z = -4; z <= 4; ++z)
            {
                for (y = -4; y <= 4; ++y)
                {
                    checkPos.x = px + x;
                    checkPos.y = py + y;
                    checkPos.z = pz + z;
                    const BlockState* nearbyState = level->getBlockState(checkPos);
                    Block* nearbyBlock = nearbyState->getBlock();
                    int index = (x + 16) * 1024 + (y + 16) * 32 + z + 16;
                    if (nearbyBlock == Blocks::log || nearbyBlock == Blocks::log2)
                        mLeafDistance[index] = 0;
                    else if (nearbyState->getMaterial() == Material::LEAVES)
                        mLeafDistance[index] = -2;
                    else
                        mLeafDistance[index] = -1;
                }
            }
        }
    }

    int* leafDistance = mLeafDistance;
    for (distance = 1; distance <= 4; ++distance)
    {
        for (x = -4; x <= 4; ++x)
        {
            for (y = -4; y <= 4; ++y)
            {
                for (z = -4; z <= 4; ++z)
                {
                    int index = (x + 16) * 1024 + (y + 16) * 32 + z + 16;
                    if (leafDistance[index] == distance - 1)
                    {
                        if (leafDistance[index - 1024] == -2)
                            leafDistance[index - 1024] = distance;
                        if (leafDistance[index + 1024] == -2)
                            leafDistance[index + 1024] = distance;
                        if (leafDistance[index - 32] == -2)
                            leafDistance[index - 32] = distance;
                        if (leafDistance[index + 32] == -2)
                            leafDistance[index + 32] = distance;
                        if (leafDistance[index - 1] == -2)
                            leafDistance[index - 1] = distance;
                        if (leafDistance[index + 1] == -2)
                            leafDistance[index + 1] = distance;
                    }
                }
            }
        }
    }

    if (leafDistance[center] >= 0)
    {
        level->setBlock(pos, state->setValue(CHECK_DECAY, false), Block::UPDATE_INVISIBLE, false);
    }
    else
    {
        die(level, pos);
    }
}

void LeafBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    if (level->isRainingAt(pos.above()) && !level->getBlockState(pos.below())->isTopSolidBlocking()
        && random->nextInt(15) == 1)
    {
        arrayWithLength<int> parameters;
        level->addParticle(PARTICLE_DRIP_WATER, pos.x + random->nextFloat(), pos.y - 0.05,
                           pos.z + random->nextFloat(), 0.0, 0.0, 0.0, parameters);
    }
}

bool LeafBlock::canContainLiquid(const BlockState* state) const
{
    return true;
}

int LeafBlock::getResourceCount(Random* random)
{
    return random->nextInt(20) == 0 ? 1 : 0;
}

Item* LeafBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Item::byBlock(Blocks::sapling);
}

void LeafBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance, int lootBonus)
{
    if (level->mIsClientSide)
        return;

    int saplingChance = getLootDropChance(state);
    if (lootBonus > 0)
    {
        saplingChance -= 2 << lootBonus;
        if (saplingChance < 10)
            saplingChance = 10;
    }

    if (level->mRandom->nextInt(saplingChance) == 0)
    {
        Item* item = getResource(state, level->mRandom, lootBonus);
        int auxValue = getSpawnResourcesAuxValue(state);
        Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(item, 1, auxValue)));
    }

    int extraChance = 200;
    if (lootBonus > 0)
    {
        extraChance -= 10 << lootBonus;
        if (extraChance < 40)
            extraChance = 40;
    }
    dropExtraLoot(level, pos, state, extraChance);
}

void LeafBlock::dropExtraLoot(Level* level, const BlockPos& pos, const BlockState* state, int lootBonus)
{
}

int LeafBlock::getLootDropChance(const BlockState* state)
{
    return 20;
}

bool LeafBlock::isSolidRender(const BlockState* state) const
{
    return !mIgnoreSimilarity;
}

void LeafBlock::setFancy(bool fancy)
{
    mIgnoreSimilarity = fancy;
    Block::ThreadStorage* storage = static_cast<Block::ThreadStorage*>(TlsGetValue(THREAD_STORAGE_INDEX));
    storage->mFastGraphics = !fancy;
}

int LeafBlock::getRenderLayer()
{
    return 0;
}

bool LeafBlock::isViewBlocking(const BlockState* state)
{
    return false;
}

bool LeafBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    if (!mIgnoreSimilarity && levelSource->getBlockState(pos.relative(direction))->getBlock() == this)
        return false;
    return Block::shouldRenderFace(state, levelSource, pos, direction);
}

bool LeafBlock::shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state)
{
    return state->getValue(CHECK_DECAY) && state->getValue(DECAYABLE);
}

int LeafBlock::getDescriptionId(int auxValue)
{
    static const int descriptions[6] = {
        IDS_TILE_LEAVES_OAK, IDS_TILE_LEAVES_SPRUCE, IDS_TILE_LEAVES_BIRCH,
        static_cast<int>(IDS_TILE_LEAVES_JUNGLE), IDS_TILE_LEAVES_ACACIA, static_cast<int>(IDS_TILE_LEAVES_DARK_OAK)
    };
    const BlockState* state = auxValue == -1 ? defaultBlockState() : getBlockState(auxValue);
    return descriptions[getWoodType(state)];
}

TextureAtlasSprite* LeafBlock::getTexture(const Direction* direction, const BlockState* state)
{
    int woodType = getWoodType(state);
    Block::ThreadStorage* storage = static_cast<Block::ThreadStorage*>(TlsGetValue(THREAD_STORAGE_INDEX));
    return mIcons[storage->mFastGraphics * 6 + woodType];
}

void LeafBlock::registerIcons(IconRegister* iconRegister)
{
    int i;
    for (i = 0; i < 12; ++i)
        mIcons[i] = iconRegister->registerIcon(LEAF_TEXTURES[i]);
}

bool LeafBlock::canProvideSupport(const BlockState* state, const Direction* direction, EBlockSupportType supportType)
{
    return false;
}
