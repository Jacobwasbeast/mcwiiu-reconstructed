#ifndef GAME_CORE_OCEANMONUMENTPIECES_HPP
#define GAME_CORE_OCEANMONUMENTPIECES_HPP
#include <cstdint>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "game/world/generation/StructurePiece.hpp"
class BlockState; class BoundingBox; class DataInputStream; class DataOutputStream; class Direction; class Level; class Random; class StructureManager;
class OceanMonumentPieces
{
public:
    static const BlockState* prismarineState;
    static const BlockState* seaLanternState;
    static const BlockState* goldState;
    static void loadStatic();
    OceanMonumentPieces();
    class RoomDefinition;
    class OceanMonumentPiece;
    class MonumentRoomFitter;
    class FitSimpleRoom; class FitSimpleTopRoom; class FitDoubleXRoom; class FitDoubleYRoom; class FitDoubleXYRoom; class FitDoubleYZRoom; class FitDoubleZRoom;
    class RoomDefinition
    {
    public:
        explicit RoomDefinition(int index);
        void setConnection(const Direction* direction, const boost::shared_ptr<RoomDefinition>& connection);
        boost::shared_ptr<RoomDefinition> fjConnection(int index) const;
        boost::shared_ptr<RoomDefinition> fjConnection(const Direction* direction) const;
        void updateOpenings(); bool findSource(int index); bool isSpecial(); unsigned char countOpenings();
        int mIndex; boost::shared_ptr<RoomDefinition> mConnections[6]; bool mOpenings[6]; bool mClaimed; int mSource;
    };
    class OceanMonumentPiece : public StructurePiece
    {
    public:
        explicit OceanMonumentPiece(int genDepth);
        OceanMonumentPiece(int genDepth, const Direction* orientation, const boost::shared_ptr<RoomDefinition>& definition, int x, int y, int z);
        OceanMonumentPiece(const Direction* orientation, BoundingBox* box);
        virtual ~OceanMonumentPiece();
        static int getRoomIndex(int x, int y, int z); static int getRoomX(int index); static int getRoomY(int index); static int getRoomZ(int index);
        static void staticCtor(); void fjKeepReference(const boost::shared_ptr<RoomDefinition>& definition);
        virtual void addAdditionalSaveData(DataOutputStream* output); virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        void generateWaterBox(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1, int z1, bool existingOnly);
        void generateDefaultFloor(Level* level, BoundingBox* box, int x, int z, bool existingOnly);
        void generateBoxOnFillOnly(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1, int z1, const BlockState* state);
        bool chunkIntersects(BoundingBox* box, int x0, int z0, int x1, int z1); bool spawnElder(Level* level, BoundingBox* box, int x, int y, int z);
        int mRoomIndex; boost::shared_ptr<RoomDefinition> mRoomDefinition; std::vector<boost::shared_ptr<RoomDefinition> > mChildPieces;
    };
    class MonumentRoomFitter
    {
    public:
        virtual ~MonumentRoomFitter();
        virtual bool fits(const boost::shared_ptr<RoomDefinition>& definition) = 0;
        virtual boost::shared_ptr<OceanMonumentPiece> create(const Direction* orientation, const boost::shared_ptr<RoomDefinition>& definition, Random& random) = 0;
    };
    class FitSimpleRoom : public MonumentRoomFitter { public: virtual ~FitSimpleRoom();
virtual bool fits(const boost::shared_ptr<RoomDefinition>&);
virtual boost::shared_ptr<OceanMonumentPiece> create(const Direction*, const boost::shared_ptr<RoomDefinition>&, Random&);
};
    class FitSimpleTopRoom : public MonumentRoomFitter { public: virtual ~FitSimpleTopRoom();
virtual bool fits(const boost::shared_ptr<RoomDefinition>&);
virtual boost::shared_ptr<OceanMonumentPiece> create(const Direction*, const boost::shared_ptr<RoomDefinition>&, Random&);
};
    class FitDoubleXRoom : public MonumentRoomFitter { public: virtual ~FitDoubleXRoom();
virtual bool fits(const boost::shared_ptr<RoomDefinition>&);
virtual boost::shared_ptr<OceanMonumentPiece> create(const Direction*, const boost::shared_ptr<RoomDefinition>&, Random&);
};
    class FitDoubleYRoom : public MonumentRoomFitter { public: virtual ~FitDoubleYRoom();
virtual bool fits(const boost::shared_ptr<RoomDefinition>&);
virtual boost::shared_ptr<OceanMonumentPiece> create(const Direction*, const boost::shared_ptr<RoomDefinition>&, Random&);
};
    class FitDoubleXYRoom : public MonumentRoomFitter { public: virtual ~FitDoubleXYRoom();
virtual bool fits(const boost::shared_ptr<RoomDefinition>&);
virtual boost::shared_ptr<OceanMonumentPiece> create(const Direction*, const boost::shared_ptr<RoomDefinition>&, Random&);
};
    class FitDoubleYZRoom : public MonumentRoomFitter { public: virtual ~FitDoubleYZRoom();
virtual bool fits(const boost::shared_ptr<RoomDefinition>&);
virtual boost::shared_ptr<OceanMonumentPiece> create(const Direction*, const boost::shared_ptr<RoomDefinition>&, Random&);
};
    class FitDoubleZRoom : public MonumentRoomFitter { public: virtual ~FitDoubleZRoom();
virtual bool fits(const boost::shared_ptr<RoomDefinition>&);
virtual boost::shared_ptr<OceanMonumentPiece> create(const Direction*, const boost::shared_ptr<RoomDefinition>&, Random&);
};
    class OceanMonumentCoreRoom : public OceanMonumentPiece { public: OceanMonumentCoreRoom();
OceanMonumentCoreRoom(const Direction*, const boost::shared_ptr<RoomDefinition>&, Random&);
virtual ~OceanMonumentCoreRoom();
virtual bool postProcess(Level*, Random&, BoundingBox*);
static void Create();
virtual EStructurePiece GetType();
};
    class OceanMonumentDoubleXRoom : public OceanMonumentPiece { public: OceanMonumentDoubleXRoom();
OceanMonumentDoubleXRoom(const Direction*, const boost::shared_ptr<RoomDefinition>&, Random&);
virtual ~OceanMonumentDoubleXRoom();
virtual bool postProcess(Level*, Random&, BoundingBox*);
static void Create();
virtual EStructurePiece GetType();
};
    class OceanMonumentDoubleXYRoom : public OceanMonumentPiece { public: OceanMonumentDoubleXYRoom();
OceanMonumentDoubleXYRoom(const Direction*, const boost::shared_ptr<RoomDefinition>&, Random&);
virtual ~OceanMonumentDoubleXYRoom();
virtual bool postProcess(Level*, Random&, BoundingBox*);
static void Create();
virtual EStructurePiece GetType();
};
    class OceanMonumentDoubleYRoom : public OceanMonumentPiece { public: OceanMonumentDoubleYRoom();
OceanMonumentDoubleYRoom(const Direction*, const boost::shared_ptr<RoomDefinition>&, Random&);
virtual ~OceanMonumentDoubleYRoom();
virtual bool postProcess(Level*, Random&, BoundingBox*);
static void Create();
virtual EStructurePiece GetType();
};
    class OceanMonumentDoubleYZRoom : public OceanMonumentPiece { public: OceanMonumentDoubleYZRoom();
OceanMonumentDoubleYZRoom(const Direction*, const boost::shared_ptr<RoomDefinition>&, Random&);
virtual ~OceanMonumentDoubleYZRoom();
virtual bool postProcess(Level*, Random&, BoundingBox*);
static void Create();
virtual EStructurePiece GetType();
};
    class OceanMonumentDoubleZRoom : public OceanMonumentPiece { public: OceanMonumentDoubleZRoom();
OceanMonumentDoubleZRoom(const Direction*, const boost::shared_ptr<RoomDefinition>&, Random&);
virtual ~OceanMonumentDoubleZRoom();
virtual bool postProcess(Level*, Random&, BoundingBox*);
static void Create();
virtual EStructurePiece GetType();
};
    class OceanMonumentSimpleRoom : public OceanMonumentPiece { public: OceanMonumentSimpleRoom();
OceanMonumentSimpleRoom(const Direction*, const boost::shared_ptr<RoomDefinition>&, Random&);
virtual ~OceanMonumentSimpleRoom();
virtual bool postProcess(Level*, Random&, BoundingBox*);
static void Create();
virtual EStructurePiece GetType();
};
    class OceanMonumentSimpleTopRoom : public OceanMonumentPiece { public: OceanMonumentSimpleTopRoom();
OceanMonumentSimpleTopRoom(const Direction*, const boost::shared_ptr<RoomDefinition>&, Random&);
virtual ~OceanMonumentSimpleTopRoom();
virtual bool postProcess(Level*, Random&, BoundingBox*);
static void Create();
virtual EStructurePiece GetType();
};
    class OceanMonumentEntryRoom : public OceanMonumentPiece { public: OceanMonumentEntryRoom();
OceanMonumentEntryRoom(const Direction*, const boost::shared_ptr<RoomDefinition>&);
virtual ~OceanMonumentEntryRoom();
virtual bool postProcess(Level*, Random&, BoundingBox*);
static void Create();
virtual EStructurePiece GetType();
};
    class OceanMonumentPenthouse : public OceanMonumentPiece { public: OceanMonumentPenthouse();
OceanMonumentPenthouse(const Direction*, BoundingBox*);
virtual ~OceanMonumentPenthouse();
virtual bool postProcess(Level*, Random&, BoundingBox*);
static void Create();
virtual EStructurePiece GetType();
};
    class OceanMonumentWingRoom : public OceanMonumentPiece { public: OceanMonumentWingRoom();
OceanMonumentWingRoom(const Direction*, BoundingBox*, int);
virtual ~OceanMonumentWingRoom();
virtual bool postProcess(Level*, Random&, BoundingBox*);
virtual EStructurePiece GetType();
};
    class MonumentBuilding : public OceanMonumentPiece
    {
    public:
        MonumentBuilding();
MonumentBuilding(Random&, int, int, const Direction*);
virtual ~MonumentBuilding();
int generateRoomGraph(Random&);
void generateWing(bool, int, Level*, Random&, BoundingBox*);
void generateEntranceArchs(Level*, Random&, BoundingBox*);
void generateEntranceWall(Level*, Random&, BoundingBox*);
void generateRoofPiece(Level*, Random&, BoundingBox*);
void generateLowerWall(Level*, Random&, BoundingBox*);
void generateMiddleWall(Level*, Random&, BoundingBox*);
void generateUpperWall(Level*, Random&, BoundingBox*);
virtual bool postProcess(Level*, Random&, BoundingBox*);
static void Create();
virtual EStructurePiece GetType();
        int mRoomGraphSize; boost::shared_ptr<RoomDefinition> mSourceRoom; int mRoomCount; boost::shared_ptr<RoomDefinition> mRootRoom; std::vector<OceanMonumentPiece*> mPieces;
    };
};
#endif
