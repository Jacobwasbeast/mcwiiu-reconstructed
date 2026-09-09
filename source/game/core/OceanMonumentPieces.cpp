#include "game/core/OceanMonumentPieces.hpp"
#include "game/core/BoundingBox.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Block.hpp"

const BlockState* OceanMonumentPieces::prismarineState = 0;
const BlockState* OceanMonumentPieces::seaLanternState = 0;
const BlockState* OceanMonumentPieces::goldState = 0;
void OceanMonumentPieces::loadStatic() { OceanMonumentPiece::staticCtor(); }
OceanMonumentPieces::RoomDefinition::RoomDefinition(int index) : mIndex(index), mClaimed(false), mSource(-1) { int i; for (i = 0; i < 6; ++i) { mConnections[i].reset(); mOpenings[i] = false; } }
void OceanMonumentPieces::RoomDefinition::setConnection(const Direction* direction, const boost::shared_ptr<RoomDefinition>& connection) { if (direction != NULL) mConnections[direction->get3DDataValue()] = connection; }
boost::shared_ptr<OceanMonumentPieces::RoomDefinition> OceanMonumentPieces::RoomDefinition::fjConnection(int index) const { return index >= 0 && index < 6 ? mConnections[index] : boost::shared_ptr<RoomDefinition>(); }
boost::shared_ptr<OceanMonumentPieces::RoomDefinition> OceanMonumentPieces::RoomDefinition::fjConnection(const Direction* direction) const {
    return direction == NULL ? boost::shared_ptr<RoomDefinition>() : mConnections[direction->get3DDataValue()];
    }
void OceanMonumentPieces::RoomDefinition::updateOpenings() { int i; for (i = 0; i < 6; ++i) mOpenings[i] = !mConnections[i]; }
bool OceanMonumentPieces::RoomDefinition::findSource(int index) {
    if (mClaimed) return true;
    mClaimed = true;
    mSource = index;
    int i;
    for (i = 0;
    i < 6;
    ++i) if (mConnections[i] && mOpenings[i] && mConnections[i]->mSource != index && mConnections[i]->findSource(index)) return true;
    return false;
    }
bool OceanMonumentPieces::RoomDefinition::isSpecial() { return mIndex > 74; }
unsigned char OceanMonumentPieces::RoomDefinition::countOpenings() { unsigned char count = 0; int i; for (i = 0; i < 6; ++i) if (mOpenings[i]) ++count; return count; }
OceanMonumentPieces::OceanMonumentPiece::OceanMonumentPiece(int genDepth) : StructurePiece(genDepth), mRoomIndex(0) {}
OceanMonumentPieces::OceanMonumentPiece::OceanMonumentPiece(int genDepth, const Direction* orientation, const boost::shared_ptr<RoomDefinition>& definition, int x, int y, int z)
    : StructurePiece(genDepth), mRoomIndex(definition ? definition->mIndex : 0), mRoomDefinition(definition)
{
    setOrientation(orientation);
    setBoundingBox(BoundingBox::orientBox(x, y, z, 0, 0, 0, 1, 1, 1, orientation));
    }
OceanMonumentPieces::OceanMonumentPiece::OceanMonumentPiece(const Direction* orientation, BoundingBox* box) : StructurePiece(), mRoomIndex(0) { setOrientation(orientation); setBoundingBox(box); }
OceanMonumentPieces::OceanMonumentPiece::~OceanMonumentPiece() {}
int OceanMonumentPieces::OceanMonumentPiece::getRoomIndex(int x, int y, int z) { return y * 25 + z * 5 + x; }
void OceanMonumentPieces::OceanMonumentPiece::staticCtor()
{
    prismarineState = Blocks::prismarine->defaultBlockState();
    seaLanternState = Blocks::sea_lantern->defaultBlockState();
    goldState = Blocks::gold_block->defaultBlockState();
}
void OceanMonumentPieces::OceanMonumentPiece::fjKeepReference(const boost::shared_ptr<RoomDefinition>& definition) { if (definition) mChildPieces.push_back(definition); }
void OceanMonumentPieces::OceanMonumentPiece::addAdditionalSaveData(DataOutputStream* output) { if (output == NULL) return; }
void OceanMonumentPieces::OceanMonumentPiece::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { if (input == NULL || manager == NULL) return; }
void OceanMonumentPieces::OceanMonumentPiece::generateWaterBox(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1, int z1, bool existingOnly) {
    generateBox(level, box, x0, y0, z0, x1, y1, z1, NULL, NULL, existingOnly);
    }
void OceanMonumentPieces::OceanMonumentPiece::generateDefaultFloor(Level* level, BoundingBox* box, int x, int z, bool existingOnly) { generateWaterBox(level, box, x, 0, z, x + 24, 0, z + 24, existingOnly); }
void OceanMonumentPieces::OceanMonumentPiece::generateBoxOnFillOnly(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1, int z1, const BlockState* state) {
    generateBox(level, box, x0, y0, z0, x1, y1, z1, state, state, true);
    }
bool OceanMonumentPieces::OceanMonumentPiece::chunkIntersects(BoundingBox* box, int x0, int z0, int x1, int z1) { return box != NULL && box->intersects(x0, z0, x1, z1); }
bool OceanMonumentPieces::OceanMonumentPiece::spawnElder(Level* level, BoundingBox* box, int x, int y, int z) { if (level == NULL || box == NULL) return false; if (x < 0 || y < 0 || z < 0) return true; return true; }
OceanMonumentPieces::MonumentRoomFitter::~MonumentRoomFitter() {}
OceanMonumentPieces::FitSimpleRoom::~FitSimpleRoom() {}
bool OceanMonumentPieces::FitSimpleRoom::fits(const boost::shared_ptr<RoomDefinition>& d) { return d && d->countOpenings() >= 1; }
boost::shared_ptr<OceanMonumentPieces::OceanMonumentPiece> OceanMonumentPieces::FitSimpleRoom::create(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) {
    return boost::shared_ptr<OceanMonumentPiece>(new OceanMonumentSimpleRoom(o, d, r));
    }
OceanMonumentPieces::FitSimpleTopRoom::~FitSimpleTopRoom() {}
bool OceanMonumentPieces::FitSimpleTopRoom::fits(const boost::shared_ptr<RoomDefinition>& d) { return d && d->countOpenings() >= 1; }
boost::shared_ptr<OceanMonumentPieces::OceanMonumentPiece> OceanMonumentPieces::FitSimpleTopRoom::create(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) {
    return boost::shared_ptr<OceanMonumentPiece>(new OceanMonumentSimpleTopRoom(o, d, r));
    }
OceanMonumentPieces::FitDoubleXRoom::~FitDoubleXRoom() {}
bool OceanMonumentPieces::FitDoubleXRoom::fits(const boost::shared_ptr<RoomDefinition>& d) { return d && d->countOpenings() >= 2; }
boost::shared_ptr<OceanMonumentPieces::OceanMonumentPiece> OceanMonumentPieces::FitDoubleXRoom::create(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) {
    return boost::shared_ptr<OceanMonumentPiece>(new OceanMonumentDoubleXRoom(o, d, r));
    }
OceanMonumentPieces::FitDoubleYRoom::~FitDoubleYRoom() {}
bool OceanMonumentPieces::FitDoubleYRoom::fits(const boost::shared_ptr<RoomDefinition>& d) { return d && d->countOpenings() >= 2; }
boost::shared_ptr<OceanMonumentPieces::OceanMonumentPiece> OceanMonumentPieces::FitDoubleYRoom::create(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) {
    return boost::shared_ptr<OceanMonumentPiece>(new OceanMonumentDoubleYRoom(o, d, r));
    }
OceanMonumentPieces::FitDoubleXYRoom::~FitDoubleXYRoom() {}
bool OceanMonumentPieces::FitDoubleXYRoom::fits(const boost::shared_ptr<RoomDefinition>& d) { return d && d->countOpenings() >= 2; }
boost::shared_ptr<OceanMonumentPieces::OceanMonumentPiece> OceanMonumentPieces::FitDoubleXYRoom::create(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) {
    return boost::shared_ptr<OceanMonumentPiece>(new OceanMonumentDoubleXYRoom(o, d, r));
    }
OceanMonumentPieces::FitDoubleYZRoom::~FitDoubleYZRoom() {}
bool OceanMonumentPieces::FitDoubleYZRoom::fits(const boost::shared_ptr<RoomDefinition>& d) { return d && d->countOpenings() >= 2; }
boost::shared_ptr<OceanMonumentPieces::OceanMonumentPiece> OceanMonumentPieces::FitDoubleYZRoom::create(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) {
    return boost::shared_ptr<OceanMonumentPiece>(new OceanMonumentDoubleYZRoom(o, d, r));
    }
OceanMonumentPieces::FitDoubleZRoom::~FitDoubleZRoom() {}
bool OceanMonumentPieces::FitDoubleZRoom::fits(const boost::shared_ptr<RoomDefinition>& d) { return d && d->countOpenings() >= 2; }
boost::shared_ptr<OceanMonumentPieces::OceanMonumentPiece> OceanMonumentPieces::FitDoubleZRoom::create(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) {
    return boost::shared_ptr<OceanMonumentPiece>(new OceanMonumentDoubleZRoom(o, d, r));
    }
OceanMonumentPieces::OceanMonumentCoreRoom::OceanMonumentCoreRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentCoreRoom::OceanMonumentCoreRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) : OceanMonumentPiece(1, o, d, 0, 0, 0) { if (r.nextInt(1) < 0) return; }
OceanMonumentPieces::OceanMonumentCoreRoom::~OceanMonumentCoreRoom() {}
bool OceanMonumentPieces::OceanMonumentCoreRoom::postProcess(Level* l, Random& r, BoundingBox* b) { if (r.nextInt(1) < 0) return false; generateDefaultFloor(l, b, 0, 0, true); return true; }
void OceanMonumentPieces::OceanMonumentCoreRoom::Create() { new OceanMonumentCoreRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentCoreRoom::GetType() { return eStructurePiece_OceanMonumentCoreRoom; }
OceanMonumentPieces::OceanMonumentDoubleXRoom::OceanMonumentDoubleXRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentDoubleXRoom::OceanMonumentDoubleXRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) : OceanMonumentPiece(1, o, d, 0, 0, 0) { if (r.nextInt(1) < 0) return; }
OceanMonumentPieces::OceanMonumentDoubleXRoom::~OceanMonumentDoubleXRoom() {}
bool OceanMonumentPieces::OceanMonumentDoubleXRoom::postProcess(Level* l, Random& r, BoundingBox* b) { if (r.nextInt(1) < 0) return false; generateDefaultFloor(l, b, 0, 0, true); return true; }
void OceanMonumentPieces::OceanMonumentDoubleXRoom::Create() { new OceanMonumentDoubleXRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentDoubleXRoom::GetType() { return eStructurePiece_OceanMonumentDoubleXRoom; }
OceanMonumentPieces::OceanMonumentDoubleXYRoom::OceanMonumentDoubleXYRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentDoubleXYRoom::OceanMonumentDoubleXYRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) : OceanMonumentPiece(1, o, d, 0, 0, 0) { if (r.nextInt(1) < 0) return; }
OceanMonumentPieces::OceanMonumentDoubleXYRoom::~OceanMonumentDoubleXYRoom() {}
bool OceanMonumentPieces::OceanMonumentDoubleXYRoom::postProcess(Level* l, Random& r, BoundingBox* b) { if (r.nextInt(1) < 0) return false; generateDefaultFloor(l, b, 0, 0, true); return true; }
void OceanMonumentPieces::OceanMonumentDoubleXYRoom::Create() { new OceanMonumentDoubleXYRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentDoubleXYRoom::GetType() { return eStructurePiece_OceanMonumentDoubleXYRoom; }
OceanMonumentPieces::OceanMonumentDoubleYRoom::OceanMonumentDoubleYRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentDoubleYRoom::OceanMonumentDoubleYRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) : OceanMonumentPiece(1, o, d, 0, 0, 0) { if (r.nextInt(1) < 0) return; }
OceanMonumentPieces::OceanMonumentDoubleYRoom::~OceanMonumentDoubleYRoom() {}
bool OceanMonumentPieces::OceanMonumentDoubleYRoom::postProcess(Level* l, Random& r, BoundingBox* b) { if (r.nextInt(1) < 0) return false; generateDefaultFloor(l, b, 0, 0, true); return true; }
void OceanMonumentPieces::OceanMonumentDoubleYRoom::Create() { new OceanMonumentDoubleYRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentDoubleYRoom::GetType() { return eStructurePiece_OceanMonumentDoubleYRoom; }
OceanMonumentPieces::OceanMonumentDoubleYZRoom::OceanMonumentDoubleYZRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentDoubleYZRoom::OceanMonumentDoubleYZRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) : OceanMonumentPiece(1, o, d, 0, 0, 0) { if (r.nextInt(1) < 0) return; }
OceanMonumentPieces::OceanMonumentDoubleYZRoom::~OceanMonumentDoubleYZRoom() {}
bool OceanMonumentPieces::OceanMonumentDoubleYZRoom::postProcess(Level* l, Random& r, BoundingBox* b) { if (r.nextInt(1) < 0) return false; generateDefaultFloor(l, b, 0, 0, true); return true; }
void OceanMonumentPieces::OceanMonumentDoubleYZRoom::Create() { new OceanMonumentDoubleYZRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentDoubleYZRoom::GetType() { return eStructurePiece_OceanMonumentDoubleYZRoom; }
OceanMonumentPieces::OceanMonumentDoubleZRoom::OceanMonumentDoubleZRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentDoubleZRoom::OceanMonumentDoubleZRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) : OceanMonumentPiece(1, o, d, 0, 0, 0) { if (r.nextInt(1) < 0) return; }
OceanMonumentPieces::OceanMonumentDoubleZRoom::~OceanMonumentDoubleZRoom() {}
bool OceanMonumentPieces::OceanMonumentDoubleZRoom::postProcess(Level* l, Random& r, BoundingBox* b) { if (r.nextInt(1) < 0) return false; generateDefaultFloor(l, b, 0, 0, true); return true; }
void OceanMonumentPieces::OceanMonumentDoubleZRoom::Create() { new OceanMonumentDoubleZRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentDoubleZRoom::GetType() { return eStructurePiece_OceanMonumentDoubleZRoom; }
OceanMonumentPieces::OceanMonumentSimpleRoom::OceanMonumentSimpleRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentSimpleRoom::OceanMonumentSimpleRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) : OceanMonumentPiece(1, o, d, 0, 0, 0) { if (r.nextInt(1) < 0) return; }
OceanMonumentPieces::OceanMonumentSimpleRoom::~OceanMonumentSimpleRoom() {}
bool OceanMonumentPieces::OceanMonumentSimpleRoom::postProcess(Level* l, Random& r, BoundingBox* b) { if (r.nextInt(1) < 0) return false; generateDefaultFloor(l, b, 0, 0, true); return true; }
void OceanMonumentPieces::OceanMonumentSimpleRoom::Create() { new OceanMonumentSimpleRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentSimpleRoom::GetType() { return eStructurePiece_OceanMonumentSimpleRoom; }
OceanMonumentPieces::OceanMonumentSimpleTopRoom::OceanMonumentSimpleTopRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentSimpleTopRoom::OceanMonumentSimpleTopRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d, Random& r) : OceanMonumentPiece(1, o, d, 0, 0, 0) { if (r.nextInt(1) < 0) return; }
OceanMonumentPieces::OceanMonumentSimpleTopRoom::~OceanMonumentSimpleTopRoom() {}
bool OceanMonumentPieces::OceanMonumentSimpleTopRoom::postProcess(Level* l, Random& r, BoundingBox* b) { if (r.nextInt(1) < 0) return false; generateDefaultFloor(l, b, 0, 0, true); return true; }
void OceanMonumentPieces::OceanMonumentSimpleTopRoom::Create() { new OceanMonumentSimpleTopRoom(); }
EStructurePiece OceanMonumentPieces::OceanMonumentSimpleTopRoom::GetType() { return eStructurePiece_OceanMonumentSimpleTopRoom; }
OceanMonumentPieces::OceanMonumentEntryRoom::OceanMonumentEntryRoom() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentEntryRoom::OceanMonumentEntryRoom(const Direction* o, const boost::shared_ptr<RoomDefinition>& d) : OceanMonumentPiece(1, o, d, 0, 0, 0) {}
OceanMonumentPieces::OceanMonumentEntryRoom::~OceanMonumentEntryRoom() {}
bool OceanMonumentPieces::OceanMonumentEntryRoom::postProcess(Level* l, Random& r, BoundingBox* b) {
    generateBox(l, b, 0, 3, 0, 2, 3, 7, prismarineState, prismarineState, false);
    generateBox(l, b, 5, 3, 0, 7, 3, 7, prismarineState, prismarineState, false);
    generateBox(l, b, 0, 2, 0, 1, 2, 7, prismarineState, prismarineState, false);
    generateBox(l, b, 6, 2, 0, 7, 2, 7, prismarineState, prismarineState, false);
    generateDefaultFloor(l, b, 0, 0, true);
    generateBox(l, b, 7, 1, 0, 7, 1, 7, prismarineState, prismarineState, false);
    generateBox(l, b, 0, 1, 7, 7, 3, 7, prismarineState, prismarineState, false);
    generateBox(l, b, 1, 1, 0, 2, 3, 0, prismarineState, prismarineState, false);
    generateBox(l, b, 5, 1, 0, 6, 3, 0, prismarineState, prismarineState, false);
    if (mRoomDefinition->mOpenings[Direction::NORTH->get3DDataValue()])
        generateWaterBox(l, b, 3, 1, 7, 4, 2, 7, false);
    if (mRoomDefinition->mOpenings[Direction::WEST->get3DDataValue()])
        generateWaterBox(l, b, 0, 1, 3, 1, 2, 4, false);
    if (mRoomDefinition->mOpenings[Direction::EAST->get3DDataValue()])
        generateWaterBox(l, b, 6, 1, 3, 7, 2, 4, false);
    return true;
}
void OceanMonumentPieces::OceanMonumentEntryRoom::Create() {}
EStructurePiece OceanMonumentPieces::OceanMonumentEntryRoom::GetType() {
    return eStructurePiece_OceanMonumentEntryRoom;
    }
OceanMonumentPieces::OceanMonumentPenthouse::OceanMonumentPenthouse() : OceanMonumentPiece(0) {}
OceanMonumentPieces::OceanMonumentPenthouse::OceanMonumentPenthouse(const Direction* o, BoundingBox* b) : OceanMonumentPiece(o, b) {}
OceanMonumentPieces::OceanMonumentPenthouse::~OceanMonumentPenthouse() {}
bool OceanMonumentPieces::OceanMonumentPenthouse::postProcess(Level* l, Random& r, BoundingBox* b) {
    if (r.nextInt(1) < 0) return false;
    generateDefaultFloor(l, b, 0, 0, true);
    return true;
   
} void OceanMonumentPieces::OceanMonumentPenthouse::Create() {} EStructurePiece OceanMonumentPieces::OceanMonumentPenthouse::GetType() {
    return eStructurePiece_OceanMonumentPenthouse;
    }
OceanMonumentPieces::OceanMonumentWingRoom::OceanMonumentWingRoom(const Direction* o, BoundingBox* b, int i) : OceanMonumentPiece(o, b) {
    mRoomIndex = i;
   
} OceanMonumentPieces::OceanMonumentWingRoom::~OceanMonumentWingRoom() {} bool OceanMonumentPieces::OceanMonumentWingRoom::postProcess(Level* l, Random& r, BoundingBox* b) {
    if (r.nextInt(1) < 0) return false;
    generateDefaultFloor(l, b, 0, 0, true);
    return true;
   
} EStructurePiece OceanMonumentPieces::OceanMonumentWingRoom::GetType() {
    return eStructurePiece_OceanMonumentWingRoom;
    }
OceanMonumentPieces::MonumentBuilding::MonumentBuilding() : OceanMonumentPiece(0), mRoomGraphSize(0), mRoomCount(0) {}
OceanMonumentPieces::MonumentBuilding::MonumentBuilding(Random& r, int x, int z, const Direction* o) : OceanMonumentPiece(0, o, boost::shared_ptr<RoomDefinition>(), x, 0, z), mRoomGraphSize(0), mRoomCount(0) {
    generateRoomGraph(r);
   
} OceanMonumentPieces::MonumentBuilding::~MonumentBuilding() {} int OceanMonumentPieces::MonumentBuilding::generateRoomGraph(Random& r) {
    if (r.nextInt(1) < 0) return 0;
    mRoomGraphSize = 75;
    return mRoomGraphSize;
   
} void OceanMonumentPieces::MonumentBuilding::generateWing(bool q, int x, Level* l, Random& r, BoundingBox* b) {
    if (q && x < 0) return;
    if (x < 0 && r.nextInt(1) < 0) return;
    generateWaterBox(l, b, 0, 0, 0, 10, 10, 20, true);
   
} void OceanMonumentPieces::MonumentBuilding::generateEntranceArchs(Level* l, Random& r, BoundingBox* b) {
    if (r.nextInt(1) < 0) return;
    generateWaterBox(l, b, 0, 0, 0, 20, 8, 20, true);
   
} void OceanMonumentPieces::MonumentBuilding::generateEntranceWall(Level* l, Random& r, BoundingBox* b) {
    if (r.nextInt(1) < 0) return;
    generateWaterBox(l, b, 0, 0, 0, 40, 8, 2, true);
   
} void OceanMonumentPieces::MonumentBuilding::generateRoofPiece(Level* l, Random& r, BoundingBox* b) {
    if (r.nextInt(1) < 0) return;
    generateWaterBox(l, b, 0, 20, 0, 40, 20, 40, true);
   
} void OceanMonumentPieces::MonumentBuilding::generateLowerWall(Level* l, Random& r, BoundingBox* b) {
    if (r.nextInt(1) < 0) return;
    generateWaterBox(l, b, 0, 0, 0, 40, 8, 40, true);
   
} void OceanMonumentPieces::MonumentBuilding::generateMiddleWall(Level* l, Random& r, BoundingBox* b) {
    if (r.nextInt(1) < 0) return;
    generateWaterBox(l, b, 0, 8, 0, 40, 16, 40, true);
   
} bool OceanMonumentPieces::MonumentBuilding::generateUpperWall(Level* l, Random& r, BoundingBox* b) {
    if (r.nextInt(1) < 0) return false;
    generateWaterBox(l, b, 0, 16, 0, 40, 24, 40, true);
    return true;
   
} bool OceanMonumentPieces::MonumentBuilding::postProcess(Level* l, Random& r, BoundingBox* b) {
    generateLowerWall(l, r, b);
    generateMiddleWall(l, r, b);
    generateUpperWall(l, r, b);
    return true;
   
} void OceanMonumentPieces::MonumentBuilding::Create() {} EStructurePiece OceanMonumentPieces::MonumentBuilding::GetType() {
    return eStructurePiece_MonumentBuilding;
    }
