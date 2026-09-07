#include "game/core/NetherBridgePieces.hpp"

void NetherBridgePieces::loadStatic() { }

StructurePiece* NetherBridgePieces::findAndCreateBridgePieceFactory(PieceWeight* w, std::vector<StructurePiece*>* p,
    Random& r, int x, int y, int z, const Direction* d, int n)
{
    switch (w->mPieceClass)
    {
    case 0:
        return BridgeStraight::createPiece(p, r, x, y, z, d, n);
    case 2:
        return BridgeCrossing::createPiece(p, r, x, y, z, d, n);
    case 3:
        return RoomCrossing::createPiece(p, r, x, y, z, d, n);
    case 4:
        return StairsRoom::createPiece(p, r, x, y, z, d, n);
    case 5:
        return MonsterThrone::createPiece(p, r, x, y, z, d, n);
    case 6:
        return CastleEntrance::createPiece(p, r, x, y, z, d, n);
    case 7:
        return CastleStalkRoom::createPiece(p, r, x, y, z, d, n);
    case 8:
        return CastleSmallCorridorPiece::createPiece(p, r, x, y, z, d, n);
    case 9:
        return CastleSmallCorridorCrossingPiece::createPiece(p, r, x, y, z, d, n);
    case 10:
        return CastleSmallCorridorRightTurnPiece::createPiece(p, r, x, y, z, d, n);
    case 11:
        return CastleSmallCorridorLeftTurnPiece::createPiece(p, r, x, y, z, d, n);
    case 12:
        return CastleCorridorStairsPiece::createPiece(p, r, x, y, z, d, n);
    case 13:
        return CastleCorridorTBalconyPiece::createPiece(p, r, x, y, z, d, n);
    default:
        return NULL;
    }
}

NetherBridgePieces::NetherBridgePiece::NetherBridgePiece(int n):StructurePiece(n) {}
NetherBridgePieces::NetherBridgePiece::NetherBridgePiece():StructurePiece() {}
NetherBridgePieces::NetherBridgePiece::~NetherBridgePiece() {}
bool NetherBridgePieces::NetherBridgePiece::isOkBox(BoundingBox* b, StartPiece*) { return b != NULL; }
void NetherBridgePieces::NetherBridgePiece::readAdditionalSaveData(DataInputStream*, StructureManager*) {}
void NetherBridgePieces::NetherBridgePiece::addAdditionalSaveData(DataOutputStream*) {}
void NetherBridgePieces::NetherBridgePiece::updatePieceWeight(std::list<PieceWeight*>*) {}
StructurePiece* NetherBridgePieces::NetherBridgePiece::generatePiece(
    StartPiece*, std::list<PieceWeight*>*, std::vector<StructurePiece*>*, Random&,
    int, int, int, const Direction*, int)
{
    return NULL;
}
StructurePiece* NetherBridgePieces::NetherBridgePiece::generateAndAddPiece(StartPiece*,std::vector<StructurePiece*>*,Random&,int,int,int,const Direction*,int,bool) { return NULL; }
StructurePiece* NetherBridgePieces::NetherBridgePiece::generateChildForward(StartPiece*,std::vector<StructurePiece*>*,Random&,int,int,bool) { return NULL; }
StructurePiece* NetherBridgePieces::NetherBridgePiece::generateChildLeft(StartPiece*,std::vector<StructurePiece*>*,Random&,int,int,bool) { return NULL; }
StructurePiece* NetherBridgePieces::NetherBridgePiece::generateChildRight(StartPiece*,std::vector<StructurePiece*>*,Random&,int,int,bool) { return NULL; }
void NetherBridgePieces::NetherBridgePiece::generateLightPost(Level*,Random&,BoundingBox*,int,int,int,int,int) {}

NetherBridgePieces::PieceWeight::PieceWeight(EPieceClass c,int w,int m,bool v):mPieceClass(c),mWeight(w),mMaxPlaceCount(m),mPlaceCount(0),mIsValid(v) {}
NetherBridgePieces::PieceWeight::PieceWeight(EPieceClass c,int w,int m):mPieceClass(c),mWeight(w),mMaxPlaceCount(m),mPlaceCount(0),mIsValid(true) {}
bool NetherBridgePieces::PieceWeight::doPlace(int n) { if(mMaxPlaceCount && mPlaceCount>=mMaxPlaceCount)return false; ++mPlaceCount; return n>=0; }
bool NetherBridgePieces::PieceWeight::isValid() { return mIsValid && (!mMaxPlaceCount || mPlaceCount<mMaxPlaceCount); }

NetherBridgePieces::BridgeCrossing::BridgeCrossing():NetherBridgePiece() {}
NetherBridgePieces::BridgeCrossing::BridgeCrossing(Random&,int,int):NetherBridgePiece() {}
NetherBridgePieces::BridgeCrossing::BridgeCrossing(int d,Random&,BoundingBox* b,const Direction* o):NetherBridgePiece(d){setBoundingBox(b);setOrientation(o);}
StructurePiece* NetherBridgePieces::BridgeCrossing::createPiece(std::vector<StructurePiece*>* p, Random& r,
    int x, int y, int z, const Direction* o, int d)
{
    BoundingBox* b = BoundingBox::orientBox(x, y, z, 0, -1, 0, 5, 10, 19, o);
    if (b == NULL || StructurePiece::findCollisionPiece(p, b) != NULL)
    {
        delete b;
        return NULL;
    }
    return new BridgeCrossing(d, r, b, o);
}
void NetherBridgePieces::BridgeCrossing::addChildren(StructurePiece*,std::vector<StructurePiece*>*,Random&) {}
bool NetherBridgePieces::BridgeCrossing::postProcess(Level*,Random&,BoundingBox*) { return true; }
StructurePiece* NetherBridgePieces::BridgeCrossing::Create(){return new BridgeCrossing();}
EStructurePiece NetherBridgePieces::BridgeCrossing::GetType(){return eStructurePiece_BridgeCrossing;}

NetherBridgePieces::BridgeEndFiller::BridgeEndFiller():NetherBridgePiece() {}
NetherBridgePieces::BridgeEndFiller::BridgeEndFiller(int d,Random&,BoundingBox* b,const Direction* o):NetherBridgePiece(d){setBoundingBox(b);setOrientation(o);}
StructurePiece* NetherBridgePieces::BridgeEndFiller::createPiece(std::vector<StructurePiece*>* p, Random& r,
    int x, int y, int z, const Direction* o, int d)
{
    BoundingBox* b = BoundingBox::orientBox(x, y, z, 0, -1, 0, 5, 10, 19, o);
    if (b == NULL || StructurePiece::findCollisionPiece(p, b) != NULL)
    {
        delete b;
        return NULL;
    }
    return new BridgeEndFiller(d, r, b, o);
}
bool NetherBridgePieces::BridgeEndFiller::postProcess(Level*,Random&,BoundingBox*) { return true; }
StructurePiece* NetherBridgePieces::BridgeEndFiller::Create(){return new BridgeEndFiller();}
EStructurePiece NetherBridgePieces::BridgeEndFiller::GetType(){return eStructurePiece_BridgeEndFiller;}

NetherBridgePieces::BridgeStraight::BridgeStraight():NetherBridgePiece() {}
NetherBridgePieces::BridgeStraight::BridgeStraight(int d,Random&,BoundingBox* b,const Direction* o):NetherBridgePiece(d){setBoundingBox(b);setOrientation(o);}
StructurePiece* NetherBridgePieces::BridgeStraight::createPiece(std::vector<StructurePiece*>* p, Random& r,
    int x, int y, int z, const Direction* o, int d)
{
    BoundingBox* b = BoundingBox::orientBox(x, y, z, 0, -1, 0, 5, 10, 19, o);
    if (b == NULL || StructurePiece::findCollisionPiece(p, b) != NULL)
    {
        delete b;
        return NULL;
    }
    return new BridgeStraight(d, r, b, o);
}
void NetherBridgePieces::BridgeStraight::addChildren(StructurePiece*,std::vector<StructurePiece*>*,Random&) {}
bool NetherBridgePieces::BridgeStraight::postProcess(Level*,Random&,BoundingBox*) { return true; }
StructurePiece* NetherBridgePieces::BridgeStraight::Create(){return new BridgeStraight();}
EStructurePiece NetherBridgePieces::BridgeStraight::GetType(){return eStructurePiece_BridgeStraight;}

NetherBridgePieces::CastleCorridorStairsPiece::CastleCorridorStairsPiece():NetherBridgePiece() {}
NetherBridgePieces::CastleCorridorStairsPiece::CastleCorridorStairsPiece(int d,Random&,BoundingBox* b,const Direction* o):NetherBridgePiece(d){setBoundingBox(b);setOrientation(o);}
StructurePiece* NetherBridgePieces::CastleCorridorStairsPiece::createPiece(std::vector<StructurePiece*>* p, Random& r,
    int x, int y, int z, const Direction* o, int d)
{
    BoundingBox* b = BoundingBox::orientBox(x, y, z, 0, -1, 0, 5, 10, 19, o);
    if (b == NULL || StructurePiece::findCollisionPiece(p, b) != NULL)
    {
        delete b;
        return NULL;
    }
    return new CastleCorridorStairsPiece(d, r, b, o);
}
void NetherBridgePieces::CastleCorridorStairsPiece::addChildren(StructurePiece*,std::vector<StructurePiece*>*,Random&) {}
bool NetherBridgePieces::CastleCorridorStairsPiece::postProcess(Level*,Random&,BoundingBox*) { return true; }
StructurePiece* NetherBridgePieces::CastleCorridorStairsPiece::Create(){return new CastleCorridorStairsPiece();}
EStructurePiece NetherBridgePieces::CastleCorridorStairsPiece::GetType(){return eStructurePiece_CastleCorridorStairsPiece;}

NetherBridgePieces::CastleCorridorTBalconyPiece::CastleCorridorTBalconyPiece():NetherBridgePiece() {}
NetherBridgePieces::CastleCorridorTBalconyPiece::CastleCorridorTBalconyPiece(
    int d, Random&, BoundingBox* b, const Direction* o)
    : NetherBridgePiece(d)
{
    setBoundingBox(b);
    setOrientation(o);
}
StructurePiece* NetherBridgePieces::CastleCorridorTBalconyPiece::createPiece(std::vector<StructurePiece*>* p, Random& r,
    int x, int y, int z, const Direction* o, int d)
{
    BoundingBox* b = BoundingBox::orientBox(x, y, z, 0, -1, 0, 5, 10, 19, o);
    if (b == NULL || StructurePiece::findCollisionPiece(p, b) != NULL)
    {
        delete b;
        return NULL;
    }
    return new CastleCorridorTBalconyPiece(d, r, b, o);
}
void NetherBridgePieces::CastleCorridorTBalconyPiece::addChildren(StructurePiece*,std::vector<StructurePiece*>*,Random&) {}
bool NetherBridgePieces::CastleCorridorTBalconyPiece::postProcess(Level*,Random&,BoundingBox*) { return true; }
StructurePiece* NetherBridgePieces::CastleCorridorTBalconyPiece::Create(){return new CastleCorridorTBalconyPiece();}
EStructurePiece NetherBridgePieces::CastleCorridorTBalconyPiece::GetType(){return eStructurePiece_CastleCorridorTBalconyPiece;}

NetherBridgePieces::CastleEntrance::CastleEntrance():NetherBridgePiece() {}
NetherBridgePieces::CastleEntrance::CastleEntrance(int d,Random&,BoundingBox* b,const Direction* o):NetherBridgePiece(d){setBoundingBox(b);setOrientation(o);}
StructurePiece* NetherBridgePieces::CastleEntrance::createPiece(std::vector<StructurePiece*>* p, Random& r,
    int x, int y, int z, const Direction* o, int d)
{
    BoundingBox* b = BoundingBox::orientBox(x, y, z, 0, -1, 0, 5, 10, 19, o);
    if (b == NULL || StructurePiece::findCollisionPiece(p, b) != NULL)
    {
        delete b;
        return NULL;
    }
    return new CastleEntrance(d, r, b, o);
}
void NetherBridgePieces::CastleEntrance::addChildren(StructurePiece*,std::vector<StructurePiece*>*,Random&) {}
bool NetherBridgePieces::CastleEntrance::postProcess(Level*,Random&,BoundingBox*) { return true; }
StructurePiece* NetherBridgePieces::CastleEntrance::Create(){return new CastleEntrance();}
EStructurePiece NetherBridgePieces::CastleEntrance::GetType(){return eStructurePiece_CastleEntrance;}

NetherBridgePieces::CastleSmallCorridorCrossingPiece::CastleSmallCorridorCrossingPiece():NetherBridgePiece() {}
NetherBridgePieces::CastleSmallCorridorCrossingPiece::CastleSmallCorridorCrossingPiece(
    int d, Random&, BoundingBox* b, const Direction* o)
    : NetherBridgePiece(d)
{
    setBoundingBox(b);
    setOrientation(o);
}
StructurePiece* NetherBridgePieces::CastleSmallCorridorCrossingPiece::createPiece(
    std::vector<StructurePiece*>* p, Random& r, int x, int y, int z, const Direction* o, int d)
{
    BoundingBox* b = BoundingBox::orientBox(x, y, z, 0, -1, 0, 5, 10, 19, o);
    if (b == NULL || StructurePiece::findCollisionPiece(p, b) != NULL)
    {
        delete b;
        return NULL;
    }
    return new CastleSmallCorridorCrossingPiece(d, r, b, o);
}
void NetherBridgePieces::CastleSmallCorridorCrossingPiece::addChildren(StructurePiece*,std::vector<StructurePiece*>*,Random&) {}
bool NetherBridgePieces::CastleSmallCorridorCrossingPiece::postProcess(Level*,Random&,BoundingBox*) { return true; }
StructurePiece* NetherBridgePieces::CastleSmallCorridorCrossingPiece::Create(){return new CastleSmallCorridorCrossingPiece();}
EStructurePiece NetherBridgePieces::CastleSmallCorridorCrossingPiece::GetType(){return eStructurePiece_CastleSmallCorridorCrossingPiece;}

NetherBridgePieces::CastleSmallCorridorPiece::CastleSmallCorridorPiece():NetherBridgePiece() {}
NetherBridgePieces::CastleSmallCorridorPiece::CastleSmallCorridorPiece(int d,Random&,BoundingBox* b,const Direction* o):NetherBridgePiece(d){setBoundingBox(b);setOrientation(o);}
StructurePiece* NetherBridgePieces::CastleSmallCorridorPiece::createPiece(std::vector<StructurePiece*>* p, Random& r,
    int x, int y, int z, const Direction* o, int d)
{
    BoundingBox* b = BoundingBox::orientBox(x, y, z, 0, -1, 0, 5, 10, 19, o);
    if (b == NULL || StructurePiece::findCollisionPiece(p, b) != NULL)
    {
        delete b;
        return NULL;
    }
    return new CastleSmallCorridorPiece(d, r, b, o);
}
void NetherBridgePieces::CastleSmallCorridorPiece::addChildren(StructurePiece*,std::vector<StructurePiece*>*,Random&) {}
bool NetherBridgePieces::CastleSmallCorridorPiece::postProcess(Level*,Random&,BoundingBox*) { return true; }
StructurePiece* NetherBridgePieces::CastleSmallCorridorPiece::Create(){return new CastleSmallCorridorPiece();}
EStructurePiece NetherBridgePieces::CastleSmallCorridorPiece::GetType(){return eStructurePiece_CastleSmallCorridorPiece;}

NetherBridgePieces::CastleStalkRoom::CastleStalkRoom():NetherBridgePiece() {}
NetherBridgePieces::CastleStalkRoom::CastleStalkRoom(int d,Random&,BoundingBox* b,const Direction* o):NetherBridgePiece(d){setBoundingBox(b);setOrientation(o);}
StructurePiece* NetherBridgePieces::CastleStalkRoom::createPiece(std::vector<StructurePiece*>* p, Random& r,
    int x, int y, int z, const Direction* o, int d)
{
    BoundingBox* b = BoundingBox::orientBox(x, y, z, 0, -1, 0, 5, 10, 19, o);
    if (b == NULL || StructurePiece::findCollisionPiece(p, b) != NULL)
    {
        delete b;
        return NULL;
    }
    return new CastleStalkRoom(d, r, b, o);
}
void NetherBridgePieces::CastleStalkRoom::addChildren(StructurePiece*,std::vector<StructurePiece*>*,Random&) {}
bool NetherBridgePieces::CastleStalkRoom::postProcess(Level*,Random&,BoundingBox*) { return true; }
StructurePiece* NetherBridgePieces::CastleStalkRoom::Create(){return new CastleStalkRoom();}
EStructurePiece NetherBridgePieces::CastleStalkRoom::GetType(){return eStructurePiece_CastleStalkRoom;}

NetherBridgePieces::RoomCrossing::RoomCrossing():NetherBridgePiece() {}
NetherBridgePieces::RoomCrossing::RoomCrossing(int d,Random&,BoundingBox* b,const Direction* o):NetherBridgePiece(d){setBoundingBox(b);setOrientation(o);}
StructurePiece* NetherBridgePieces::RoomCrossing::createPiece(std::vector<StructurePiece*>* p, Random& r,
    int x, int y, int z, const Direction* o, int d)
{
    BoundingBox* b = BoundingBox::orientBox(x, y, z, 0, -1, 0, 5, 10, 19, o);
    if (b == NULL || StructurePiece::findCollisionPiece(p, b) != NULL)
    {
        delete b;
        return NULL;
    }
    return new RoomCrossing(d, r, b, o);
}
void NetherBridgePieces::RoomCrossing::addChildren(StructurePiece*,std::vector<StructurePiece*>*,Random&) {}
bool NetherBridgePieces::RoomCrossing::postProcess(Level*,Random&,BoundingBox*) { return true; }
StructurePiece* NetherBridgePieces::RoomCrossing::Create(){return new RoomCrossing();}
EStructurePiece NetherBridgePieces::RoomCrossing::GetType(){return eStructurePiece_RoomCrossing;}

NetherBridgePieces::StairsRoom::StairsRoom():NetherBridgePiece() {}
NetherBridgePieces::StairsRoom::StairsRoom(int d,Random&,BoundingBox* b,const Direction* o):NetherBridgePiece(d){setBoundingBox(b);setOrientation(o);}
StructurePiece* NetherBridgePieces::StairsRoom::createPiece(std::vector<StructurePiece*>* p, Random& r,
    int x, int y, int z, const Direction* o, int d)
{
    BoundingBox* b = BoundingBox::orientBox(x, y, z, 0, -1, 0, 5, 10, 19, o);
    if (b == NULL || StructurePiece::findCollisionPiece(p, b) != NULL)
    {
        delete b;
        return NULL;
    }
    return new StairsRoom(d, r, b, o);
}
void NetherBridgePieces::StairsRoom::addChildren(StructurePiece*,std::vector<StructurePiece*>*,Random&) {}
bool NetherBridgePieces::StairsRoom::postProcess(Level*,Random&,BoundingBox*) { return true; }
StructurePiece* NetherBridgePieces::StairsRoom::Create(){return new StairsRoom();}
EStructurePiece NetherBridgePieces::StairsRoom::GetType(){return eStructurePiece_StairsRoom;}

NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::CastleSmallCorridorLeftTurnPiece():NetherBridgePiece() {}
NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::CastleSmallCorridorLeftTurnPiece(
    int d, Random&, BoundingBox* b, const Direction* o)
    : NetherBridgePiece(d)
{
    setBoundingBox(b);
    setOrientation(o);
}
StructurePiece* NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::createPiece(
    std::vector<StructurePiece*>* p, Random& r, int x, int y, int z, const Direction* o, int d)
{
    BoundingBox* b = BoundingBox::orientBox(x, y, z, 0, -1, 0, 5, 10, 19, o);
    if (b == NULL || StructurePiece::findCollisionPiece(p, b) != NULL)
    {
        delete b;
        return NULL;
    }
    return new CastleSmallCorridorLeftTurnPiece(d, r, b, o);
}
void NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::addChildren(StructurePiece*,std::vector<StructurePiece*>*,Random&) {}
bool NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::postProcess(Level*,Random&,BoundingBox*) { return true; }
StructurePiece* NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::Create(){return new CastleSmallCorridorLeftTurnPiece();}
EStructurePiece NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::GetType(){return eStructurePiece_CastleSmallCorridorLeftTurnPiece;}

NetherBridgePieces::CastleSmallCorridorRightTurnPiece::CastleSmallCorridorRightTurnPiece():NetherBridgePiece() {}
NetherBridgePieces::CastleSmallCorridorRightTurnPiece::CastleSmallCorridorRightTurnPiece(
    int d, Random&, BoundingBox* b, const Direction* o)
    : NetherBridgePiece(d)
{
    setBoundingBox(b);
    setOrientation(o);
}
StructurePiece* NetherBridgePieces::CastleSmallCorridorRightTurnPiece::createPiece(
    std::vector<StructurePiece*>* p, Random& r, int x, int y, int z, const Direction* o, int d)
{
    BoundingBox* b = BoundingBox::orientBox(x, y, z, 0, -1, 0, 5, 10, 19, o);
    if (b == NULL || StructurePiece::findCollisionPiece(p, b) != NULL)
    {
        delete b;
        return NULL;
    }
    return new CastleSmallCorridorRightTurnPiece(d, r, b, o);
}
void NetherBridgePieces::CastleSmallCorridorRightTurnPiece::addChildren(StructurePiece*,std::vector<StructurePiece*>*,Random&) {}
bool NetherBridgePieces::CastleSmallCorridorRightTurnPiece::postProcess(Level*,Random&,BoundingBox*) { return true; }
StructurePiece* NetherBridgePieces::CastleSmallCorridorRightTurnPiece::Create(){return new CastleSmallCorridorRightTurnPiece();}
EStructurePiece NetherBridgePieces::CastleSmallCorridorRightTurnPiece::GetType(){return eStructurePiece_CastleSmallCorridorRightTurnPiece;}

NetherBridgePieces::MonsterThrone::MonsterThrone():NetherBridgePiece() {}
NetherBridgePieces::MonsterThrone::MonsterThrone(int d,Random&,BoundingBox* b,const Direction* o):NetherBridgePiece(d){setBoundingBox(b);setOrientation(o);}
StructurePiece* NetherBridgePieces::MonsterThrone::createPiece(std::vector<StructurePiece*>* p, Random& r,
    int x, int y, int z, const Direction* o, int d)
{
    BoundingBox* b = BoundingBox::orientBox(x, y, z, 0, -1, 0, 5, 10, 19, o);
    if (b == NULL || StructurePiece::findCollisionPiece(p, b) != NULL)
    {
        delete b;
        return NULL;
    }
    return new MonsterThrone(d, r, b, o);
}
bool NetherBridgePieces::MonsterThrone::postProcess(Level*,Random&,BoundingBox*) { return true; }
StructurePiece* NetherBridgePieces::MonsterThrone::Create(){return new MonsterThrone();}
EStructurePiece NetherBridgePieces::MonsterThrone::GetType(){return eStructurePiece_MonsterThrone;}

void NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::readAdditionalSaveData(DataInputStream* i,StructureManager* m){NetherBridgePiece::readAdditionalSaveData(i,m);}
void NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::addAdditionalSaveData(DataOutputStream* o){NetherBridgePiece::addAdditionalSaveData(o);}
void NetherBridgePieces::CastleSmallCorridorRightTurnPiece::readAdditionalSaveData(DataInputStream* i,StructureManager* m){NetherBridgePiece::readAdditionalSaveData(i,m);}
void NetherBridgePieces::CastleSmallCorridorRightTurnPiece::addAdditionalSaveData(DataOutputStream* o){NetherBridgePiece::addAdditionalSaveData(o);}
void NetherBridgePieces::MonsterThrone::readAdditionalSaveData(DataInputStream* i,StructureManager* m){NetherBridgePiece::readAdditionalSaveData(i,m);}
void NetherBridgePieces::MonsterThrone::addAdditionalSaveData(DataOutputStream* o){NetherBridgePiece::addAdditionalSaveData(o);}
void NetherBridgePieces::BridgeEndFiller::readAdditionalSaveData(DataInputStream* i,StructureManager* m){NetherBridgePiece::readAdditionalSaveData(i,m);}
void NetherBridgePieces::BridgeEndFiller::addAdditionalSaveData(DataOutputStream* o){NetherBridgePiece::addAdditionalSaveData(o);}
NetherBridgePieces::StartPiece::StartPiece(Random&,int,int,Level*):BridgeCrossing(){mPreviousPieceWeight=0;mPreviousPieceDepth=0;}
NetherBridgePieces::StartPiece::~StartPiece(){}
EStructurePiece NetherBridgePieces::StartPiece::GetType(){return eStructurePiece_NetherBridgeStartPiece;}

NetherBridgePieces::BridgeCrossing::~BridgeCrossing() {}
NetherBridgePieces::BridgeEndFiller::~BridgeEndFiller() {}
NetherBridgePieces::BridgeStraight::~BridgeStraight() {}
NetherBridgePieces::CastleCorridorStairsPiece::~CastleCorridorStairsPiece() {}
NetherBridgePieces::CastleCorridorTBalconyPiece::~CastleCorridorTBalconyPiece() {}
NetherBridgePieces::CastleEntrance::~CastleEntrance() {}
NetherBridgePieces::CastleSmallCorridorCrossingPiece::~CastleSmallCorridorCrossingPiece() {}
NetherBridgePieces::CastleSmallCorridorPiece::~CastleSmallCorridorPiece() {}
NetherBridgePieces::CastleStalkRoom::~CastleStalkRoom() {}
NetherBridgePieces::RoomCrossing::~RoomCrossing() {}
NetherBridgePieces::StairsRoom::~StairsRoom() {}
NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::~CastleSmallCorridorLeftTurnPiece() {}
NetherBridgePieces::CastleSmallCorridorRightTurnPiece::~CastleSmallCorridorRightTurnPiece() {}
NetherBridgePieces::MonsterThrone::~MonsterThrone() {}
