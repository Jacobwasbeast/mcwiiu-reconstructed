#include "game/world/generation/WoodlandMansionPieces.hpp"

#include <algorithm>

#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/world/Level.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/EvocationIllager.hpp"
#include "game/core/VindicationIllager.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
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

WoodlandMansionPieces::MansionGrid::MansionGrid(Random& r) : Random(r), mBaseGrid(new SimpleGrid(11, 11, 0)), mSecondFloorGrid(0), mThirdFloorGrid(0), mThirdFloorRooms(0), mStartX(0), mStartY(0), mWidth(11), mHeight(11)
{ mSecondFloorGrid = new SimpleGrid(11, 11, 0); mStartX = 5; mStartY = 5; }
WoodlandMansionPieces::MansionGrid::~MansionGrid() { delete mBaseGrid; delete mSecondFloorGrid; delete mThirdFloorGrid; delete mThirdFloorRooms; }
bool WoodlandMansionPieces::MansionGrid::isHouse(SimpleGrid* g, int x, int y)
{
    unsigned int value = static_cast<unsigned int>(g->get(x, y));
    return value - 1U < 4U;
}
void WoodlandMansionPieces::MansionGrid::recursiveCorridor(SimpleGrid* g, int x, int y, const Direction* d, int length)
{ if (length <= 0 || !g || !d) return; g->set(x, y, 1); recursiveCorridor(g, x + d->getNormal().x, y + d->getNormal().z, d, length - 1); }
void WoodlandMansionPieces::MansionGrid::cleanEdges(SimpleGrid* g) { if (!g) return; for (int x=0;x<g->mWidth;++x) for(int y=0;y<g->mHeight;++y) if(g->get(x,y)==1 && !g->edgesTo(x,y,1)) g->set(x,y,0); }
void WoodlandMansionPieces::MansionGrid::identifyRooms(SimpleGrid* floor, SimpleGrid* rooms) { if (!floor || !rooms) return; for(int x=0;x<floor->mWidth;++x) for(int y=0;y<floor->mHeight;++y) rooms->set(x,y,isHouse(floor,x,y)?1:0); }
bool WoodlandMansionPieces::MansionGrid::isRoomId(SimpleGrid* g,int x,int y,int id,int value) { return g && g->get(x,y)==id && g->edgesTo(x,y,value); }
int WoodlandMansionPieces::MansionGrid::get1x2RoomDirection(SimpleGrid* g,int x,int y,int id,int value)
{
    if (!g) return -1;
    if (isRoomId(g,x+1,y,id,value)) return 0;
    if (isRoomId(g,x-1,y,id,value)) return 1;
    if (isRoomId(g,x,y+1,id,value)) return 2;
    if (isRoomId(g,x,y-1,id,value)) return 3;
    return -1;
}
void WoodlandMansionPieces::MansionGrid::setupThirdFloor()
{
    delete mThirdFloorGrid;
    delete mThirdFloorRooms;
    mThirdFloorGrid = new SimpleGrid(mWidth,mHeight,0);
    mThirdFloorRooms = new SimpleGrid(mWidth,mHeight,0);
    identifyRooms(mSecondFloorGrid,mThirdFloorRooms);
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
    mMirror=Mirror::NONE;
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
    p.push_back(new WoodlandMansionPiece(mStructureManager,eMinecraftStructureTemplate_WoodlandMansion,d.position,d.rotation));
    d.position=d.position.relative(d.rotation->rotate(Direction::SOUTH),16);
}
void WoodlandMansionPieces::MansionPiecePlacer::traverseTurn(std::vector<WoodlandMansionPiece*>&,PlacementData& d)
{
    d.rotation=d.rotation->getRotated(Rotation::CLOCKWISE_90);
    d.position=d.position.relative(d.rotation->rotate(Direction::EAST),-1);
}
void WoodlandMansionPieces::MansionPiecePlacer::traverseWallPiece(std::vector<WoodlandMansionPiece*>& p,PlacementData& d)
{
    p.push_back(new WoodlandMansionPiece(mStructureManager,d.templateType,d.position,d.rotation));
    d.position=d.position.relative(d.rotation->rotate(Direction::SOUTH),8);
}
void WoodlandMansionPieces::MansionPiecePlacer::traverseInnerTurn(std::vector<WoodlandMansionPiece*>&,PlacementData& d)
{
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
void WoodlandMansionPieces::MansionPiecePlacer::addRoom1x1(std::vector<WoodlandMansionPiece*>& p,const BlockPos& pos,Rotation* r,const Direction*,FloorRoomCollection* rooms)
{
    p.push_back(new WoodlandMansionPiece(mStructureManager,(eMinecraftStructureTemplate)rooms->get1x1(mRandom),pos,r));
}
void WoodlandMansionPieces::MansionPiecePlacer::addRoom1x2(std::vector<WoodlandMansionPiece*>& p,const BlockPos& pos,Rotation* r,const Direction*,const Direction*,FloorRoomCollection* rooms,bool secret)
{
    p.push_back(new WoodlandMansionPiece(mStructureManager,(eMinecraftStructureTemplate)(secret?rooms->get1x2Secret(mRandom):rooms->get1x2FrontEntrance(mRandom,false)),pos,r));
}
void WoodlandMansionPieces::MansionPiecePlacer::addRoom2x2(std::vector<WoodlandMansionPiece*>& p,BlockPos pos,Rotation* r,const Direction*,const Direction*,FloorRoomCollection* rooms)
{
    p.push_back(new WoodlandMansionPiece(mStructureManager,(eMinecraftStructureTemplate)rooms->get2x2(mRandom),pos,r));
}
void WoodlandMansionPieces::MansionPiecePlacer::addRoom2x2Secret(std::vector<WoodlandMansionPiece*>& p,const BlockPos& pos,Rotation* r,FloorRoomCollection* rooms)
{
    p.push_back(new WoodlandMansionPiece(mStructureManager,(eMinecraftStructureTemplate)rooms->get2x2Secret(mRandom),pos,r));
}
void WoodlandMansionPieces::MansionPiecePlacer::createMansion(const BlockPos& pos,Rotation* r,std::vector<WoodlandMansionPiece*>& p,MansionGrid* g)
{
    if(g)
    {
        mRoofStartX = g->mStartX + 1;
        mRoofStartZ = g->mStartY + 1;
        PlacementData d={r,pos,eMinecraftStructureTemplate_WoodlandMansion};
        entrance(p,d);
    }
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
