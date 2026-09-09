#include "game/core/NetherBridgePieces.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/MobSpawnerBlockEntity.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
#include <cstdlib>

static const BlockState* netherBricks() { return Blocks::nether_brick->defaultBlockState(); }
static const BlockState* netherBrickFence() { return Blocks::nether_brick_fence->defaultBlockState(); }
static const BlockState* netherBrickStairs() { return Blocks::nether_brick_stairs->defaultBlockState(); }
static const BlockState* air() { return Blocks::AIR->defaultBlockState(); }

static bool buildNetherRoom(StructurePiece* piece, Level* level, BoundingBox* box,
                            int width, int height, int depth)
{
    if (piece == NULL || level == NULL || box == NULL)
        return false;
    const BlockState* brick = netherBricks();
    const BlockState* empty = air();
    piece->generateBox(level, box, 0, 0, 0, width, 1, depth, brick, brick, false);
    piece->generateBox(level, box, 0, 2, 0, width, height - 1, depth, empty, empty, false);
    piece->generateBox(level, box, 0, 2, 0, 0, height - 1, depth, brick, brick, false);
    piece->generateBox(level, box, width, 2, 0, width, height - 1, depth, brick, brick, false);
    piece->generateBox(level, box, 0, height, 0, width, height, depth, brick, brick, false);
    for (int x = 0; x <= width; ++x)
        for (int z = 0; z <= depth; ++z)
            piece->fillColumnDown(level, brick, x, -1, z, box);
    return true;
}

void NetherBridgePieces::loadStatic()
{
    StructureFeatureIO::setPieceId(eStructurePiece_BridgeCrossing, &BridgeCrossing::Create, L"NeBCr");
    StructureFeatureIO::setPieceId(eStructurePiece_BridgeEndFiller, &BridgeEndFiller::Create, L"NeBEF");
    StructureFeatureIO::setPieceId(eStructurePiece_BridgeStraight, &BridgeStraight::Create, L"NeBS");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleCorridorStairsPiece, &CastleCorridorStairsPiece::Create, L"NeCCS");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleCorridorTBalconyPiece, &CastleCorridorTBalconyPiece::Create, L"NeCTB");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleEntrance, &CastleEntrance::Create, L"NeCE");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleSmallCorridorCrossingPiece, &CastleSmallCorridorCrossingPiece::Create, L"NeSCSC");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleSmallCorridorLeftTurnPiece, &CastleSmallCorridorLeftTurnPiece::Create, L"NeSCLT");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleSmallCorridorPiece, &CastleSmallCorridorPiece::Create, L"NeSC");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleSmallCorridorRightTurnPiece, &CastleSmallCorridorRightTurnPiece::Create, L"NeSCRT");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleStalkRoom, &CastleStalkRoom::Create, L"NeCSR");
    StructureFeatureIO::setPieceId(eStructurePiece_MonsterThrone, &MonsterThrone::Create, L"NeMT");
    StructureFeatureIO::setPieceId(eStructurePiece_RoomCrossing, &RoomCrossing::Create, L"NeRC");
    StructureFeatureIO::setPieceId(eStructurePiece_StairsRoom, &StairsRoom::Create, L"NeSR");
    StructureFeatureIO::setPieceId(eStructurePiece_NetherBridgeStartPiece, &BridgeCrossing::Create, L"NeStart");
}

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
int NetherBridgePieces::NetherBridgePiece::updatePieceWeight(std::list<PieceWeight*>* weights) {
    bool valid = false;
    int total = 0;
    for (std::list<PieceWeight*>::iterator it = weights->begin(); it != weights->end(); ++it) {
        PieceWeight* weight = *it;
        if (weight->mMaxPlaceCount > 0 && weight->mPlaceCount < weight->mMaxPlaceCount)
            valid = true;
        total += weight->mWeight;
    }
    return valid ? total : -1;
}
StructurePiece* NetherBridgePieces::NetherBridgePiece::generatePiece(
    StartPiece* start, std::list<PieceWeight*>* weights, std::vector<StructurePiece*>* pieces,
    Random& random, int x, int y, int z, const Direction* direction, int depth)
{
    if (!start || !weights || depth >= 31)
        return BridgeEndFiller::createPiece(pieces, random, x, y, z, direction, depth);
    const int total = updatePieceWeight(weights);
    if (total < 0)
        return BridgeEndFiller::createPiece(pieces, random, x, y, z, direction, depth);
    for (int attempt = 0; attempt < 5; ++attempt)
    {
        int pick = random.nextInt(total);
        for (std::list<PieceWeight*>::iterator it = weights->begin(); it != weights->end(); ++it)
        {
            PieceWeight* weight = *it;
            if (!weight || !weight->isValid() || (weight->mWeight <= 0))
                continue;
            pick -= weight->mWeight;
            if (pick >= 0)
                continue;
            if (!weight->doPlace(depth))
                break;
            StructurePiece* piece = findAndCreateBridgePieceFactory(weight, pieces, random,
                                                                       x, y, z, direction, depth);
            if (piece)
            {
                start->mPreviousPieceWeight = weight->mPieceClass;
                if (!weight->isValid())
                    weights->erase(it);
                return piece;
            }
            break;
        }
    }
    return BridgeEndFiller::createPiece(pieces, random, x, y, z, direction, depth);
}
StructurePiece* NetherBridgePieces::NetherBridgePiece::generateAndAddPiece(
    StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random,
    int x, int y, int z, const Direction* direction, int depth, bool allowBelow)
{
    if (!start || !pieces || !direction || depth >= 31 || (!allowBelow && y < 0))
        return BridgeEndFiller::createPiece(pieces, random, x, y, z, direction, depth);
    BoundingBox* startBox = start->getBoundingBox();
    if (!startBox || std::abs(x - startBox->x0) >= 113 || std::abs(z - startBox->z0) >= 113)
        return BridgeEndFiller::createPiece(pieces, random, x, y, z, direction, depth);
    StructurePiece* piece = generatePiece(start, &start->mAvailablePieces, pieces,
                                           random, x, y, z, direction, depth + 1);
    if (piece && isOkBox(piece->getBoundingBox(), start))
    {
        pieces->push_back(piece);
        start->mPendingChildren.push_back(piece);
        return piece;
    }
    delete piece;
    return NULL;
}
StructurePiece* NetherBridgePieces::NetherBridgePiece::generateChildForward(StartPiece* s,std::vector<StructurePiece*>* p,Random& r,int x,int y,bool a)
{
    const Direction* d=getOrientation(); if(!d || d->get3DDataValue()<=1) return NULL;
    BoundingBox* b=getBoundingBox(); int z;
    if(d==Direction::NORTH) return generateAndAddPiece(s,p,r,b->x0+x,b->y0+y,b->z0-1,d,getGenDepth(),a);
    if(d==Direction::SOUTH) return generateAndAddPiece(s,p,r,b->x0+x,b->y0+y,b->z1+1,d,getGenDepth(),a);
    if(d==Direction::WEST) return generateAndAddPiece(s,p,r,b->x0-1,b->y0+y,b->z0+x,d,getGenDepth(),a);
    z=b->z0+y; return generateAndAddPiece(s,p,r,b->x1+1,b->y0+x,z,d,getGenDepth(),a);
}
StructurePiece* NetherBridgePieces::NetherBridgePiece::generateChildLeft(StartPiece* s,std::vector<StructurePiece*>* p,Random& r,int x,int y,bool a)
{
    const Direction* d=getOrientation(); if(!d || d->get3DDataValue()<=1) return NULL; BoundingBox* b=getBoundingBox();
    if(d==Direction::NORTH || d==Direction::SOUTH) return generateAndAddPiece(s,p,r,b->x0-1,b->y0+x,b->z0+y,Direction::WEST,getGenDepth(),a);
    return generateAndAddPiece(s,p,r,b->x0+x,b->y0+y,b->z0-1,Direction::NORTH,getGenDepth(),a);
}
StructurePiece* NetherBridgePieces::NetherBridgePiece::generateChildRight(StartPiece* s,std::vector<StructurePiece*>* p,Random& r,int x,int y,bool a)
{
    const Direction* d=getOrientation(); if(!d || d->get3DDataValue()<=1) return NULL; BoundingBox* b=getBoundingBox();
    if(d==Direction::NORTH || d==Direction::SOUTH) return generateAndAddPiece(s,p,r,b->x1+1,b->y0+x,b->z0+y,Direction::EAST,getGenDepth(),a);
    return generateAndAddPiece(s,p,r,b->x0+x,b->y0+y,b->z1+1,Direction::SOUTH,getGenDepth(),a);
}
void NetherBridgePieces::NetherBridgePiece::generateLightPost(Level* level,Random&,BoundingBox* box,
                                                               int x,int y,int z,int xOffset,int zOffset)
{
    if (!level || !box)
        return;
    BlockPos pos(getWorldX(x, z), getWorldY(y), getWorldZ(x, z));
    if (!box->isInside(pos))
        return;
    const BlockState* fence = netherBrickFence();
    const BlockState* glowstone = Blocks::glowstone->defaultBlockState();
    for (int i = 0; i < 4; ++i)
        if (level->getBlock(pos.above(i)) != Blocks::AIR)
            return;
    for (int i = 0; i < 4; ++i)
        level->setBlock(pos.above(i), fence, 2, false);
    placeBlock(level, fence, x + xOffset, y + 3, z + zOffset, box);
    placeBlock(level, glowstone, x + xOffset, y + 2, z + zOffset, box);
}

NetherBridgePieces::PieceWeight::PieceWeight(EPieceClass c,int w,int m,bool v):mPieceClass(c),mWeight(w),mMaxPlaceCount(m),mPlaceCount(0),mIsValid(v) {}
NetherBridgePieces::PieceWeight::PieceWeight(EPieceClass c,int w,int m):mPieceClass(c),mWeight(w),mMaxPlaceCount(m),mPlaceCount(0),mIsValid(true) {}
bool NetherBridgePieces::PieceWeight::doPlace(int)
{
    return mMaxPlaceCount == 0 || mPlaceCount < mMaxPlaceCount;
}

bool NetherBridgePieces::PieceWeight::isValid()
{
    return mMaxPlaceCount == 0 || mPlaceCount < mMaxPlaceCount;
}

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
void NetherBridgePieces::BridgeCrossing::addChildren(StructurePiece* s,std::vector<StructurePiece*>* p,Random& r) { generateChildForward((StartPiece*)s,p,r,3,8,false); generateChildLeft((StartPiece*)s,p,r,3,8,false); generateChildRight((StartPiece*)s,p,r,3,8,false); }
bool NetherBridgePieces::BridgeCrossing::postProcess(Level* level,Random&,BoundingBox* box)
{
    const BlockState* brick = netherBricks();
    const BlockState* empty = air();
    generateBox(level, box, 0, 0, 0, 4, 1, 4, brick, brick, false);
    generateBox(level, box, 0, 2, 0, 4, 7, 4, empty, empty, false);
    generateBox(level, box, 0, 2, 0, 0, 7, 4, brick, brick, false);
    generateBox(level, box, 4, 2, 0, 4, 7, 4, brick, brick, false);
    generateBox(level, box, 0, 2, 5, 4, 2, 18, brick, brick, false);
    generateBox(level, box, 0, 3, 5, 0, 4, 18, brick, brick, false);
    generateBox(level, box, 4, 3, 5, 4, 4, 18, brick, brick, false);
    generateBox(level, box, 0, 5, 5, 4, 5, 18, brick, brick, false);
    for (int x = 0; x <= 4; ++x)
        for (int z = 0; z <= 18; ++z)
            fillColumnDown(level, brick, x, -1, z, box);
    return true;
}
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
bool NetherBridgePieces::BridgeEndFiller::postProcess(Level* level,Random& random,BoundingBox* box)
{
    const BlockState* brick = netherBricks();
    for (int x = 0; x <= 4; x += 2)
    {
        generateBox(level, box, x, 3, 0, x, 3, random.nextInt(8), brick, brick, false);
        generateBox(level, box, x, 4, 0, x, 4, random.nextInt(8), brick, brick, false);
    }
    generateBox(level, box, 4, 3, 0, 4, 3, random.nextInt(8), brick, brick, false);
    generateBox(level, box, 4, 4, 0, 4, 4, random.nextInt(8), brick, brick, false);
    generateBox(level, box, 4, 5, 0, 4, 5, random.nextInt(8), brick, brick, false);
    for (int x = 1; x <= 4; ++x)
        generateBox(level, box, x, 2, 0, x, 2, random.nextInt(5), brick, brick, false);
    for (int x = 0; x <= 4; x += 2)
    {
        generateBox(level, box, x, 0, 0, x, 0, random.nextInt(3), brick, brick, false);
        generateBox(level, box, x, 1, 0, x, 1, random.nextInt(3), brick, brick, false);
    }
    return true;
}
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
bool NetherBridgePieces::BridgeStraight::postProcess(Level* level, Random&, BoundingBox* box)
{
    const BlockState* brick = netherBricks();
    const BlockState* fence = netherBrickFence();
    generateBox(level, box, 0, 3, 0, 4, 4, 18, brick, brick, false);
    generateBox(level, box, 1, 5, 0, 3, 7, 18, brick, brick, false);
    generateBox(level, box, 0, 2, 0, 4, 2, 5, brick, brick, false);
    generateBox(level, box, 0, 2, 13, 4, 2, 18, brick, brick, false);
    generateBox(level, box, 0, 0, 0, 4, 1, 3, brick, brick, false);
    generateBox(level, box, 0, 0, 15, 4, 1, 18, brick, brick, false);
    for (int x = 0; x <= 6; x += 2)
    {
        placeBlock(level, fence, x, -1, 0, box);
        placeBlock(level, fence, x, -1, 18, box);
        placeBlock(level, fence, x, -1, 1, box);
        placeBlock(level, fence, x, -1, 17, box);
        placeBlock(level, fence, x, -1, 2, box);
        placeBlock(level, fence, x, -1, 16, box);
    }
    return true;
}
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
bool NetherBridgePieces::CastleCorridorStairsPiece::postProcess(Level* level, Random&, BoundingBox* box)
{
    const BlockState* brick = netherBricks();
    const BlockState* stairs = netherBrickStairs();
    for (int z = 0, height = 7; z < 9; ++z, --height)
    {
        int top = height < 1 ? 1 : height;
        int innerTop = top + 5;
        if (innerTop > 14) innerTop = 14;
        generateBox(level, box, 0, 0, z, 4, height, z, brick, brick, false);
        generateBox(level, box, 1, height + 1, z, 3, innerTop - 1, z, stairs, stairs, false);
        if (z < 7)
        {
            placeBlock(level, stairs, 1, height + 1, z, box);
            placeBlock(level, stairs, 2, height + 1, z, box);
            placeBlock(level, stairs, 3, height + 1, z, box);
        }
        generateBox(level, box, 0, innerTop, z, 4, innerTop, z, brick, brick, false);
        generateBox(level, box, 0, height + 1, z, 0, innerTop - 1, z, brick, brick, false);
        generateBox(level, box, 4, height + 1, z, 4, innerTop - 1, z, brick, brick, false);
        placeBlock(level, brick, 0, -1, z, box);
        placeBlock(level, brick, 1, -1, z, box);
        placeBlock(level, brick, 2, -1, z, box);
        placeBlock(level, brick, 3, -1, z, box);
        placeBlock(level, brick, 4, -1, z, box);
    }
    return true;
}
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
void NetherBridgePieces::CastleCorridorTBalconyPiece::addChildren(StructurePiece* s,std::vector<StructurePiece*>* p,Random& r) { int y=(getOrientation()==Direction::WEST||getOrientation()==Direction::NORTH)?5:1; generateChildLeft((StartPiece*)s,p,r,0,y,r.nextInt(8)!=0); generateChildRight((StartPiece*)s,p,r,0,y,r.nextInt(8)!=0); }
bool NetherBridgePieces::CastleCorridorTBalconyPiece::postProcess(Level* level,Random&,BoundingBox* box)
{ return buildNetherRoom(this, level, box, 8, 6, 8); }
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
bool NetherBridgePieces::CastleEntrance::postProcess(Level* level,Random&,BoundingBox* box)
{ return buildNetherRoom(this, level, box, 7, 8, 7); }
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
void NetherBridgePieces::CastleSmallCorridorCrossingPiece::addChildren(StructurePiece* s,std::vector<StructurePiece*>* p,Random& r) { generateChildForward((StartPiece*)s,p,r,0,1,true); generateChildLeft((StartPiece*)s,p,r,0,1,true); generateChildRight((StartPiece*)s,p,r,0,1,true); }
bool NetherBridgePieces::CastleSmallCorridorCrossingPiece::postProcess(Level* level,Random&,BoundingBox* box)
{
    const BlockState* brick = netherBricks();
    const BlockState* fence = netherBrickFence();
    const BlockState* empty = air();
    generateBox(level, box, 0, 0, 0, 4, 1, 4, brick, brick, false);
    generateBox(level, box, 0, 2, 0, 4, 5, 4, empty, empty, false);
    generateBox(level, box, 0, 2, 0, 0, 5, 4, brick, brick, false);
    generateBox(level, box, 4, 2, 0, 4, 5, 4, brick, brick, false);
    generateBox(level, box, 0, 3, 1, 0, 4, 1, fence, fence, false);
    generateBox(level, box, 0, 3, 3, 0, 4, 3, fence, fence, false);
    generateBox(level, box, 4, 3, 1, 4, 4, 1, fence, fence, false);
    generateBox(level, box, 4, 3, 3, 4, 4, 3, fence, fence, false);
    generateBox(level, box, 0, 6, 0, 4, 6, 4, brick, brick, false);
    for (int x = 0; x <= 4; ++x)
        for (int z = 0; z <= 4; ++z)
            fillColumnDown(level, brick, x, -1, z, box);
    return true;
}
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
bool NetherBridgePieces::CastleSmallCorridorPiece::postProcess(Level* level,Random&,BoundingBox* box)
{
    const BlockState* brick = netherBricks();
    const BlockState* fence = netherBrickFence();
    const BlockState* empty = air();
    generateBox(level, box, 0,0,0,4,1,4, brick, brick, false);
    generateBox(level, box, 0,2,0,4,5,4, empty, empty, false);
    generateBox(level, box, 0,2,0,0,5,4, brick, brick, false);
    generateBox(level, box, 4,2,0,4,5,4, brick, brick, false);
    generateBox(level, box, 0,3,1,0,4,1, fence, fence, false);
    generateBox(level, box, 0,3,3,0,4,3, fence, fence, false);
    generateBox(level, box, 4,3,1,4,4,1, fence, fence, false);
    generateBox(level, box, 4,3,3,4,4,3, fence, fence, false);
    generateBox(level, box, 0,6,0,4,6,4, brick, brick, false);
    for (int x = 0; x <= 4; ++x)
        for (int z = 0; z <= 4; ++z)
            fillColumnDown(level, brick, x, -1, z, box);
    return true;
}
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
void NetherBridgePieces::CastleStalkRoom::addChildren(StructurePiece* s,std::vector<StructurePiece*>* p,Random& r) { generateChildForward((StartPiece*)s,p,r,5,3,true); generateChildForward((StartPiece*)s,p,r,11,5,true); }
bool NetherBridgePieces::CastleStalkRoom::postProcess(Level* level,Random&,BoundingBox* box)
{ return buildNetherRoom(this, level, box, 7, 8, 7); }
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
void NetherBridgePieces::RoomCrossing::addChildren(StructurePiece* s,std::vector<StructurePiece*>* p,Random& r) { generateChildForward((StartPiece*)s,p,r,0,2,false); generateChildLeft((StartPiece*)s,p,r,0,2,false); generateChildRight((StartPiece*)s,p,r,0,2,false); }
bool NetherBridgePieces::RoomCrossing::postProcess(Level* level,Random&,BoundingBox* box)
{ return buildNetherRoom(this, level, box, 6, 6, 6); }
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
bool NetherBridgePieces::StairsRoom::postProcess(Level* level,Random&,BoundingBox* box)
{
    const BlockState* brick = netherBricks();
    const BlockState* fence = netherBrickFence();
    const BlockState* empty = air();
    generateBox(level,box,0,0,0,6,1,6,brick,brick,false);
    generateBox(level,box,0,2,0,6,10,6,empty,empty,false);
    generateBox(level,box,0,2,0,1,8,0,brick,brick,false);
    generateBox(level,box,5,2,0,6,8,0,brick,brick,false);
    generateBox(level,box,0,2,1,0,8,6,brick,brick,false);
    generateBox(level,box,6,2,1,6,8,6,brick,brick,false);
    generateBox(level,box,1,2,6,5,8,6,brick,brick,false);
    generateBox(level,box,0,3,2,0,5,4,fence,fence,false);
    generateBox(level,box,6,3,2,6,5,2,fence,fence,false);
    generateBox(level,box,6,3,4,6,5,4,fence,fence,false);
    placeBlock(level,brick,5,2,5,box);
    generateBox(level,box,4,2,5,4,3,5,brick,brick,false);
    generateBox(level,box,3,2,5,3,4,5,brick,brick,false);
    generateBox(level,box,2,2,5,2,5,5,brick,brick,false);
    generateBox(level,box,1,2,5,1,6,5,brick,brick,false);
    generateBox(level,box,1,7,1,5,7,4,brick,brick,false);
    generateBox(level,box,6,8,2,6,8,4,empty,empty,false);
    generateBox(level,box,2,6,0,4,8,0,brick,brick,false);
    generateBox(level,box,2,5,0,4,5,0,fence,fence,false);
    for (int x = 0; x <= 6; ++x)
        for (int z = 0; z <= 6; ++z)
            fillColumnDown(level,brick,x,-1,z,box);
    return true;
}
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
bool NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::postProcess(Level* level,Random&,BoundingBox* box)
{ return buildNetherRoom(this, level, box, 4, 6, 4); }
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
bool NetherBridgePieces::CastleSmallCorridorRightTurnPiece::postProcess(Level* level,Random&,BoundingBox* box)
{ return buildNetherRoom(this, level, box, 4, 6, 4); }
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
bool NetherBridgePieces::MonsterThrone::postProcess(Level* level,Random&,BoundingBox* box)
{
    const BlockState* brick = netherBricks();
    const BlockState* fence = netherBrickFence();
    const BlockState* empty = air();
    generateBox(level,box,0,2,0,6,7,7,empty,empty,false);
    generateBox(level,box,1,0,0,5,1,7,brick,brick,false);
    generateBox(level,box,1,2,1,5,2,7,brick,brick,false);
    generateBox(level,box,1,3,2,5,3,7,brick,brick,false);
    generateBox(level,box,1,4,3,5,4,7,brick,brick,false);
    generateBox(level,box,1,2,0,1,4,2,brick,brick,false);
    generateBox(level,box,5,2,0,5,4,2,brick,brick,false);
    generateBox(level,box,1,5,2,1,5,3,brick,brick,false);
    generateBox(level,box,5,5,2,5,5,3,brick,brick,false);
    generateBox(level,box,0,5,3,0,5,8,brick,brick,false);
    generateBox(level,box,6,5,3,6,5,8,brick,brick,false);
    generateBox(level,box,1,5,8,5,5,8,brick,brick,false);
    placeBlock(level,empty,1,6,3,box); placeBlock(level,empty,5,6,3,box);
    generateBox(level,box,0,6,3,0,6,8,fence,fence,false);
    generateBox(level,box,6,6,3,6,6,8,fence,fence,false);
    generateBox(level,box,1,6,8,5,7,8,fence,fence,false);
    generateBox(level,box,2,8,8,4,8,8,fence,fence,false);
    if (!mHasSpawner)
    {
        BlockPos pos(getWorldX(3,5), getWorldY(5), getWorldZ(3,5));
        if (box->isInside(pos))
        {
            mHasSpawner = true;
            setDirty();
            level->setBlock(pos, Blocks::mob_spawner->defaultBlockState(), 2, false);
            boost::shared_ptr<MobSpawnerBlockEntity> spawner =
                boost::dynamic_pointer_cast<MobSpawnerBlockEntity>(level->getBlockEntity(pos));
            if (spawner) spawner->setEntityId(EntityIO::getKey(0x000000005D540045ULL).toString());
        }
    }
    for (int x = 0; x <= 6; ++x)
        for (int z = 0; z <= 6; ++z)
            fillColumnDown(level,brick,x,-1,z,box);
    return true;
}
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
