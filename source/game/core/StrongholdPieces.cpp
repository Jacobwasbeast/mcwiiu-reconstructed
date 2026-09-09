#include "game/core/StrongholdPieces.hpp"

#include <algorithm>
#include <list>

#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

static StrongholdPieces::SmoothStoneSelector* sStrongholdStoneSelector = NULL;
static std::list<StrongholdPieces::PieceWeight*> sStrongholdPieceWeights;
static unsigned int sStrongholdWeightTotal;

StrongholdPieces::PieceWeight::PieceWeight(EPieceClass value, int maximum, int chance)
    : pieceClass(value), maxCount(maximum), count(0), frequency(chance)
{
}

StrongholdPieces::PieceWeight::~PieceWeight() { }

bool StrongholdPieces::PieceWeight::isValid()
{
    return frequency != 0 && count < frequency;
}

bool StrongholdPieces::PieceWeight::doPlace(int depth)
{
    (void)depth;
    return frequency != 0 && count < frequency;
}

StrongholdPieces::PieceWeight_Library::~PieceWeight_Library() { }

bool StrongholdPieces::PieceWeight_Library::doPlace(int depth)
{
    return PieceWeight::doPlace(depth) && depth > 4;
}

StrongholdPieces::PieceWeight_PortalRoom::~PieceWeight_PortalRoom() { }

bool StrongholdPieces::PieceWeight_PortalRoom::doPlace(int depth)
{
    return PieceWeight::doPlace(depth) && depth > 5;
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

StructurePiece* StrongholdPieces::StrongholdPiece::generateSmallDoorChildForward(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random, int x, int z)
{
    const Direction* direction = getOrientation();
    if (direction == NULL) return NULL;
    return StrongholdPieces::generateAndAddPiece(start, pieces, random, mBoundingBox->x0 + x, mBoundingBox->y0, mBoundingBox->z0 + z, direction, getGenDepth());
}

StructurePiece* StrongholdPieces::StrongholdPiece::generateSmallDoorChildLeft(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random, int x, int z)
{
    return generateSmallDoorChildForward(start, pieces, random, x, z);
}

StructurePiece* StrongholdPieces::StrongholdPiece::generateSmallDoorChildRight(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random, int x, int z)
{
    return generateSmallDoorChildForward(start, pieces, random, x, z);
}

void StrongholdPieces::resetPieces()
{
    for (std::list<PieceWeight*>::iterator it=sStrongholdPieceWeights.begin(); it!=sStrongholdPieceWeights.end(); ++it) delete *it;
    sStrongholdPieceWeights.clear();
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_None, 1, 40));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_Straight, 2, 5));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_PrisonHall, 3, 20));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_LeftTurn, 4, 20));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_RightTurn, 5, 20));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_RoomCrossing, 10, 6));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_StraightStairsDown, 5, 5));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_StairsDown, 5, 5));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_FiveCrossing, 5, 4));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_ChestCorridor, 5, 4));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_Library, 10, 2));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_PortalRoom, 20, 1));
}
unsigned long long StrongholdPieces::staticCtor()
{
    if (sStrongholdStoneSelector == NULL)
        sStrongholdStoneSelector = new SmoothStoneSelector();
    return 0;
}
unsigned long long StrongholdPieces::loadStatic()
{
    StructureFeatureIO::setPieceId(eStructurePiece_ChestCorridor, &ChestCorridor::Create, L"SHCC");
    StructureFeatureIO::setPieceId(eStructurePiece_FillerCorridor, &FillerCorridor::Create, L"SHFC");
    StructureFeatureIO::setPieceId(eStructurePiece_FiveCrossing, &FiveCrossing::Create, L"SH5C");
    StructureFeatureIO::setPieceId(eStructurePiece_LeftTurn, &LeftTurn::Create, L"SHLT");
    StructureFeatureIO::setPieceId(eStructurePiece_Library, &Library::Create, L"SHLi");
    StructureFeatureIO::setPieceId(eStructurePiece_PortalRoom, &PortalRoom::Create, L"SHPR");
    StructureFeatureIO::setPieceId(eStructurePiece_PrisonHall, &PrisonHall::Create, L"SHPH");
    StructureFeatureIO::setPieceId(eStructurePiece_RightTurn, &RightTurn::Create, L"SHRT");
    StructureFeatureIO::setPieceId(eStructurePiece_StrongholdRoomCrossing, &RoomCrossing::Create, L"SHRC");
    StructureFeatureIO::setPieceId(eStructurePiece_StairsDown, &StairsDown::Create, L"SHSD");
    StructureFeatureIO::setPieceId(eStructurePiece_StrongholdStartPiece, &StairsDown::Create, L"SHStart");
    StructureFeatureIO::setPieceId(eStructurePiece_Straight, &Straight::Create, L"SHS");
    StructureFeatureIO::setPieceId(eStructurePiece_StraightStairsDown, &StraightStairsDown::Create, L"SHSSD");
    return 0;
}
unsigned int StrongholdPieces::updatePieceWeight()
{
    unsigned int valid = 0;
    sStrongholdWeightTotal = 0;
    for (std::list<PieceWeight*>::iterator it=sStrongholdPieceWeights.begin(); it!=sStrongholdPieceWeights.end(); ++it)
    {
        PieceWeight* weight = *it;
        if (weight->isValid()) valid = 1;
        sStrongholdWeightTotal += static_cast<unsigned int>(weight->maxCount);
    }
    return valid;
}

StructurePiece* StrongholdPieces::findAndCreatePieceFactory(EPieceClass pieceClass, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth)
{
    switch (pieceClass)
    {
    case ePieceClass_Straight: return Straight::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_PrisonHall: return PrisonHall::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_LeftTurn:
    case ePieceClass_RightTurn: return LeftTurn::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_RoomCrossing: return RoomCrossing::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_StraightStairsDown: return StraightStairsDown::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_StairsDown: return StairsDown::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_FiveCrossing: return FiveCrossing::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_ChestCorridor: return ChestCorridor::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_Library: return Library::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_PortalRoom: return PortalRoom::createPiece(pieces, random, x, y, z, direction, depth);
    default: return NULL;
    }
}

StructurePiece* StrongholdPieces::generatePieceFromSmallDoor(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth)
{
    if (start == NULL || pieces == NULL || depth > 50) return NULL;
    return findAndCreatePieceFactory(static_cast<EPieceClass>(random.nextInt(11)), pieces,
                                     random, x, y, z, direction, depth);
}

StructurePiece* StrongholdPieces::generateAndAddPiece(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth)
{
    if (start == NULL || pieces == NULL || depth > 50) return NULL;
    StructurePiece* piece = generatePieceFromSmallDoor(start, pieces, random, x, y, z, direction, depth + 1);
    if (piece != NULL)
    {
        pieces->push_back(piece);
        start->mPendingChildren.push_back(piece);
    }
    return piece;
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
STRONGHOLD_PIECE_METHODS(PrisonHall, eStructurePiece_PrisonHall)
STRONGHOLD_PIECE_METHODS(RoomCrossing, eStructurePiece_StrongholdRoomCrossing)
STRONGHOLD_PIECE_METHODS(StairsDown, eStructurePiece_StairsDown)
STRONGHOLD_PIECE_METHODS(Straight, eStructurePiece_Straight)
STRONGHOLD_PIECE_METHODS(StraightStairsDown, eStructurePiece_StraightStairsDown)

void StrongholdPieces::PortalRoom::addAdditionalSaveData(DataOutputStream* output)
{
    StrongholdPiece::addAdditionalSaveData(output);
    output->writeInt(mData);
}

void StrongholdPieces::PortalRoom::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    StrongholdPiece::readAdditionalSaveData(input, manager);
    mData = input->readInt();
}

void StrongholdPieces::PortalRoom::addChildren(StructurePiece* start, std::vector<StructurePiece*>*, Random&)
{
    if (start != NULL)
        static_cast<StartPiece*>(start)->mPortalRoom = this;
}

bool StrongholdPieces::PortalRoom::postProcess(Level* level, Random&, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateAirBox(level, box, 0, 0, 0, 4, 4, 6);
    return true;
}

StructurePiece* StrongholdPieces::PortalRoom::Create() { return new PortalRoom(); }
EStructurePiece StrongholdPieces::PortalRoom::GetType() { return eStructurePiece_PortalRoom; }

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
void StrongholdPieces::RightTurn::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random)
{
    const Direction* orientation = getOrientation();
    if (orientation == Direction::NORTH || orientation == Direction::SOUTH)
        generateSmallDoorChildRight(static_cast<StartPiece*>(start), pieces, random, 1, 1);
    else
        generateSmallDoorChildLeft(static_cast<StartPiece*>(start), pieces, random, 1, 1);
}
bool StrongholdPieces::RightTurn::postProcess(Level* level, Random& random, BoundingBox* box) { return LeftTurn::postProcess(level, random, box); }
StructurePiece* StrongholdPieces::RightTurn::Create() { return new RightTurn(); }
EStructurePiece StrongholdPieces::RightTurn::GetType() { return eStructurePiece_RightTurn; }

StrongholdPieces::StartPiece::StartPiece(int depth, Random& random, int x, int z, Level* level) : StairsDown(depth, random, BoundingBox::orientBox(x, 0, z, 0, 0, 0, 5, 5, 7, NULL), NULL), mPortalRoom(NULL), mLevel(level) { }
StrongholdPieces::StartPiece::~StartPiece() { }
BlockPos StrongholdPieces::StartPiece::getLocatorPosition()
{
    if (mPortalRoom != NULL)
        return mPortalRoom->getLocatorPosition();
    return StructurePiece::getLocatorPosition();
}
EStructurePiece StrongholdPieces::StartPiece::GetType() { return eStructurePiece_StrongholdStartPiece; }

StrongholdPieces::SmoothStoneSelector::~SmoothStoneSelector() { }
void StrongholdPieces::SmoothStoneSelector::next(Random& random, int, int, int, bool edge)
{
    if (!edge) { mNext = Blocks::stone->defaultBlockState(); return; }
    const float roll = random.nextFloat();
    mNext = roll < 0.2f ? Blocks::stonebrick->defaultBlockState() : Blocks::stone->defaultBlockState();
}

#undef STRONGHOLD_PIECE_METHODS
