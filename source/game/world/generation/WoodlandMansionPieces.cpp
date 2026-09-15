#include "game/world/generation/WoodlandMansionPieces.hpp"

#include <algorithm>
#include <utility>

#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/world/Level.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Mth.hpp"
#include "game/core/EvocationIllager.hpp"
#include "game/core/VindicationIllager.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/generation/StructureTemplate.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"

WoodlandMansionPieces::SimpleGrid::SimpleGrid(int width, int height, int value)
    : mWidth(width), mHeight(height), mDefaultValue(value), mValues(new int*[width]), mRows(new int[width * height])
{
    int x;
    for (x = 0; x < width; ++x)
    {
        mValues[x] = mRows + x * height;
        int y;
        for (y = 0; y < height; ++y) mValues[x][y] = value;
    }
}

WoodlandMansionPieces::SimpleGrid::~SimpleGrid() { delete[] mValues; delete[] mRows; }
void WoodlandMansionPieces::SimpleGrid::set(int x, int y, int value)
{ if (x >= 0 && x < mWidth && y >= 0 && y < mHeight) mValues[x][y] = value; }
void WoodlandMansionPieces::SimpleGrid::set(int x0, int y0, int x1, int y1, int value)
{ int x, y; for (x = x0; x <= x1; ++x) for (y = y0; y <= y1; ++y) set(x, y, value); }
int WoodlandMansionPieces::SimpleGrid::get(int x, int y)
{ return x >= 0 && x < mWidth && y >= 0 && y < mHeight ? mValues[x][y] : mDefaultValue; }
void WoodlandMansionPieces::SimpleGrid::setIf(int x, int y, int oldValue, int newValue)
{ if (get(x, y) == oldValue) set(x, y, newValue); }
bool WoodlandMansionPieces::SimpleGrid::edgesTo(int x, int y, int value)
{ return get(x - 1, y) == value || get(x + 1, y) == value || get(x, y - 1) == value || get(x, y + 1) == value; }

WoodlandMansionPieces::FloorRoomCollection::~FloorRoomCollection() { }
WoodlandMansionPieces::FirstFloorRoomCollection::~FirstFloorRoomCollection() { }
int WoodlandMansionPieces::FirstFloorRoomCollection::get1x1(Random& r) { return r.nextInt(5) + 62; }
int WoodlandMansionPieces::FirstFloorRoomCollection::get1x1Secret(Random& r) { return r.nextInt(4) + 67; }
int WoodlandMansionPieces::FirstFloorRoomCollection::get1x2SideEntrance(Random& r, bool) { return r.nextInt(9) + 76; }
int WoodlandMansionPieces::FirstFloorRoomCollection::get1x2FrontEntrance(Random& r, bool) { return r.nextInt(5) + 85; }
int WoodlandMansionPieces::FirstFloorRoomCollection::get1x2Secret(Random& r) { return r.nextInt(2) + 101; }
int WoodlandMansionPieces::FirstFloorRoomCollection::get2x2(Random& r) { return r.nextInt(4) + 104; }
int WoodlandMansionPieces::FirstFloorRoomCollection::get2x2Secret(Random&) { return 113; }

WoodlandMansionPieces::SecondFloorRoomCollection::~SecondFloorRoomCollection() { }
int WoodlandMansionPieces::SecondFloorRoomCollection::get1x1(Random& r) { return r.nextInt(5) + 0; }
int WoodlandMansionPieces::SecondFloorRoomCollection::get1x1Secret(Random& r) { return r.nextInt(4) + 5; }
int WoodlandMansionPieces::SecondFloorRoomCollection::get1x2SideEntrance(Random& r, bool staircase) { return r.nextInt(5) + (staircase ? 14 : 9); }
int WoodlandMansionPieces::SecondFloorRoomCollection::get1x2FrontEntrance(Random& r, bool staircase) { return r.nextInt(5) + (staircase ? 23 : 18); }
int WoodlandMansionPieces::SecondFloorRoomCollection::get1x2Secret(Random&) { return 103; }
int WoodlandMansionPieces::SecondFloorRoomCollection::get2x2(Random& r) { return r.nextInt(4) + 28; }
int WoodlandMansionPieces::SecondFloorRoomCollection::get2x2Secret(Random&) { return 113; }
WoodlandMansionPieces::ThirdFloorRoomCollection::~ThirdFloorRoomCollection() { }

WoodlandMansionPieces::MansionGrid::MansionGrid(Random& r)
    : Random(r), mBaseGrid(new SimpleGrid(11, 11, 5)), mSecondFloorGrid(0),
      mThirdFloorRooms(0), mThirdFloorRoomsCount(0), mStartX(7), mStartY(4)
{
    mBaseGrid->set(mStartX, mStartY, mStartX, mStartY + 1, 3);
    mBaseGrid->set(mStartX - 1, mStartY, mStartX - 1, mStartY + 1, 2);
    mBaseGrid->set(mStartX + 2, mStartY - 2, mStartX + 3, mStartY + 3, 5);
    mBaseGrid->set(mStartX + 1, mStartY - 2, mStartX + 1, mStartY - 1, 1);
    mBaseGrid->set(mStartX + 1, mStartY + 2, mStartX + 1, mStartY + 3, 1);
    mBaseGrid->set(mStartX - 1, mStartY - 1, 1);
    mBaseGrid->set(mStartX - 1, mStartY + 2, 1);
    mBaseGrid->set(0, 0, 0, 10, 5);
    mBaseGrid->set(0, 9, 10, 10, 5);
    recursiveCorridor(mBaseGrid, mStartX, mStartY - 2, Direction::SOUTH, 6);
    recursiveCorridor(mBaseGrid, mStartX, mStartY + 3, Direction::SOUTH, 6);
    recursiveCorridor(mBaseGrid, mStartX - 2, mStartY - 1, Direction::SOUTH, 3);
    recursiveCorridor(mBaseGrid, mStartX - 2, mStartY + 2, Direction::SOUTH, 3);
    while (cleanEdges(mBaseGrid)) { }

    mThirdFloorRoomsCount = 3;
    mThirdFloorRooms = new SimpleGrid*[mThirdFloorRoomsCount];
    for (int i = 0; i < mThirdFloorRoomsCount; ++i)
        mThirdFloorRooms[i] = new SimpleGrid(11, 11, 5);
    identifyRooms(mBaseGrid, mThirdFloorRooms[0]);
    identifyRooms(mBaseGrid, mThirdFloorRooms[1]);
    mThirdFloorRooms[0]->set(mStartX + 1, mStartY, mStartX + 1, mStartY + 1, 0x800000);
    mThirdFloorRooms[1]->set(mStartX + 1, mStartY, mStartX + 1, mStartY + 1, 0x800000);
    mSecondFloorGrid = new SimpleGrid(11, 11, 5);
    identifyRooms(mBaseGrid, mSecondFloorGrid);
    setupThirdFloor();
    identifyRooms(mSecondFloorGrid, mThirdFloorRooms[2]);
}
WoodlandMansionPieces::MansionGrid::~MansionGrid()
{
    delete mBaseGrid;
    delete mSecondFloorGrid;
    if (mThirdFloorRooms)
    {
        for (int i = 0; i < mThirdFloorRoomsCount; ++i) delete mThirdFloorRooms[i];
        delete[] mThirdFloorRooms;
    }
}
bool WoodlandMansionPieces::MansionGrid::isHouse(SimpleGrid* g, int x, int y)
{
    unsigned int value = static_cast<unsigned int>(g->get(x, y));
    return value - 1U < 4U;
}
void WoodlandMansionPieces::MansionGrid::recursiveCorridor(SimpleGrid* g, int x, int y, const Direction* d, int length)
{
    if (length <= 0) return;
    g->set(x, y, 1);
    const Vec3i normal = d->getNormal();
    g->setIf(x + normal.x, y + normal.z, 0, 1);
    for (int attempt = 0; attempt < 4; ++attempt)
    {
        const Direction* next = Direction::from2DDataValue(mRandom.nextInt(4));
        if (next == d->getOpposite() ||
            (next == Direction::EAST && !mRandom.nextBoolean())) continue;
        const Vec3i step = next->getNormal();
        const int nextX = x + normal.x + step.x;
        const int nextY = y + normal.z + step.z;
        if (g->get(nextX, nextY) == 0 &&
            g->get(nextX + step.x, nextY + step.z) == 0)
        {
            recursiveCorridor(g, nextX, nextY, next, length - 1);
            break;
        }
    }
    const Vec3i clockwise = d->getClockWise()->getNormal();
    const Vec3i counterClockwise = d->getCounterClockWise()->getNormal();
    g->setIf(x + clockwise.x, y + clockwise.z, 0, 2);
    g->setIf(x + counterClockwise.x, y + counterClockwise.z, 0, 2);
    g->setIf(x + normal.x + clockwise.x, y + normal.z + clockwise.z, 0, 2);
    g->setIf(x + normal.x + counterClockwise.x, y + normal.z + counterClockwise.z, 0, 2);
    g->setIf(x + normal.x * 2, y + normal.z * 2, 0, 2);
    g->setIf(x + clockwise.x * 2, y + clockwise.z * 2, 0, 2);
    g->setIf(x + counterClockwise.x * 2, y + counterClockwise.z * 2, 0, 2);
}
bool WoodlandMansionPieces::MansionGrid::cleanEdges(SimpleGrid* g)
{
    bool changed = false;
    for (int x = 0; x < g->mWidth; ++x)
        for (int y = 0; y < g->mHeight; ++y)
            if (g->get(x, y) == 0)
            {
                int count = 0;
                count += isHouse(g, x + 1, y) ? 1 : 0;
                count += isHouse(g, x - 1, y) ? 1 : 0;
                count += isHouse(g, x, y + 1) ? 1 : 0;
                count += isHouse(g, x, y - 1) ? 1 : 0;
                if (count < 3)
                {
                    if (count == 2)
                    {
                        int diagonal = 0;
                        diagonal += isHouse(g, x + 1, y + 1) ? 1 : 0;
                        diagonal += isHouse(g, x - 1, y + 1) ? 1 : 0;
                        diagonal += isHouse(g, x + 1, y - 1) ? 1 : 0;
                        diagonal += isHouse(g, x - 1, y - 1) ? 1 : 0;
                        if (diagonal < 2) g->set(x, y, 2);
                    }
                    else g->set(x, y, 2);
                    changed = true;
                }
            }
    return changed;
}
void WoodlandMansionPieces::MansionGrid::identifyRooms(SimpleGrid* floor, SimpleGrid* rooms)
{
    std::vector<std::pair<int, int> > candidates;
    for (int y = 0; y < floor->mHeight; ++y)
        for (int x = 0; x < floor->mWidth; ++x)
            if (floor->get(x, y) == 2) candidates.push_back(std::make_pair(x, y));
    Mth::shuffle(&candidates, *this);
    int roomId = 10;
    for (std::vector<std::pair<int, int> >::const_iterator it = candidates.begin();
         it != candidates.end(); ++it, ++roomId)
    {
        int x = it->first;
        int y = it->second;
        if (rooms->get(x, y) != 0) continue;
        int width = 1;
        int height = 1;
        int type = 0x10000;
        if (rooms->get(x + 1, y) == 0 && rooms->get(x, y + 1) == 0 &&
            rooms->get(x + 1, y + 1) == 0 && floor->get(x + 1, y) == 2 &&
            floor->get(x, y + 1) == 2 && floor->get(x + 1, y + 1) == 2 &&
            mRandom.nextBoolean())
        { width = height = 2; type = 0x40000; }
        else if (rooms->get(x + 1, y) == 0 && floor->get(x + 1, y) == 2)
        { width = 2; type = 0x20000; }
        else if (rooms->get(x, y + 1) == 0 && floor->get(x, y + 1) == 2)
        { height = 2; type = 0x20000; }
        else if (rooms->get(x - 1, y) == 0 && floor->get(x - 1, y) == 2)
        { --x; width = 2; type = 0x20000; }
        else if (rooms->get(x, y - 1) == 0 && floor->get(x, y - 1) == 2)
        { --y; height = 2; type = 0x20000; }
        int flags = type | roomId;
        if (floor->edgesTo(x, y, 1) || floor->edgesTo(x + width - 1, y + height - 1, 1))
            flags |= 0x200000;
        for (int xx = x; xx < x + width; ++xx)
            for (int yy = y; yy < y + height; ++yy)
                rooms->set(xx, yy, flags | ((xx == x && yy == y) ? 0x100000 : 0));
    }
}
bool WoodlandMansionPieces::MansionGrid::isRoomId(SimpleGrid*, int x, int y, int floor, int roomId)
{
    return floor >= 0 && floor < mThirdFloorRoomsCount &&
        (mThirdFloorRooms[floor]->get(x, y) & 0xffff) == roomId;
}
const Direction* WoodlandMansionPieces::MansionGrid::get1x2RoomDirection(
    SimpleGrid*, int x, int y, int floor, int roomId)
{
    arrayWithLength<const Direction*> faces = Direction::Plane::HORIZONTAL->getFaces();
    for (unsigned int i = 0; i < faces.length; ++i)
    {
        const Direction* direction = faces.data[i];
        const Vec3i normal = direction->getNormal();
        if (isRoomId(NULL, x + normal.x, y + normal.z, floor, roomId)) return direction;
    }
    return NULL;
}
void WoodlandMansionPieces::MansionGrid::setupThirdFloor()
{
    std::vector<std::pair<int, int> > candidates;
    SimpleGrid* secondRooms = mThirdFloorRooms[1];
    for (int y = 0; y < secondRooms->mHeight; ++y)
        for (int x = 0; x < secondRooms->mWidth; ++x)
        {
            const int value = secondRooms->get(x, y);
            if ((value & 0xf0000) == 0x20000 && (value & 0x200000) != 0)
                candidates.push_back(std::make_pair(x, y));
        }
    if (candidates.empty())
    {
        mSecondFloorGrid->set(0, 0, mSecondFloorGrid->mWidth - 1,
                              mSecondFloorGrid->mHeight - 1, 5);
        return;
    }
    const std::pair<int, int>& selected = candidates.at(mRandom.nextInt((int)candidates.size()));
    const int x = selected.first;
    const int y = selected.second;
    const int value = mSecondFloorGrid->get(x, y);
    mSecondFloorGrid->set(x, y, value | 0x400000);
    const Direction* direction = get1x2RoomDirection(mBaseGrid, x, y, 1, value & 0xffff);
    int endX = x;
    int endY = y;
    if (direction != NULL)
    {
        const Vec3i normal = direction->getNormal();
        endX += normal.x;
        endY += normal.z;
        for (int yy = 0; yy < mSecondFloorGrid->mHeight; ++yy)
            for (int xx = 0; xx < mSecondFloorGrid->mWidth; ++xx)
            {
                if (!isHouse(mBaseGrid, xx, yy)) mSecondFloorGrid->set(xx, yy, 5);
                else if ((xx == endX && yy == endY) ||
                         (xx == x && yy == y))
                {
                    mSecondFloorGrid->set(xx, yy, 3);
                    if (xx == endX && yy == endY)
                        mThirdFloorRooms[2]->set(xx, yy, 0x800000);
                }
            }
    }
    std::vector<const Direction*> faces;
    arrayWithLength<const Direction*> horizontal = Direction::Plane::HORIZONTAL->getFaces();
    for (unsigned int i = 0; i < horizontal.length; ++i)
    {
        const Vec3i normal = horizontal.data[i]->getNormal();
        const bool empty = mSecondFloorGrid->get(endX + normal.x, endY + normal.z) == 0;
        if (empty)
            mSecondFloorGrid->set(endX + normal.x, endY + normal.z, 5);
        if (empty)
            faces.push_back(horizontal.data[i]);
    }
    if (faces.empty())
    {
        mSecondFloorGrid->set(0, 0, mSecondFloorGrid->mWidth - 1,
                              mSecondFloorGrid->mHeight - 1, 5);
        mSecondFloorGrid->set(x, y, value);
    }
    else
    {
        const Direction* corridor = faces.at(mRandom.nextInt((int)faces.size()));
        const Vec3i normal = corridor->getNormal();
        recursiveCorridor(mSecondFloorGrid, endX + normal.x, endY + normal.z, corridor, 4);
        while (cleanEdges(mSecondFloorGrid)) { }
    }
}
WoodlandMansionPieces::WoodlandMansionPiece::WoodlandMansionPiece(StructureManager* m,eMinecraftStructureTemplate t,const BlockPos& p,Rotation* r)
    : TemplateStructurePiece(0),mTemplateType(t),mRotation(r),mMirror(Mirror::NONE)
{
    _init(m,t,p,r,Mirror::NONE);
}
WoodlandMansionPieces::WoodlandMansionPiece::WoodlandMansionPiece(StructureManager* m,eMinecraftStructureTemplate t,const BlockPos& p,Rotation* r,Mirror* mi)
    : TemplateStructurePiece(0),mTemplateType(t),mRotation(r),mMirror(mi)
{
    _init(m,t,p,r,mi);
}
WoodlandMansionPieces::WoodlandMansionPiece::WoodlandMansionPiece() : TemplateStructurePiece(0),mTemplateType(eMinecraftStructureTemplate_WoodlandMansion),mRotation(Rotation::NONE),mMirror(Mirror::NONE) { }
WoodlandMansionPieces::WoodlandMansionPiece::~WoodlandMansionPiece() { }
void WoodlandMansionPieces::WoodlandMansionPiece::loadTemplate(StructureManager* m)
{
    StructurePlaceSettings* s = new StructurePlaceSettings();
    s->setIgnoreEntities(true);
    s->setRotation(mRotation);
    s->setMirror(mMirror);
    setup(m->getOrCreate(mTemplateType),mTemplatePosition,s);
}
void WoodlandMansionPieces::WoodlandMansionPiece::_init(StructureManager* m,eMinecraftStructureTemplate t,const BlockPos& p,Rotation* r,Mirror* mi) { mTemplateType=t; mRotation=r; mMirror=mi; mTemplatePosition=p; loadTemplate(m); }
void WoodlandMansionPieces::WoodlandMansionPiece::addAdditionalSaveData(DataOutputStream* o)
{
    TemplateStructurePiece::addAdditionalSaveData(o);
    o->writeVarInt(mTemplateType);
    o->writeByte((unsigned char)mRotation->getType());
    o->writeByte((unsigned char)mMirror->getType());
}
void WoodlandMansionPieces::WoodlandMansionPiece::readAdditionalSaveData(DataInputStream* i,StructureManager* m)
{
    mTemplateType=(eMinecraftStructureTemplate)i->readVarInt();
    mRotation=Rotation::byType((Rotation::eTYPE)i->readByte());
    const unsigned char mirrorType = i->readByte();
    mMirror = mirrorType == Mirror::eTYPE_LeftRight ? Mirror::LEFT_RIGHT :
        mirrorType == Mirror::eTYPE_FrontBack ? Mirror::FRONT_BACK : Mirror::NONE;
    loadTemplate(m);
}
void WoodlandMansionPieces::WoodlandMansionPiece::handleDataMarker(
    const std::wstring& marker, const BlockPos& position, Level* level,
    Random& random, BoundingBox* box)
{
    if (marker.compare(0, 5, L"Chest") == 0)
    {
        const Direction* facing = NULL;
        if (marker == L"ChestWest") facing = mPlaceSettings->getRotation()->rotate(Direction::WEST);
        else if (marker == L"ChestEast") facing = mPlaceSettings->getRotation()->rotate(Direction::EAST);
        else if (marker == L"ChestSouth") facing = mPlaceSettings->getRotation()->rotate(Direction::SOUTH);
        else if (marker == L"ChestNorth") facing = mPlaceSettings->getRotation()->rotate(Direction::NORTH);
        const BlockState* state = Blocks::chest->defaultBlockState();
        if (facing != NULL)
        {
            TypedBoxed<const Direction*> boxedFacing(facing);
            state = state->setBoxedValue(ChestBlock::FACING, &boxedFacing);
        }
        createChest(level, box, random, position, &BuiltInLootTables::WOODLAND_MANSION, state);
        return;
    }
    if (marker == L"Mage")
    {
        boost::shared_ptr<EvocationIllager> illager(new EvocationIllager(level));
        illager->setPersistenceRequired(true);
        illager->moveTo(position, 0.0f, 0.0f);
        level->addEntity(illager);
        level->setBlock(position, Blocks::AIR->defaultBlockState(), 2, false);
        return;
    }
    if (marker == L"Warrior")
    {
        boost::shared_ptr<VindicationIllager> illager(new VindicationIllager(level));
        illager->setPersistenceRequired(true);
        illager->moveTo(position, 0.0f, 0.0f);
        illager->finalizeMobSpawn(level->getCurrentDifficultyAt(position), NULL, L"unknown");
        level->addEntity(illager);
        level->setBlock(position, Blocks::AIR->defaultBlockState(), 2, false);
    }
}
TemplateStructurePiece* WoodlandMansionPieces::WoodlandMansionPiece::create() { return new WoodlandMansionPiece(); }
StructurePiece* WoodlandMansionPieces::WoodlandMansionPiece::Create() { return new WoodlandMansionPiece(); }
EStructurePiece WoodlandMansionPieces::WoodlandMansionPiece::GetType() { return eStructurePiece_WoodlandMansionPiece; }

WoodlandMansionPieces::MansionPiecePlacer::MansionPiecePlacer(StructureManager* m,Random& r) : mStructureManager(m),mRandom(r),mRoofStartX(0),mRoofStartZ(0) { }
void WoodlandMansionPieces::MansionPiecePlacer::entrance(std::vector<WoodlandMansionPiece*>& p,PlacementData& d)
{
    p.push_back(new WoodlandMansionPiece(mStructureManager,
        static_cast<eMinecraftStructureTemplate>(0x3a),
        d.position.relative(d.rotation->rotate(Direction::SOUTH), 9), d.rotation));
    d.position=d.position.relative(d.rotation->rotate(Direction::SOUTH),16);
}
void WoodlandMansionPieces::MansionPiecePlacer::traverseTurn(std::vector<WoodlandMansionPiece*>& p,PlacementData& d)
{
    p.push_back(new WoodlandMansionPiece(mStructureManager,
        static_cast<eMinecraftStructureTemplate>(0x3b),
        d.position.relative(d.rotation->rotate(Direction::EAST), -9)
            .relative(d.rotation->rotate(Direction::SOUTH), -10), d.rotation));
    d.rotation=d.rotation->getRotated(Rotation::CLOCKWISE_90);
    d.position=d.position.relative(d.rotation->rotate(Direction::EAST),-1);
}
void WoodlandMansionPieces::MansionPiecePlacer::traverseWallPiece(std::vector<WoodlandMansionPiece*>& p,PlacementData& d)
{
    p.push_back(new WoodlandMansionPiece(mStructureManager,d.templateType,
        d.position.relative(d.rotation->rotate(Direction::EAST), 7), d.rotation));
    d.position=d.position.relative(d.rotation->rotate(Direction::SOUTH),8);
}
void WoodlandMansionPieces::MansionPiecePlacer::traverseInnerTurn(std::vector<WoodlandMansionPiece*>&,PlacementData& d)
{
    d.position=d.position.relative(d.rotation->rotate(Direction::EAST),6);
    d.position=d.position.relative(d.rotation->rotate(Direction::SOUTH),8);
    d.rotation=d.rotation->getRotated(Rotation::CLOCKWISE_90);
}
void WoodlandMansionPieces::MansionPiecePlacer::traverseOuterWalls(
    std::vector<WoodlandMansionPiece*>& pieces, PlacementData& data, SimpleGrid* grid,
    const Direction* direction, int x, int y, int targetX, int targetY)
{
    const Direction* current = direction;
    for (;;)
    {
        if (!MansionGrid::isHouse(grid, x + current->getNormal().x,
                                  y + current->getNormal().z))
        {
            traverseTurn(pieces, data);
            current = current->getClockWise();
            if (x == targetX && y == targetY && direction == current)
                return;
            traverseWallPiece(pieces, data);
        }
        else
        {
            const Direction* left = current->getCounterClockWise();
            int nextX = x + current->getNormal().x;
            int nextY = y + current->getNormal().z;
            if (MansionGrid::isHouse(grid, nextX + left->getNormal().x,
                                     nextY + left->getNormal().z))
            {
                traverseInnerTurn(pieces, data);
                x = nextX;
                y = nextY;
                current = left;
            }
            else
            {
                x = nextX;
                y = nextY;
                traverseWallPiece(pieces, data);
            }
        }
        if (x == targetX && y == targetY && direction == current)
            return;
    }
}
void WoodlandMansionPieces::MansionPiecePlacer::createRoof(
    std::vector<WoodlandMansionPiece*>& pieces, const BlockPos& position,
    Rotation* rotation, SimpleGrid* grid, SimpleGrid* upperGrid)
{
    if (grid == NULL || rotation == NULL) return;
    const Direction* south = rotation->rotate(Direction::SOUTH);
    const Direction* east = rotation->rotate(Direction::EAST);
    const Direction* west = rotation->rotate(Direction::WEST);
    const Direction* north = rotation->rotate(Direction::NORTH);
    const auto baseAt = [&](int x, int z) -> BlockPos {
        return position.relative(south, (z - mRoofStartZ) * 8 + 8)
            .relative(east, (x - mRoofStartX) * 8);
    };
    const auto add = [&](int id, const BlockPos& at, Rotation* turn) {
        pieces.push_back(new WoodlandMansionPiece(mStructureManager,
            static_cast<eMinecraftStructureTemplate>(id), at, turn));
    };
    for (int z = 0; z < grid->mHeight; ++z)
    {
        for (int x = 0; x < grid->mWidth; ++x)
        {
            BlockPos base = baseAt(x, z);
            if (MansionGrid::isHouse(grid, x, z) &&
                (upperGrid == NULL || !MansionGrid::isHouse(upperGrid, x, z)))
            {
                add(0x34, base.above(3), rotation);
                if (!MansionGrid::isHouse(grid, x + 1, z))
                    add(0x35, base.relative(east, 6), rotation);
                if (!MansionGrid::isHouse(grid, x - 1, z))
                    add(0x35, base.relative(east, 0).relative(south, 7),
                        rotation->getRotated(Rotation::CLOCKWISE_90));
                if (!MansionGrid::isHouse(grid, x, z - 1))
                    add(0x35, base.relative(west, 1),
                        rotation->getRotated(Rotation::COUNTERCLOCKWISE_90));
                if (!MansionGrid::isHouse(grid, x, z + 1))
                    add(0x35, base.relative(east, 6).relative(south, 6),
                        rotation->getRotated(Rotation::CLOCKWISE_180));
            }
        }
    }
    if (upperGrid == NULL) return;
    for (int z = 0; z < grid->mHeight; ++z)
        for (int x = 0; x < grid->mWidth; ++x)
        {
            BlockPos base = baseAt(x, z);
            if (!MansionGrid::isHouse(upperGrid, x, z) ||
                !MansionGrid::isHouse(grid, x, z)) continue;
            if (!MansionGrid::isHouse(grid, x + 1, z)) add(0x36, base.relative(east, 7), rotation);
            if (!MansionGrid::isHouse(grid, x - 1, z)) add(0x36, base.relative(west, 1).relative(south, 6), rotation->getRotated(Rotation::COUNTERCLOCKWISE_90));
            if (!MansionGrid::isHouse(grid, x, z - 1)) add(0x36, base.relative(west, 0).relative(north, 1), rotation->getRotated(Rotation::CLOCKWISE_90));
            if (!MansionGrid::isHouse(grid, x, z + 1)) add(0x36, base.relative(east, 6).relative(south, 7), rotation->getRotated(Rotation::CLOCKWISE_180));
            if (!MansionGrid::isHouse(grid, x + 1, z) && !MansionGrid::isHouse(grid, x, z - 1)) add(0x37, base.relative(east, 7).relative(north, 2), rotation);
            if (!MansionGrid::isHouse(grid, x - 1, z) && !MansionGrid::isHouse(grid, x, z - 1)) add(0x37, base.relative(west, 2).relative(north, 1), rotation->getRotated(Rotation::CLOCKWISE_90));
            if (!MansionGrid::isHouse(grid, x + 1, z) && !MansionGrid::isHouse(grid, x, z + 1)) add(0x38, base.relative(east, 6).relative(south, 6), rotation->getRotated(Rotation::CLOCKWISE_180));
            if (MansionGrid::isHouse(grid, x + 1, z) && MansionGrid::isHouse(grid, x, z - 1)) add(0x39, base.relative(east, 9).relative(north, 2), rotation->getRotated(Rotation::CLOCKWISE_180));
            if (!MansionGrid::isHouse(grid, x - 1, z) && !MansionGrid::isHouse(grid, x, z + 1)) add(0x38, base.relative(east, 0).relative(south, 6), rotation->getRotated(Rotation::CLOCKWISE_180));
            if (MansionGrid::isHouse(grid, x - 1, z) && MansionGrid::isHouse(grid, x, z - 1)) add(0x39, base.relative(south, 1), rotation->getRotated(Rotation::COUNTERCLOCKWISE_90));
        }
}
void WoodlandMansionPieces::MansionPiecePlacer::addRoom1x1(std::vector<WoodlandMansionPiece*>& p,const BlockPos& pos,Rotation* r,const Direction* direction,FloorRoomCollection* rooms)
{
    Rotation* turn = r;
    if (direction == Direction::EAST) turn = r->getRotated(Rotation::CLOCKWISE_90);
    else if (direction == Direction::WEST) turn = r->getRotated(Rotation::COUNTERCLOCKWISE_90);
    else if (direction == Direction::NORTH) turn = r->getRotated(Rotation::CLOCKWISE_180);
    BlockPos local(1, 0, 0);
    local = StructureTemplate::getZeroPositionWithTransform(local, Mirror::NONE, turn, 7, 7);
    local = local.rotate(r);
    const int templateId = direction == Direction::SOUTH
        ? rooms->get1x1Secret(mRandom) : rooms->get1x1(mRandom);
    p.push_back(new WoodlandMansionPiece(mStructureManager,
        static_cast<eMinecraftStructureTemplate>(templateId),
        pos.offset(local.x, 0, local.z), turn));
}
void WoodlandMansionPieces::MansionPiecePlacer::addRoom1x2(std::vector<WoodlandMansionPiece*>& p,const BlockPos& pos,Rotation* r,const Direction* first,const Direction* second,FloorRoomCollection* rooms,bool secret)
{
    Rotation* turn = r;
    if (first == Direction::EAST) turn = r->getRotated(Rotation::CLOCKWISE_90);
    else if (first == Direction::WEST) turn = r->getRotated(Rotation::COUNTERCLOCKWISE_90);
    else if (first == Direction::NORTH) turn = r->getRotated(Rotation::CLOCKWISE_180);
    BlockPos local(1, 0, 0);
    local = StructureTemplate::getZeroPositionWithTransform(local, Mirror::NONE, turn, 7, 7);
    local = local.rotate(r);
    const bool staircase = second == Direction::NORTH || second == Direction::SOUTH;
    int templateId;
    if (secret) templateId = rooms->get1x2Secret(mRandom);
    else if (first != second) templateId = rooms->get1x2SideEntrance(mRandom, staircase);
    else templateId = rooms->get1x2FrontEntrance(mRandom, staircase);
    p.push_back(new WoodlandMansionPiece(mStructureManager,
        static_cast<eMinecraftStructureTemplate>(templateId),
        pos.offset(local.x, 0, local.z), turn));
}
void WoodlandMansionPieces::MansionPiecePlacer::addRoom2x2(std::vector<WoodlandMansionPiece*>& p,BlockPos pos,Rotation* r,const Direction* first,const Direction* second,FloorRoomCollection* rooms)
{
    Rotation* turn = r;
    if (first == Direction::EAST) turn = r->getRotated(Rotation::CLOCKWISE_90);
    else if (first == Direction::WEST) turn = r->getRotated(Rotation::COUNTERCLOCKWISE_90);
    else if (first == Direction::NORTH) turn = r->getRotated(Rotation::CLOCKWISE_180);
    BlockPos local(1, 0, 0);
    local = StructureTemplate::getZeroPositionWithTransform(local, Mirror::NONE, turn, 7, 7);
    local = local.rotate(r);
    p.push_back(new WoodlandMansionPiece(mStructureManager,
        static_cast<eMinecraftStructureTemplate>(rooms->get2x2(mRandom)),
        pos.offset(local.x, 0, local.z), turn));
}
void WoodlandMansionPieces::MansionPiecePlacer::addRoom2x2Secret(std::vector<WoodlandMansionPiece*>& p,const BlockPos& pos,Rotation* r,FloorRoomCollection* rooms)
{
    BlockPos local(1, 0, 0);
    local = StructureTemplate::getZeroPositionWithTransform(local, Mirror::NONE, r, 7, 7);
    local = local.rotate(r);
    p.push_back(new WoodlandMansionPiece(mStructureManager,
        static_cast<eMinecraftStructureTemplate>(rooms->get2x2Secret(mRandom)),
        pos.offset(local.x, 0, local.z), r, Mirror::NONE));
}
void WoodlandMansionPieces::MansionPiecePlacer::createMansion(const BlockPos& pos,Rotation* r,std::vector<WoodlandMansionPiece*>& p,MansionGrid* g)
{
    if (!g || !r) return;
    mRoofStartX = g->mStartX + 1;
    mRoofStartZ = g->mStartY + 1;
    PlacementData data = { r,
        pos.relative(r->rotate(Direction::SOUTH), 7)
           .relative(r->rotate(Direction::EAST), 7),
        eMinecraftStructureTemplate_WoodlandMansion };
    entrance(p, data);

    for (int floor = 0; floor < g->mThirdFloorRoomsCount; ++floor)
    {
        SimpleGrid* roomsGrid = g->mThirdFloorRooms[floor];
        FloorRoomCollection* rooms = floor == 0
            ? static_cast<FloorRoomCollection*>(new FirstFloorRoomCollection())
            : static_cast<FloorRoomCollection*>(new SecondFloorRoomCollection());
        for (int y = 0; y < roomsGrid->mHeight; ++y)
            for (int x = 0; x < roomsGrid->mWidth; ++x)
            {
                const int value = roomsGrid->get(x, y);
                if ((value & 0x100000) == 0) continue;
                const Direction* direction = g->get1x2RoomDirection(g->mBaseGrid, x, y, floor, value & 0xffff);
                if (direction == NULL) direction = Direction::SOUTH;
                const BlockPos roomPos = pos.relative(r->rotate(Direction::SOUTH),
                    (y - mRoofStartZ) * 8 + 8).relative(r->rotate(Direction::EAST),
                    (x - mRoofStartX) * 8);
                const int type = value & 0xf0000;
                if (type == 0x10000) addRoom1x1(p, roomPos, r, direction, rooms);
                else if (type == 0x20000)
                    addRoom1x2(p, roomPos, r, direction, direction, rooms, (value & 0x200000) == 0);
                else if (type == 0x40000)
                    addRoom2x2(p, roomPos, r, direction, direction, rooms);
            }
        delete rooms;
    }
    createRoof(p, pos, r, g->mBaseGrid, g->mSecondFloorGrid);
}

void WoodlandMansionPieces::generateMansion(StructureManager* m,const BlockPos& p,Rotation* r,std::vector<WoodlandMansionPiece*>& pieces,Random& random)
{
    MansionGrid grid(random);
    MansionPiecePlacer placer(m,random);
    placer.createMansion(p,r,pieces,&grid);
}
void WoodlandMansionPieces::loadStatic()
{
    StructureFeatureIO::setPieceId(eStructurePiece_WoodlandMansionPiece,
                                   &WoodlandMansionPieces::WoodlandMansionPiece::Create,
                                   L"WMP");
}
