#include "game/world/generation/VillageFeature.hpp"

#include <cwchar>
#include <vector>

#include "game/core/BoundingBox.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/world/generation/VillagePieces.hpp"

static std::vector<Biome*> gVillageBiomes;

VillageFeature::VillageFeature(int size)
    : StructureFeature()
{
    _init(size);
}

VillageFeature::VillageFeature(SuperflatConfig* config, int size)
    : StructureFeature()
{
    FlatGeneratorInfo generatorInfo(config, false);
    _init(size);
    mVillageSize = generatorInfo.getVillageSize();
    mSpacing = generatorInfo.getVillageDistance();
}

VillageFeature::~VillageFeature()
{
}

void VillageFeature::staticCtor()
{
    gVillageBiomes.push_back(Biome::PLAINS);
    gVillageBiomes.push_back(Biome::DESERT);
    gVillageBiomes.push_back(Biome::SAVANNA);
    gVillageBiomes.push_back(Biome::TAIGA);
    gVillageBiomes.push_back(Biome::COLD_TAIGA);
    gVillageBiomes.push_back(Biome::ICE_PLAINS);
}

void VillageFeature::_init(int size)
{
    mVillageSize = 0;
    mSpacing = 0x20;
    mSeparation = 8;
    mSize = size << 4;
}

std::wstring VillageFeature::getFeatureName()
{
    return std::wstring(L"Village");
}

bool VillageFeature::isFeatureChunk(int chunkX, int chunkZ, bool flatWorld)
{
    int spacing = flatWorld ? 0x10 : mSpacing;
    int regionX = chunkX < 0 ? chunkX + 1 - spacing : chunkX;
    int regionZ = chunkZ < 0 ? chunkZ + 1 - spacing : chunkZ;
    Random chunkRandom = level->getRandomFor(regionX / spacing, regionZ / spacing, 0x9E7F70);
    int offsetX = chunkRandom.nextInt(spacing - mSeparation);
    int offsetZ = chunkRandom.nextInt(spacing - mSeparation);
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if ((options != NULL && options->isFeatureChunk(chunkX, chunkZ, eFeatureType_Village, NULL)) ||
        (chunkX == regionX / spacing * spacing + offsetX &&
         chunkZ == regionZ / spacing * spacing + offsetZ))
    {
        return level->getBiomeSource()->containsOnly(chunkX * 16 + 8, chunkZ * 16 + 8, 0,
                                                     gVillageBiomes);
    }
    return false;
}

BlockPos* VillageFeature::getNearestGeneratedFeature(Level* levelIn, const BlockPos& pos,
                                                       bool skipExistingChunks)
{
    level = levelIn;
    return StructureFeature::findNearestFeaturePositionBySpacing(
        levelIn, this, pos, mSpacing, mSeparation, 0x9E7F70, false, 100,
        skipExistingChunks);
}

StructureStart* VillageFeature::createStructureStart(int chunkX, int chunkZ)
{
    g_pApp->AddTerrainFeaturePosition((_eTerrainFeatureType)3, chunkX, chunkZ);
    return new VillageStart(level, random, chunkX, chunkZ, mVillageSize, mSize);
}

VillageFeature::VillageStart::VillageStart(Level* levelIn, Random& random, int chunkX, int chunkZ,
                                            int size, int distance)
    : StructureStart(chunkX, chunkZ), mValid(false), mDistance(distance)
{
    std::list<VillagePieces::PieceWeight*>* pieceSet =
        VillagePieces::createPieceSet(random, size);
    VillagePieces::StartPiece* startPiece = new VillagePieces::StartPiece(
        levelIn->getBiomeSource(), 0, random, chunkX * 16 + 2, chunkZ * 16 + 2,
        pieceSet, size, levelIn);
    push_back(startPiece);
    startPiece->addChildren(startPiece, this, random);

    while ((!startPiece->mPendingPieces.empty()) || (!startPiece->mRoadPieces.empty()))
    {
        if (!startPiece->mPendingPieces.empty())
        {
            std::vector<StructurePiece*>::iterator it = startPiece->mPendingPieces.begin();
            std::advance(it, random.nextInt((int)startPiece->mPendingPieces.size()));
            StructurePiece* piece = *it;
            startPiece->mPendingPieces.erase(it);
            piece->addChildren(piece, this, random);
        }
        else
        {
            std::vector<StructurePiece*>::iterator it = startPiece->mRoadPieces.begin();
            std::advance(it, random.nextInt((int)startPiece->mRoadPieces.size()));
            StructurePiece* piece = *it;
            startPiece->mRoadPieces.erase(it);
            piece->addChildren(piece, this, random);
        }
    }

    calculateBoundingBox();
    int villagePieceCount = 0;
    for (std::vector<StructurePiece*>::iterator it = begin(); it != end(); ++it)
    {
        if (*it == NULL || dynamic_cast<VillagePieces::VillagePiece*>(*it) == NULL)
        {
            ++villagePieceCount;
        }
    }
    mValid = villagePieceCount > 2;
}

VillageFeature::VillageStart::VillageStart()
    : StructureStart(), mValid(false), mDistance(0)
{
}

VillageFeature::VillageStart::~VillageStart()
{
}

StructureStart* VillageFeature::VillageStart::Create()
{
    return new VillageStart();
}

bool VillageFeature::VillageStart::isValid()
{
    BoundingBox* box = getBoundingBox();
    int minX = (-mDistance) / 2;
    int maxX = mDistance / 2;
    if (minX <= box->x0 && box->x1 <= maxX && minX <= box->z0 && box->z1 <= maxX)
    {
        return mValid;
    }
    mValid = false;
    return false;
}

void VillageFeature::VillageStart::addAdditionalSaveData(DataOutputStream* output)
{
    StructureStart::addAdditionalSaveData(output);
    output->writeBoolean(mValid);
}

void VillageFeature::VillageStart::readAdditionalSaveData(DataInputStream* input)
{
    StructureStart::readAdditionalSaveData(input);
    mValid = input->readBoolean();
}

EStructureStart VillageFeature::VillageStart::GetType()
{
    return eStructureStart_Village;
}
