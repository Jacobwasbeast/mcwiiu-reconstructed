#ifndef GAME_WORLD_GENERATION_VILLAGEPIECES_HPP
#define GAME_WORLD_GENERATION_VILLAGEPIECES_HPP
#include <list>
#include <vector>
#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructurePiece.hpp"
class VillagePieces
{
public:
    enum EPieceClass
    {
        ePieceClass_SimpleHouse,
        ePieceClass_SmallTemple,
        ePieceClass_BookHouse,
        ePieceClass_SmallHut,
        ePieceClass_PigHouse,
        ePieceClass_DoubleFarmland,
        ePieceClass_Farmland,
        ePieceClass_Smithy,
        ePieceClass_TwoRoomHouse,
        ePieceClass_StraightRoad,
        ePieceClass_LightPost,
        ePieceClass_Well
    };
    class PieceWeight;
    class VillagePiece;
    class VillageRoadPiece;
    class Well;
    class StartPiece;
    class StraightRoad;
    class SimpleHouse;
    class SmallTemple;
    class BookHouse;
    class SmallHut;
    class PigHouse;
    class TwoRoomHouse;
    class Smithy;
    class Farmland;
    class DoubleFarmland;
    class LightPost;
    class CobbleStoneSelector;
    static std::list<PieceWeight*>* createPieceSet(Random&, int);
    static void loadStatic();
    static int updatePieceWeight(std::list<PieceWeight*>*);
    static StructurePiece* findAndCreatePieceFactory(StartPiece*, PieceWeight*,
        std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
    static StructurePiece* generatePieceFromSmallDoor(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
    static StructurePiece* generateAndAddPiece(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
    static StructurePiece* generateAndAddRoadPiece(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
    class PieceWeight
    {
    public:
        PieceWeight(EPieceClass, int, int);
        PieceWeight();
        bool doPlace(int);
        bool isValid();
        EPieceClass pieceClass;
        int weight;
        int placeCount;
        int maxPlaceCount;
    };
    class VillagePiece : public StructurePiece
    {
    public:
        VillagePiece(StartPiece*, int);
        VillagePiece();
        virtual ~VillagePiece();
        void setStructureType(int);
        static bool isOkBox(BoundingBox*, StartPiece*);
        virtual void addAdditionalSaveData(DataOutputStream*);
        virtual void readAdditionalSaveData(DataInputStream*, StructureManager*);
        unsigned int generateHouseNorthernLeft(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int);
        unsigned int generateHouseNorthernRight(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int);
        int getAverageGroundHeight(Level*, BoundingBox*);
        void spawnVillagers(Level*, BoundingBox*, int, int, int, int);
        virtual int getVillagerProfession(int, int);
        virtual const BlockState* biomeBlock(const BlockState*);
        virtual DoorBlock* biomeDoor();
        virtual void createVillageDoor(Level*, BoundingBox*, Random&, int, int, int, const Direction*);
        virtual void placeTorch(Level*, const Direction*, int, int, int, BoundingBox*);
        using StructurePiece::placeBlock;
        void placeBlock(Level*, Random&, const BlockState*, StructurePiece::BlockSelector*, int, int, int, BoundingBox*);
    protected:
        int mType;
        int mWidth;
        int mHeight;
        bool mHasMadeChest;
        CobbleStoneSelector* mCobblestoneSelector;
        int mAverageGroundHeight;
        int mVillagerCount;
        std::vector<bool> mFirstBools;
        std::vector<bool> mSecondBools;
        std::vector<int> mFirstInts;
        std::vector<int> mSecondInts;
        StartPiece* mStartPiece;
    };
    class VillageRoadPiece : public VillagePiece
    {
    public:
        virtual ~VillageRoadPiece();
    };
    class Well : public VillagePiece
    {
    public:
        Well(StartPiece*, int, Random&, int, int);
        Well(StartPiece*, int, Random&, BoundingBox*, const Direction*);
        Well();
        virtual ~Well();
        virtual void addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };
    class StartPiece : public Well
    {
    public:
        StartPiece(BiomeSource*, int, Random&, int, int, std::list<PieceWeight*>*, int, Level*);
        StartPiece();
        virtual ~StartPiece();
        BiomeSource* getBiomeSource();
        virtual EStructurePiece GetType();
        BiomeSource* mBiomeSource;
        int mSize;
        bool mHasPlacedWell;
        PieceWeight* mLastPieceWeight;
        Level* mLevel;
        std::vector<StructurePiece*> mPendingPieces;
        std::vector<StructurePiece*> mRoadPieces;
        std::list<PieceWeight*>* mPieceWeights;
    };
    class StraightRoad : public VillagePiece
    {
    public:
        StraightRoad();
        StraightRoad(StartPiece*, int, Random&, BoundingBox*, const Direction*);
        virtual ~StraightRoad();
        static StructurePiece* createPiece(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void addAdditionalSaveData(DataOutputStream*);
        virtual void readAdditionalSaveData(DataInputStream*, StructureManager*);
        virtual void addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&);
        static BoundingBox* findPieceBox(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        virtual int getVillagerProfession(int, int);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };
    class SimpleHouse : public VillagePiece
    {
    public:
        SimpleHouse();
        SimpleHouse(StartPiece*, int, Random&, BoundingBox*, const Direction*);
        virtual ~SimpleHouse();
        static StructurePiece* createPiece(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void addAdditionalSaveData(DataOutputStream*);
        virtual void readAdditionalSaveData(DataInputStream*, StructureManager*);
        static BoundingBox* findPieceBox(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        virtual int getVillagerProfession(int, int);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        bool mHasMadeDoor;
    };
    class SmallTemple : public VillagePiece
    {
    public:
        SmallTemple();
        SmallTemple(StartPiece*, int, Random&, BoundingBox*, const Direction*);
        virtual ~SmallTemple();
        static StructurePiece* createPiece(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        static BoundingBox* findPieceBox(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        virtual int getVillagerProfession(int, int);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };
    class BookHouse : public VillagePiece
    {
    public:
        BookHouse();
        BookHouse(StartPiece*, int, Random&, BoundingBox*, const Direction*);
        virtual ~BookHouse();
        static StructurePiece* createPiece(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        static BoundingBox* findPieceBox(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        virtual int getVillagerProfession(int, int);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };
    class SmallHut : public VillagePiece
    {
    public:
        SmallHut();
        SmallHut(StartPiece*, int, Random&, BoundingBox*, const Direction*);
        virtual ~SmallHut();
        static StructurePiece* createPiece(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void addAdditionalSaveData(DataOutputStream*);
        virtual void readAdditionalSaveData(DataInputStream*, StructureManager*);
        static BoundingBox* findPieceBox(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        virtual int getVillagerProfession(int, int);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        int mVillagersSpawned;
        bool mHasMadeDoor;
    };
    class PigHouse : public VillagePiece
    {
    public:
        PigHouse();
        PigHouse(StartPiece*, int, Random&, BoundingBox*, const Direction*);
        virtual ~PigHouse();
        static StructurePiece* createPiece(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        static BoundingBox* findPieceBox(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        virtual int getVillagerProfession(int, int);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };
    class TwoRoomHouse : public VillagePiece
    {
    public:
        TwoRoomHouse();
        TwoRoomHouse(StartPiece*, int, Random&, BoundingBox*, const Direction*);
        virtual ~TwoRoomHouse();
        static StructurePiece* createPiece(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        static BoundingBox* findPieceBox(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        virtual int getVillagerProfession(int, int);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };
    class Smithy : public VillagePiece
    {
    public:
        Smithy();
        Smithy(StartPiece*, int, Random&, BoundingBox*, const Direction*);
        virtual ~Smithy();
        static StructurePiece* createPiece(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void addAdditionalSaveData(DataOutputStream*);
        virtual void readAdditionalSaveData(DataInputStream*, StructureManager*);
        static BoundingBox* findPieceBox(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*);
        static void staticCtor();
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        virtual int getVillagerProfession(int, int);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };
    class Farmland : public VillagePiece
    {
    public:
        Farmland();
        Farmland(StartPiece*, int, Random&, BoundingBox*, const Direction*);
        virtual ~Farmland();
        unsigned int selectCrops(Random&);
        static StructurePiece* createPiece(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int,
            const Direction*, int);
        int selectOldCrops(int);
        virtual void addAdditionalSaveData(DataOutputStream*);
        virtual void readAdditionalSaveData(DataInputStream*, StructureManager*);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        int mCropA;
        int mCropB;
    };
    class DoubleFarmland : public Farmland
    {
    public:
        DoubleFarmland();
        DoubleFarmland(StartPiece*, int, Random&, BoundingBox*, const Direction*);
        virtual ~DoubleFarmland();
        unsigned int selectCrops(Random&);
        static StructurePiece* createPiece(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int,
            const Direction*, int);
        int selectOldCrops(int);
        virtual void addAdditionalSaveData(DataOutputStream*);
        virtual void readAdditionalSaveData(DataInputStream*, StructureManager*);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        int mCropC;
        int mCropD;
    };
    class LightPost : public VillagePiece
    {
    public:
        LightPost();
        LightPost(StartPiece*, int, Random&, BoundingBox*, const Direction*);
        virtual ~LightPost();
        static BoundingBox* findPieceBox(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int,
            const Direction*);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };
    class CobbleStoneSelector : public StructurePiece::BlockSelector
    {
    public:
        CobbleStoneSelector(bool, int);
        virtual ~CobbleStoneSelector();
        virtual void next(Random&, int, int, int, bool);
        bool mMossy;
        int mType;
    };
};
#endif
