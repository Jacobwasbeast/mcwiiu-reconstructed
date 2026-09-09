#ifndef GAME_WORLD_GENERATION_SCATTEREDFEATUREPIECES_HPP
#define GAME_WORLD_GENERATION_SCATTEREDFEATUREPIECES_HPP

#include <vector>

#include "game/world/generation/StructurePiece.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"

class BlockState;
class DataInputStream;
class DataOutputStream;
class Level;
class Random;
class StructureManager;

class ScatteredFeaturePieces
{
public:
    static void loadStatic();

    class ScatteredFeaturePiece : public StructurePiece
    {
    public:
        ScatteredFeaturePiece();
        ScatteredFeaturePiece(Level* level, Random& random, int x, int y, int z, int width, int height, int depth);
        virtual ~ScatteredFeaturePiece();
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);
        bool updateAverageGroundHeight(Level* level, BoundingBox* box, int offset, bool useHeightmap);

    protected:
        StructurePlaceSettings mPlaceSettings;
        int mWidth;
        int mHeight;
        int mDepth;
        int mGroundHeight;
        std::vector<bool> mFirstBools;
        std::vector<bool> mSecondBools;
        std::vector<int> mFirstInts;
        std::vector<int> mSecondInts;
    };

    class DesertPyramidPiece : public ScatteredFeaturePiece
    {
    public:
        DesertPyramidPiece();
        DesertPyramidPiece(Level* level, Random& random, int x, int z);
        virtual ~DesertPyramidPiece();
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    private:
        bool mTrap[4];
    };

    class Igloo : public ScatteredFeaturePiece
    {
    public:
        Igloo();
        Igloo(Level* level, Random& random, int x, int z);
        virtual ~Igloo();
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    private:
        bool mHasBasement;
        int mHeightOffset;
    };

    class JunglePyramidPiece : public ScatteredFeaturePiece
    {
    public:
        class MossStoneSelector : public StructurePiece::BlockSelector
        {
        public:
            virtual ~MossStoneSelector();
            virtual void next(Random& random, int x, int y, int z, bool edge);
        };

        static MossStoneSelector* mStoneSelector;

        JunglePyramidPiece();
        JunglePyramidPiece(Level* level, Random& random, int x, int z);
        virtual ~JunglePyramidPiece();
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static void staticCtor();
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    private:
        bool mTrap[4];
    };

    class SwamplandHut : public ScatteredFeaturePiece
    {
    public:
        SwamplandHut();
        SwamplandHut(Level* level, Random& random, int x, int z);
        virtual ~SwamplandHut();
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    private:
        bool mHasWitch;
    };
};

#endif
