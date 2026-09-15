#ifndef GAME_CORE_NETHERBRIDGEPIECES_HPP
#define GAME_CORE_NETHERBRIDGEPIECES_HPP

#include <list>
#include <vector>

#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructurePiece.hpp"

class NetherBridgePieces
{
public:
    enum EPieceClass
    {
        ePieceClass_BridgeStraight = 0,
        ePieceClass_BridgeCrossing = 2,
        ePieceClass_RoomCrossing = 3,
        ePieceClass_StairsRoom = 4,
        ePieceClass_MonsterThrone = 5,
        ePieceClass_CastleEntrance = 6,
        ePieceClass_CastleStalkRoom = 7,
        ePieceClass_CastleSmallCorridorPiece = 8,
        ePieceClass_CastleSmallCorridorCrossingPiece = 9,
        ePieceClass_CastleSmallCorridorRightTurnPiece = 10,
        ePieceClass_CastleSmallCorridorLeftTurnPiece = 11,
        ePieceClass_CastleCorridorStairsPiece = 12,
        ePieceClass_CastleCorridorTBalconyPiece = 13
    };

    class PieceWeight;
    class StartPiece;
    class NetherBridgePiece;
    class BridgeCrossing;
    class BridgeEndFiller;
    class BridgeStraight;
    class CastleCorridorStairsPiece;
    class CastleCorridorTBalconyPiece;
    class CastleEntrance;
    class CastleSmallCorridorCrossingPiece;
    class CastleSmallCorridorLeftTurnPiece;
    class CastleSmallCorridorPiece;
    class CastleSmallCorridorRightTurnPiece;
    class CastleStalkRoom;
    class MonsterThrone;
    class RoomCrossing;
    class StairsRoom;

    static void loadStatic();
    static StructurePiece* findAndCreateBridgePieceFactory(PieceWeight*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);

    class NetherBridgePiece : public StructurePiece
    {
    public:
        NetherBridgePiece(int genDepth);
        NetherBridgePiece();
        virtual ~NetherBridgePiece();
        static bool isOkBox(BoundingBox*, StartPiece*);
        virtual void readAdditionalSaveData(DataInputStream*, StructureManager*);
        virtual void addAdditionalSaveData(DataOutputStream*);
        int updatePieceWeight(std::list<PieceWeight*>*);
        StructurePiece* generatePiece(StartPiece*, std::list<PieceWeight*>*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        StructurePiece* generateAndAddPiece(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int, bool);
        StructurePiece* generateChildForward(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, bool);
        StructurePiece* generateChildLeft(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, bool);
        StructurePiece* generateChildRight(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, bool);
        void generateLightPost(Level*, Random&, BoundingBox*, int, int, int, int, int);
        void generateLightPostFacingRight(Level*, Random&, BoundingBox*, int, int, int);
        void generateLightPostFacingLeft(Level*, Random&, BoundingBox*, int, int, int);
        void generateLightPostFacingUp(Level*, Random&, BoundingBox*, int, int, int);
        void generateLightPostFacingDown(Level*, Random&, BoundingBox*, int, int, int);
    };

    class PieceWeight
    {
    public:
        PieceWeight(EPieceClass, int, int, bool);
        PieceWeight(EPieceClass, int, int);
        bool doPlace(int);
        bool isValid();
        EPieceClass mPieceClass;
        int mWeight;
        int mMaxPlaceCount;
        int mPlaceCount;
        bool mIsValid;
    };

    class BridgeStraight : public NetherBridgePiece
    {
    public:
        BridgeStraight();
        virtual ~BridgeStraight();
        BridgeStraight(int, Random&, BoundingBox*, const Direction*);
        static StructurePiece* createPiece(std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class CastleCorridorStairsPiece : public NetherBridgePiece
    {
    public:
        CastleCorridorStairsPiece();
        virtual ~CastleCorridorStairsPiece();
        CastleCorridorStairsPiece(int, Random&, BoundingBox*, const Direction*);
        static StructurePiece* createPiece(std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class CastleCorridorTBalconyPiece : public NetherBridgePiece
    {
    public:
        CastleCorridorTBalconyPiece();
        virtual ~CastleCorridorTBalconyPiece();
        CastleCorridorTBalconyPiece(int, Random&, BoundingBox*, const Direction*);
        static StructurePiece* createPiece(std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class CastleEntrance : public NetherBridgePiece
    {
    public:
        CastleEntrance();
        virtual ~CastleEntrance();
        CastleEntrance(int, Random&, BoundingBox*, const Direction*);
        static StructurePiece* createPiece(std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class CastleSmallCorridorCrossingPiece : public NetherBridgePiece
    {
    public:
        CastleSmallCorridorCrossingPiece();
        virtual ~CastleSmallCorridorCrossingPiece();
        CastleSmallCorridorCrossingPiece(int, Random&, BoundingBox*, const Direction*);
        static StructurePiece* createPiece(std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class CastleSmallCorridorPiece : public NetherBridgePiece
    {
    public:
        CastleSmallCorridorPiece();
        virtual ~CastleSmallCorridorPiece();
        CastleSmallCorridorPiece(int, Random&, BoundingBox*, const Direction*);
        static StructurePiece* createPiece(std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class CastleStalkRoom : public NetherBridgePiece
    {
    public:
        CastleStalkRoom();
        virtual ~CastleStalkRoom();
        CastleStalkRoom(int, Random&, BoundingBox*, const Direction*);
        static StructurePiece* createPiece(std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class RoomCrossing : public NetherBridgePiece
    {
    public:
        RoomCrossing();
        virtual ~RoomCrossing();
        RoomCrossing(int, Random&, BoundingBox*, const Direction*);
        static StructurePiece* createPiece(std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class StairsRoom : public NetherBridgePiece
    {
    public:
        StairsRoom();
        virtual ~StairsRoom();
        StairsRoom(int, Random&, BoundingBox*, const Direction*);
        static StructurePiece* createPiece(std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class BridgeCrossing : public NetherBridgePiece
    {
    public:
        BridgeCrossing();
        BridgeCrossing(Random&, int, int);
        BridgeCrossing(int, Random&, BoundingBox*, const Direction*);
        virtual ~BridgeCrossing();
        static StructurePiece* createPiece(std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };
    class BridgeEndFiller : public NetherBridgePiece
    {
    public:
        BridgeEndFiller();
        BridgeEndFiller(int, Random&, BoundingBox*, const Direction*);
        virtual ~BridgeEndFiller();
        static StructurePiece* createPiece(std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void readAdditionalSaveData(DataInputStream*, StructureManager*);
        virtual void addAdditionalSaveData(DataOutputStream*);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        int mUnknown1c;
    };

    class CastleSmallCorridorLeftTurnPiece : public NetherBridgePiece
    {
    public:
        CastleSmallCorridorLeftTurnPiece();
        CastleSmallCorridorLeftTurnPiece(int, Random&, BoundingBox*, const Direction*);
        virtual ~CastleSmallCorridorLeftTurnPiece();
        static StructurePiece* createPiece(std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void readAdditionalSaveData(DataInputStream*, StructureManager*);
        virtual void addAdditionalSaveData(DataOutputStream*);
        virtual void addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        unsigned char mState[4];
    };

    class CastleSmallCorridorRightTurnPiece : public NetherBridgePiece
    {
    public:
        CastleSmallCorridorRightTurnPiece();
        CastleSmallCorridorRightTurnPiece(int, Random&, BoundingBox*, const Direction*);
        virtual ~CastleSmallCorridorRightTurnPiece();
        static StructurePiece* createPiece(std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void readAdditionalSaveData(DataInputStream*, StructureManager*);
        virtual void addAdditionalSaveData(DataOutputStream*);
        virtual void addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        unsigned char mState[4];
    };

    class MonsterThrone : public NetherBridgePiece
    {
    public:
        MonsterThrone();
        MonsterThrone(int, Random&, BoundingBox*, const Direction*);
        virtual ~MonsterThrone();
        static StructurePiece* createPiece(std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int);
        virtual void readAdditionalSaveData(DataInputStream*, StructureManager*);
        virtual void addAdditionalSaveData(DataOutputStream*);
        virtual bool postProcess(Level*, Random&, BoundingBox*);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        bool mHasSpawner;
    };

    class StartPiece : public BridgeCrossing
    {
    public:
        StartPiece(Random&, int, int, Level*);
        StartPiece();
        virtual ~StartPiece();
        virtual EStructurePiece GetType();
        PieceWeight* mPreviousPieceWeight;
        Level* mLevel;
        std::list<PieceWeight*> mAvailablePieces;
        std::list<PieceWeight*> mPendingPieces;
        std::vector<StructurePiece*> mPendingChildren;
    };
};

#endif
