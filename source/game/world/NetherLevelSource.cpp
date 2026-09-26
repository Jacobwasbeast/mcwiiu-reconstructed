#include "game/world/NetherLevelSource.hpp"

#include <cmath>

#include "game/core/Bush.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FallingBlock.hpp"
#include "game/world/block/FjNetherrackBlockPred.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/BushFeature.hpp"
#include "game/world/generation/HellFireFeature.hpp"
#include "game/world/generation/HellPortalFeature.hpp"
#include "game/world/generation/HellSpringFeature.hpp"
#include "game/world/generation/LargeHellCaveFeature.hpp"
#include "game/world/generation/LightGemFeature.hpp"
#include "game/world/generation/OreFeature.hpp"
#include "game/core/XMem.hpp"


static FjNetherrackBlockPred sNetherrackPredicate;

NetherLevelSource::NetherLevelSource(Level* level, bool generateStructures, long long seed)
{
    mLevelSize = static_cast<int>(std::ceil(
        static_cast<float>(level->getLevelData()->getXZSize()) /
        static_cast<float>(level->getLevelData()->getHellScale())));
    mGenerateStructures = generateStructures;
    mLevel = level;
    mRandom = Random(seed);
    mPopulationRandom = Random(seed);

    mNoise1 = new PerlinNoise(&mRandom, 16);
    mNoise2 = new PerlinNoise(&mRandom, 16);
    mNoise3 = new PerlinNoise(&mRandom, 8);
    mNoise4 = new PerlinNoise(&mRandom, 4);
    mNoise5 = new PerlinNoise(&mRandom, 4);
    mNoise6 = new PerlinNoise(&mRandom, 10);
    mNoise7 = new PerlinNoise(&mRandom, 16);

    level->setSeaLevel(63);

    mFireFeature = new HellFireFeature();
    mLightGemFeature = new LightGemFeature();
    mHellPortalFeature = new HellPortalFeature();
    mQuartzOre = new OreFeature(Blocks::quartz_ore->defaultBlockState(), 14, &sNetherrackPredicate);
    mMagmaOre = new OreFeature(Blocks::magma->defaultBlockState(), 33, &sNetherrackPredicate);
    mLavaSpring = new HellSpringFeature(Blocks::flowing_lava, true);
    mLavaSpringOpen = new HellSpringFeature(Blocks::flowing_lava, false);
    mBrownMushroom = new BushFeature(static_cast<Bush*>(Blocks::brown_mushroom));
    mRedMushroom = new BushFeature(static_cast<Bush*>(Blocks::red_mushroom));
    mNetherBridgeFeature = new NetherBridgeFeature();
    mCaves = new LargeHellCaveFeature();
}

NetherLevelSource::~NetherLevelSource()
{
    delete mNoise1;
    delete mNoise2;
    delete mNoise3;
    delete mNoise4;
    delete mNoise5;
    delete mNoise6;
    delete mNoise7;
    delete mFireFeature;
    delete mLightGemFeature;
    delete mHellPortalFeature;
    delete mQuartzOre;
    delete mMagmaOre;
    delete mLavaSpring;
    delete mLavaSpringOpen;
    delete mBrownMushroom;
    delete mRedMushroom;
    delete mNetherBridgeFeature;
    delete mCaves;
}

void NetherLevelSource::prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer)
{
    const int lavaLevel = mLevel->getSeaLevel() / 2 + 1;
    arrayWithLength<double> buffer = getHeights(arrayWithLength<double>(), chunkX * 4, 0, chunkZ * 4, 5, 17, 5);

    for (int xc = 0; xc < 4; ++xc)
    {
        for (int zc = 0; zc < 4; ++zc)
        {
            for (int yc = 0; yc < 16; ++yc)
            {
                double s0 = buffer.data[((xc + 0) * 5 + zc + 0) * 17 + yc + 0];
                double s1 = buffer.data[((xc + 0) * 5 + zc + 1) * 17 + yc + 0];
                double s2 = buffer.data[((xc + 1) * 5 + zc + 0) * 17 + yc + 0];
                double s3 = buffer.data[((xc + 1) * 5 + zc + 1) * 17 + yc + 0];
                const double s0a = (buffer.data[((xc + 0) * 5 + zc + 0) * 17 + yc + 1] - s0) * 0.125;
                const double s1a = (buffer.data[((xc + 0) * 5 + zc + 1) * 17 + yc + 1] - s1) * 0.125;
                const double s2a = (buffer.data[((xc + 1) * 5 + zc + 0) * 17 + yc + 1] - s2) * 0.125;
                const double s3a = (buffer.data[((xc + 1) * 5 + zc + 1) * 17 + yc + 1] - s3) * 0.125;

                for (int y = 0; y < 8; ++y)
                {
                    double x0 = s0;
                    double x1 = s1;
                    const double xStep0 = (s2 - s0) * 0.25;
                    const double xStep1 = (s3 - s1) * 0.25;

                    for (int x = 0; x < 4; ++x)
                    {
                        const double zStep = (x1 - x0) * 0.25;
                        double value = x0;

                        for (int z = 0; z < 4; ++z)
                        {
                            const BlockState* state = NULL;
                            if (yc * 8 + y < lavaLevel)
                            {
                                state = Blocks::lava->defaultBlockState();
                            }
                            if (value > 0.0)
                            {
                                state = Blocks::netherrack->defaultBlockState();
                            }
                            primer->setState(x + xc * 4, y + yc * 8, z + zc * 4, state);
                            value += zStep;
                        }

                        x0 += xStep0;
                        x1 += xStep1;
                    }

                    s0 += s0a;
                    s1 += s1a;
                    s2 += s2a;
                    s3 += s3a;
                }
            }
        }
    }

    delete[] buffer.data;
}

void NetherLevelSource::buildSurfaces(int chunkX, int chunkZ, ChunkPrimer* primer)
{
    const int seaLevel = mLevel->getSeaLevel();

    arrayWithLength<double> sandBuffer(256, true);
    arrayWithLength<double> gravelBuffer(256, true);
    arrayWithLength<double> depthBuffer(256, true);

    sandBuffer = mNoise4->getRegion(sandBuffer, chunkX * 16, chunkZ * 16, 0, 16, 16, 1, 0.03125, 0.03125, 1.0);
    gravelBuffer = mNoise4->getRegion(gravelBuffer, chunkX * 16, 109, chunkZ * 16, 16, 1, 16, 0.03125, 1.0, 0.03125);
    depthBuffer = mNoise5->getRegion(depthBuffer, chunkX * 16, chunkZ * 16, 0, 16, 16, 1, 0.0625, 0.0625, 0.0625);

    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            const bool sand = mRandom.nextDouble() * 0.2 + sandBuffer.data[x + z * 16] > 0.0;
            const bool gravel = mRandom.nextDouble() * 0.2 + gravelBuffer.data[x + z * 16] > 0.0;
            const int runDepth = static_cast<int>(depthBuffer.data[x + z * 16] / 3.0 + 3.0 + mRandom.nextDouble() * 0.25);

            int run = -1;
            const BlockState* top = Blocks::netherrack->defaultBlockState();
            const BlockState* material = Blocks::netherrack->defaultBlockState();

            for (int y = 127; y >= 0; --y)
            {
                const int offs = ((x << 11) | (z << 7)) + y;
                bool edge = false;

                if (chunkX <= -mLevelSize / 2)
                {
                    if (x - mRandom.nextInt(4) <= 0 || chunkX < -mLevelSize / 2)
                    {
                        primer->setState(offs, Blocks::bedrock->defaultBlockState());
                        edge = true;
                    }
                }
                if (chunkZ <= -mLevelSize / 2)
                {
                    if (z - mRandom.nextInt(4) <= 0 || chunkZ < -mLevelSize / 2)
                    {
                        primer->setState(offs, Blocks::bedrock->defaultBlockState());
                        edge = true;
                    }
                }
                if (chunkX >= mLevelSize / 2 - 1)
                {
                    if (x + mRandom.nextInt(4) >= 15 || chunkX > mLevelSize / 2)
                    {
                        primer->setState(offs, Blocks::bedrock->defaultBlockState());
                        edge = true;
                    }
                }
                if (chunkZ >= mLevelSize / 2 - 1)
                {
                    if (z + mRandom.nextInt(4) >= 15 || chunkZ > mLevelSize / 2)
                    {
                        primer->setState(offs, Blocks::bedrock->defaultBlockState());
                        edge = true;
                    }
                }

                if (edge)
                {
                    continue;
                }

                if (y >= 127 - mRandom.nextInt(5) || y <= mRandom.nextInt(5))
                {
                    primer->setState(offs, Blocks::bedrock->defaultBlockState());
                    continue;
                }

                const BlockState* old = primer->getState(offs);
                if (old->getBlock() == NULL || old->getBlock()->mMaterial == Material::AIR)
                {
                    run = -1;
                }
                else if (old->getBlock() == Blocks::netherrack)
                {
                    if (run == -1)
                    {
                        if (runDepth <= 0)
                        {
                            top = Blocks::air->defaultBlockState();
                            material = Blocks::netherrack->defaultBlockState();
                        }
                        else if (y >= seaLevel - 3 && y <= seaLevel + 2)
                        {
                            top = Blocks::netherrack->defaultBlockState();
                            material = Blocks::netherrack->defaultBlockState();
                            if (gravel)
                            {
                                top = Blocks::gravel->defaultBlockState();
                                material = Blocks::netherrack->defaultBlockState();
                            }
                            if (sand)
                            {
                                top = Blocks::soul_sand->defaultBlockState();
                                material = Blocks::soul_sand->defaultBlockState();
                                if (mRandom.nextInt(16) == 0)
                                {
                                    int wartY = y + 1;
                                    if (wartY > 127)
                                    {
                                        wartY = 127;
                                    }
                                    const int wartOffs = ((x << 11) | (z << 7)) + wartY;
                                    if (primer->getState(wartOffs)->getBlock() != Blocks::bedrock)
                                    {
                                        primer->setState(wartOffs, Blocks::nether_wart->defaultBlockState());
                                    }
                                }
                            }
                        }

                        if (y < seaLevel + 1 && (top == NULL || top->getMaterial() == Material::AIR))
                        {
                            top = Blocks::lava->defaultBlockState();
                        }

                        run = runDepth;
                        if (y >= seaLevel || sand)
                        {
                            if (primer->getState(offs)->getBlock() != Blocks::bedrock)
                            {
                                primer->setState(offs, top);
                            }
                        }
                        else
                        {
                            if (primer->getState(offs)->getBlock() != Blocks::bedrock)
                            {
                                primer->setState(offs, material);
                            }
                        }
                    }
                    else if (run > 0)
                    {
                        --run;
                        if (primer->getState(z, y, x)->getBlock() != Blocks::bedrock)
                        {
                            primer->setState(z, y, x, material);
                        }
                    }
                }
            }
        }
    }

    delete[] sandBuffer.data;
    delete[] gravelBuffer.data;
    delete[] depthBuffer.data;
}

void NetherLevelSource::lightChunk(LevelChunk* chunk)
{
    chunk->recalcHeightmap();
}

arrayWithLength<double> NetherLevelSource::getHeights(arrayWithLength<double> buffer, int x, int y, int z,
                                                       int xSize, int ySize, int zSize)
{
    if (buffer.data == NULL)
    {
        buffer = arrayWithLength<double>(xSize * ySize * zSize, true);
    }

    const double s = 684.412;
    const double hs = 2053.236;

    arrayWithLength<double> pnr;
    arrayWithLength<double> ar;
    arrayWithLength<double> br;
    arrayWithLength<double> sr;
    arrayWithLength<double> dr;
    arrayWithLength<double> fi;
    arrayWithLength<double> fis;

    sr = mNoise6->getRegion(sr, x, y, z, xSize, 1, zSize, 1.0, 0.0, 1.0);
    dr = mNoise7->getRegion(dr, x, y, z, xSize, 1, zSize, 100.0, 0.0, 100.0);
    pnr = mNoise3->getRegion(pnr, x, y, z, xSize, ySize, zSize, s / 80.0, hs / 60.0, s / 80.0);
    ar = mNoise1->getRegion(ar, x, y, z, xSize, ySize, zSize, s, hs, s);
    br = mNoise2->getRegion(br, x, y, z, xSize, ySize, zSize, s, hs, s);

    int p = 0;
    arrayWithLength<double> yoffs(ySize, true);
    for (int yy = 0; yy < ySize; ++yy)
    {
        yoffs.data[yy] = std::cos(static_cast<double>(yy * 3.1415927f * 6.0f) / static_cast<double>(ySize)) * 2.0;

        double dd = yy;
        if (yy > ySize / 2)
        {
            dd = ySize - 1 - yy;
        }
        if (dd < 4.0)
        {
            dd = 4.0 - dd;
            yoffs.data[yy] -= dd * dd * dd * 10.0;
        }
    }

    for (int xx = 0; xx < xSize; ++xx)
    {
        for (int zz = 0; zz < zSize; ++zz)
        {
            for (int yy = 0; yy < ySize; ++yy)
            {
                double val = 0.0;
                const double yOffs = yoffs.data[yy];
                const double bb = ar.data[p] / 512.0;
                const double cc = br.data[p] / 512.0;
                const double v = (pnr.data[p] / 10.0 + 1.0) / 2.0;

                if (v < 0.0)
                {
                    val = bb;
                }
                else if (v > 1.0)
                {
                    val = cc;
                }
                else
                {
                    val = bb + (cc - bb) * v;
                }
                val -= yOffs;

                if (yy > ySize - 4)
                {
                    const double slide = static_cast<float>(yy - (ySize - 4)) / 3.0f;
                    val = val * (1.0 - slide) + -10.0 * slide;
                }

                if (static_cast<double>(yy) < 0.0)
                {
                    double slide = (0.0 - yy) / 4.0;
                    slide = Mth::clamp(slide, 0.0, 1.0);
                    val = val * (1.0 - slide) + -10.0 * slide;
                }

                buffer.data[p] = val;
                ++p;
            }
        }
    }

    delete[] pnr.data;
    delete[] ar.data;
    delete[] br.data;
    delete[] sr.data;
    delete[] dr.data;
    delete[] fi.data;
    delete[] fis.data;
    delete[] yoffs.data;

    return buffer;
}

LevelChunk* NetherLevelSource::createChunk(int chunkX, int chunkZ)
{
    mRandom.setSeed(static_cast<long long>(chunkX) * 341873128712LL + static_cast<long long>(chunkZ) * 132897987541LL);

    unsigned short* elementIds = static_cast<unsigned short*>(XPhysicalAlloc(65536, 0xFFFFFFFF, 0x1000, 4));
    XMemSet128(elementIds, 0, 65536);
    arrayWithLength<unsigned short> elements;
    elements.data = elementIds;
    elements.length = 32768;
    ChunkPrimer primer(false, elements);

    prepareHeights(chunkX, chunkZ, &primer);
    buildSurfaces(chunkX, chunkZ, &primer);
    mCaves->apply(mLevel, chunkX, chunkZ, &primer);
    if (mGenerateStructures)
    {
        mNetherBridgeFeature->apply(mLevel, chunkX, chunkZ, &primer);
    }

    LevelChunk* chunk = new LevelChunk(mLevel, &primer, chunkX, chunkZ);
    chunk->setCheckAllLight();
    XPhysicalFree(elementIds);
    return chunk;
}

void NetherLevelSource::postProcess(int chunkX, int chunkZ)
{
    FallingBlock::fallInstantly = true;

    const BlockPos origin(chunkX * 16, 0, chunkZ * 16);
    Biome* biome = mLevel->getBiome(origin.offset(16, 0, 16));
    ChunkPos chunkPos(chunkX, chunkZ);

    mPopulationRandom.setSeed(mLevel->getSeed());
    const long long xScale = mPopulationRandom.nextLong() / 2 * 2 + 1;
    const long long zScale = mPopulationRandom.nextLong() / 2 * 2 + 1;
    mPopulationRandom.setSeed((static_cast<long long>(chunkZ) * xScale + static_cast<long long>(chunkZ) * zScale) ^ mLevel->getSeed());

    mNetherBridgeFeature->postProcess(mLevel, mPopulationRandom, &chunkPos);

    for (int i = 0; i < 8; ++i)
    {
        const int x = mPopulationRandom.nextInt(16) + 8;
        const int y = mPopulationRandom.nextInt(120) + 4;
        const int z = mPopulationRandom.nextInt(16) + 8;
        mLavaSpringOpen->place(mLevel, mPopulationRandom, origin.offset(x, y, z));
    }

    for (int i = 0; i < mPopulationRandom.nextInt(mPopulationRandom.nextInt(10) + 1) + 1; ++i)
    {
        const int x = mPopulationRandom.nextInt(16) + 8;
        const int y = mPopulationRandom.nextInt(120) + 4;
        const int z = mPopulationRandom.nextInt(16) + 8;
        mFireFeature->place(mLevel, mPopulationRandom, origin.offset(x, y, z));
    }

    for (int i = 0; i < mPopulationRandom.nextInt(mPopulationRandom.nextInt(10) + 1); ++i)
    {
        const int x = mPopulationRandom.nextInt(16) + 8;
        const int y = mPopulationRandom.nextInt(120) + 4;
        const int z = mPopulationRandom.nextInt(16) + 8;
        mLightGemFeature->place(mLevel, mPopulationRandom, origin.offset(x, y, z));
    }

    for (int i = 0; i < 10; ++i)
    {
        const int x = mPopulationRandom.nextInt(16) + 8;
        const int y = mPopulationRandom.nextInt(128);
        const int z = mPopulationRandom.nextInt(16) + 8;
        mHellPortalFeature->place(mLevel, mPopulationRandom, origin.offset(x, y, z));
    }

    if (mPopulationRandom.nextBoolean())
    {
        const int x = mPopulationRandom.nextInt(16) + 8;
        const int y = mPopulationRandom.nextInt(128);
        const int z = mPopulationRandom.nextInt(16) + 8;
        mBrownMushroom->place(mLevel, mPopulationRandom, origin.offset(x, y, z));
    }

    if (mPopulationRandom.nextBoolean())
    {
        const int x = mPopulationRandom.nextInt(16) + 8;
        const int y = mPopulationRandom.nextInt(128);
        const int z = mPopulationRandom.nextInt(16) + 8;
        mRedMushroom->place(mLevel, mPopulationRandom, origin.offset(x, y, z));
    }

    for (int i = 0; i < 16; ++i)
    {
        const int x = mPopulationRandom.nextInt(16);
        const int y = mPopulationRandom.nextInt(108) + 10;
        const int z = mPopulationRandom.nextInt(16);
        mQuartzOre->place(mLevel, mPopulationRandom, origin.offset(x, y, z));
    }

    const int magmaLevel = mLevel->getSeaLevel() / 2 - 4;
    for (int i = 0; i < 4; ++i)
    {
        const int x = mPopulationRandom.nextInt(16);
        const int y = magmaLevel + mPopulationRandom.nextInt(10);
        const int z = mPopulationRandom.nextInt(16);
        mMagmaOre->place(mLevel, mPopulationRandom, origin.offset(x, y, z));
    }

    for (int i = 0; i < 16; ++i)
    {
        const int x = mPopulationRandom.nextInt(16);
        const int y = mPopulationRandom.nextInt(108) + 10;
        const int z = mPopulationRandom.nextInt(16);
        mLavaSpring->place(mLevel, mPopulationRandom, origin.offset(x, y, z));
    }

    biome->decorate(mLevel, mPopulationRandom, BlockPos(chunkX * 16, 0, chunkZ * 16));

    FallingBlock::fallInstantly = false;

    gApp.processSchematics(mLevel->getChunk(chunkX, chunkZ));
}

bool NetherLevelSource::postProcessLoadedChunk(LevelChunk* chunk, int chunkX, int chunkZ)
{
    return false;
}

std::vector<Biome::MobSpawnerData*>* NetherLevelSource::getMobsAt(MobCategory* category, const BlockPos& pos)
{
    if (category == MobCategory::MONSTER)
    {
        if (mNetherBridgeFeature->isInsideFeature(pos))
        {
            return mNetherBridgeFeature->getBridgeEnemies();
        }

        if (mNetherBridgeFeature->isInsideBoundingFeature(mLevel, pos)
            && mLevel->getBlockState(pos.below())->getBlock() == Blocks::nether_brick)
        {
            return mNetherBridgeFeature->getBridgeEnemies();
        }
    }

    return mLevel->getBiome(pos)->getMobs(category);
}

BlockPos* NetherLevelSource::findNearestMapFeature(Level* level, const std::wstring& feature,
                                                    const BlockPos& pos, bool findUnexplored)
{
    if (feature == L"Fortress" && mNetherBridgeFeature != NULL)
    {
        return mNetherBridgeFeature->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    return NULL;
}

void NetherLevelSource::recreateLogicStructuresForChunk(LevelChunk* chunk, int chunkX, int chunkZ)
{
    mNetherBridgeFeature->apply(mLevel, chunkX, chunkZ, NULL);
}

bool NetherLevelSource::isPosInFeature(Level* level, const std::wstring& feature, const BlockPos& pos)
{
    if (feature == L"Fortress" && mNetherBridgeFeature != NULL)
    {
        return mNetherBridgeFeature->isInsideFeature(pos);
    }
    return false;
}
