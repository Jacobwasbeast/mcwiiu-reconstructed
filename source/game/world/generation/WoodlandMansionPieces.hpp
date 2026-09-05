#ifndef GAME_WORLD_GENERATION_WOODLANDMANSIONPIECES_HPP
#define GAME_WORLD_GENERATION_WOODLANDMANSIONPIECES_HPP

#include <string>
#include <vector>

#include "game/world/block/BlockPos.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/TemplateStructurePiece.hpp"

class BoundingBox;
class DataInputStream;
class DataOutputStream;
class Level;

class WoodlandMansionPieces
{
public:
    class SimpleGrid
    {
    public:
        SimpleGrid(int width, int height, int value);
        ~SimpleGrid();
        void set(int x, int y, int value);
        void set(int x0, int y0, int x1, int y1, int value);
        int get(int x, int y);
        void setIf(int x, int y, int oldValue, int newValue);
        bool edgesTo(int x, int y, int value);
        int mWidth;
        int mHeight;
        int mDefaultValue;
        int** mValues;
        int* mRows;
    };

    class FloorRoomCollection
    {
    public:
        virtual ~FloorRoomCollection();
        virtual int get1x1(Random& random) = 0;
        virtual int get1x1Secret(Random& random) = 0;
        virtual int get1x2SideEntrance(Random& random, bool staircase) = 0;
        virtual int get1x2FrontEntrance(Random& random, bool staircase) = 0;
        virtual int get1x2Secret(Random& random) = 0;
        virtual int get2x2(Random& random) = 0;
        virtual int get2x2Secret(Random& random) = 0;
    };

    class FirstFloorRoomCollection : public FloorRoomCollection
    {
    public:
        virtual ~FirstFloorRoomCollection();
        virtual int get1x1(Random& random);
        virtual int get1x1Secret(Random& random);
        virtual int get1x2SideEntrance(Random& random, bool staircase);
        virtual int get1x2FrontEntrance(Random& random, bool staircase);
        virtual int get1x2Secret(Random& random);
        virtual int get2x2(Random& random);
        virtual int get2x2Secret(Random& random);
    };

    class SecondFloorRoomCollection : public FloorRoomCollection
    {
    public:
        virtual ~SecondFloorRoomCollection();
        virtual int get1x1(Random& random);
        virtual int get1x1Secret(Random& random);
        virtual int get1x2SideEntrance(Random& random, bool staircase);
        virtual int get1x2FrontEntrance(Random& random, bool staircase);
        virtual int get1x2Secret(Random& random);
        virtual int get2x2(Random& random);
        virtual int get2x2Secret(Random& random);
    };

    class ThirdFloorRoomCollection : public SecondFloorRoomCollection
    {
    public:
        virtual ~ThirdFloorRoomCollection();
    };

    class MansionGrid : public Random
    {
    public:
        explicit MansionGrid(Random& random);
        ~MansionGrid();
        void recursiveCorridor(SimpleGrid* grid, int x, int y, const Direction* direction, int length);
        static bool isHouse(SimpleGrid* grid, int x, int y);
        void cleanEdges(SimpleGrid* grid);
        void identifyRooms(SimpleGrid* floor, SimpleGrid* rooms);
        bool isRoomId(SimpleGrid* grid, int x, int y, int roomId, int value);
        int get1x2RoomDirection(SimpleGrid* grid, int x, int y, int roomId, int value);
        void setupThirdFloor();
        void print();
        SimpleGrid* mBaseGrid;
        SimpleGrid* mSecondFloorGrid;
        SimpleGrid* mThirdFloorGrid;
        SimpleGrid* mThirdFloorRooms;
        int mStartX;
        int mStartY;
        int mWidth;
        int mHeight;
    };

    struct PlacementData
    {
        Rotation* rotation;
        BlockPos position;
        eMinecraftStructureTemplate templateType;
    };

    class WoodlandMansionPiece : public TemplateStructurePiece
    {
    public:
        WoodlandMansionPiece(StructureManager* manager, eMinecraftStructureTemplate type,
            const BlockPos& position, Rotation* rotation);
        WoodlandMansionPiece(StructureManager* manager, eMinecraftStructureTemplate type,
            const BlockPos& position, Rotation* rotation, Mirror* mirror);
        WoodlandMansionPiece();
        virtual ~WoodlandMansionPiece();
        void loadTemplate(StructureManager* manager);
        void _init(StructureManager* manager, eMinecraftStructureTemplate type,
            const BlockPos& position, Rotation* rotation, Mirror* mirror);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        virtual void handleDataMarker(const std::wstring& marker, const BlockPos& position,
            Level* level, Random& random, BoundingBox* box);
        static TemplateStructurePiece* create();
        virtual EStructurePiece GetType();
        eMinecraftStructureTemplate mTemplateType;
        Rotation* mRotation;
        Mirror* mMirror;
    };

    class MansionPiecePlacer
    {
    public:
        MansionPiecePlacer(StructureManager* manager, Random& random);
        void entrance(std::vector<WoodlandMansionPiece*>& pieces, PlacementData& data);
        void traverseTurn(std::vector<WoodlandMansionPiece*>& pieces, PlacementData& data);
        void traverseWallPiece(std::vector<WoodlandMansionPiece*>& pieces, PlacementData& data);
        void traverseInnerTurn(std::vector<WoodlandMansionPiece*>& pieces, PlacementData& data);
        void traverseOuterWalls(std::vector<WoodlandMansionPiece*>& pieces, PlacementData& data,
            SimpleGrid* grid, const Direction* direction, int x, int y, int depth, int roomId);
        void createRoof(std::vector<WoodlandMansionPiece*>& pieces, const BlockPos& position,
            Rotation* rotation, SimpleGrid* grid, SimpleGrid* upperGrid);
        void addRoom1x1(std::vector<WoodlandMansionPiece*>& pieces, const BlockPos& position,
            Rotation* rotation, const Direction* direction, FloorRoomCollection* rooms);
        void addRoom1x2(std::vector<WoodlandMansionPiece*>& pieces, const BlockPos& position,
            Rotation* rotation, const Direction* first, const Direction* second,
            FloorRoomCollection* rooms, bool secret);
        void addRoom2x2(std::vector<WoodlandMansionPiece*>& pieces, BlockPos position,
            Rotation* rotation, const Direction* first, const Direction* second,
            FloorRoomCollection* rooms);
        void addRoom2x2Secret(std::vector<WoodlandMansionPiece*>& pieces, const BlockPos& position,
            Rotation* rotation, FloorRoomCollection* rooms);
        void createMansion(const BlockPos& position, Rotation* rotation,
            std::vector<WoodlandMansionPiece*>& pieces, MansionGrid* grid);
        StructureManager* mStructureManager;
        Random mRandom;
    };

    static void generateMansion(StructureManager* manager, const BlockPos& position,
        Rotation* rotation, std::vector<WoodlandMansionPiece*>& pieces, Random& random);
    static void loadStatic();
    static void main(std::basic_string<wchar_t> args);
};

#endif
