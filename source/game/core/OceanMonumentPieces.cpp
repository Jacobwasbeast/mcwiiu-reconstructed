#include "game/core/OceanMonumentPieces.hpp"
#include "game/core/BoundingBox.hpp"
#include "game/core/Direction.hpp"
#include "game/core/ElderGuardian.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"

const BlockState* OceanMonumentPieces::prismarineState = 0;
const BlockState* OceanMonumentPieces::prismarineBricksState = 0;
const BlockState* OceanMonumentPieces::darkPrismarineState = 0;
const BlockState* OceanMonumentPieces::seaLanternState = 0;
const BlockState* OceanMonumentPieces::goldState = 0;
void OceanMonumentPieces::loadStatic()
{
    OceanMonumentPiece::staticCtor();
    StructureFeatureIO::setPieceId(eStructurePiece_MonumentBuilding, &MonumentBuilding::Create, L"OMB");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentCoreRoom, &OceanMonumentCoreRoom::Create, L"OMCR");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentDoubleXRoom, &OceanMonumentDoubleXRoom::Create, L"OMDXR");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentDoubleXYRoom, &OceanMonumentDoubleXYRoom::Create, L"OMDXYR");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentDoubleYRoom, &OceanMonumentDoubleYRoom::Create, L"OMDYR");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentDoubleYZRoom, &OceanMonumentDoubleYZRoom::Create, L"OMDYZR");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentDoubleZRoom, &OceanMonumentDoubleZRoom::Create, L"OMDZR");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentEntryRoom, &OceanMonumentEntryRoom::Create, L"OMEntry");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentPenthouse, &OceanMonumentPenthouse::Create, L"OMPenthouse");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentSimpleRoom, &OceanMonumentSimpleRoom::Create, L"OMSimple");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentSimpleTopRoom, &OceanMonumentSimpleTopRoom::Create, L"OMSimpleT");
}
OceanMonumentPieces::RoomDefinition::RoomDefinition(int index) : mIndex(index), mClaimed(false), mSource(-1) { int i; for (i = 0; i < 6; ++i) { mConnections[i].reset(); mOpenings[i] = false; } }
void OceanMonumentPieces::RoomDefinition::setConnection(const Direction* direction, const boost::shared_ptr<RoomDefinition>& connection)
{
    mConnections[direction->get3DDataValue()] = connection;
    connection->mConnections[direction->getOpposite()->get3DDataValue()] = shared_from_this();
}
boost::shared_ptr<OceanMonumentPieces::RoomDefinition> OceanMonumentPieces::RoomDefinition::fjConnection(int index) const { return index >= 0 && index < 6 ? mConnections[index].lock() : boost::shared_ptr<RoomDefinition>(); }
boost::shared_ptr<OceanMonumentPieces::RoomDefinition> OceanMonumentPieces::RoomDefinition::fjConnection(const Direction* direction) const {
    return mConnections[direction->get3DDataValue()].lock();
    }
void OceanMonumentPieces::RoomDefinition::updateOpenings() { int i; for (i = 0; i < 6; ++i) mOpenings[i] = mConnections[i].lock() ? true : false; }
bool OceanMonumentPieces::RoomDefinition::findSource(int index) {
    if (mSource >= 0) return mSource == index;
    mSource = index;
    int i;
    for (i = 0; i < 6; ++i)
    {
        boost::shared_ptr<RoomDefinition> neighbor = mConnections[i].lock();
        if (neighbor && mOpenings[i] && neighbor->findSource(index)) return true;
    }
    return false;
    }
bool OceanMonumentPieces::RoomDefinition::isSpecial() { return mIndex > 74; }
unsigned char OceanMonumentPieces::RoomDefinition::countOpenings() { unsigned char count = 0; int i; for (i = 0; i < 6; ++i) if (mOpenings[i]) ++count; return count; }
OceanMonumentPieces::OceanMonumentPiece::OceanMonumentPiece(int genDepth) : StructurePiece(genDepth), mRoomIndex(0) {}
OceanMonumentPieces::OceanMonumentPiece::OceanMonumentPiece(int genDepth, const Direction* orientation, const boost::shared_ptr<RoomDefinition>& definition, int x, int y, int z)
    : StructurePiece(genDepth), mRoomIndex(0)
{
    setOrientation(orientation);
    mRoomDefinition = definition;
    int roomIndex = mRoomDefinition->mIndex;
    mRoomIndex = roomIndex;
    int roomY = roomIndex / 25;
    int roomX = roomIndex % 5;
    int roomZ = (roomIndex / 5) % 5;
    BoundingBox* box;
    if (orientation == Direction::NORTH || orientation == Direction::SOUTH)
        box = new BoundingBox(0, 0, 0, x * 8 - 1, y * 4 - 1, z * 8 - 1);
    else
        box = new BoundingBox(0, 0, 0, z * 8 - 1, y * 4 - 1, x * 8 - 1);
    setBoundingBox(box);
    int dir = orientation->get3DDataValue();
    if (dir > 1)
    {
        if (dir == 2)
            box->move(roomX * 8, roomY * 4, -((roomZ + z) * 8) + 1);
        else if (dir < 4)
            box->move(roomX * 8, roomY * 4, roomZ * 8);
        else if (dir == 4)
            box->move(-((roomZ + z) * 8) + 1, roomY * 4, roomX * 8);
        else
            box->move(roomZ * 8, roomY * 4, roomX * 8);
    }
    else
    {
        box->move(roomZ * 8, roomY * 4, roomX * 8);
    }
    }
OceanMonumentPieces::OceanMonumentPiece::OceanMonumentPiece(const Direction* orientation, BoundingBox* box) : StructurePiece(1), mRoomIndex(0) { setOrientation(orientation); setBoundingBox(box); }
OceanMonumentPieces::OceanMonumentPiece::~OceanMonumentPiece() { mRoomDefinition.reset(); }
int OceanMonumentPieces::OceanMonumentPiece::getRoomIndex(int x, int y, int z) { return y * 25 + z * 5 + x; }
void OceanMonumentPieces::OceanMonumentPiece::staticCtor()
{
    prismarineState = Blocks::prismarine->defaultBlockState();
    prismarineBricksState = Blocks::prismarine->getBlockState(1);
    darkPrismarineState = Blocks::prismarine->getBlockState(2);
    seaLanternState = Blocks::sea_lantern->defaultBlockState();
    goldState = Blocks::gold_block->defaultBlockState();
}
void OceanMonumentPieces::OceanMonumentPiece::fjKeepReference(const boost::shared_ptr<RoomDefinition>& definition) { if (definition) mChildPieces.push_back(definition); }
void OceanMonumentPieces::OceanMonumentPiece::addAdditionalSaveData(DataOutputStream* output) { }
void OceanMonumentPieces::OceanMonumentPiece::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { }
void OceanMonumentPieces::OceanMonumentPiece::generateWaterBox(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1, int z1, bool existingOnly) {
    for (int x = x0; x <= x1; ++x)
        for (int y = y0; y <= y1; ++y)
            for (int z = z0; z <= z1; ++z)
            {
                if (existingOnly && level->isEmptyBlock(BlockPos(getWorldX(x, z), getWorldY(y), getWorldZ(x, z))))
                    continue;
                const BlockState* state = getWorldY(y) < level->getSeaLevel()
                    ? Blocks::WATER->defaultBlockState()
                    : Blocks::AIR->defaultBlockState();
                placeBlock(level, state, x, y, z, box);
            }
    }
void OceanMonumentPieces::OceanMonumentPiece::generateDefaultFloor(Level* level, BoundingBox* box, int x, int z, bool hasOpening)
{
    if (hasOpening)
    {
        generateBox(level, box, x, 0, z, x + 2, 0, z + 7, prismarineState, prismarineState, false);
        generateBox(level, box, x + 5, 0, z, x + 7, 0, z + 7, prismarineState, prismarineState, false);
        generateBox(level, box, x + 3, 0, z, x + 4, 0, z + 1, prismarineState, prismarineState, false);
        generateBox(level, box, x + 3, 0, z + 5, x + 4, 0, z + 7, prismarineState, prismarineState, false);
        generateBox(level, box, x + 3, 0, z + 2, x + 4, 0, z + 2, prismarineBricksState, prismarineBricksState, false);
        generateBox(level, box, x + 3, 0, z + 5, x + 4, 0, z + 5, prismarineBricksState, prismarineBricksState, false);
        generateBox(level, box, x + 2, 0, z + 3, x + 2, 0, z + 4, prismarineBricksState, prismarineBricksState, false);
        generateBox(level, box, x + 5, 0, z + 3, x + 5, 0, z + 4, prismarineBricksState, prismarineBricksState, false);
    }
    else
        generateBox(level, box, x, 0, z, x + 7, 0, z + 7, prismarineState, prismarineState, false);
}
void OceanMonumentPieces::OceanMonumentPiece::generateBoxOnFillOnly(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1, int z1, const BlockState* state) {
    generateBox(level, box, x0, y0, z0, x1, y1, z1, state, state, true);
    }
bool OceanMonumentPieces::OceanMonumentPiece::chunkIntersects(BoundingBox* box, int x0, int z0, int x1, int z1)
{
    int worldX0 = getWorldX(x0, z0);
    int worldZ0 = getWorldZ(x0, z0);
    int worldX1 = getWorldX(x1, z1);
    int worldZ1 = getWorldZ(x1, z1);
    int minX = worldX0;
    if (worldX1 < worldX0)
        minX = worldX1;
    int minZ = worldZ0;
    if (worldZ1 < worldZ0)
        minZ = worldZ1;
    if (worldX0 < worldX1)
        worldX0 = worldX1;
    if (worldZ1 < worldZ0)
        worldZ1 = worldZ0;
    return box->intersects(minX, minZ, worldX0, worldZ1);
}
bool OceanMonumentPieces::OceanMonumentPiece::spawnElder(Level* level, BoundingBox* box, int x, int y, int z)
{
    int worldX = getWorldX(x, z);
    int worldY = getWorldY(y);
    int worldZ = getWorldZ(x, z);
    if (!box->isInside(BlockPos(worldX, worldY, worldZ)))
        return false;
    boost::shared_ptr<ElderGuardian> elder(new ElderGuardian(level));
    elder->heal(elder->getMaxHealth());
    elder->moveTo((double)worldX + 0.5, (double)worldY, (double)worldZ + 0.5, 0.0f, 0.0f);
    level->addEntity(elder);
    return true;
}
OceanMonumentPieces::MonumentRoomFitter::~MonumentRoomFitter() {}
OceanMonumentPieces::FitSimpleRoom::~FitSimpleRoom() {}
bool OceanMonumentPieces::FitSimpleRoom::fits(const boost::shared_ptr<RoomDefinition>&) { return true; }
boost::shared_ptr<OceanMonumentPieces::OceanMonumentPiece> OceanMonumentPieces::FitSimpleRoom::create(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) {
    return boost::shared_ptr<OceanMonumentPiece>(new OceanMonumentSimpleRoom(o, d, r));
    }
OceanMonumentPieces::FitSimpleTopRoom::~FitSimpleTopRoom() {}
bool OceanMonumentPieces::FitSimpleTopRoom::fits(const boost::shared_ptr<RoomDefinition>& d)
{
    return !d->mOpenings[Direction::WEST->get3DDataValue()]
        && !d->mOpenings[Direction::EAST->get3DDataValue()]
        && !d->mOpenings[Direction::NORTH->get3DDataValue()]
        && !d->mOpenings[Direction::SOUTH->get3DDataValue()]
        && !d->mOpenings[Direction::UP->get3DDataValue()];
}
boost::shared_ptr<OceanMonumentPieces::OceanMonumentPiece> OceanMonumentPieces::FitSimpleTopRoom::create(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) {
    return boost::shared_ptr<OceanMonumentPiece>(new OceanMonumentSimpleTopRoom(o, d, r));
    }
OceanMonumentPieces::FitDoubleXRoom::~FitDoubleXRoom() {}
bool OceanMonumentPieces::FitDoubleXRoom::fits(const boost::shared_ptr<RoomDefinition>& d)
{
    if (!d->mOpenings[Direction::EAST->get3DDataValue()])
        return false;
    return !d->fjConnection(Direction::EAST)->mClaimed;
}
boost::shared_ptr<OceanMonumentPieces::OceanMonumentPiece> OceanMonumentPieces::FitDoubleXRoom::create(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) {
    return boost::shared_ptr<OceanMonumentPiece>(new OceanMonumentDoubleXRoom(o, d, r));
    }
OceanMonumentPieces::FitDoubleYRoom::~FitDoubleYRoom() {}
bool OceanMonumentPieces::FitDoubleYRoom::fits(const boost::shared_ptr<RoomDefinition>& d)
{
    if (!d->mOpenings[Direction::UP->get3DDataValue()])
        return false;
    return !d->fjConnection(Direction::UP)->mClaimed;
}
boost::shared_ptr<OceanMonumentPieces::OceanMonumentPiece> OceanMonumentPieces::FitDoubleYRoom::create(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) {
    return boost::shared_ptr<OceanMonumentPiece>(new OceanMonumentDoubleYRoom(o, d, r));
    }
OceanMonumentPieces::FitDoubleXYRoom::~FitDoubleXYRoom() {}
bool OceanMonumentPieces::FitDoubleXYRoom::fits(const boost::shared_ptr<RoomDefinition>& d)
{
    bool eastOpen = false;
    if (d->mOpenings[Direction::EAST->get3DDataValue()])
        eastOpen = !d->fjConnection(Direction::EAST)->mClaimed;
    if (eastOpen)
    {
        bool upOpen = false;
        if (d->mOpenings[Direction::UP->get3DDataValue()])
            upOpen = !d->fjConnection(Direction::UP)->mClaimed;
        if (upOpen)
        {
            boost::shared_ptr<RoomDefinition> east = d->fjConnection(Direction::EAST);
            bool cornerOpen = false;
            if (east->mOpenings[Direction::UP->get3DDataValue()])
                cornerOpen = !east->fjConnection(Direction::UP)->mClaimed;
            return cornerOpen;
        }
    }
    return false;
}
boost::shared_ptr<OceanMonumentPieces::OceanMonumentPiece> OceanMonumentPieces::FitDoubleXYRoom::create(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r)
{
    d->mClaimed = true;
    d->fjConnection(Direction::EAST)->mClaimed = true;
    d->fjConnection(Direction::UP)->mClaimed = true;
    d->fjConnection(Direction::EAST)->fjConnection(Direction::UP)->mClaimed = true;
    return boost::shared_ptr<OceanMonumentPiece>(new OceanMonumentDoubleXYRoom(o, d, r));
}
OceanMonumentPieces::FitDoubleYZRoom::~FitDoubleYZRoom() {}
bool OceanMonumentPieces::FitDoubleYZRoom::fits(const boost::shared_ptr<RoomDefinition>& d)
{
    bool northOpen = false;
    if (d->mOpenings[Direction::NORTH->get3DDataValue()])
        northOpen = !d->fjConnection(Direction::NORTH)->mClaimed;
    if (northOpen)
    {
        bool upOpen = false;
        if (d->mOpenings[Direction::UP->get3DDataValue()])
            upOpen = !d->fjConnection(Direction::UP)->mClaimed;
        if (upOpen)
        {
            boost::shared_ptr<RoomDefinition> north = d->fjConnection(Direction::NORTH);
            bool cornerOpen = false;
            if (north->mOpenings[Direction::UP->get3DDataValue()])
                cornerOpen = !north->fjConnection(Direction::UP)->mClaimed;
            return cornerOpen;
        }
    }
    return false;
}
boost::shared_ptr<OceanMonumentPieces::OceanMonumentPiece> OceanMonumentPieces::FitDoubleYZRoom::create(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r)
{
    d->mClaimed = true;
    d->fjConnection(Direction::NORTH)->mClaimed = true;
    d->fjConnection(Direction::UP)->mClaimed = true;
    d->fjConnection(Direction::NORTH)->fjConnection(Direction::UP)->mClaimed = true;
    return boost::shared_ptr<OceanMonumentPiece>(new OceanMonumentDoubleYZRoom(o, d, r));
}
OceanMonumentPieces::FitDoubleZRoom::~FitDoubleZRoom() {}
bool OceanMonumentPieces::FitDoubleZRoom::fits(const boost::shared_ptr<RoomDefinition>& d)
{
    if (!d->mOpenings[Direction::NORTH->get3DDataValue()])
        return false;
    return !d->fjConnection(Direction::NORTH)->mClaimed;
}
boost::shared_ptr<OceanMonumentPieces::OceanMonumentPiece> OceanMonumentPieces::FitDoubleZRoom::create(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r)
{
    boost::shared_ptr<RoomDefinition> room = d;
    bool northOpen = false;
    if (d->mOpenings[Direction::NORTH->get3DDataValue()])
        northOpen = !d->fjConnection(Direction::NORTH)->mClaimed;
    if (!northOpen)
        room = d->fjConnection(Direction::SOUTH);
    room->mClaimed = true;
    room->fjConnection(Direction::NORTH)->mClaimed = true;
    return boost::shared_ptr<OceanMonumentPiece>(new OceanMonumentDoubleZRoom(o, room, r));
}
OceanMonumentPieces::OceanMonumentCoreRoom::OceanMonumentCoreRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentCoreRoom::OceanMonumentCoreRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random&) : OceanMonumentPiece(1, o, d, 0, 0, 0) {}
OceanMonumentPieces::OceanMonumentCoreRoom::~OceanMonumentCoreRoom() {}
bool OceanMonumentPieces::OceanMonumentCoreRoom::postProcess(Level* l, Random& r, BoundingBox* b)
{

    generateBox(l, b, 1, 8, 0, 14, 8, 14, prismarineState, prismarineState, false);


    int i = 7;
    generateBox(l, b,  0, i,  0,  0, i, 15, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 15, i,  0, 15, i, 15, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b,  1, i,  0, 15, i,  0, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b,  1, i, 15, 14, i, 15, prismarineBricksState, prismarineBricksState, false);

    for (i = 1; i <= 6; ++i)
    {

        const BlockState* meta = (i == 2 || i == 6) ? prismarineState : prismarineBricksState;

        for (int j = 0; j <= 15; j += 15)
        {

            generateBox(l, b, j, i, 0, j, i, 1,   meta, meta, false);
            generateBox(l, b, j, i, 6, j, i, 9,   meta, meta,false);
            generateBox(l, b, j, i,14, j, i,15,   meta, meta,false);
        }
        generateBox(l, b,  1, i,  0,  1, i,  0, meta, meta,false);
        generateBox(l, b,  6, i,  0,  9, i,  0, meta, meta,false);
        generateBox(l, b, 14, i,  0, 14, i,  0, meta, meta,false);
        generateBox(l, b,  1, i, 15, 14, i, 15, meta, meta,false);
    }


    generateBox(l, b, 6, 3, 6, 9, 6, 9, darkPrismarineState, darkPrismarineState, false);

    generateBox(l, b, 7, 4, 7, 8, 5, 8, goldState, goldState, false);

    for (i = 3; i <= 6; i += 3)
    {
        for (int k = 6; k <= 9; k += 3)
        {
            placeBlock(l, seaLanternState, k, i, 6, b);
            placeBlock(l, seaLanternState, k, i, 9, b);
        }
    }


    generateBox(l, b,  5, 1,  6,  5, 2,  6, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  5, 1,  9,  5, 2,  9, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 10, 1,  6, 10, 2,  6, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 10, 1,  9, 10, 2,  9, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  6, 1,  5,  6, 2,  5, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  9, 1,  5,  9, 2,  5, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  6, 1, 10,  6, 2, 10, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  9, 1, 10,  9, 2, 10, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  5, 2,  5,  5, 6,  5, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  5, 2, 10,  5, 6, 10, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 10, 2,  5, 10, 6,  5, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 10, 2, 10, 10, 6, 10, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  5, 7,  1,  5, 7,  6, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 10, 7,  1, 10, 7,  6, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  5, 7,  9,  5, 7, 14, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 10, 7,  9, 10, 7, 14, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  1, 7,  5,  6, 7,  5, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  1, 7, 10,  6, 7, 10, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  9, 7,  5, 14, 7,  5, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  9, 7, 10, 14, 7, 10, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  2, 1,  2,  2, 1,  3, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  3, 1,  2,  3, 1,  2, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 13, 1,  2, 13, 1,  3, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 12, 1,  2, 12, 1,  2, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  2, 1, 12,  2, 1, 13, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  3, 1, 13,  3, 1, 13, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 13, 1, 12, 13, 1, 13, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 12, 1, 13, 12, 1, 13, prismarineBricksState, prismarineBricksState,false);

    return true;
}
StructurePiece* OceanMonumentPieces::OceanMonumentCoreRoom::Create() { return new OceanMonumentCoreRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentCoreRoom::GetType() { return eStructurePiece_OceanMonumentCoreRoom; }
OceanMonumentPieces::OceanMonumentDoubleXRoom::OceanMonumentDoubleXRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentDoubleXRoom::OceanMonumentDoubleXRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) : OceanMonumentPiece(1, o, d, 2, 1, 1)
{
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::EAST));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::EAST)->fjConnection(Direction::UP));
}
OceanMonumentPieces::OceanMonumentDoubleXRoom::~OceanMonumentDoubleXRoom() {}
bool OceanMonumentPieces::OceanMonumentDoubleXRoom::postProcess(Level* l, Random& r, BoundingBox* b)
{
    boost::shared_ptr<RoomDefinition> eastRoom = mRoomDefinition.get()->fjConnection(Direction::EAST);
    RoomDefinition* thisRoom = mRoomDefinition.get();

    if (mRoomDefinition->mIndex / 25 > 0)
    {
        generateDefaultFloor(l, b, 8, 0, eastRoom->mOpenings[Direction::DOWN->get3DDataValue()]);
        generateDefaultFloor(l, b, 0, 0, thisRoom->mOpenings[Direction::DOWN->get3DDataValue()]);
    }


    if (thisRoom->fjConnection(Direction::UP) == nullptr)
        generateBoxOnFillOnly(l, b, 1, 4, 1, 7, 4, 6, prismarineState);
    if (eastRoom->fjConnection(Direction::UP) == nullptr)
        generateBoxOnFillOnly(l, b, 8, 4, 1, 14, 4, 6, prismarineState);


    generateBox(l, b,  0, 3,  0,  0, 3,  7, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 15, 3,  0, 15, 3,  7, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  1, 3,  0, 15, 3,  0, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  1, 3,  7, 14, 3,  7, prismarineBricksState, prismarineBricksState,false);

    generateBox(l, b,  0, 2,  0,  0, 2,  7, prismarineState, prismarineState, false);
    generateBox(l, b, 15, 2,  0, 15, 2,  7, prismarineState, prismarineState, false);
    generateBox(l, b,  1, 2,  0, 15, 2,  0, prismarineState, prismarineState, false);
    generateBox(l, b,  1, 2,  7, 14, 2,  7, prismarineState, prismarineState, false);

    generateBox(l, b,  0, 1,  0,  0, 1,  7, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 15, 1,  0, 15, 1,  7, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  1, 1,  0, 15, 1,  0, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  1, 1,  7, 14, 1,  7, prismarineBricksState, prismarineBricksState,false);

    generateBox(l, b,  5, 1,  0, 10, 1,  4, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  6, 2,  0,  9, 2,  3, prismarineState, prismarineState, false);
    generateBox(l, b,  5, 3,  0, 10, 3,  4, prismarineBricksState, prismarineBricksState, false);


    placeBlock(l, seaLanternState, 6, 2, 3, b);
    placeBlock(l, seaLanternState, 9, 2, 3, b);

    if (thisRoom->mOpenings[Direction::SOUTH->get3DDataValue()]) generateWaterBox(l, b,  3, 1,  0,  4, 2,  0, false);
    if (thisRoom->mOpenings[Direction::NORTH->get3DDataValue()]) generateWaterBox(l, b,  3, 1,  7,  4, 2,  7, false);
    if (thisRoom->mOpenings[Direction::WEST->get3DDataValue()])  generateWaterBox(l, b,  0, 1,  3,  0, 2,  4, false);
    if (eastRoom->mOpenings[Direction::SOUTH->get3DDataValue()]) generateWaterBox(l, b, 11, 1,  0, 12, 2,  0, false);
    if (eastRoom->mOpenings[Direction::NORTH->get3DDataValue()]) generateWaterBox(l, b, 11, 1,  7, 12, 2,  7, false);
    if (eastRoom->mOpenings[Direction::EAST->get3DDataValue()])  generateWaterBox(l, b, 15, 1,  3, 15, 2,  4, false);

    return true;
}
StructurePiece* OceanMonumentPieces::OceanMonumentDoubleXRoom::Create() { return new OceanMonumentDoubleXRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentDoubleXRoom::GetType() { return eStructurePiece_OceanMonumentDoubleXRoom; }
OceanMonumentPieces::OceanMonumentDoubleXYRoom::OceanMonumentDoubleXYRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentDoubleXYRoom::OceanMonumentDoubleXYRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) : OceanMonumentPiece(1, o, d, 2, 2, 1)
{
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::EAST));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::EAST)->fjConnection(Direction::UP));
}
OceanMonumentPieces::OceanMonumentDoubleXYRoom::~OceanMonumentDoubleXYRoom() {}
bool OceanMonumentPieces::OceanMonumentDoubleXYRoom::postProcess(Level* l, Random& r, BoundingBox* b)
{
    RoomDefinition* r0 = mRoomDefinition.get();
    boost::shared_ptr<RoomDefinition> r1 = mRoomDefinition.get()->fjConnection(Direction::EAST);
    boost::shared_ptr<RoomDefinition> r2 = r0->fjConnection(Direction::UP);
    boost::shared_ptr<RoomDefinition> r3 = r1->fjConnection(Direction::UP);

    if (mRoomDefinition->mIndex / 25 > 0)
    {
        generateDefaultFloor(l, b, 8, 0, r1->mOpenings[Direction::DOWN->get3DDataValue()]);
        generateDefaultFloor(l, b, 0, 0, r0->mOpenings[Direction::DOWN->get3DDataValue()]);
    }


    if (r2->fjConnection(Direction::UP) == nullptr) generateBoxOnFillOnly(l, b, 1, 8, 1, 7, 8, 6, prismarineState);
    if (r3->fjConnection(Direction::UP) == nullptr) generateBoxOnFillOnly(l, b, 8, 8, 1, 14, 8, 6, prismarineState);

    for (int i = 1; i <= 7; ++i)
    {

        const BlockState* meta = (i == 2 || i == 6) ? prismarineState : prismarineBricksState;
        generateBox(l, b,  0, i,  0,  0, i,  7, meta, meta,false);
        generateBox(l, b, 15, i,  0, 15, i,  7, meta, meta,false);
        generateBox(l, b,  1, i,  0, 15, i,  0, meta, meta,false);
        generateBox(l, b,  1, i,  7, 14, i,  7, meta, meta,false);
    }


    generateBox(l, b,  2, 1,  3,  2, 7,  4, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  3, 1,  2,  4, 7,  2, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  3, 1,  5,  4, 7,  5, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 13, 1,  3, 13, 7,  4, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 11, 1,  2, 12, 7,  2, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 11, 1,  5, 12, 7,  5, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  5, 1,  3,  5, 3,  4, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 10, 1,  3, 10, 3,  4, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  5, 7,  2, 10, 7,  5, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  5, 5,  2,  5, 7,  2, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 10, 5,  2, 10, 7,  2, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  5, 5,  5,  5, 7,  5, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 10, 5,  5, 10, 7,  5, prismarineBricksState, prismarineBricksState,false);


    placeBlock(l, prismarineBricksState,  6, 6, 2, b);
    placeBlock(l, prismarineBricksState,  9, 6, 2, b);
    placeBlock(l, prismarineBricksState,  6, 6, 5, b);
    placeBlock(l, prismarineBricksState,  9, 6, 5, b);

    generateBox(l, b,  5, 4,  3,  6, 4,  4, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  9, 4,  3, 10, 4,  4, prismarineBricksState, prismarineBricksState,false);


    placeBlock(l, seaLanternState,  5, 4, 2, b);
    placeBlock(l, seaLanternState,  5, 4, 5, b);
    placeBlock(l, seaLanternState, 10, 4, 2, b);
    placeBlock(l, seaLanternState, 10, 4, 5, b);

    if (r0->mOpenings[Direction::SOUTH->get3DDataValue()]) generateWaterBox(l, b,  3, 1,  0,  4, 2,  0, false);
    if (r0->mOpenings[Direction::NORTH->get3DDataValue()]) generateWaterBox(l, b,  3, 1,  7,  4, 2,  7, false);
    if (r0->mOpenings[Direction::WEST->get3DDataValue()])  generateWaterBox(l, b,  0, 1,  3,  0, 2,  4, false);
    if (r1->mOpenings[Direction::SOUTH->get3DDataValue()]) generateWaterBox(l, b, 11, 1,  0, 12, 2,  0, false);
    if (r1->mOpenings[Direction::NORTH->get3DDataValue()]) generateWaterBox(l, b, 11, 1,  7, 12, 2,  7, false);
    if (r1->mOpenings[Direction::EAST->get3DDataValue()])  generateWaterBox(l, b, 15, 1,  3, 15, 2,  4, false);
    if (r2->mOpenings[Direction::SOUTH->get3DDataValue()]) generateWaterBox(l, b,  3, 5,  0,  4, 6,  0, false);
    if (r2->mOpenings[Direction::NORTH->get3DDataValue()]) generateWaterBox(l, b,  3, 5,  7,  4, 6,  7, false);
    if (r2->mOpenings[Direction::WEST->get3DDataValue()])  generateWaterBox(l, b,  0, 5,  3,  0, 6,  4, false);
    if (r3->mOpenings[Direction::SOUTH->get3DDataValue()]) generateWaterBox(l, b, 11, 5,  0, 12, 6,  0, false);
    if (r3->mOpenings[Direction::NORTH->get3DDataValue()]) generateWaterBox(l, b, 11, 5,  7, 12, 6,  7, false);
    if (r3->mOpenings[Direction::EAST->get3DDataValue()])  generateWaterBox(l, b, 15, 5,  3, 15, 6,  4, false);

    return true;
}
StructurePiece* OceanMonumentPieces::OceanMonumentDoubleXYRoom::Create() { return new OceanMonumentDoubleXYRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentDoubleXYRoom::GetType() { return eStructurePiece_OceanMonumentDoubleXYRoom; }
OceanMonumentPieces::OceanMonumentDoubleYRoom::OceanMonumentDoubleYRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentDoubleYRoom::OceanMonumentDoubleYRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) : OceanMonumentPiece(1, o, d, 1, 2, 1)
{
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP)->fjConnection(Direction::UP));
}
OceanMonumentPieces::OceanMonumentDoubleYRoom::~OceanMonumentDoubleYRoom() {}
bool OceanMonumentPieces::OceanMonumentDoubleYRoom::postProcess(Level* l, Random& r, BoundingBox* b)
{
    if (mRoomDefinition->mIndex / 25 > 0)
        generateDefaultFloor(l, b, 0, 0, mRoomDefinition->mOpenings[Direction::DOWN->get3DDataValue()]);

    boost::shared_ptr<RoomDefinition> upRoom = mRoomDefinition.get()->fjConnection(Direction::UP);

    if (upRoom->fjConnection(Direction::UP) == nullptr)
        generateBoxOnFillOnly(l, b, 1, 8, 1, 6, 8, 6, prismarineState);


    generateBox(l, b, 0, 4, 0, 0, 4, 7, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 7, 4, 0, 7, 4, 7, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 1, 4, 0, 6, 4, 0, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 1, 4, 7, 6, 4, 7, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 2, 4, 1, 2, 4, 2, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 1, 4, 2, 1, 4, 2, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 5, 4, 1, 5, 4, 2, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 6, 4, 2, 6, 4, 2, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 2, 4, 5, 2, 4, 6, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 1, 4, 5, 1, 4, 5, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 5, 4, 5, 5, 4, 6, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 6, 4, 5, 6, 4, 5, prismarineBricksState, prismarineBricksState,false);

    RoomDefinition* cur = mRoomDefinition.get();
    for (int i = 1; i <= 5; i += 4)
    {
        int j = 0;
        if (cur->mOpenings[Direction::SOUTH->get3DDataValue()]) {
            generateBox(l, b, 2, i, j, 2, i+2, j, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, 5, i, j, 5, i+2, j, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, 3, i+2, j, 4, i+2, j, prismarineBricksState, prismarineBricksState, false);
        } else {
            generateBox(l, b, 0, i, j, 7, i+2, j, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, 0, i+1, j, 7, i+1, j, prismarineState, prismarineState, false);
        }

        j = 7;
        if (cur->mOpenings[Direction::NORTH->get3DDataValue()]) {
            generateBox(l, b, 2, i, j, 2, i+2, j, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, 5, i, j, 5, i+2, j, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, 3, i+2, j, 4, i+2, j, prismarineBricksState, prismarineBricksState, false);
        } else {
            generateBox(l, b, 0, i, j, 7, i+2, j, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, 0, i+1, j, 7, i+1, j, prismarineState, prismarineState, false);
        }

        int k = 0;
        if (cur->mOpenings[Direction::WEST->get3DDataValue()]) {
            generateBox(l, b, k, i, 2, k, i+2, 2, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, k, i, 5, k, i+2, 5, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, k, i+2, 3, k, i+2, 4, prismarineBricksState, prismarineBricksState, false);
        } else {
            generateBox(l, b, k, i, 0, k, i+2, 7, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, k, i+1, 0, k, i+1, 7, prismarineState, prismarineState, false);
        }

        k = 7;
        if (cur->mOpenings[Direction::EAST->get3DDataValue()]) {
            generateBox(l, b, k, i, 2, k, i+2, 2, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, k, i, 5, k, i+2, 5, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, k, i+2, 3, k, i+2, 4, prismarineBricksState, prismarineBricksState, false);
        } else {
            generateBox(l, b, k, i, 0, k, i+2, 7, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, k, i+1, 0, k, i+1, 7, prismarineState, prismarineState, false);
        }

        cur = upRoom;
    }
    return true;
}
StructurePiece* OceanMonumentPieces::OceanMonumentDoubleYRoom::Create() { return new OceanMonumentDoubleYRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentDoubleYRoom::GetType() { return eStructurePiece_OceanMonumentDoubleYRoom; }
OceanMonumentPieces::OceanMonumentDoubleYZRoom::OceanMonumentDoubleYZRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentDoubleYZRoom::OceanMonumentDoubleYZRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) : OceanMonumentPiece(1, o, d, 1, 2, 2)
{
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP)->fjConnection(Direction::UP));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::NORTH));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::NORTH)->fjConnection(Direction::UP));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::NORTH)->fjConnection(Direction::UP)->fjConnection(Direction::UP));
}
OceanMonumentPieces::OceanMonumentDoubleYZRoom::~OceanMonumentDoubleYZRoom() {}
bool OceanMonumentPieces::OceanMonumentDoubleYZRoom::postProcess(Level* l, Random& r, BoundingBox* b)
{
    RoomDefinition* r0 = mRoomDefinition.get();
    boost::shared_ptr<RoomDefinition> r1 = mRoomDefinition.get()->fjConnection(Direction::NORTH);
    boost::shared_ptr<RoomDefinition> r2 = r1->fjConnection(Direction::UP);
    boost::shared_ptr<RoomDefinition> r3 = r0->fjConnection(Direction::UP);

    if (mRoomDefinition->mIndex / 25 > 0)
    {
        generateDefaultFloor(l, b, 0, 8, r1->mOpenings[Direction::DOWN->get3DDataValue()]);
        generateDefaultFloor(l, b, 0, 0, r0->mOpenings[Direction::DOWN->get3DDataValue()]);
    }


    if (r3->fjConnection(Direction::UP) == nullptr) generateBoxOnFillOnly(l, b, 1, 8, 1, 6, 8, 7, prismarineState);
    if (r2->fjConnection(Direction::UP) == nullptr) generateBoxOnFillOnly(l, b, 1, 8, 8, 6, 8, 14, prismarineState);

    for (int i = 1; i <= 7; ++i)
    {

        const BlockState* meta = (i == 2 || i == 6) ? prismarineState : prismarineBricksState;
        generateBox(l, b, 0, i,  0, 0, i, 15, meta, meta, false);
        generateBox(l, b, 7, i,  0, 7, i, 15, meta, meta, false);
        generateBox(l, b, 1, i,  0, 6, i,  0, meta, meta, false);
        generateBox(l, b, 1, i, 15, 6, i, 15, meta, meta, false);
    }

    for (int j = 1; j <= 7; ++j)
    {

        const BlockState* state = (j == 2 || j == 6) ? seaLanternState : darkPrismarineState;
        generateBox(l, b, 3, j, 7, 4, j, 8, state, state, false);
    }

    if (r0->mOpenings[Direction::SOUTH->get3DDataValue()]) generateWaterBox(l, b, 3, 1,  0, 4, 2,  0, false);
    if (r0->mOpenings[Direction::EAST->get3DDataValue()])  generateWaterBox(l, b, 7, 1,  3, 7, 2,  4, false);
    if (r0->mOpenings[Direction::WEST->get3DDataValue()])  generateWaterBox(l, b, 0, 1,  3, 0, 2,  4, false);
    if (r1->mOpenings[Direction::NORTH->get3DDataValue()]) generateWaterBox(l, b, 3, 1, 15, 4, 2, 15, false);
    if (r1->mOpenings[Direction::WEST->get3DDataValue()])  generateWaterBox(l, b, 0, 1, 11, 0, 2, 12, false);
    if (r1->mOpenings[Direction::EAST->get3DDataValue()])  generateWaterBox(l, b, 7, 1, 11, 7, 2, 12, false);
    if (r3->mOpenings[Direction::SOUTH->get3DDataValue()]) generateWaterBox(l, b, 3, 5,  0, 4, 6,  0, false);


    if (r3->mOpenings[Direction::EAST->get3DDataValue()])  {
        generateWaterBox(l, b, 7, 5,  3, 7, 6,  4, false);
        generateBox(l, b, 5, 4,  2, 6, 4,  5, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 6, 1,  2, 6, 3,  2, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 6, 1,  5, 6, 3,  5, prismarineBricksState, prismarineBricksState, false);
    }
    if (r3->mOpenings[Direction::WEST->get3DDataValue()])  {
        generateWaterBox(l, b, 0, 5,  3, 0, 6,  4, false);
        generateBox(l, b, 1, 4,  2, 2, 4,  5, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 1, 1,  2, 1, 3,  2, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 1, 1,  5, 1, 3,  5, prismarineBricksState, prismarineBricksState, false);
    }
    if (r2->mOpenings[Direction::NORTH->get3DDataValue()]) generateWaterBox(l, b, 3, 5, 15, 4, 6, 15, false);
    if (r2->mOpenings[Direction::WEST->get3DDataValue()])  {
        generateWaterBox(l, b, 0, 5, 11, 0, 6, 12, false);
        generateBox(l, b, 1, 4, 10, 2, 4, 13, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 1, 1, 10, 1, 3, 10, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 1, 1, 13, 1, 3, 13, prismarineBricksState, prismarineBricksState,false);
    }
    if (r2->mOpenings[Direction::EAST->get3DDataValue()])  {
        generateWaterBox(l, b, 7, 5, 11, 7, 6, 12, false);
        generateBox(l, b, 5, 4, 10, 6, 4, 13, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 6, 1, 10, 6, 3, 10, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 6, 1, 13, 6, 3, 13, prismarineBricksState, prismarineBricksState, false);
    }
    return true;
}
StructurePiece* OceanMonumentPieces::OceanMonumentDoubleYZRoom::Create() { return new OceanMonumentDoubleYZRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentDoubleYZRoom::GetType() { return eStructurePiece_OceanMonumentDoubleYZRoom; }
OceanMonumentPieces::OceanMonumentDoubleZRoom::OceanMonumentDoubleZRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentDoubleZRoom::OceanMonumentDoubleZRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) : OceanMonumentPiece(1, o, d, 1, 1, 2)
{
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::NORTH));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::NORTH)->fjConnection(Direction::UP));
}
OceanMonumentPieces::OceanMonumentDoubleZRoom::~OceanMonumentDoubleZRoom() {}
bool OceanMonumentPieces::OceanMonumentDoubleZRoom::postProcess(Level* l, Random& r, BoundingBox* b)
{
    RoomDefinition* r0 = mRoomDefinition.get();
    boost::shared_ptr<RoomDefinition> r1 = mRoomDefinition.get()->fjConnection(Direction::NORTH);

    if (mRoomDefinition->mIndex / 25 > 0)
    {
        generateDefaultFloor(l, b, 0, 8, r1->mOpenings[Direction::DOWN->get3DDataValue()]);
        generateDefaultFloor(l, b, 0, 0, r0->mOpenings[Direction::DOWN->get3DDataValue()]);
    }


    if (r0->fjConnection(Direction::UP) == nullptr) generateBoxOnFillOnly(l, b, 1, 4, 1, 6, 4, 7, prismarineState);
    if (r1->fjConnection(Direction::UP) == nullptr) generateBoxOnFillOnly(l, b, 1, 4, 8, 6, 4, 14, prismarineState);


    generateBox(l, b, 0, 3,  0, 0, 3, 15, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 7, 3,  0, 7, 3, 15, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 1, 3,  0, 7, 3,  0, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 1, 3, 15, 6, 3, 15, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 0, 2,  0, 0, 2, 15, prismarineState, prismarineState, false);
    generateBox(l, b, 7, 2,  0, 7, 2, 15, prismarineState, prismarineState, false);
    generateBox(l, b, 1, 2,  0, 7, 2,  0, prismarineState, prismarineState, false);
    generateBox(l, b, 1, 2, 15, 6, 2, 15, prismarineState, prismarineState, false);
    generateBox(l, b, 0, 1,  0, 0, 1, 15, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 7, 1,  0, 7, 1, 15, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 1, 1,  0, 7, 1,  0, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 1, 1, 15, 6, 1, 15, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 1, 1,  1, 1, 1,  2, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 6, 1,  1, 6, 1,  2, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 1, 3,  1, 1, 3,  2, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 6, 3,  1, 6, 3,  2, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 1, 1, 13, 1, 1, 14, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 6, 1, 13, 6, 1, 14, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 1, 3, 13, 1, 3, 14, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 6, 3, 13, 6, 3, 14, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 2, 1,  6, 2, 3,  6, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 5, 1,  6, 5, 3,  6, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 2, 1,  9, 2, 3,  9, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 5, 1,  9, 5, 3,  9, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 3, 2,  6, 4, 2,  6, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 3, 2,  9, 4, 2,  9, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 2, 2,  7, 2, 2,  8, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 5, 2,  7, 5, 2,  8, prismarineBricksState, prismarineBricksState, false);


    placeBlock(l, seaLanternState, 2, 2,  5, b);
    placeBlock(l, seaLanternState, 5, 2,  5, b);
    placeBlock(l, seaLanternState, 2, 2, 10, b);
    placeBlock(l, seaLanternState, 5, 2, 10, b);


    placeBlock(l, prismarineBricksState, 2, 3,  5, b);
    placeBlock(l, prismarineBricksState, 5, 3,  5, b);
    placeBlock(l, prismarineBricksState, 2, 3, 10, b);
    placeBlock(l, prismarineBricksState, 5, 3, 10, b);

    if (r0->mOpenings[Direction::SOUTH->get3DDataValue()]) generateWaterBox(l, b, 3, 1,  0, 4, 2,  0, false);
    if (r0->mOpenings[Direction::EAST->get3DDataValue()])  generateWaterBox(l, b, 7, 1,  3, 7, 2,  4, false);
    if (r0->mOpenings[Direction::WEST->get3DDataValue()])  generateWaterBox(l, b, 0, 1,  3, 0, 2,  4, false);
    if (r1->mOpenings[Direction::NORTH->get3DDataValue()]) generateWaterBox(l, b, 3, 1, 15, 4, 2, 15, false);
    if (r1->mOpenings[Direction::WEST->get3DDataValue()])  generateWaterBox(l, b, 0, 1, 11, 0, 2, 12, false);
    if (r1->mOpenings[Direction::EAST->get3DDataValue()])  generateWaterBox(l, b, 7, 1, 11, 7, 2, 12, false);
    return true;
}
StructurePiece* OceanMonumentPieces::OceanMonumentDoubleZRoom::Create() { return new OceanMonumentDoubleZRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentDoubleZRoom::GetType() { return eStructurePiece_OceanMonumentDoubleZRoom; }
OceanMonumentPieces::OceanMonumentSimpleRoom::OceanMonumentSimpleRoom() : OceanMonumentPiece(0), mVariant(0) {}
OceanMonumentPieces::OceanMonumentSimpleRoom::OceanMonumentSimpleRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) : OceanMonumentPiece(1, o, d, 0, 0, 0), mVariant(r.nextInt(3)) {}
OceanMonumentPieces::OceanMonumentSimpleRoom::~OceanMonumentSimpleRoom() {}
bool OceanMonumentPieces::OceanMonumentSimpleRoom::postProcess(Level* l, Random& r, BoundingBox* b)
{
    if (mRoomDefinition->mIndex / 25 > 0)
        generateDefaultFloor(l, b, 0, 0, mRoomDefinition->mOpenings[Direction::DOWN->get3DDataValue()]);

    if (mRoomDefinition.get()->fjConnection(Direction::UP) == nullptr)
        generateBoxOnFillOnly(l, b, 1, 4, 1, 6, 4, 6, prismarineState);

    bool placeChest = mVariant != 0 && r.nextBoolean() &&
                      !mRoomDefinition->mOpenings[Direction::DOWN->get3DDataValue()] && !mRoomDefinition->mOpenings[Direction::UP->get3DDataValue()] &&
                      mRoomDefinition->countOpenings() > 1;

    if (mVariant == 0)
    {

        generateBox(l, b, 0, 1, 0, 2, 1, 2, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 0, 3, 0, 2, 3, 2, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 0, 2, 0, 0, 2, 2, prismarineState, prismarineState, false);
        generateBox(l, b, 1, 2, 0, 2, 2, 0, prismarineState, prismarineState, false);
        placeBlock(l, seaLanternState, 1, 2, 1, b);
        generateBox(l, b, 5, 1, 0, 7, 1, 2, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 5, 3, 0, 7, 3, 2, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 7, 2, 0, 7, 2, 2, prismarineState, prismarineState, false);
        generateBox(l, b, 5, 2, 0, 6, 2, 0, prismarineState, prismarineState, false);
        placeBlock(l, seaLanternState, 6, 2, 1, b);
        generateBox(l, b, 0, 1, 5, 2, 1, 7, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 0, 3, 5, 2, 3, 7, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 0, 2, 5, 0, 2, 7, prismarineState, prismarineState, false);
        generateBox(l, b, 1, 2, 7, 2, 2, 7, prismarineState, prismarineState, false);
        placeBlock(l, seaLanternState, 1, 2, 6, b);
        generateBox(l, b, 5, 1, 5, 7, 1, 7, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 5, 3, 5, 7, 3, 7, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 7, 2, 5, 7, 2, 7, prismarineState, prismarineState, false);
        generateBox(l, b, 5, 2, 7, 6, 2, 7, prismarineState, prismarineState, false);
        placeBlock(l, seaLanternState, 6, 2, 6, b);

        if (mRoomDefinition->mOpenings[Direction::SOUTH->get3DDataValue()]) { generateBox(l, b, 3, 3, 0, 4, 3, 0, prismarineBricksState, prismarineBricksState,false); }
        else {
            generateBox(l, b, 3, 3, 0, 4, 3, 1, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, 3, 2, 0, 4, 2, 0, prismarineState, prismarineState, false);
            generateBox(l, b, 3, 1, 0, 4, 1, 1, prismarineBricksState, prismarineBricksState, false);
        }
        if (mRoomDefinition->mOpenings[Direction::NORTH->get3DDataValue()]) { generateBox(l, b, 3, 3, 7, 4, 3, 7, prismarineBricksState, prismarineBricksState,false); }
        else {
            generateBox(l, b, 3, 3, 6, 4, 3, 7, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, 3, 2, 7, 4, 2, 7, prismarineState, prismarineState, false);
            generateBox(l, b, 3, 1, 6, 4, 1, 7, prismarineBricksState, prismarineBricksState, false);
        }
        if (mRoomDefinition->mOpenings[Direction::WEST->get3DDataValue()]) { generateBox(l, b, 0, 3, 3, 0, 3, 4, prismarineBricksState, prismarineBricksState, false); }
        else {
            generateBox(l, b, 0, 3, 3, 1, 3, 4, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, 0, 2, 3, 0, 2, 4, prismarineState, prismarineState, false);
            generateBox(l, b, 0, 1, 3, 1, 1, 4, prismarineBricksState, prismarineBricksState, false);
        }
        if (mRoomDefinition->mOpenings[Direction::EAST->get3DDataValue()]) { generateBox(l, b, 7, 3, 3, 7, 3, 4, prismarineBricksState, prismarineBricksState, false); }
        else {
            generateBox(l, b, 6, 3, 3, 7, 3, 4, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, 7, 2, 3, 7, 2, 4, prismarineState, prismarineState, false);
            generateBox(l, b, 6, 1, 3, 7, 1, 4, prismarineBricksState, prismarineBricksState, false);
        }
    }
    else if (mVariant == 1)
    {
        generateBox(l, b, 2, 1, 2, 2, 3, 2, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 2, 1, 5, 2, 3, 5, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 5, 1, 5, 5, 3, 5, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 5, 1, 2, 5, 3, 2, prismarineBricksState, prismarineBricksState,false);
        placeBlock(l, seaLanternState, 2, 2, 2, b);
        placeBlock(l, seaLanternState, 2, 2, 5, b);
        placeBlock(l, seaLanternState, 5, 2, 5, b);
        placeBlock(l, seaLanternState, 5, 2, 2, b);
        generateBox(l, b, 0, 1, 0, 1, 3, 0, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 0, 1, 1, 0, 3, 1, prismarineBricksState, prismarineBricksState,false);
        generateBox(l, b, 0, 1, 7, 1, 3, 7, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 0, 1, 6, 0, 3, 6, prismarineBricksState, prismarineBricksState,false);
        generateBox(l, b, 6, 1, 7, 7, 3, 7, prismarineBricksState, prismarineBricksState,false);
        generateBox(l, b, 7, 1, 6, 7, 3, 6, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 6, 1, 0, 7, 3, 0, prismarineBricksState, prismarineBricksState,false);
        generateBox(l, b, 7, 1, 1, 7, 3, 1, prismarineBricksState, prismarineBricksState,false);

        placeBlock(l, prismarineState, 1, 2, 0, b);
        placeBlock(l, prismarineState, 0, 2, 1, b);
        placeBlock(l, prismarineState, 1, 2, 7, b);
        placeBlock(l, prismarineState, 0, 2, 6, b);
        placeBlock(l, prismarineState, 6, 2, 7, b);
        placeBlock(l, prismarineState, 7, 2, 6, b);
        placeBlock(l, prismarineState, 6, 2, 0, b);
        placeBlock(l, prismarineState, 7, 2, 1, b);

        if (!mRoomDefinition->mOpenings[Direction::SOUTH->get3DDataValue()]) { generateBox(l, b, 1, 3, 0, 6, 3, 0, prismarineBricksState, prismarineBricksState,false); generateBox(l, b, 1, 2, 0, 6, 2, 0, prismarineState, prismarineState, false); generateBox(l, b, 1, 1, 0, 6, 1, 0, prismarineBricksState, prismarineBricksState,false); }
        if (!mRoomDefinition->mOpenings[Direction::NORTH->get3DDataValue()]) { generateBox(l, b, 1, 3, 7, 6, 3, 7, prismarineBricksState, prismarineBricksState, false); generateBox(l, b, 1, 2, 7, 6, 2, 7, prismarineState, prismarineState, false); generateBox(l, b, 1, 1, 7, 6, 1, 7, prismarineBricksState, prismarineBricksState,false); }
        if (!mRoomDefinition->mOpenings[Direction::WEST->get3DDataValue()])  { generateBox(l, b, 0, 3, 1, 0, 3, 6, prismarineBricksState, prismarineBricksState, false); generateBox(l, b, 0, 2, 1, 0, 2, 6, prismarineState, prismarineState, false); generateBox(l, b, 0, 1, 1, 0, 1, 6, prismarineBricksState, prismarineBricksState,false); }
        if (!mRoomDefinition->mOpenings[Direction::EAST->get3DDataValue()])  { generateBox(l, b, 7, 3, 1, 7, 3, 6, prismarineBricksState, prismarineBricksState, false); generateBox(l, b, 7, 2, 1, 7, 2, 6, prismarineState, prismarineState, false); generateBox(l, b, 7, 1, 1, 7, 1, 6, prismarineBricksState, prismarineBricksState,false); }
    }
    else
    {
        generateBox(l, b, 0, 1, 0, 0, 1, 7, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 7, 1, 0, 7, 1, 7, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 1, 1, 0, 6, 1, 0, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 1, 1, 7, 6, 1, 7, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 0, 2, 0, 0, 2, 7, darkPrismarineState,darkPrismarineState,  false);
        generateBox(l, b, 7, 2, 0, 7, 2, 7, darkPrismarineState,darkPrismarineState,  false);
        generateBox(l, b, 1, 2, 0, 6, 2, 0, darkPrismarineState,darkPrismarineState,  false);
        generateBox(l, b, 1, 2, 7, 6, 2, 7, darkPrismarineState,darkPrismarineState,  false);
        generateBox(l, b, 0, 3, 0, 0, 3, 7, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 7, 3, 0, 7, 3, 7, prismarineBricksState, prismarineBricksState,false);
        generateBox(l, b, 1, 3, 0, 6, 3, 0, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 1, 3, 7, 6, 3, 7, prismarineBricksState, prismarineBricksState,false);
        generateBox(l, b, 0, 1, 3, 0, 2, 4, darkPrismarineState,darkPrismarineState,  false);
        generateBox(l, b, 7, 1, 3, 7, 2, 4, darkPrismarineState,darkPrismarineState,  false);
        generateBox(l, b, 3, 1, 0, 4, 2, 0, darkPrismarineState,darkPrismarineState,  false);
        generateBox(l, b, 3, 1, 7, 4, 2, 7, darkPrismarineState,darkPrismarineState,  false);
        if (mRoomDefinition->mOpenings[Direction::SOUTH->get3DDataValue()]) generateWaterBox(l, b, 3, 1, 0, 4, 2, 0, false);
        if (mRoomDefinition->mOpenings[Direction::NORTH->get3DDataValue()]) generateWaterBox(l, b, 3, 1, 7, 4, 2, 7, false);
        if (mRoomDefinition->mOpenings[Direction::WEST->get3DDataValue()])  generateWaterBox(l, b, 0, 1, 3, 0, 2, 4, false);
        if (mRoomDefinition->mOpenings[Direction::EAST->get3DDataValue()])  generateWaterBox(l, b, 7, 1, 3, 7, 2, 4, false);
    }

    if (placeChest)
    {
        generateBox(l, b, 3, 1, 3, 4, 1, 4, prismarineBricksState, prismarineBricksState,false);
        generateBox(l, b, 3, 2, 3, 4, 2, 4, prismarineState, prismarineState, false);
        generateBox(l, b, 3, 3, 3, 4, 3, 4, prismarineBricksState, prismarineBricksState, false);
    }
    return true;
}
StructurePiece* OceanMonumentPieces::OceanMonumentSimpleRoom::Create() { return new OceanMonumentSimpleRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentSimpleRoom::GetType() { return eStructurePiece_OceanMonumentSimpleRoom; }
OceanMonumentPieces::OceanMonumentSimpleTopRoom::OceanMonumentSimpleTopRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentSimpleTopRoom::OceanMonumentSimpleTopRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random&) : OceanMonumentPiece(1, o, d, 0, 0, 0) {}
OceanMonumentPieces::OceanMonumentSimpleTopRoom::~OceanMonumentSimpleTopRoom() {}
bool OceanMonumentPieces::OceanMonumentSimpleTopRoom::postProcess(Level* l, Random& r, BoundingBox* b)
{
    if (mRoomDefinition->mIndex / 25 > 0)
        generateDefaultFloor(l, b, 0, 0, mRoomDefinition->mOpenings[Direction::DOWN->get3DDataValue()]);

    if (mRoomDefinition.get()->fjConnection(Direction::UP) == nullptr)
        generateBox(l, b, 1, 4, 1, 6, 4, 6, prismarineState, prismarineState, false);


    for (int i = 1; i <= 6; ++i)
    for (int j = 1; j <= 6; ++j)
    {
        if (r.nextInt(3) != 0)
        {
            int k = 2 + (r.nextInt(4) == 0 ? 0 : 1);
            generateBox(l, b, i, k, j, i, 3, j, Blocks::sponge->getBlockState(1), Blocks::sponge->getBlockState(1), false);
        }
    }

    generateBox(l, b, 0, 1, 0, 0, 1, 7, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 7, 1, 0, 7, 1, 7, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 1, 1, 0, 6, 1, 0, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 1, 1, 7, 6, 1, 7, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 0, 2, 0, 0, 2, 7, darkPrismarineState, darkPrismarineState,false);
    generateBox(l, b, 7, 2, 0, 7, 2, 7, darkPrismarineState, darkPrismarineState, false);
    generateBox(l, b, 1, 2, 0, 6, 2, 0, darkPrismarineState, darkPrismarineState, false);
    generateBox(l, b, 1, 2, 7, 6, 2, 7, darkPrismarineState, darkPrismarineState, false);
    generateBox(l, b, 0, 3, 0, 0, 3, 7, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 7, 3, 0, 7, 3, 7, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 1, 3, 0, 6, 3, 0, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 1, 3, 7, 6, 3, 7, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 0, 1, 3, 0, 2, 4, darkPrismarineState, darkPrismarineState, false);
    generateBox(l, b, 7, 1, 3, 7, 2, 4, darkPrismarineState, darkPrismarineState, false);
    generateBox(l, b, 3, 1, 0, 4, 2, 0, darkPrismarineState, darkPrismarineState,false);
    generateBox(l, b, 3, 1, 7, 4, 2, 7, darkPrismarineState, darkPrismarineState, false);
    if (mRoomDefinition->mOpenings[Direction::SOUTH->get3DDataValue()]) generateWaterBox(l, b, 3, 1, 0, 4, 2, 0, false);

    return true;
}
StructurePiece* OceanMonumentPieces::OceanMonumentSimpleTopRoom::Create() { return new OceanMonumentSimpleTopRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentSimpleTopRoom::GetType() { return eStructurePiece_OceanMonumentSimpleTopRoom; }
OceanMonumentPieces::OceanMonumentEntryRoom::OceanMonumentEntryRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentEntryRoom::OceanMonumentEntryRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d) : OceanMonumentPiece(1, o, d, 0, 0, 0) {}
OceanMonumentPieces::OceanMonumentEntryRoom::~OceanMonumentEntryRoom() {}
bool OceanMonumentPieces::OceanMonumentEntryRoom::postProcess(Level* l, Random& r, BoundingBox* b)
{
    generateBox(l, b, 0, 1, 0, 2, 1, 2, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 5, 3, 0, 7, 3, 7, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 6, 2, 0, 7, 2, 7, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 0, 2, 0, 1, 2, 7, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 0, 1, 0, 0, 1, 7, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 7, 1, 0, 7, 1, 7, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 0, 1, 7, 7, 3, 7, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 1, 1, 0, 2, 3, 0, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 5, 1, 0, 6, 3, 0, prismarineBricksState, prismarineBricksState, false);

    if (mRoomDefinition->mOpenings[Direction::NORTH->get3DDataValue()]) generateWaterBox(l, b, 3, 1, 7, 4, 2, 7, false);
    if (mRoomDefinition->mOpenings[Direction::WEST->get3DDataValue()])  generateWaterBox(l, b, 0, 1, 3, 1, 2, 4, false);
    if (mRoomDefinition->mOpenings[Direction::EAST->get3DDataValue()])  generateWaterBox(l, b, 6, 1, 3, 7, 2, 4, false);
    return true;
}
StructurePiece* OceanMonumentPieces::OceanMonumentEntryRoom::Create()
{
    return new OceanMonumentEntryRoom();
}
EStructurePiece OceanMonumentPieces::OceanMonumentEntryRoom::GetType() {
    return eStructurePiece_OceanMonumentEntryRoom;
    }
OceanMonumentPieces::OceanMonumentPenthouse::OceanMonumentPenthouse() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentPenthouse::OceanMonumentPenthouse(const Direction* o, BoundingBox* b) : OceanMonumentPiece(o, b) {}
OceanMonumentPieces::OceanMonumentPenthouse::~OceanMonumentPenthouse() {}
bool OceanMonumentPieces::OceanMonumentPenthouse::postProcess(Level* l, Random& r, BoundingBox* b)
{
    generateBox(l, b,  2, -1,  2, 11, -1, 11, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b,  0, -1,  0,  1, -1, 11, prismarineState, prismarineState, false);
    generateBox(l, b, 12, -1,  0, 13, -1, 11, prismarineState, prismarineState, false);
    generateBox(l, b,  2, -1,  0, 11, -1,  1, prismarineState, prismarineState, false);
    generateBox(l, b,  2, -1, 12, 11, -1, 13, prismarineState, prismarineState, false);
    generateBox(l, b,  0,  0,  0,  0,  0, 13, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 13,  0,  0, 13,  0, 13, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b,  1,  0,  0, 12,  0,  0, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b,  1,  0, 13, 12,  0, 13, prismarineBricksState, prismarineBricksState, false);

    for (int i = 2; i <= 11; i += 3)
    {
        placeBlock(l, seaLanternState,  0, 0, i, b);
        placeBlock(l, seaLanternState, 13, 0, i, b);
        placeBlock(l, seaLanternState,  i, 0, 0, b);
    }

    generateBox(l, b,  2, 0,  3,  4, 0,  9, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b,  9, 0,  3, 11, 0,  9, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b,  4, 0,  9,  9, 0, 11, prismarineBricksState, prismarineBricksState,false);


    placeBlock(l, prismarineBricksState,  5, 0, 8, b);
    placeBlock(l, prismarineBricksState,  8, 0, 8, b);
    placeBlock(l, prismarineBricksState, 10, 0,10, b);
    placeBlock(l, prismarineBricksState,  3, 0,10, b);

    generateBox(l, b,  3, 0,  3,  3, 0,  7, darkPrismarineState, darkPrismarineState, false);
    generateBox(l, b, 10, 0,  3, 10, 0,  7, darkPrismarineState, darkPrismarineState, false);
    generateBox(l, b,  6, 0, 10,  7, 0, 10, darkPrismarineState, darkPrismarineState, false);

    int col = 3;
    for (int j = 0; j < 2; ++j)
    {
        for (int k = 2; k <= 8; k += 3)
            generateBox(l, b, col, 0, k, col, 2, k, prismarineBricksState, prismarineBricksState, false);
        col = 10;
    }
    generateBox(l, b,  5, 0, 10,  5, 2, 10, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b,  8, 0, 10,  8, 2, 10, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b,  6, -1,  7,  7, -1,  8, darkPrismarineState, darkPrismarineState, false);
    generateWaterBox(l, b,  6, -1,  3,  7, -1,  4, false);
    spawnElder(l, b, 6, 1, 6);
    return true;
}
StructurePiece* OceanMonumentPieces::OceanMonumentPenthouse::Create()
{
    return new OceanMonumentPenthouse();
}
EStructurePiece OceanMonumentPieces::OceanMonumentPenthouse::GetType() {
    return eStructurePiece_OceanMonumentPenthouse;
    }
OceanMonumentPieces::OceanMonumentWingRoom::OceanMonumentWingRoom(
    const Direction* o, BoundingBox* b, int i)
    : OceanMonumentPiece(o, b), mWingType(i & 1)
{
}

OceanMonumentPieces::OceanMonumentWingRoom::~OceanMonumentWingRoom()
{
}

bool OceanMonumentPieces::OceanMonumentWingRoom::postProcess(Level* l, Random& r, BoundingBox* b)
{
    if (mWingType == 0)
    {
        for (int i = 0; i < 4; ++i)
            generateBox(l, b, 10-i, 3-i, 20-i, 12+i, 3-i, 20, prismarineBricksState, prismarineBricksState,false);

        generateBox(l, b,  7, 0,  6, 15, 0, 16, prismarineBricksState, prismarineBricksState,false);
        generateBox(l, b,  6, 0,  6,  6, 3, 20, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 16, 0,  6, 16, 3, 20, prismarineBricksState, prismarineBricksState,false);
        generateBox(l, b,  7, 1,  7,  7, 1, 20, prismarineBricksState, prismarineBricksState,false);
        generateBox(l, b, 15, 1,  7, 15, 1, 20, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b,  7, 1,  6,  9, 3,  6, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 13, 1,  6, 15, 3,  6, prismarineBricksState, prismarineBricksState,false);
        generateBox(l, b,  8, 1,  7,  9, 1,  7, prismarineBricksState, prismarineBricksState,false);
        generateBox(l, b, 13, 1,  7, 14, 1,  7, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b,  9, 0,  5, 13, 0,  5, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 10, 0,  7, 12, 0,  7, darkPrismarineState, darkPrismarineState, false);
        generateBox(l, b,  8, 0, 10,  8, 0, 12, darkPrismarineState, darkPrismarineState,false);
        generateBox(l, b, 14, 0, 10, 14, 0, 12, darkPrismarineState, darkPrismarineState,false);

        for (int i1 = 18; i1 >= 7; i1 -= 3)
        {
            placeBlock(l, seaLanternState,  6, 3, i1, b);
            placeBlock(l, seaLanternState, 16, 3, i1, b);
        }
        placeBlock(l, seaLanternState, 10, 0, 10, b);
        placeBlock(l, seaLanternState, 12, 0, 10, b);
        placeBlock(l, seaLanternState, 10, 0, 12, b);
        placeBlock(l, seaLanternState, 12, 0, 12, b);
        placeBlock(l, seaLanternState,  8, 3,  6, b);
        placeBlock(l, seaLanternState, 14, 3,  6, b);


        placeBlock(l, prismarineBricksState,  4, 2,  4, b);
        placeBlock(l, seaLanternState,  4, 1,  4, b);
        placeBlock(l, prismarineBricksState,  4, 0,  4, b);
        placeBlock(l, prismarineBricksState, 18, 2,  4, b);
        placeBlock(l, seaLanternState, 18, 1,  4, b);
        placeBlock(l, prismarineBricksState, 18, 0,  4, b);
        placeBlock(l, prismarineBricksState,  4, 2, 18, b);
        placeBlock(l, seaLanternState,  4, 1, 18, b);
        placeBlock(l, prismarineBricksState,  4, 0, 18, b);
        placeBlock(l, prismarineBricksState, 18, 2, 18, b);
        placeBlock(l, seaLanternState, 18, 1, 18, b);
        placeBlock(l, prismarineBricksState, 18, 0, 18, b);
        placeBlock(l, prismarineBricksState,  9, 7, 20, b);
        placeBlock(l, prismarineBricksState, 13, 7, 20, b);

        generateBox(l, b,  6, 0, 21,  7, 4, 21, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 15, 0, 21, 16, 4, 21, prismarineBricksState, prismarineBricksState, false);
        spawnElder(l, b, 11, 2, 16);
    }
    else
    {
        generateBox(l, b,  9, 3, 18, 13, 3, 20, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b,  9, 0, 18,  9, 2, 18, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 13, 0, 18, 13, 2, 18, prismarineBricksState, prismarineBricksState, false);

        int j1 = 9;
        for (int q = 0; q < 2; ++q)
        {
            placeBlock(l, prismarineBricksState, j1, 6, 20, b);
            placeBlock(l, seaLanternState, j1, 5, 20, b);
            placeBlock(l, prismarineBricksState, j1, 4, 20, b);
            j1 = 13;
        }

        generateBox(l, b,  7, 3,  7, 15, 3, 14, prismarineBricksState, prismarineBricksState,false);
        j1 = 10;
        for (int k1 = 0; k1 < 2; ++k1)
        {
            generateBox(l, b, j1, 0, 10, j1, 6, 10, prismarineBricksState, prismarineBricksState, false);
            generateBox(l, b, j1, 0, 12, j1, 6, 12, prismarineBricksState, prismarineBricksState, false);
            placeBlock(l, seaLanternState, j1, 0, 10, b);
            placeBlock(l, seaLanternState, j1, 0, 12, b);
            placeBlock(l, seaLanternState, j1, 4, 10, b);
            placeBlock(l, seaLanternState, j1, 4, 12, b);
            j1 = 12;
        }

        j1 = 8;
        for (int l1 = 0; l1 < 2; ++l1)
        {
            generateBox(l, b, j1, 0,  7, j1, 2,  7, prismarineBricksState, prismarineBricksState,false);
            generateBox(l, b, j1, 0, 14, j1, 2, 14, prismarineBricksState, prismarineBricksState, false);
            j1 = 14;
        }

        generateBox(l, b,  8, 3,  8,  8, 3, 13, darkPrismarineState, prismarineBricksState,false);
        generateBox(l, b, 14, 3,  8, 14, 3, 13, darkPrismarineState, prismarineBricksState,false);
        spawnElder(l, b, 11, 5, 13);
    }
    return true;
}
EStructurePiece OceanMonumentPieces::OceanMonumentWingRoom::GetType()
{
    return eStructurePiece_OceanMonumentWingRoom;
    }
OceanMonumentPieces::MonumentBuilding::MonumentBuilding() : OceanMonumentPiece(0) {}
OceanMonumentPieces::MonumentBuilding::MonumentBuilding(Random& r, int x, int z, const Direction* o) : OceanMonumentPiece(0), mRoomGraphSize(0), mRoomCount(0)
{
    setOrientation(o);
    setBoundingBox(new BoundingBox(x, 39, z, x + 57, 61, z + 57));
    generateRoomGraph(r);
}
OceanMonumentPieces::MonumentBuilding::~MonumentBuilding()
{
    std::vector<OceanMonumentPiece*>::iterator it;
    for (it = mPieces.begin(); it != mPieces.end(); ++it)
    {
        if (*it != NULL)
            delete *it;
    }
}
int OceanMonumentPieces::MonumentBuilding::generateRoomGraph(Random& r)
{
    boost::shared_ptr<RoomDefinition> grid[75];
    for (int y = 0; y < 3; ++y)
        for (int z = 0; z < 5; ++z)
            for (int x = 0; x < 5; ++x)
            {
                if ((y < 2 && z < 4) || (y == 2 && x > 0 && x < 4 && z < 2))
                {
                    int index = OceanMonumentPiece::getRoomIndex(x, y, z);
                    grid[index].reset(new RoomDefinition(index));
                    fjKeepReference(grid[index]);
                }
            }
    mSourceRoom = grid[OceanMonumentPiece::getRoomIndex(2, 0, 0)];
    for (int y = 0; y < 3; ++y)
        for (int z = 0; z < 5; ++z)
            for (int x = 0; x < 5; ++x)
            {
                int index = OceanMonumentPiece::getRoomIndex(x, y, z);
                if (!grid[index]) continue;
                if (x + 1 < 5) { int n = OceanMonumentPiece::getRoomIndex(x + 1, y, z); if (grid[n]) grid[index]->setConnection(Direction::EAST, grid[n]); }
                if (z + 1 < 5) { int n = OceanMonumentPiece::getRoomIndex(x, y, z + 1); if (grid[n]) grid[index]->setConnection(Direction::SOUTH, grid[n]); }
                if (y + 1 < 3) { int n = OceanMonumentPiece::getRoomIndex(x, y + 1, z); if (grid[n]) grid[index]->setConnection(Direction::UP, grid[n]); }
            }
    boost::shared_ptr<RoomDefinition> specialUp(new RoomDefinition(1003));
    boost::shared_ptr<RoomDefinition> specialWing1(new RoomDefinition(1001));
    boost::shared_ptr<RoomDefinition> specialWing2(new RoomDefinition(1002));
    fjKeepReference(specialUp); fjKeepReference(specialWing1); fjKeepReference(specialWing2);
    grid[52]->setConnection(Direction::UP, specialUp);
    grid[25]->setConnection(Direction::SOUTH, specialWing1);
    grid[29]->setConnection(Direction::SOUTH, specialWing2);
    specialUp->mClaimed = specialWing1->mClaimed = specialWing2->mClaimed = true;
    std::vector<boost::shared_ptr<RoomDefinition> > rooms;
    for (int i = 0; i < 75; ++i) if (grid[i]) { grid[i]->updateOpenings(); rooms.push_back(grid[i]); }
    specialUp->updateOpenings();
    for (int i = static_cast<int>(rooms.size()) - 1; i > 0; --i)
    {
        int j = r.nextInt(i + 1);
        boost::shared_ptr<RoomDefinition> swap = rooms[i];
        rooms[i] = rooms[j];
        rooms[j] = swap;
    }
    int tag = 1;
    for (std::vector<boost::shared_ptr<RoomDefinition> >::iterator it = rooms.begin(); it != rooms.end(); ++it)
    {
        boost::shared_ptr<RoomDefinition> room = *it; int removed = 0, attempts = 0;
        while (removed < 2 && attempts++ < 5)
        {
            int f = r.nextInt(6); if (!room->mOpenings[f]) continue;
            boost::shared_ptr<RoomDefinition> other = room->fjConnection(f); if (!other) continue;
            const Direction* d = Direction::from3DDataValue(f); const Direction* od = d->getOpposite();
            room->mOpenings[f] = false; other->mOpenings[od->get3DDataValue()] = false;
            if (room->findSource(tag++) && other->findSource(tag++)) ++removed;
            else { room->mOpenings[f] = true; other->mOpenings[od->get3DDataValue()] = true; }
        }
    }
    rooms.push_back(specialUp); rooms.push_back(specialWing1); rooms.push_back(specialWing2);
    mSourceRoom->mClaimed = true;
    mPieces.push_back(new OceanMonumentEntryRoom(getOrientation(), mSourceRoom));
    mRootRoom = grid[getRoomIndex(r.nextInt(4), 0, 2)];
    mRootRoom->mClaimed = true;
    boost::shared_ptr<RoomDefinition> east = mRootRoom->fjConnection(Direction::EAST), north = mRootRoom->fjConnection(Direction::NORTH), up = mRootRoom->fjConnection(Direction::UP);
    east->mClaimed = north->mClaimed = up->mClaimed = true;
    east->fjConnection(Direction::NORTH)->mClaimed = true; east->fjConnection(Direction::UP)->mClaimed = true; north->fjConnection(Direction::UP)->mClaimed = true; east->fjConnection(Direction::NORTH)->fjConnection(Direction::UP)->mClaimed = true;
    mPieces.push_back(new OceanMonumentCoreRoom(getOrientation(), mRootRoom, r));
    for (std::vector<boost::shared_ptr<RoomDefinition> >::iterator it = rooms.begin(); it != rooms.end(); ++it)
    {
        boost::shared_ptr<RoomDefinition> room = *it; if (room->mClaimed || room->isSpecial()) continue;
        OceanMonumentPiece* piece = NULL;
        FitDoubleXYRoom xy; FitDoubleYZRoom yz; FitDoubleZRoom zz; FitDoubleXRoom xx; FitDoubleYRoom yy; FitSimpleTopRoom top;
        if (xy.fits(room)) { room->mClaimed = true; room->fjConnection(Direction::EAST)->mClaimed = true; room->fjConnection(Direction::UP)->mClaimed = true; room->fjConnection(Direction::EAST)->fjConnection(Direction::UP)->mClaimed = true; piece = new OceanMonumentDoubleXYRoom(getOrientation(), room, r); }
        else if (yz.fits(room)) { room->mClaimed = true; room->fjConnection(Direction::NORTH)->mClaimed = true; room->fjConnection(Direction::UP)->mClaimed = true; room->fjConnection(Direction::NORTH)->fjConnection(Direction::UP)->mClaimed = true; piece = new OceanMonumentDoubleYZRoom(getOrientation(), room, r); }
        else if (zz.fits(room)) { boost::shared_ptr<RoomDefinition> base = room->mOpenings[Direction::NORTH->get3DDataValue()] && !room->fjConnection(Direction::NORTH)->mClaimed ? room : room->fjConnection(Direction::SOUTH); base->mClaimed = true; base->fjConnection(Direction::NORTH)->mClaimed = true; piece = new OceanMonumentDoubleZRoom(getOrientation(), base, r); }
        else if (xx.fits(room)) { room->mClaimed = true; room->fjConnection(Direction::EAST)->mClaimed = true; piece = new OceanMonumentDoubleXRoom(getOrientation(), room, r); }
        else if (yy.fits(room)) { room->mClaimed = true; room->fjConnection(Direction::UP)->mClaimed = true; piece = new OceanMonumentDoubleYRoom(getOrientation(), room, r); }
        else if (top.fits(room)) { room->mClaimed = true; piece = new OceanMonumentSimpleTopRoom(getOrientation(), room, r); }
        else { room->mClaimed = true; piece = new OceanMonumentSimpleRoom(getOrientation(), room, r); }
        if (piece) mPieces.push_back(piece);
    }
    int baseX = getWorldX(9, 22), baseZ = getWorldZ(9, 22), baseY = getBoundingBox()->y0;
    for (std::vector<OceanMonumentPiece*>::iterator it = mPieces.begin(); it != mPieces.end(); ++it) (*it)->getBoundingBox()->move(baseX, baseY, baseZ);
    int seed = r.nextInt();
    mPieces.push_back(new OceanMonumentWingRoom(getOrientation(), new BoundingBox(getWorldX(1,1),getWorldY(1),getWorldZ(1,1),getWorldX(23,21),getWorldY(8),getWorldZ(23,21)), seed++));
    mPieces.push_back(new OceanMonumentWingRoom(getOrientation(), new BoundingBox(getWorldX(34,1),getWorldY(1),getWorldZ(34,1),getWorldX(56,21),getWorldY(8),getWorldZ(56,21)), seed++));
    mPieces.push_back(new OceanMonumentPenthouse(getOrientation(), new BoundingBox(getWorldX(22,22),getWorldY(13),getWorldZ(22,22),getWorldX(35,35),getWorldY(17),getWorldZ(35,35))));
    mRoomGraphSize = 75; mRoomCount = (int)mPieces.size(); return mRoomGraphSize;
} void OceanMonumentPieces::MonumentBuilding::generateWing(bool wide, int offX, Level* l, Random& r, BoundingBox* b){
    if (!chunkIntersects(b, offX, 0, offX + 23, 20)) return;

    generateBox(l, b, offX+0, 0, 0, offX+24, 0, 20, prismarineState, prismarineState, false);
    generateWaterBox(l, b, offX+0, 1, 0, offX+24, 10, 20, false);

    for (int j = 0; j < 4; ++j)
    {
        generateBox(l, b, offX+j,    j+1, j,    offX+j,    j+1, 20,   prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, offX+j+7,  j+5, j+7,  offX+j+7,  j+5, 20,   prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, offX+17-j, j+5, j+7,  offX+17-j, j+5, 20,   prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, offX+24-j, j+1, j,    offX+24-j, j+1, 20,   prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, offX+j+1,  j+1, j,    offX+23-j, j+1, j,    prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, offX+j+8,  j+5, j+7,  offX+16-j, j+5, j+7,  prismarineBricksState, prismarineBricksState, false);
    }

    generateBox(l, b, offX+4, 4, 4, offX+6, 4, 20, prismarineState, prismarineState, false);
    generateBox(l, b, offX+7, 4, 4, offX+17, 4, 6, prismarineState, prismarineState, false);
    generateBox(l, b, offX+18, 4, 4, offX+20, 4, 20, prismarineState, prismarineState, false);
    generateBox(l, b, offX+11, 8, 11, offX+13, 8, 20, prismarineState, prismarineState, false);
    placeBlock(l, prismarineBricksState, offX + 12, 9, 12, b);
    placeBlock(l, prismarineBricksState, offX + 12, 9, 15, b);
    placeBlock(l, prismarineBricksState, offX + 12, 9, 18, b);

    int j1 = wide ? offX+19 : offX+5;
    int k  = wide ? offX+5  : offX+19;

    for (int q = 20; q >= 5; q -= 3)
        placeBlock(l, prismarineBricksState, j1, 5, q, b);
    for (int k1 = 19; k1 >= 7; k1 -= 3)
        placeBlock(l, prismarineBricksState, k, 5, k1, b);
    for (int l1 = 0; l1 < 4; ++l1)
    {
        int i1 = wide ? offX + (24 - (17 - l1*3)) : offX + 17 - l1*3;
        placeBlock(l, prismarineBricksState, i1, 5, 5, b);
    }
    placeBlock(l, prismarineBricksState, k, 5, 5, b);

    generateBox(l, b, offX+11, 1, 12, offX+13, 7, 12, prismarineState, prismarineState, false);
    generateBox(l, b, offX+12, 1, 11, offX+12, 7, 13, prismarineState, prismarineState, false);
} void OceanMonumentPieces::MonumentBuilding::generateEntranceArchs(Level* l, Random& r, BoundingBox* b){
    if (!chunkIntersects(b, 22, 5, 35, 17)) return;

    generateWaterBox(l, b, 25, 0, 0, 32, 8, 20, false);
    for (int i = 0; i < 4; ++i)
    {
        generateBox(l, b, 24, 2, 5+i*4, 24, 4, 5+i*4, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 22, 4, 5+i*4, 23, 4, 5+i*4, prismarineBricksState, prismarineBricksState,false);
        placeBlock(l, prismarineBricksState, 25, 5, 5 + i * 4, b);
        placeBlock(l, prismarineBricksState, 26, 6, 5 + i * 4, b);
        placeBlock(l, seaLanternState, 26, 5, 5+i*4, b);
        generateBox(l, b, 33, 2, 5+i*4, 33, 4, 5+i*4, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 34, 4, 5+i*4, 35, 4, 5+i*4, prismarineBricksState, prismarineBricksState, false);
        placeBlock(l, prismarineBricksState, 32, 5, 5 + i * 4, b);
        placeBlock(l, prismarineBricksState, 31, 6, 5 + i * 4, b);
        placeBlock(l, seaLanternState, 31, 5, 5+i*4, b);
        generateBox(l, b, 27, 6, 5+i*4, 30, 6, 5+i*4, prismarineState, prismarineState, false);
    }
} void OceanMonumentPieces::MonumentBuilding::generateEntranceWall(Level* l, Random& r, BoundingBox* b){
    if (!chunkIntersects(b, 15, 20, 42, 21)) return;

    generateBox(l, b, 15, 0, 21, 42, 0, 21, prismarineState, prismarineState, false);
    generateWaterBox(l, b, 26, 1, 21, 31, 3, 21, false);
    generateBox(l, b, 21, 12, 21, 36, 12, 21, prismarineState, prismarineState, false);
    generateBox(l, b, 17, 11, 21, 40, 11, 21, prismarineState, prismarineState, false);
    generateBox(l, b, 16, 10, 21, 41, 10, 21, prismarineState, prismarineState, false);
    generateBox(l, b, 15,  7, 21, 42,  9, 21, prismarineState, prismarineState, false);
    generateBox(l, b, 16,  6, 21, 41,  6, 21, prismarineState, prismarineState, false);
    generateBox(l, b, 17,  5, 21, 40,  5, 21, prismarineState, prismarineState, false);
    generateBox(l, b, 21,  4, 21, 36,  4, 21, prismarineState, prismarineState, false);
    generateBox(l, b, 22,  3, 21, 26,  3, 21, prismarineState, prismarineState, false);
    generateBox(l, b, 31,  3, 21, 35,  3, 21, prismarineState, prismarineState, false);
    generateBox(l, b, 23,  2, 21, 25,  2, 21, prismarineState, prismarineState, false);
    generateBox(l, b, 32,  2, 21, 34,  2, 21, prismarineState, prismarineState, false);
    generateBox(l, b, 28,  4, 20, 29,  4, 21, prismarineBricksState, prismarineBricksState, false);
    placeBlock(l, prismarineBricksState, 27, 3, 21, b);
    placeBlock(l, prismarineBricksState, 26, 2, 21, b);
    placeBlock(l, prismarineBricksState, 31, 2, 21, b);
    placeBlock(l, prismarineBricksState, 25, 1, 21, b);
    placeBlock(l, prismarineBricksState, 32, 1, 21, b);
    placeBlock(l, prismarineBricksState, 30, 3, 21, b);

    for (int i = 0; i < 7; ++i) {
        placeBlock(l, darkPrismarineState, 28-i, 6+i, 21, b);
        placeBlock(l, darkPrismarineState, 29+i, 6+i, 21, b);
    }
    for (int j = 0; j < 4; ++j) {
        placeBlock(l, darkPrismarineState, 28-j, 9+j, 21, b);
        placeBlock(l, darkPrismarineState, 29+j, 9+j, 21, b);
    }
    placeBlock(l, darkPrismarineState, 28, 12, 21, b);
    placeBlock(l, darkPrismarineState, 29, 12, 21, b);

    for (int k = 0; k < 3; ++k) {
        placeBlock(l, darkPrismarineState, 22-k*2,  8, 21, b);
        placeBlock(l, darkPrismarineState, 22-k*2,  9, 21, b);
        placeBlock(l, darkPrismarineState, 35+k*2,  8, 21, b);
        placeBlock(l, darkPrismarineState, 35+k*2,  9, 21, b);
    }

    generateWaterBox(l, b, 15, 13, 21, 42, 15, 21, false);
    generateWaterBox(l, b, 15,  1, 21, 15,  6, 21, false);
    generateWaterBox(l, b, 16,  1, 21, 16,  5, 21, false);
    generateWaterBox(l, b, 17,  1, 21, 20,  4, 21, false);
    generateWaterBox(l, b, 21,  1, 21, 21,  3, 21, false);
    generateWaterBox(l, b, 22,  1, 21, 22,  2, 21, false);
    generateWaterBox(l, b, 23,  1, 21, 24,  1, 21, false);
    generateWaterBox(l, b, 42,  1, 21, 42,  6, 21, false);
    generateWaterBox(l, b, 41,  1, 21, 41,  5, 21, false);
    generateWaterBox(l, b, 37,  1, 21, 40,  4, 21, false);
    generateWaterBox(l, b, 36,  1, 21, 36,  3, 21, false);
    generateWaterBox(l, b, 33,  1, 21, 34,  1, 21, false);
    generateWaterBox(l, b, 35,  1, 21, 35,  2, 21, false);
} void OceanMonumentPieces::MonumentBuilding::generateRoofPiece(Level* l, Random& r, BoundingBox* b){
    if (!chunkIntersects(b, 21, 21, 36, 36)) return;

    generateBox(l, b, 21, 0, 22, 36, 0, 36, prismarineState, prismarineState, false);
    generateWaterBox(l, b, 21, 1, 22, 36, 23, 36, false);

    for (int i = 0; i < 4; ++i) {
        generateBox(l, b, 21+i, 13+i, 21+i, 36-i, 13+i, 21+i, prismarineBricksState, prismarineBricksState,false);
        generateBox(l, b, 21+i, 13+i, 36-i, 36-i, 13+i, 36-i, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 21+i, 13+i, 22+i, 21+i, 13+i, 35-i, prismarineBricksState, prismarineBricksState,false);
        generateBox(l, b, 36-i, 13+i, 22+i, 36-i, 13+i, 35-i, prismarineBricksState, prismarineBricksState, false);
    }

    generateBox(l, b, 25, 16, 25, 32, 16, 32, prismarineState, prismarineState, false);
    generateBox(l, b, 25, 17, 25, 25, 19, 25, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 32, 17, 25, 32, 19, 25, prismarineBricksState, prismarineBricksState,false);
    generateBox(l, b, 25, 17, 32, 25, 19, 32, prismarineBricksState, prismarineBricksState, false);
    generateBox(l, b, 32, 17, 32, 32, 19, 32, prismarineBricksState, prismarineBricksState,false);
    placeBlock(l, prismarineBricksState, 26, 20, 26, b);
    placeBlock(l, prismarineBricksState, 27, 21, 27, b);
    placeBlock(l, seaLanternState, 27, 20, 27, b);
    placeBlock(l, prismarineBricksState, 26, 20, 31, b);
    placeBlock(l, prismarineBricksState, 27, 21, 30, b);
    placeBlock(l, seaLanternState, 27, 20, 30, b);
    placeBlock(l, prismarineBricksState, 31, 20, 31, b);
    placeBlock(l, prismarineBricksState, 30, 21, 30, b);
    placeBlock(l, seaLanternState, 30, 20, 30, b);
    placeBlock(l, prismarineBricksState, 31, 20, 26, b);
    placeBlock(l, prismarineBricksState, 30, 21, 27, b);
    placeBlock(l, seaLanternState, 30, 20, 27, b);
    generateBox(l, b, 28, 21, 27, 29, 21, 27, prismarineState, prismarineState, false);
    generateBox(l, b, 27, 21, 28, 27, 21, 29, prismarineState, prismarineState, false);
    generateBox(l, b, 28, 21, 30, 29, 21, 30, prismarineState, prismarineState, false);
    generateBox(l, b, 30, 21, 28, 30, 21, 29, prismarineState, prismarineState, false);
} void OceanMonumentPieces::MonumentBuilding::generateLowerWall(Level* l, Random& r, BoundingBox* b){
    if (chunkIntersects(b, 0, 21, 6, 58))
    {
        generateBox(l, b, 0, 0, 21, 6, 0, 57, prismarineState, prismarineState, false);
        generateWaterBox(l, b, 0, 1, 21, 6, 7, 57, false);
        generateBox(l, b, 4, 4, 21, 6, 4, 53, prismarineState, prismarineState, false);
        for (int i = 0; i < 4; ++i)
            generateBox(l, b, i, i+1, 21, i, i+1, 57-i, prismarineBricksState, prismarineBricksState, false);
        for (int j = 23; j < 53; j += 3)
            placeBlock(l, prismarineBricksState, 5, 5, j, b);
        placeBlock(l, prismarineBricksState, 5, 5, 52, b);
        generateBox(l, b, 4, 1, 52, 6, 3, 52, prismarineState, prismarineState, false);
        generateBox(l, b, 5, 1, 51, 5, 3, 53, prismarineState, prismarineState, false);
    }
    if (chunkIntersects(b, 51, 21, 58, 58))
    {
        generateBox(l, b, 51, 0, 21, 57, 0, 57, prismarineState, prismarineState, false);
        generateWaterBox(l, b, 51, 1, 21, 57, 7, 57, false);
        generateBox(l, b, 51, 4, 21, 53, 4, 53, prismarineState, prismarineState, false);
        for (int q = 0; q < 4; ++q)
            generateBox(l, b, 57-q, q+1, 21, 57-q, q+1, 57-q, prismarineBricksState, prismarineBricksState, false);
        for (int i1 = 23; i1 < 53; i1 += 3)
            placeBlock(l, prismarineBricksState, 52, 5, i1, b);
        placeBlock(l, prismarineBricksState, 52, 5, 52, b);
        generateBox(l, b, 51, 1, 52, 53, 3, 52, prismarineState, prismarineState, false);
        generateBox(l, b, 52, 1, 51, 52, 3, 53, prismarineState, prismarineState, false);
    }
    if (chunkIntersects(b, 0, 51, 57, 57))
    {
        generateBox(l, b, 7, 0, 51, 50, 0, 57, prismarineState, prismarineState, false);
        generateWaterBox(l, b, 7, 1, 51, 50, 10, 57, false);
        for (int j1 = 0; j1 < 4; ++j1)
            generateBox(l, b, j1+1, j1+1, 57-j1, 56-j1, j1+1, 57-j1, prismarineBricksState, prismarineBricksState, false);
    }
} void OceanMonumentPieces::MonumentBuilding::generateMiddleWall(Level* l, Random& r, BoundingBox* b){
    if (chunkIntersects(b, 7, 21, 13, 54))
    {
        generateBox(l, b, 7, 0, 21, 13, 0, 50, prismarineState, prismarineState, false);
        generateWaterBox(l, b, 7, 1, 21, 13, 10, 50, false);
        generateBox(l, b, 11, 8, 21, 13, 8, 53, prismarineState, prismarineState, false);
        for (int i = 0; i < 4; ++i)
            generateBox(l, b, i+7, i+5, 21, i+7, i+5, 54, prismarineBricksState, prismarineBricksState, false);
        for (int j = 21; j <= 45; j += 3)
            placeBlock(l, prismarineBricksState, 12, 9, j, b);
    }
    if (chunkIntersects(b, 44, 21, 50, 54))
    {
        generateBox(l, b, 44, 0, 21, 50, 0, 50, prismarineState, prismarineState, false);
        generateWaterBox(l, b, 44, 1, 21, 50, 10, 50, false);
        generateBox(l, b, 44, 8, 21, 46, 8, 53, prismarineState, prismarineState, false);
        for (int k = 0; k < 4; ++k)
            generateBox(l, b, 50-k, k+5, 21, 50-k, k+5, 54, prismarineBricksState, prismarineBricksState, false);
        for (int q = 21; q <= 45; q += 3)
            placeBlock(l, prismarineBricksState, 45, 9, q, b);
    }
    if (chunkIntersects(b, 8, 44, 49, 54))
    {
        generateBox(l, b, 14, 0, 44, 43, 0, 50, prismarineState, prismarineState, false);
        generateWaterBox(l, b, 14, 1, 44, 43, 10, 50, false);
        for (int i1 = 12; i1 <= 45; i1 += 3)
        {
            placeBlock(l, prismarineBricksState, i1, 9, 45, b);
            placeBlock(l, prismarineBricksState, i1, 9, 52, b);
            if (i1==12||i1==18||i1==24||i1==33||i1==39||i1==45)
            {
                placeBlock(l, prismarineBricksState, i1, 9,  47, b);
                placeBlock(l, prismarineBricksState, i1, 9,  50, b);
                placeBlock(l, prismarineBricksState, i1, 10, 45, b);
                placeBlock(l, prismarineBricksState,  i1, 10, 46, b);
                placeBlock(l, prismarineBricksState,  i1, 10, 51, b);
                placeBlock(l, prismarineBricksState,  i1, 10, 52, b);
                placeBlock(l, prismarineBricksState,  i1, 11, 47, b);
                placeBlock(l, prismarineBricksState,  i1, 11, 50, b);
                placeBlock(l, prismarineBricksState,  i1, 12, 48, b);
                placeBlock(l, prismarineBricksState, i1, 12, 49, b);

            }
        }
        for (int j1 = 0; j1 < 3; ++j1)
        generateBox(l, b, 7+j1, 5+j1, 54, 50-j1, 5+j1, 54, prismarineState, prismarineState, false);
        generateBox(l, b, 10, 8, 54, 47, 8, 54, prismarineBricksState, prismarineBricksState, false);
        generateBox(l, b, 14, 8, 44, 43, 8, 53, prismarineState, prismarineState, false);
    }
} void OceanMonumentPieces::MonumentBuilding::generateUpperWall(Level* l, Random& r, BoundingBox* b){
    if (chunkIntersects(b, 14, 21, 20, 43))
    {
        generateBox(l, b, 14, 0, 21, 20, 0, 43, prismarineState, prismarineState, false);
        generateWaterBox(l, b, 14, 1, 22, 20, 14, 43, false);
        generateBox(l, b, 18, 12, 22, 20, 12, 39, prismarineState, prismarineState, false);
        generateBox(l, b, 18, 12, 21, 20, 12, 21, prismarineBricksState, prismarineBricksState, false);
        for (int i = 0; i < 4; ++i)
            generateBox(l, b, i+14, i+9, 21, i+14, i+9, 43-i, prismarineBricksState, prismarineBricksState, false);
        for (int j = 23; j <= 39; j += 3)
            placeBlock(l, prismarineBricksState, 19, 13, j, b);
    }
    if (chunkIntersects(b, 37, 21, 43, 43))
    {
        generateBox(l, b, 37, 0, 21, 43, 0, 43, prismarineState, prismarineState, false);
        generateWaterBox(l, b, 37, 1, 22, 43, 14, 43, false);
        generateBox(l, b, 37, 12, 22, 39, 12, 39, prismarineState, prismarineState, false);
        generateBox(l, b, 37, 12, 21, 39, 12, 21, prismarineBricksState, prismarineBricksState, false);
        for (int k = 0; k < 4; ++k)
            generateBox(l, b, 43-k, k+9, 21, 43-k, k+9, 43-k, prismarineBricksState, prismarineBricksState, false);
        for (int q = 23; q <= 39; q += 3)
            placeBlock(l, prismarineBricksState,  38, 13, q, b);
    }
    if (chunkIntersects(b, 15, 37, 42, 43))
    {
        generateBox(l, b, 21, 0, 37, 36, 0, 43, prismarineState, prismarineState, false);
        generateWaterBox(l, b, 21, 1, 37, 36, 14, 43, false);
        generateBox(l, b, 21, 12, 37, 36, 12, 39, prismarineState, prismarineState, false);
        for (int i1 = 0; i1 < 4; ++i1)
            generateBox(l, b, 15+i1, i1+9, 43-i1, 42-i1, i1+9, 43-i1, prismarineBricksState, prismarineBricksState,false);
        for (int j1 = 21; j1 <= 36; j1 += 3)
            placeBlock(l, prismarineBricksState, j1, 13, 38, b);
    }
} bool OceanMonumentPieces::MonumentBuilding::postProcess(Level* l, Random& r, BoundingBox* b) {
    int seaLevel = l->getSeaLevel();
    if (seaLevel < 64)
        seaLevel = 64;
    generateWaterBox(l, b, 0, 0, 0, 58, seaLevel - getBoundingBox()->y0, 58, false);
    generateWing(false, 0, l, r, b);
    generateWing(true, 33, l, r, b);
    generateEntranceArchs(l, r, b);
    generateEntranceWall(l, r, b);
    generateRoofPiece(l, r, b);
    generateLowerWall(l, r, b);
    generateMiddleWall(l, r, b);
    generateUpperWall(l, r, b);
    generateWaterBox(l, b, -1, 0, -1, 57, 23, 58, false);
    generateWaterBox(l, b, 58, 0, -1, 58, 23, 58, false);
    generateWaterBox(l, b, -2, 2, -2, -2, 23, 59, false);
    generateWaterBox(l, b, 59, 2, -2, 59, 23, 59, false);
    generateWaterBox(l, b, -1, 2, -2, 58, 23, -2, false);
    generateWaterBox(l, b, -1, 2, 59, 58, 23, 59, false);
    generateWaterBox(l, b, -3, 4, -3, -3, 23, 60, false);
    generateWaterBox(l, b, 60, 4, -3, 60, 23, 60, false);
    generateWaterBox(l, b, -2, 4, -3, 59, 23, -3, false);
    generateWaterBox(l, b, -2, 4, 60, 59, 23, 60, false);
    generateWaterBox(l, b, -4, 6, -4, -4, 23, 61, false);
    generateWaterBox(l, b, 61, 6, -4, 61, 23, 61, false);
    generateWaterBox(l, b, -3, 6, -4, 60, 23, -4, false);
    generateWaterBox(l, b, -3, 6, 61, 60, 23, 61, false);
    generateWaterBox(l, b, -5, 8, -5, -5, 23, 62, false);
    generateWaterBox(l, b, 62, 8, -5, 62, 23, 62, false);
    generateWaterBox(l, b, -4, 8, -5, 61, 23, -5, false);
    generateWaterBox(l, b, -4, 8, 62, 61, 23, 62, false);
    for (std::vector<OceanMonumentPiece*>::iterator it = mPieces.begin(); it != mPieces.end(); ++it)
    {
        if ((*it)->getBoundingBox()->intersects(b))
            (*it)->postProcess(l, r, b);
    }
    return true;
}
StructurePiece* OceanMonumentPieces::MonumentBuilding::Create()
{
    return new MonumentBuilding();
}
EStructurePiece OceanMonumentPieces::MonumentBuilding::GetType() {
    return eStructurePiece_MonumentBuilding;
    }
