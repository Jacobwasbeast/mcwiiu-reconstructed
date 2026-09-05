#ifndef GAME_CORE_OCEANRUIN_HPP
#define GAME_CORE_OCEANRUIN_HPP

#include "game/world/generation/ScatteredFeaturePieces.hpp"
#include "game/world/generation/StructureManager.hpp"

class BlockState;
class BlockPos;
class BoundingBox;
class DataInputStream;
class DataOutputStream;
class Level;
class Mirror;
class Random;
class Rotation;
class StructureManager;
class StructurePlaceSettings;
class StructureTemplate;

class OceanRuin : public ScatteredFeaturePieces::ScatteredFeaturePiece
{
public:
    enum ERuinType
    {
        eRuinType_Cold = 0,
        eRuinType_Warm = 1,
        eRuinType_ColdBrick = 2
    };

    OceanRuin();
    OceanRuin(Level* level, Random& random, int x, int z, int width, int height, int depth,
        bool isLarge, bool isCold);
    virtual ~OceanRuin();

    unsigned int getBigWarmRuin(Random& random);
    unsigned int getSmallWarmRuin(Random& random);
    unsigned int getRuin(int index, bool large, ERuinType type);
    void updateChestReplacer(eMinecraftStructureTemplate structureTemplate);
    const StructureTemplate* placeRuin(Random& random, StructureManager* structureManager, Level* level,
        const BlockPos& pos, StructurePlaceSettings& settings);
    void adaptPosition(const BlockPos& pos, Mirror* mirror, Rotation* rotation, Level* level);
    void fillBelow(const BlockPos& pos, Mirror* mirror, Rotation* rotation, Level* level);
    virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
    virtual void addAdditionalSaveData(DataOutputStream* output);
    virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);
    virtual EStructurePiece GetType();

    static StructurePiece* Create();

private:
    const BlockState* mBlockState;
    bool mIsLarge;
    bool mIsCold;
};

#endif
