#include "game/world/block/LeafBlock.hpp"
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
#include "game/world/block/Material.hpp"

void* TlsGetValue(unsigned int index);
extern unsigned int THREAD_STORAGE_INDEX;

static const std::wstring LEAF_TEXTURES[12] = {
    L"leaves", L"leaves_spruce", L"leaves_birch", L"leaves_jungle", L"leaves_acacia", L"leaves_big_oak",
    L"leaves_opaque", L"leaves_spruce_opaque", L"leaves_birch_fast", L"leaves_jungle_opaque", L"leaves_acacia_fast", L"leaves_big_oak_fast"
};

LeafBlock* LeafBlock::mOldLeaves = NULL;
LeafBlock* LeafBlock::mNewLeaves = NULL;

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
    mFancy = false;
    mLeafDistance = NULL;
    setTicking(true);
    setLightEmission(0.2f);
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

void LeafBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    int x;
    int y;
    int z;
    if (!level->hasChunksAt(pos.offset(-2, -2, -2), pos.offset(2, 2, 2)))
        return;

    for (x = -1; x <= 1; ++x)
    {
        for (z = -1; z <= 1; ++z)
        {
            for (y = -1; y <= 1; ++y)
            {
                BlockPos neighborPos = pos.offset(x, y, z);
                const BlockState* neighborState = level->getBlockState(neighborPos);
                if (neighborState->getMaterial() == Material::LEAVES)
                {
                    Block* neighborBlock = neighborState->getBlock();
                    int data = neighborBlock->convertBlockStateToLegacyData(neighborState);
                    if ((data & 8) == 0)
                        level->setBlock(neighborPos, neighborBlock->getBlockState(data | 8), 4, false);
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
    if (level->mIsClientSide || !shouldBlockTick(level, pos, state))
        return;

    if (mLeafDistance == NULL)
        mLeafDistance = new int[32768];

    int x;
    int y;
    int z;
    int distance;
    const int center = 16 * 1024 + 16 * 32 + 16;

    if (level->hasChunksAt(pos.offset(-5, -5, -5), pos.offset(5, 5, 5)))
    {
        for (x = -4; x <= 4; ++x)
        {
            for (y = -4; y <= 4; ++y)
            {
                for (z = -4; z <= 4; ++z)
                {
                    const BlockState* nearbyState = level->getBlockState(pos.offset(x, y, z));
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

    for (distance = 1; distance <= 4; ++distance)
    {
        for (x = -4; x <= 4; ++x)
        {
            for (y = -4; y <= 4; ++y)
            {
                for (z = -4; z <= 4; ++z)
                {
                    int index = (x + 16) * 1024 + (y + 16) * 32 + z + 16;
                    if (mLeafDistance[index] == distance - 1)
                    {
                        if (mLeafDistance[index - 1024] == -2)
                            mLeafDistance[index - 1024] = distance;
                        if (mLeafDistance[index + 1024] == -2)
                            mLeafDistance[index + 1024] = distance;
                        if (mLeafDistance[index - 32] == -2)
                            mLeafDistance[index - 32] = distance;
                        if (mLeafDistance[index + 32] == -2)
                            mLeafDistance[index + 32] = distance;
                        if (mLeafDistance[index - 1] == -2)
                            mLeafDistance[index - 1] = distance;
                        if (mLeafDistance[index + 1] == -2)
                            mLeafDistance[index + 1] = distance;
                    }
                }
            }
        }
    }

    if (mLeafDistance[center] >= 0)
    {
        int data = convertBlockStateToLegacyData(state);
        level->setBlock(pos, getBlockState(data & ~8), 4, false);
    }
    else
    {
        die(level, pos);
    }
}

void LeafBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    if (level->isRainingAt(pos.above()) && !level->getBlockState(pos.below())->isSolidRender()
        && random->nextInt(15) == 1)
    {
        arrayWithLength<int> parameters;
        level->addParticle(PARTICLE_DRIP_WATER, pos.x + random->nextFloat(), pos.y - 0.05,
                           pos.z + random->nextFloat(), 0.014, 0.014, 0.014, parameters);
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
    return !mFancy;
}

void LeafBlock::setFancy(bool fancy)
{
    mFancy = fancy;
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
    if (!mFancy && levelSource->getBlock(pos.relative(direction)) == this)
        return false;
    return Block::shouldRenderFace(state, levelSource, pos, direction);
}

bool LeafBlock::shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state)
{
    int data = convertBlockStateToLegacyData(state);
    return (data & 4) == 0 && (data & 8) != 0;
}

int LeafBlock::getDescriptionId(int auxValue)
{
    static const int descriptions[6] = {
        0x07F97ECD, 0x17B87ECA, 0x66F04DAA,
        static_cast<int>(0x81F6464F), 0x2F02AC98, static_cast<int>(0xA3CFE26E)
    };
    const BlockState* state = auxValue == -1 ? defaultBlockState() : getBlockState(auxValue);
    return descriptions[getWoodType(state)];
}

TextureAtlasSprite* LeafBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Block::ThreadStorage* storage = static_cast<Block::ThreadStorage*>(TlsGetValue(THREAD_STORAGE_INDEX));
    return mIcons[(storage->mFastGraphics ? 6 : 0) + getWoodType(state)];
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
