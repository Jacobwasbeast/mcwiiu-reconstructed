#include "game/core/StrongholdPieces.hpp"

#include <algorithm>

StrongholdPieces::PieceWeight::PieceWeight(EPieceClass value, int maximum, int chance)
    : pieceClass(value), maxCount(maximum), count(0), frequency(chance)
{
}

StrongholdPieces::PieceWeight::~PieceWeight() { }

bool StrongholdPieces::PieceWeight::isValid()
{
    return count < maxCount;
}

bool StrongholdPieces::PieceWeight::doPlace(int depth)
{
    return count < maxCount && depth >= 0;
}

StrongholdPieces::PieceWeight_Library::~PieceWeight_Library() { }

bool StrongholdPieces::PieceWeight_Library::doPlace(int depth)
{
    return PieceWeight::doPlace(depth);
}

StrongholdPieces::PieceWeight_PortalRoom::~PieceWeight_PortalRoom() { }

bool StrongholdPieces::PieceWeight_PortalRoom::doPlace(int depth)
{
    return PieceWeight::doPlace(depth);
}

StrongholdPieces::StrongholdPiece::StrongholdPiece(int depth)
    : StructurePiece(depth), mSmallDoor(0)
{
}

StrongholdPieces::StrongholdPiece::StrongholdPiece()
    : StructurePiece(), mSmallDoor(0)
{
}

StrongholdPieces::StrongholdPiece::~StrongholdPiece() { }

bool StrongholdPieces::StrongholdPiece::isOkBox(BoundingBox* box, StartPiece* start)
{
    if (box == NULL) return false;
    if (start == NULL) return true;
    return box->getYSpan() <= 50;
}

unsigned char StrongholdPieces::StrongholdPiece::randomSmallDoor(Random& random)
{
    return static_cast<unsigned char>(random.nextInt(4));
}

void StrongholdPieces::StrongholdPiece::addAdditionalSaveData(DataOutputStream* output)
{
    output->writeInt(mSmallDoor);
}

void StrongholdPieces::StrongholdPiece::readAdditionalSaveData(DataInputStream* input, StructureManager*)
{
    mSmallDoor = input->readInt();
}

void StrongholdPieces::StrongholdPiece::generateSmallDoor(Level* level, Random&, BoundingBox* box, SmallDoorType type, int x, int y, int z)
{
    if (type == OPENING) generateAirBox(level, box, x, y, z, x + 2, y + 2, z);
    else generateAirBox(level, box, x, y, z, x + 2, y + 1, z);
}

int StrongholdPieces::StrongholdPiece::generateSmallDoorChildForward(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random, int x, int z)
{
    const Direction* direction = getOrientation();
    if (direction == NULL) return 0;
    return StrongholdPieces::generateAndAddPiece(start, pieces, random, mBoundingBox->x0 + x, mBoundingBox->y0, mBoundingBox->z0 + z, direction, getGenDepth());
}

int StrongholdPieces::StrongholdPiece::generateSmallDoorChildLeft(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random, int x, int z)
{
    return generateSmallDoorChildForward(start, pieces, random, x, z);
}

int StrongholdPieces::StrongholdPiece::generateSmallDoorChildRight(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random, int x, int z)
{
    return generateSmallDoorChildForward(start, pieces, random, x, z);
}

void StrongholdPieces::resetPieces() { }
unsigned long long StrongholdPieces::staticCtor() { return 0; }
unsigned long long StrongholdPieces::loadStatic() { return 0; }
unsigned int StrongholdPieces::updatePieceWeight() { return 0; }

StructurePiece* StrongholdPieces::findAndCreatePieceFactory(EPieceClass, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int)
{
    return NULL;
}

int StrongholdPieces::generatePieceFromSmallDoor(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int)
{
    return 0;
}

int StrongholdPieces::generateAndAddPiece(StartPiece*, std::vector<StructurePiece*>*, Random&, int, int, int, const Direction*, int)
{
    return 0;
}

template<class T>
static T* createStrongholdPiece(std::vector<StructurePiece*>* pieces, Random& random,
                                int x, int y, int z, const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, 0, 0, 0, 5, 5, 7, direction);
    if (!StrongholdPieces::StrongholdPiece::isOkBox(box, NULL) ||
        StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return new T(depth, random, box, direction);
}

StrongholdPieces::ChestCorridor::ChestCorridor()
    : StrongholdPiece(), mFlag(false), mData(0) { }
StrongholdPieces::ChestCorridor::ChestCorridor(
    int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth), mFlag(false), mData(random.nextInt(4))
{
    setOrientation(direction);
    setBoundingBox(box);
}
StrongholdPieces::ChestCorridor::~ChestCorridor() { }

StrongholdPieces::FillerCorridor::FillerCorridor()
    : StrongholdPiece(), mFlag(false), mData(0) { }
StrongholdPieces::FillerCorridor::FillerCorridor(
    int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth), mFlag(false), mData(random.nextInt(4))
{
    setOrientation(direction);
    setBoundingBox(box);
}
StrongholdPieces::FillerCorridor::~FillerCorridor() { }

StrongholdPieces::FiveCrossing::FiveCrossing()
    : StrongholdPiece(), mFlag(false), mData(0) { }
StrongholdPieces::FiveCrossing::FiveCrossing(
    int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth), mFlag(false), mData(random.nextInt(4))
{
    setOrientation(direction);
    setBoundingBox(box);
}
StrongholdPieces::FiveCrossing::~FiveCrossing() { }

StrongholdPieces::LeftTurn::LeftTurn()
    : StrongholdPiece(), mFlag(false), mData(0) { }
StrongholdPieces::LeftTurn::LeftTurn(
    int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth), mFlag(false), mData(random.nextInt(4))
{
    setOrientation(direction);
    setBoundingBox(box);
}
StrongholdPieces::LeftTurn::~LeftTurn() { }

StrongholdPieces::Library::Library()
    : StrongholdPiece(), mFlag(false), mData(0) { }
StrongholdPieces::Library::Library(
    int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth), mFlag(false), mData(random.nextInt(4))
{
    setOrientation(direction);
    setBoundingBox(box);
}
StrongholdPieces::Library::~Library() { }

StrongholdPieces::PortalRoom::PortalRoom()
    : StrongholdPiece(), mFlag(false), mData(0) { }
StrongholdPieces::PortalRoom::PortalRoom(
    int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth), mFlag(false), mData(random.nextInt(4))
{
    setOrientation(direction);
    setBoundingBox(box);
}
StrongholdPieces::PortalRoom::~PortalRoom() { }

StrongholdPieces::PrisonHall::PrisonHall()
    : StrongholdPiece(), mFlag(false), mData(0) { }
StrongholdPieces::PrisonHall::PrisonHall(
    int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth), mFlag(false), mData(random.nextInt(4))
{
    setOrientation(direction);
    setBoundingBox(box);
}
StrongholdPieces::PrisonHall::~PrisonHall() { }

StrongholdPieces::RoomCrossing::RoomCrossing()
    : StrongholdPiece(), mFlag(false), mData(0) { }
StrongholdPieces::RoomCrossing::RoomCrossing(
    int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth), mFlag(false), mData(random.nextInt(4))
{
    setOrientation(direction);
    setBoundingBox(box);
}
StrongholdPieces::RoomCrossing::~RoomCrossing() { }

StrongholdPieces::StairsDown::StairsDown()
    : StrongholdPiece(), mFlag(false), mData(0) { }
StrongholdPieces::StairsDown::StairsDown(
    int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth), mFlag(false), mData(random.nextInt(4))
{
    setOrientation(direction);
    setBoundingBox(box);
}
StrongholdPieces::StairsDown::~StairsDown() { }

StrongholdPieces::Straight::Straight()
    : StrongholdPiece(), mFlag(false), mData(0) { }
StrongholdPieces::Straight::Straight(
    int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth), mFlag(false), mData(random.nextInt(4))
{
    setOrientation(direction);
    setBoundingBox(box);
}
StrongholdPieces::Straight::~Straight() { }

StrongholdPieces::StraightStairsDown::StraightStairsDown()
    : StrongholdPiece(), mFlag(false), mData(0) { }
StrongholdPieces::StraightStairsDown::StraightStairsDown(
    int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth), mFlag(false), mData(random.nextInt(4))
{
    setOrientation(direction);
    setBoundingBox(box);
}
StrongholdPieces::StraightStairsDown::~StraightStairsDown() { }

StrongholdPieces::StairsDown::StairsDown(int depth, Random& random, int x, int z)
    : StrongholdPiece(depth), mFlag(false), mData(random.nextInt(4))
{
    setBoundingBox(BoundingBox::orientBox(x, 0, z, 0, 0, 0, 5, 5, 7, NULL));
}

BoundingBox* StrongholdPieces::FillerCorridor::findPieceBox(
    std::vector<StructurePiece*>* pieces, Random&, int x, int y, int z,
    const Direction* direction)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, 0, 0, 0, 5, 5, 7, direction);
    if (StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return box;
}

StrongholdPieces::ChestCorridor* StrongholdPieces::ChestCorridor::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    return createStrongholdPiece<ChestCorridor>(pieces, random, x, y, z, direction, depth);
}

StrongholdPieces::FiveCrossing* StrongholdPieces::FiveCrossing::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    return createStrongholdPiece<FiveCrossing>(pieces, random, x, y, z, direction, depth);
}

StrongholdPieces::LeftTurn* StrongholdPieces::LeftTurn::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    return createStrongholdPiece<LeftTurn>(pieces, random, x, y, z, direction, depth);
}

StrongholdPieces::Library* StrongholdPieces::Library::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    return createStrongholdPiece<Library>(pieces, random, x, y, z, direction, depth);
}

StrongholdPieces::PortalRoom* StrongholdPieces::PortalRoom::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    return createStrongholdPiece<PortalRoom>(pieces, random, x, y, z, direction, depth);
}

StrongholdPieces::PrisonHall* StrongholdPieces::PrisonHall::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    return createStrongholdPiece<PrisonHall>(pieces, random, x, y, z, direction, depth);
}

StrongholdPieces::RoomCrossing* StrongholdPieces::RoomCrossing::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    return createStrongholdPiece<RoomCrossing>(pieces, random, x, y, z, direction, depth);
}

StrongholdPieces::StairsDown* StrongholdPieces::StairsDown::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    return createStrongholdPiece<StairsDown>(pieces, random, x, y, z, direction, depth);
}

StrongholdPieces::Straight* StrongholdPieces::Straight::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    return createStrongholdPiece<Straight>(pieces, random, x, y, z, direction, depth);
}

StrongholdPieces::StraightStairsDown* StrongholdPieces::StraightStairsDown::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    return createStrongholdPiece<StraightStairsDown>(pieces, random, x, y, z, direction, depth);
}

#define STRONGHOLD_PIECE_METHODS(name, type) \
void StrongholdPieces::name::addAdditionalSaveData(DataOutputStream* output) { StrongholdPiece::addAdditionalSaveData(output); output->writeInt(mData); } \
void StrongholdPieces::name::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { StrongholdPiece::readAdditionalSaveData(input, manager); mData = input->readInt(); } \
void StrongholdPieces::name::addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&) { } \
bool StrongholdPieces::name::postProcess(Level* level, Random&, BoundingBox* box) { if (edgesLiquid(level, box)) return false; generateAirBox(level, box, 0, 0, 0, 4, 4, 6); return true; } \
StructurePiece* StrongholdPieces::name::Create() { return new StrongholdPieces::name(); } \
EStructurePiece StrongholdPieces::name::GetType() { return type; }

STRONGHOLD_PIECE_METHODS(ChestCorridor, eStructurePiece_ChestCorridor)
STRONGHOLD_PIECE_METHODS(FiveCrossing, eStructurePiece_FiveCrossing)
STRONGHOLD_PIECE_METHODS(LeftTurn, eStructurePiece_LeftTurn)
STRONGHOLD_PIECE_METHODS(Library, eStructurePiece_Library)
STRONGHOLD_PIECE_METHODS(PortalRoom, eStructurePiece_PortalRoom)
STRONGHOLD_PIECE_METHODS(PrisonHall, eStructurePiece_PrisonHall)
STRONGHOLD_PIECE_METHODS(RoomCrossing, eStructurePiece_StrongholdRoomCrossing)
STRONGHOLD_PIECE_METHODS(StairsDown, eStructurePiece_StairsDown)
STRONGHOLD_PIECE_METHODS(Straight, eStructurePiece_Straight)
STRONGHOLD_PIECE_METHODS(StraightStairsDown, eStructurePiece_StraightStairsDown)

void StrongholdPieces::FillerCorridor::addAdditionalSaveData(DataOutputStream* output)
{
    StrongholdPiece::addAdditionalSaveData(output);
    output->writeInt(mData);
}

void StrongholdPieces::FillerCorridor::readAdditionalSaveData(
    DataInputStream* input, StructureManager* manager)
{
    StrongholdPiece::readAdditionalSaveData(input, manager);
    mData = input->readInt();
}

bool StrongholdPieces::FillerCorridor::postProcess(Level* level, Random&, BoundingBox* box)
{
    if (edgesLiquid(level, box)) return false;
    generateAirBox(level, box, 0, 0, 0, 4, 4, 6);
    return true;
}

StructurePiece* StrongholdPieces::FillerCorridor::Create()
{
    return new FillerCorridor();
}

EStructurePiece StrongholdPieces::FillerCorridor::GetType()
{
    return eStructurePiece_FillerCorridor;
}

StrongholdPieces::RightTurn::RightTurn() : LeftTurn() { }
StrongholdPieces::RightTurn::~RightTurn() { }
void StrongholdPieces::RightTurn::addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&) { }
bool StrongholdPieces::RightTurn::postProcess(Level* level, Random& random, BoundingBox* box) { return LeftTurn::postProcess(level, random, box); }
StructurePiece* StrongholdPieces::RightTurn::Create() { return new RightTurn(); }
EStructurePiece StrongholdPieces::RightTurn::GetType() { return eStructurePiece_RightTurn; }

StrongholdPieces::StartPiece::StartPiece(int depth, Random& random, int x, int z, Level* level) : StairsDown(depth, random, BoundingBox::orientBox(x, 0, z, 0, 0, 0, 5, 5, 7, NULL), NULL), mLevel(level) { }
StrongholdPieces::StartPiece::~StartPiece() { }
BlockPos StrongholdPieces::StartPiece::getLocatorPosition() { return getBoundingBox()->getCenter(); }
EStructurePiece StrongholdPieces::StartPiece::GetType() { return eStructurePiece_StrongholdStartPiece; }

StrongholdPieces::SmoothStoneSelector::~SmoothStoneSelector() { }
void StrongholdPieces::SmoothStoneSelector::next(Random&, int, int, int, bool) { }

#undef STRONGHOLD_PIECE_METHODS
