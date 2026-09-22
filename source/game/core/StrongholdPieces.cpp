#include "game/core/StrongholdPieces.hpp"

#include <algorithm>
#include <cmath>
#include <list>

#include "game/world/LevelData.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/ButtonBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/EndPortalFrameBlock.hpp"
#include "game/world/block/LadderBlock.hpp"
#include "game/world/block/StoneSlabBlock.hpp"
#include "game/world/block/TorchBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/BlockState.hpp"

static StrongholdPieces::SmoothStoneSelector* sStrongholdStoneSelector = NULL;
static std::list<StrongholdPieces::PieceWeight*> sStrongholdPieceWeights;
static unsigned int sStrongholdWeightTotal;
static StrongholdPieces::EPieceClass sForcedPieceClass = StrongholdPieces::ePieceClass_None;

StrongholdPieces::PieceWeight::PieceWeight(EPieceClass value, int maximum, int chance)
    : pieceClass(value), maxCount(maximum), count(0), frequency(chance)
{
}

StrongholdPieces::PieceWeight::~PieceWeight() { }

bool StrongholdPieces::PieceWeight::isValid()
{
    return frequency == 0 || count < frequency;
}

bool StrongholdPieces::PieceWeight::doPlace(int depth)
{
    (void)depth;
    return frequency == 0 || count < frequency;
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

StrongholdPieces::StrongholdPiece::~StrongholdPiece() = default;

bool StrongholdPieces::StrongholdPiece::isOkBox(BoundingBox* box, StartPiece* start)
{
    if (box == NULL)
        return false;
    bool result = box->y0 > 10;
    if (start == NULL)
        return result;
    if (start->mLevel->getOriginalSaveVersion() <= 4)
        return result;
    int halfSize = start->mLevel->getLevelData()->getXZSize() * 8;
    int minBound = 1 - halfSize;
    int maxBound = halfSize - 1;
    result = box->x1 < maxBound && minBound < box->z0 && minBound < box->x0 && result;
    if (maxBound <= box->z1)
        result = false;
    return result;
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
    if (type == WOOD_DOOR)
    {
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x, y, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x, y + 1, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x, y + 2, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 1, y + 2, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 2, y + 2, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 2, y + 1, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 2, y, z, box);
        placeBlock(level, Blocks::wooden_door->defaultBlockState(), x + 1, y, z, box);
        TypedBoxed<DoorBlock::DoorHalf*> upperHalf(DoorBlock::UPPER);
        placeBlock(level, Blocks::wooden_door->defaultBlockState()->setBoxedValue(DoorBlock::HALF, &upperHalf),
            x + 1, y + 1, z, box);
        return;
    }
    if (type == GRATES)
    {
        placeBlock(level, Blocks::AIR->defaultBlockState(), x + 1, y, z, box);
        placeBlock(level, Blocks::AIR->defaultBlockState(), x + 1, y + 1, z, box);
        placeBlock(level, Blocks::iron_bars->defaultBlockState(), x, y, z, box);
        placeBlock(level, Blocks::iron_bars->defaultBlockState(), x, y + 1, z, box);
        placeBlock(level, Blocks::iron_bars->defaultBlockState(), x, y + 2, z, box);
        placeBlock(level, Blocks::iron_bars->defaultBlockState(), x + 1, y + 2, z, box);
        placeBlock(level, Blocks::iron_bars->defaultBlockState(), x + 2, y + 2, z, box);
        placeBlock(level, Blocks::iron_bars->defaultBlockState(), x + 2, y + 1, z, box);
        placeBlock(level, Blocks::iron_bars->defaultBlockState(), x + 2, y, z, box);
        return;
    }
    if (type == IRON_DOOR)
    {
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x, y, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x, y + 1, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x, y + 2, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 1, y + 2, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 2, y + 2, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 2, y + 1, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 2, y, z, box);
        placeBlock(level, Blocks::iron_door->defaultBlockState(), x + 1, y, z, box);
        TypedBoxed<DoorBlock::DoorHalf*> upperHalf(DoorBlock::UPPER);
        placeBlock(level, Blocks::iron_door->defaultBlockState()->setBoxedValue(DoorBlock::HALF, &upperHalf),
            x + 1, y + 1, z, box);
        TypedBoxed<const Direction*> buttonNorth(Direction::NORTH);
        placeBlock(level, Blocks::stone_button->defaultBlockState()->setBoxedValue(ButtonBlock::FACING, &buttonNorth),
            x + 2, y + 1, z + 1, box);
        TypedBoxed<const Direction*> buttonSouth(Direction::SOUTH);
        placeBlock(level, Blocks::stone_button->defaultBlockState()->setBoxedValue(ButtonBlock::FACING, &buttonSouth),
            x + 2, y + 1, z - 1, box);
        return;
    }
    generateBox(level, box, x, y, z, x + 2, y + 2, z,
        Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);
}

StructurePiece* StrongholdPieces::StrongholdPiece::generateSmallDoorChildForward(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random, int x, int z)
{
    const Direction* direction = getOrientation();
    if (direction == NULL)
        return NULL;
    unsigned int axis = direction->get3DDataValue();
    if (axis <= 1)
        return NULL;
    if (axis == 2)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random,
            mBoundingBox->x0 + x, mBoundingBox->y0 + z, mBoundingBox->z0 - 1, direction, getGenDepth());
    }
    if (axis < 4)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random,
            mBoundingBox->x0 + x, mBoundingBox->y0 + z, mBoundingBox->z1 + 1, direction, getGenDepth());
    }
    if (axis == 4)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random,
            mBoundingBox->x0 - 1, mBoundingBox->y0 + z, mBoundingBox->z0 + x, direction, getGenDepth());
    }
    if (axis == 5)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random,
            mBoundingBox->x1 + 1, mBoundingBox->y0 + z, mBoundingBox->z0 + x, direction, getGenDepth());
    }
    return NULL;
}

StructurePiece* StrongholdPieces::StrongholdPiece::generateSmallDoorChildLeft(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random, int x, int z)
{
    const Direction* direction = getOrientation();
    if (direction == NULL)
        return NULL;
    unsigned int axis = direction->get3DDataValue();
    if (axis <= 1)
        return NULL;
    BoundingBox* box = getBoundingBox();
    if (box == NULL)
        return NULL;
    int depth = getGenDepth();
    if (axis < 4)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random,
            box->x0 - 1, box->y0 + x, box->z0 + z, Direction::WEST, depth);
    }
    if (axis < 6)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random,
            box->x0 + z, box->y0 + x, box->z0 - 1, Direction::SOUTH, depth);
    }
    return NULL;
}

StructurePiece* StrongholdPieces::StrongholdPiece::generateSmallDoorChildRight(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random, int x, int z)
{
    const Direction* direction = getOrientation();
    if (direction == NULL)
        return NULL;
    unsigned int axis = direction->get3DDataValue();
    if (axis <= 1)
        return NULL;
    BoundingBox* box = getBoundingBox();
    if (box == NULL)
        return NULL;
    int depth = getGenDepth();
    if (axis < 4)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random,
            box->x1 + 1, box->y0 + x, box->z0 + z, Direction::EAST, depth);
    }
    if (axis < 6)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random,
            box->x0 + z, box->y0 + x, box->z1 + 1, Direction::SOUTH, depth);
    }
    return NULL;
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
    if (!updatePieceWeight())
        return NULL;
    if (sForcedPieceClass != ePieceClass_None)
    {
        EPieceClass forced = sForcedPieceClass;
        sForcedPieceClass = ePieceClass_None;
        StructurePiece* piece = findAndCreatePieceFactory(forced, pieces, random, x, y, z, direction, depth);
        if (piece != NULL)
        {
            sForcedPieceClass = ePieceClass_None;
            return piece;
        }
    }
    for (int attempt = 0; attempt < 5; ++attempt)
    {
        int choice = random.nextInt(sStrongholdWeightTotal);
        for (std::list<PieceWeight*>::iterator it = sStrongholdPieceWeights.begin();
             it != sStrongholdPieceWeights.end(); ++it)
        {
            PieceWeight* weight = *it;
            choice -= weight->maxCount;
            if (choice < 0)
            {
                if (!weight->doPlace(depth) || weight == start->mLastPieceWeight)
                    break;
                StructurePiece* piece = findAndCreatePieceFactory(weight->pieceClass, pieces, random,
                                                                  x, y, z, direction, depth);
                if (piece != NULL)
                {
                    start->mLastPieceWeight = weight;
                    ++weight->count;
                    if (!weight->isValid())
                        sStrongholdPieceWeights.remove(weight);
                    return piece;
                }
            }
        }
    }
    BoundingBox* box = FillerCorridor::findPieceBox(pieces, random, x, y, z, direction);
    if (box != NULL)
    {
        if (box->getYSpan() > 1)
            return new FillerCorridor(depth, random, box, direction);
        delete box;
    }
    return NULL;
}

StructurePiece* StrongholdPieces::generateAndAddPiece(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth)
{
    if (depth > 50)
        return NULL;
    if (std::abs(x - start->getBoundingBox()->x0) < 49 && std::abs(z - start->getBoundingBox()->z0) < 49)
    {
        StructurePiece* piece = generatePieceFromSmallDoor(start, pieces, random, x, y, z, direction, depth + 1);
        if (piece != NULL)
        {
            pieces->push_back(piece);
            start->mPendingChildren.push_back(piece);
        }
        return piece;
    }
    if (start->mLevel->getOriginalSaveVersion() <= 4)
        return NULL;
    if (start->mLevel->getLevelData()->getHasStrongholdEndPortal())
        return NULL;
    for (std::list<PieceWeight*>::iterator it = sStrongholdPieceWeights.begin(); it != sStrongholdPieceWeights.end(); ++it)
    {
        PieceWeight* weight = *it;
        if (weight->pieceClass == ePieceClass_PortalRoom)
        {
            StructurePiece* piece = PortalRoom::createPiece(pieces, random, x, y, z, direction, depth);
            if (piece != NULL)
            {
                start->mLastPieceWeight = weight;
                weight->count++;
                if (!weight->isValid())
                {
                    delete weight;
                    sStrongholdPieceWeights.remove(weight);
                }
                return piece;
            }
        }
    }
    return NULL;
}

template<class T>
static T* createStrongholdPiece(std::vector<StructurePiece*>* pieces, Random& random,
                                int x, int y, int z, const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 7, direction);
    StrongholdPieces::StartPiece* start = NULL;
    if (pieces != NULL && !pieces->empty())
        start = static_cast<StrongholdPieces::StartPiece*>(pieces->front());
    if (!StrongholdPieces::StrongholdPiece::isOkBox(box, start) ||
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
StrongholdPieces::ChestCorridor::~ChestCorridor() = default;

StrongholdPieces::FillerCorridor::FillerCorridor()
    : StrongholdPiece(), mData(0) { }
StrongholdPieces::FillerCorridor::FillerCorridor(
    int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth), mData(random.nextInt(4))
{
    setOrientation(direction);
    setBoundingBox(box);
}
StrongholdPieces::FillerCorridor::~FillerCorridor() = default;

StrongholdPieces::FiveCrossing::FiveCrossing()
    : StrongholdPiece(), mDoorFlags(), mData(0) { }
StrongholdPieces::FiveCrossing::FiveCrossing(
    int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth), mDoorFlags(), mData(random.nextInt(4))
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
    : StrongholdPiece(), mData(0) { }
StrongholdPieces::RoomCrossing::RoomCrossing(
    int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth), mData(random.nextInt(4))
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
    : StrongholdPiece(), mData(0)
{
    mDoorFlags[0] = false;
    mDoorFlags[1] = false;
}
StrongholdPieces::Straight::Straight(
    int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth), mData(random.nextInt(4))
{
    mDoorFlags[0] = false;
    mDoorFlags[1] = false;
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
    : StrongholdPiece(depth), mFlag(true), mData(0)
{
    setOrientation(Direction::Plane::HORIZONTAL->getRandomDirection(random));
    setBoundingBox(new BoundingBox(x, 64, z, x + 4, 74, z + 4));
}

BoundingBox* StrongholdPieces::FillerCorridor::findPieceBox(
    std::vector<StructurePiece*>* pieces, Random&, int x, int y, int z,
    const Direction* direction)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 4, direction);
    StructurePiece* collision = StructurePiece::findCollisionPiece(pieces, box);
    if (collision != NULL)
    {
        BoundingBox* collisionBox = collision->getBoundingBox();
        if (collisionBox->y0 == box->y0)
        {
            delete box;
            box = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 2, direction);
            collisionBox = collision->getBoundingBox();
            if (!collisionBox->intersects(box))
                return box;
            delete box;
            box = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 1, direction);
            collisionBox = collision->getBoundingBox();
            if (!collisionBox->intersects(box))
                return box;
            delete box;
            box = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 0, direction);
            collisionBox = collision->getBoundingBox();
            if (!collisionBox->intersects(box))
            {
                delete box;
                return BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 1, direction);
            }
        }
    }
    delete box;
    return NULL;
}

StrongholdPieces::ChestCorridor* StrongholdPieces::ChestCorridor::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 7, direction);
    StrongholdPieces::StartPiece* start = NULL;
    if (pieces != NULL && !pieces->empty())
        start = static_cast<StrongholdPieces::StartPiece*>(pieces->front());
    if (!StrongholdPieces::StrongholdPiece::isOkBox(box, start) ||
        StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return new ChestCorridor(depth, random, box, direction);
}

StrongholdPieces::FiveCrossing* StrongholdPieces::FiveCrossing::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -4, -3, 0, 10, 9, 11, direction);
    StrongholdPieces::StartPiece* start = NULL;
    if (pieces != NULL && !pieces->empty())
        start = static_cast<StrongholdPieces::StartPiece*>(pieces->front());
    if (!StrongholdPieces::StrongholdPiece::isOkBox(box, start) ||
        StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return new FiveCrossing(depth, random, box, direction);
}

StrongholdPieces::LeftTurn* StrongholdPieces::LeftTurn::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 5, direction);
    StrongholdPieces::StartPiece* start = NULL;
    if (pieces != NULL && !pieces->empty())
        start = static_cast<StrongholdPieces::StartPiece*>(pieces->front());
    if (!StrongholdPieces::StrongholdPiece::isOkBox(box, start) ||
        StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return new LeftTurn(depth, random, box, direction);
}

StrongholdPieces::Library* StrongholdPieces::Library::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -4, -1, 0, 14, 11, 15, direction);
    StrongholdPieces::StartPiece* start = NULL;
    if (pieces != NULL && !pieces->empty())
        start = static_cast<StrongholdPieces::StartPiece*>(pieces->front());
    if (!StrongholdPieces::StrongholdPiece::isOkBox(box, start) ||
        StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        box = BoundingBox::orientBox(x, y, z, -4, -1, 0, 14, 6, 15, direction);
        if (!StrongholdPieces::StrongholdPiece::isOkBox(box, start) ||
            StructurePiece::findCollisionPiece(pieces, box) != NULL)
        {
            delete box;
            return NULL;
        }
    }
    return new Library(depth, random, box, direction);
}

StrongholdPieces::PortalRoom* StrongholdPieces::PortalRoom::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -4, -1, 0, 11, 8, 16, direction);
    StrongholdPieces::StartPiece* start = NULL;
    if (pieces != NULL && !pieces->empty())
        start = static_cast<StrongholdPieces::StartPiece*>(pieces->front());
    if (!StrongholdPieces::StrongholdPiece::isOkBox(box, start) ||
        StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return new PortalRoom(depth, random, box, direction);
}

StrongholdPieces::PrisonHall* StrongholdPieces::PrisonHall::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -1, 0, 9, 5, 11, direction);
    StrongholdPieces::StartPiece* start = NULL;
    if (pieces != NULL && !pieces->empty())
        start = static_cast<StrongholdPieces::StartPiece*>(pieces->front());
    if (!StrongholdPieces::StrongholdPiece::isOkBox(box, start) ||
        StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return new PrisonHall(depth, random, box, direction);
}

StrongholdPieces::RoomCrossing* StrongholdPieces::RoomCrossing::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -4, -1, 0, 11, 7, 11, direction);
    StrongholdPieces::StartPiece* start = NULL;
    if (pieces != NULL && !pieces->empty())
        start = static_cast<StrongholdPieces::StartPiece*>(pieces->front());
    if (!StrongholdPieces::StrongholdPiece::isOkBox(box, start) ||
        StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return new RoomCrossing(depth, random, box, direction);
}

StrongholdPieces::StairsDown* StrongholdPieces::StairsDown::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -7, 0, 5, 11, 5, direction);
    StrongholdPieces::StartPiece* start = NULL;
    if (pieces != NULL && !pieces->empty())
        start = static_cast<StrongholdPieces::StartPiece*>(pieces->front());
    if (!StrongholdPieces::StrongholdPiece::isOkBox(box, start) ||
        StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return new StairsDown(depth, random, box, direction);
}

StrongholdPieces::Straight* StrongholdPieces::Straight::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 7, direction);
    StrongholdPieces::StartPiece* start = NULL;
    if (pieces != NULL && !pieces->empty())
        start = static_cast<StrongholdPieces::StartPiece*>(pieces->front());
    if (!StrongholdPieces::StrongholdPiece::isOkBox(box, start) ||
        StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return new Straight(depth, random, box, direction);
}

StrongholdPieces::StraightStairsDown* StrongholdPieces::StraightStairsDown::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -7, 0, 5, 11, 8, direction);
    StrongholdPieces::StartPiece* start = NULL;
    if (pieces != NULL && !pieces->empty())
        start = static_cast<StrongholdPieces::StartPiece*>(pieces->front());
    if (!StrongholdPieces::StrongholdPiece::isOkBox(box, start) ||
        StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return new StraightStairsDown(depth, random, box, direction);
}

#define STRONGHOLD_PIECE_METHODS(name, type) \
void StrongholdPieces::name::addAdditionalSaveData(DataOutputStream* output) { StrongholdPiece::addAdditionalSaveData(output); output->writeInt(mData); } \
void StrongholdPieces::name::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { StrongholdPiece::readAdditionalSaveData(input, manager); mData = input->readInt(); } \
void StrongholdPieces::name::addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&) { } \
bool StrongholdPieces::name::postProcess(Level* level, Random&, BoundingBox* box) { if (edgesLiquid(level, box)) return false; generateAirBox(level, box, 0, 0, 0, 4, 4, 6); return true; } \
StructurePiece* StrongholdPieces::name::Create() { return new StrongholdPieces::name(); } \
EStructurePiece StrongholdPieces::name::GetType() { return type; }

void StrongholdPieces::ChestCorridor::addAdditionalSaveData(DataOutputStream* output) { StrongholdPiece::addAdditionalSaveData(output); output->writeInt(mData); }
void StrongholdPieces::ChestCorridor::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { StrongholdPiece::readAdditionalSaveData(input, manager); mData = input->readInt(); }
void StrongholdPieces::ChestCorridor::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random)
{
    generateSmallDoorChildForward(static_cast<StartPiece*>(start), pieces, random, 1, 1);
}
bool StrongholdPieces::ChestCorridor::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 4, 4, 6, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, static_cast<SmallDoorType>(mSmallDoor), 1, 1, 0);
    generateSmallDoor(level, random, box, OPENING, 1, 1, 6);
    generateBox(level, box, 3, 1, 2, 3, 1, 4,
        Blocks::stonebrick->defaultBlockState(), Blocks::stonebrick->defaultBlockState(), false);
    if (!mFlag)
    {
        BlockPos pos(getWorldX(3, 3), getWorldY(2), getWorldZ(3, 3));
        if (box->isInside(pos))
        {
            mFlag = true;
            setDirty();
        }
    }
    return true;
}
StructurePiece* StrongholdPieces::ChestCorridor::Create() { return new StrongholdPieces::ChestCorridor(); }
EStructurePiece StrongholdPieces::ChestCorridor::GetType() { return eStructurePiece_ChestCorridor; }
void StrongholdPieces::FiveCrossing::addAdditionalSaveData(DataOutputStream* output)
{
    StrongholdPiece::addAdditionalSaveData(output);
    output->writeBoolean(mDoorFlags[0]);
    output->writeBoolean(mDoorFlags[1]);
    output->writeBoolean(mDoorFlags[2]);
    output->writeBoolean(mDoorFlags[3]);
}
void StrongholdPieces::FiveCrossing::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { StrongholdPiece::readAdditionalSaveData(input, manager); mDoorFlags[0] = input->readBoolean(); mDoorFlags[1] = input->readBoolean(); mDoorFlags[2] = input->readBoolean(); mDoorFlags[3] = input->readBoolean(); }
void StrongholdPieces::FiveCrossing::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random)
{
    int leftOffset = 3;
    int rightOffset = 5;
    const Direction* orientation = getOrientation();
    if (orientation == Direction::WEST || orientation == Direction::SOUTH)
    {
        leftOffset = 5;
        rightOffset = 3;
    }
    StartPiece* startPiece = static_cast<StartPiece*>(start);
    generateSmallDoorChildForward(startPiece, pieces, random, 5, 1);
    if (mDoorFlags[0])
        generateSmallDoorChildLeft(startPiece, pieces, random, leftOffset, 1);
    if (mDoorFlags[1])
        generateSmallDoorChildLeft(startPiece, pieces, random, rightOffset, 7);
    if (mDoorFlags[2])
        generateSmallDoorChildRight(startPiece, pieces, random, leftOffset, 1);
    if (mDoorFlags[3])
        generateSmallDoorChildRight(startPiece, pieces, random, rightOffset, 7);
}
bool StrongholdPieces::FiveCrossing::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 9, 8, 10, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, static_cast<SmallDoorType>(mSmallDoor), 4, 3, 0);
    if (mDoorFlags[0])
        generateBox(level, box, 0, 3, 1, 0, 5, 3,
            Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);
    if (mDoorFlags[1])
        generateBox(level, box, 9, 3, 1, 9, 5, 3,
            Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);
    if (mDoorFlags[2])
        generateBox(level, box, 0, 5, 7, 0, 7, 9,
            Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);
    if (mDoorFlags[3])
        generateBox(level, box, 9, 5, 7, 9, 7, 9,
            Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);
    generateBox(level, box, 5, 1, 10, 7, 3, 10,
        Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);
    generateBox(level, box, 1, 2, 1, 8, 2, 6, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 4, 1, 5, 4, 4, 9, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 8, 1, 5, 8, 4, 9, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 1, 4, 7, 3, 4, 9, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 1, 3, 5, 3, 3, 6, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 1, 4, 6, 3, 4, 6,
        Blocks::stone_slab->defaultBlockState(), Blocks::stone_slab->defaultBlockState(), false);
    generateBox(level, box, 5, 1, 7, 7, 1, 8, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 5, 1, 9, 7, 1, 9,
        Blocks::stone_slab->defaultBlockState(), Blocks::stone_slab->defaultBlockState(), false);
    generateBox(level, box, 5, 2, 7, 7, 2, 7,
        Blocks::stone_slab->defaultBlockState(), Blocks::stone_slab->defaultBlockState(), false);
    generateBox(level, box, 4, 5, 7, 4, 5, 9,
        Blocks::stone_slab->defaultBlockState(), Blocks::stone_slab->defaultBlockState(), false);
    generateBox(level, box, 8, 5, 7, 8, 5, 9,
        Blocks::stone_slab->defaultBlockState(), Blocks::stone_slab->defaultBlockState(), false);
    placeBlock(level, Blocks::torch->defaultBlockState(), 6, 5, 6, box);
    return true;
}
StructurePiece* StrongholdPieces::FiveCrossing::Create() { return new StrongholdPieces::FiveCrossing(); }
EStructurePiece StrongholdPieces::FiveCrossing::GetType() { return eStructurePiece_FiveCrossing; }
void StrongholdPieces::LeftTurn::addAdditionalSaveData(DataOutputStream* output) { StrongholdPiece::addAdditionalSaveData(output); output->writeInt(mData); }
void StrongholdPieces::LeftTurn::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { StrongholdPiece::readAdditionalSaveData(input, manager); mData = input->readInt(); }
void StrongholdPieces::LeftTurn::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random)
{
    const Direction* orientation = getOrientation();
    StartPiece* startPiece = static_cast<StartPiece*>(start);
    if (orientation == Direction::NORTH || orientation == Direction::SOUTH)
        generateSmallDoorChildLeft(startPiece, pieces, random, 1, 1);
    else
        generateSmallDoorChildRight(startPiece, pieces, random, 1, 1);
}
bool StrongholdPieces::LeftTurn::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 4, 4, 4, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, static_cast<SmallDoorType>(mSmallDoor), 1, 1, 0);
    const Direction* orientation = getOrientation();
    if (orientation == Direction::NORTH || orientation == Direction::EAST)
    {
        generateBox(level, box, 0, 1, 1, 0, 3, 3,
            Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);
    }
    else
    {
        generateBox(level, box, 4, 1, 1, 4, 3, 3,
            Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);
    }
    return true;
}
StructurePiece* StrongholdPieces::LeftTurn::Create() { return new StrongholdPieces::LeftTurn(); }
EStructurePiece StrongholdPieces::LeftTurn::GetType() { return eStructurePiece_LeftTurn; }
void StrongholdPieces::Library::addAdditionalSaveData(DataOutputStream* output) { StrongholdPiece::addAdditionalSaveData(output); output->writeInt(mData); }
void StrongholdPieces::Library::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { StrongholdPiece::readAdditionalSaveData(input, manager); mData = input->readInt(); }
void StrongholdPieces::Library::addChildren(StructurePiece*, std::vector<StructurePiece*>*, Random&) { }
bool StrongholdPieces::Library::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    bool tall = mFlag;
    int height = tall ? 0xb : 6;
    int z = 1;
    generateBox(level, box, 0, 0, 0, 13, height - 1, 14, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, static_cast<SmallDoorType>(mSmallDoor), 4, 1, 0);
    generateMaybeBox(level, box, random, 0.07f, 2, 1, 1, 11, 4, 13,
        Blocks::web->defaultBlockState(), Blocks::web->defaultBlockState(), false, false);

pillarColumn:
    generateBox(level, box, 1, 1, z, 1, 4, z,
        Blocks::planks->defaultBlockState(), Blocks::planks->defaultBlockState(), false);
    generateBox(level, box, 12, 1, z, 12, 4, z,
        Blocks::planks->defaultBlockState(), Blocks::planks->defaultBlockState(), false);
    {
        placeBlock(level, Blocks::torch->defaultBlockState(), 2, 3, z, box);
        placeBlock(level, Blocks::torch->defaultBlockState(), 11, 3, z, box);
    }
    {
        int savedZ = z;
        if (tall)
        {
            generateBox(level, box, 1, 6, z, 1, 9, z,
                Blocks::planks->defaultBlockState(), Blocks::planks->defaultBlockState(), false);
            generateBox(level, box, 12, 6, z, 12, 9, z,
                Blocks::planks->defaultBlockState(), Blocks::planks->defaultBlockState(), false);
        }
        while ((z = savedZ + 1) < 14)
        {
            for (;;)
            {
                if ((savedZ & 3) == 0)
                    goto pillarColumn;
                generateBox(level, box, 1, 1, z, 1, 4, z,
                    Blocks::bookshelf->defaultBlockState(), Blocks::bookshelf->defaultBlockState(), false);
                generateBox(level, box, 12, 1, z, 12, 4, z,
                    Blocks::bookshelf->defaultBlockState(), Blocks::bookshelf->defaultBlockState(), false);
                if (tall)
                    break;
                ++z;
                ++savedZ;
                if (z > 13)
                    goto afterColumns;
            }
            generateBox(level, box, 1, 6, z, 1, 9, z,
                Blocks::bookshelf->defaultBlockState(), Blocks::bookshelf->defaultBlockState(), false);
            generateBox(level, box, 12, 6, z, 12, 9, z,
                Blocks::bookshelf->defaultBlockState(), Blocks::bookshelf->defaultBlockState(), false);
            savedZ = z;
        }
    }
afterColumns:
    {
        int count = 2;
        int col = 3;
        do
        {
            generateBox(level, box, 3, 1, col, 4, 3, col,
                Blocks::bookshelf->defaultBlockState(), Blocks::bookshelf->defaultBlockState(), false);
            generateBox(level, box, 6, 1, col, 7, 3, col,
                Blocks::bookshelf->defaultBlockState(), Blocks::bookshelf->defaultBlockState(), false);
            generateBox(level, box, 9, 1, col, 10, 3, col,
                Blocks::bookshelf->defaultBlockState(), Blocks::bookshelf->defaultBlockState(), false);
            generateBox(level, box, 3, 1, col + 2, 4, 3, col + 2,
                Blocks::bookshelf->defaultBlockState(), Blocks::bookshelf->defaultBlockState(), false);
            generateBox(level, box, 6, 1, col + 2, 7, 3, col + 2,
                Blocks::bookshelf->defaultBlockState(), Blocks::bookshelf->defaultBlockState(), false);
            generateBox(level, box, 9, 1, col + 2, 10, 3, col + 2,
                Blocks::bookshelf->defaultBlockState(), Blocks::bookshelf->defaultBlockState(), false);
            --count;
            col += 4;
        } while (count != 0);
        generateBox(level, box, 3, 1, col, 4, 3, col,
            Blocks::bookshelf->defaultBlockState(), Blocks::bookshelf->defaultBlockState(), false);
        generateBox(level, box, 6, 1, col, 7, 3, col,
            Blocks::bookshelf->defaultBlockState(), Blocks::bookshelf->defaultBlockState(), false);
        generateBox(level, box, 9, 1, col, 10, 3, col,
            Blocks::bookshelf->defaultBlockState(), Blocks::bookshelf->defaultBlockState(), false);
    }
    if (tall)
    {
        generateBox(level, box, 1, 5, 1, 3, 5, 13,
            Blocks::planks->defaultBlockState(), Blocks::planks->defaultBlockState(), false);
        generateBox(level, box, 10, 5, 1, 12, 5, 13,
            Blocks::planks->defaultBlockState(), Blocks::planks->defaultBlockState(), false);
        generateBox(level, box, 4, 5, 1, 9, 5, 2,
            Blocks::planks->defaultBlockState(), Blocks::planks->defaultBlockState(), false);
        generateBox(level, box, 4, 5, 12, 9, 5, 13,
            Blocks::planks->defaultBlockState(), Blocks::planks->defaultBlockState(), false);
        placeBlock(level, Blocks::planks->defaultBlockState(), 9, 5, 11, box);
        placeBlock(level, Blocks::planks->defaultBlockState(), 8, 5, 11, box);
        placeBlock(level, Blocks::planks->defaultBlockState(), 9, 5, 10, box);
        generateBox(level, box, 3, 6, 2, 3, 6, 12,
            Blocks::fence->defaultBlockState(), Blocks::fence->defaultBlockState(), false);
        generateBox(level, box, 10, 6, 2, 10, 6, 10,
            Blocks::fence->defaultBlockState(), Blocks::fence->defaultBlockState(), false);
        generateBox(level, box, 4, 6, 2, 9, 6, 2,
            Blocks::fence->defaultBlockState(), Blocks::fence->defaultBlockState(), false);
        generateBox(level, box, 4, 6, 12, 8, 6, 12,
            Blocks::fence->defaultBlockState(), Blocks::fence->defaultBlockState(), false);
        placeBlock(level, Blocks::fence->defaultBlockState(), 9, 6, 11, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 8, 6, 11, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 9, 6, 10, box);
        TypedBoxed<const Direction*> ladderSouth(Direction::SOUTH);
        const BlockState* ladderState =
            Blocks::ladder->defaultBlockState()->setBoxedValue(LadderBlock::FACING, &ladderSouth);
        placeBlock(level, ladderState, 10, 1, 13, box);
        placeBlock(level, ladderState, 10, 2, 13, box);
        placeBlock(level, ladderState, 10, 3, 13, box);
        placeBlock(level, ladderState, 10, 4, 13, box);
        placeBlock(level, ladderState, 10, 5, 13, box);
        placeBlock(level, ladderState, 10, 6, 13, box);
        placeBlock(level, ladderState, 10, 7, 13, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 6, 9, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 7, 9, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 6, 8, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 7, 8, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 6, 7, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 7, 7, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 5, 7, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 8, 7, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 6, 7, 6, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 6, 7, 8, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 7, 7, 6, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 7, 7, 8, box);
        const BlockState* torchUpState = Blocks::torch->defaultBlockState();
        placeBlock(level, torchUpState, 5, 8, 7, box);
        placeBlock(level, torchUpState, 8, 8, 7, box);
        placeBlock(level, torchUpState, 6, 8, 6, box);
        placeBlock(level, torchUpState, 6, 8, 8, box);
        placeBlock(level, torchUpState, 7, 8, 6, box);
        placeBlock(level, torchUpState, 7, 8, 8, box);
    }
    if (tall)
        placeBlock(level, Blocks::AIR->defaultBlockState(), 12, 9, 1, box);
    return true;
}
StructurePiece* StrongholdPieces::Library::Create() { return new StrongholdPieces::Library(); }
EStructurePiece StrongholdPieces::Library::GetType() { return eStructurePiece_Library; }
void StrongholdPieces::PrisonHall::addAdditionalSaveData(DataOutputStream* output) { StrongholdPiece::addAdditionalSaveData(output); output->writeInt(mData); }
void StrongholdPieces::PrisonHall::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { StrongholdPiece::readAdditionalSaveData(input, manager); mData = input->readInt(); }
void StrongholdPieces::PrisonHall::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random)
{
    generateSmallDoorChildForward(static_cast<StartPiece*>(start), pieces, random, 1, 1);
}
bool StrongholdPieces::PrisonHall::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 8, 4, 10, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, static_cast<SmallDoorType>(mSmallDoor), 1, 1, 0);
    generateBox(level, box, 1, 1, 10, 3, 3, 10,
        Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);
    generateBox(level, box, 4, 1, 1, 4, 3, 1, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 4, 1, 3, 4, 3, 3, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 4, 1, 7, 4, 3, 7, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 4, 1, 9, 4, 3, 9, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 4, 1, 4, 4, 3, 6,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 5, 1, 5, 7, 3, 5,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    placeBlock(level, Blocks::iron_bars->defaultBlockState(), 4, 3, 2, box);
    placeBlock(level, Blocks::iron_bars->defaultBlockState(), 4, 3, 8, box);

    TypedBoxed<const Direction*> doorWest(Direction::WEST);
    const BlockState* lowerDoor =
        Blocks::iron_door->defaultBlockState()->setBoxedValue(DoorBlock::FACING, &doorWest);
    TypedBoxed<const Direction*> doorWest2(Direction::WEST);
    TypedBoxed<DoorBlock::DoorHalf*> upperHalf(DoorBlock::UPPER);
    const BlockState* upperDoor =
        Blocks::iron_door->defaultBlockState()->setBoxedValue(DoorBlock::FACING, &doorWest2)
            ->setBoxedValue(DoorBlock::HALF, &upperHalf);
    placeBlock(level, lowerDoor, 4, 1, 2, box);
    placeBlock(level, upperDoor, 4, 2, 2, box);
    placeBlock(level, lowerDoor, 4, 1, 8, box);
    placeBlock(level, upperDoor, 4, 2, 8, box);
    return true;
}
StructurePiece* StrongholdPieces::PrisonHall::Create() { return new StrongholdPieces::PrisonHall(); }
EStructurePiece StrongholdPieces::PrisonHall::GetType() { return eStructurePiece_PrisonHall; }
void StrongholdPieces::RoomCrossing::addAdditionalSaveData(DataOutputStream* output) { StrongholdPiece::addAdditionalSaveData(output); output->writeInt(mData); }
void StrongholdPieces::RoomCrossing::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { StrongholdPiece::readAdditionalSaveData(input, manager); mData = input->readInt(); }
void StrongholdPieces::RoomCrossing::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random)
{
    StartPiece* startPiece = static_cast<StartPiece*>(start);
    generateSmallDoorChildForward(startPiece, pieces, random, 1, 4);
    generateSmallDoorChildLeft(startPiece, pieces, random, 1, 4);
    generateSmallDoorChildRight(startPiece, pieces, random, 1, 4);
}
bool StrongholdPieces::RoomCrossing::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 10, 6, 10, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, static_cast<SmallDoorType>(mSmallDoor), 4, 1, 0);
    generateBox(level, box, 4, 1, 10, 6, 3, 10,
        Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);
    generateBox(level, box, 0, 1, 4, 0, 3, 6,
        Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);
    generateBox(level, box, 10, 1, 4, 10, 3, 6,
        Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);

    if (mData == 0)
    {
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 5, 1, 5, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 5, 2, 5, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 5, 3, 5, box);
        placeBlock(level, Blocks::torch->defaultBlockState(), 4, 3, 5, box);
        placeBlock(level, Blocks::torch->defaultBlockState(), 6, 3, 5, box);
        placeBlock(level, Blocks::torch->defaultBlockState(), 5, 3, 4, box);
        placeBlock(level, Blocks::torch->defaultBlockState(), 5, 3, 6, box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 4, 1, 4, box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 4, 1, 5, box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 4, 1, 6, box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 6, 1, 4, box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 6, 1, 5, box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 6, 1, 6, box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 5, 1, 4, box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 5, 1, 6, box);
    }
    else if (mData == 1)
    {
        int b = 1;
        int count = 2;
        int a = 0;
        int last = 0;
        do
        {
            last = a;
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, 1, last + 3, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 7, 1, last + 3, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), last + 3, 1, 3, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), last + 3, 1, 7, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, 1, b + 3, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 7, 1, b + 3, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), b + 3, 1, 3, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), b + 3, 1, 7, box);
            --count;
            b += 2;
            a = last + 2;
        } while (count != 0);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, 1, last + 5, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 7, 1, last + 5, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), last + 5, 1, 3, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), last + 5, 1, 7, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 5, 1, 5, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 5, 2, 5, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 5, 3, 5, box);
        placeBlock(level, Blocks::FLOWING_WATER->defaultBlockState(), 5, 4, 5, box);
    }
    else if (mData == 2)
    {
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 1, 3, 1, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 9, 3, 1, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 1, 3, 2, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 9, 3, 2, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 1, 3, 3, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 9, 3, 3, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 1, 3, 4, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 9, 3, 4, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 1, 3, 5, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 9, 3, 5, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 1, 3, 6, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 9, 3, 6, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 1, 3, 7, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 9, 3, 7, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 1, 3, 8, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 9, 3, 8, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 1, 3, 9, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 9, 3, 9, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 1, 3, 1, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 1, 3, 9, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 2, 3, 1, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 2, 3, 9, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 3, 3, 1, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 3, 3, 9, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 4, 3, 1, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 4, 3, 9, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 5, 3, 1, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 5, 3, 9, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 6, 3, 1, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 6, 3, 9, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 7, 3, 1, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 7, 3, 9, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 8, 3, 1, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 8, 3, 9, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 9, 3, 1, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 9, 3, 9, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 5, 1, 4, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 5, 1, 6, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 5, 3, 4, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 5, 3, 6, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 4, 1, 5, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 6, 1, 5, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 4, 3, 5, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 6, 3, 5, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 4, 1, 4, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 6, 1, 4, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 4, 1, 6, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 6, 1, 6, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 4, 2, 4, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 6, 2, 4, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 4, 2, 6, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 6, 2, 6, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 4, 3, 4, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 6, 3, 4, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 4, 3, 6, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 6, 3, 6, box);
        placeBlock(level, Blocks::torch->defaultBlockState(), 5, 3, 5, box);

        int count = 3;
        unsigned int a = 0xffffffffu;
        unsigned int b = 0xfffffffeu;
        int z = 2;
        int lastZ = 0;
        do
        {
            lastZ = z;
            placeBlock(level, Blocks::planks->defaultBlockState(), 2, 3, lastZ, box);
            placeBlock(level, Blocks::planks->defaultBlockState(), 3, 3, lastZ, box);
            if (2 < b)
            {
                placeBlock(level, Blocks::planks->defaultBlockState(), 4, 3, lastZ, box);
                placeBlock(level, Blocks::planks->defaultBlockState(), 5, 3, lastZ, box);
                placeBlock(level, Blocks::planks->defaultBlockState(), 6, 3, lastZ, box);
            }
            placeBlock(level, Blocks::planks->defaultBlockState(), 7, 3, lastZ, box);
            placeBlock(level, Blocks::planks->defaultBlockState(), 8, 3, lastZ, box);
            placeBlock(level, Blocks::planks->defaultBlockState(), 2, 3, lastZ + 1, box);
            placeBlock(level, Blocks::planks->defaultBlockState(), 3, 3, lastZ + 1, box);
            if (2 < a)
            {
                placeBlock(level, Blocks::planks->defaultBlockState(), 4, 3, lastZ + 1, box);
                placeBlock(level, Blocks::planks->defaultBlockState(), 5, 3, lastZ + 1, box);
                placeBlock(level, Blocks::planks->defaultBlockState(), 6, 3, lastZ + 1, box);
            }
            placeBlock(level, Blocks::planks->defaultBlockState(), 7, 3, lastZ + 1, box);
            placeBlock(level, Blocks::planks->defaultBlockState(), 8, 3, lastZ + 1, box);
            a += 2;
            --count;
            z = lastZ + 2;
            b += 2;
        } while (count != 0);
        placeBlock(level, Blocks::planks->defaultBlockState(), 2, 3, z, box);
        placeBlock(level, Blocks::planks->defaultBlockState(), 3, 3, z, box);
        if (2 < static_cast<unsigned int>(lastZ - 2))
        {
            placeBlock(level, Blocks::planks->defaultBlockState(), 4, 3, z, box);
            placeBlock(level, Blocks::planks->defaultBlockState(), 5, 3, z, box);
            placeBlock(level, Blocks::planks->defaultBlockState(), 6, 3, z, box);
        }
        placeBlock(level, Blocks::planks->defaultBlockState(), 7, 3, z, box);
        placeBlock(level, Blocks::planks->defaultBlockState(), 8, 3, z, box);

        TypedBoxed<const Direction*> ladderWest(Direction::WEST);
        const BlockState* ladderState =
            Blocks::ladder->defaultBlockState()->setBoxedValue(LadderBlock::FACING, &ladderWest);
        placeBlock(level, ladderState, 9, 1, 3, box);
        placeBlock(level, ladderState, 9, 2, 3, box);
        placeBlock(level, ladderState, 9, 3, 3, box);
    }
    return true;
}
StructurePiece* StrongholdPieces::RoomCrossing::Create() { return new StrongholdPieces::RoomCrossing(); }
EStructurePiece StrongholdPieces::RoomCrossing::GetType() { return eStructurePiece_StrongholdRoomCrossing; }
void StrongholdPieces::StairsDown::addAdditionalSaveData(DataOutputStream* output) { StrongholdPiece::addAdditionalSaveData(output); output->writeInt(mData); }
void StrongholdPieces::StairsDown::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { StrongholdPiece::readAdditionalSaveData(input, manager); mData = input->readInt(); }
void StrongholdPieces::StairsDown::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random)
{
    if (mFlag)
        sForcedPieceClass = ePieceClass_FiveCrossing;
    const Direction* orientation = getOrientation();
    if (orientation == NULL)
        return;
    unsigned int axis = orientation->get3DDataValue();
    if (axis <= 1)
        return;
    BoundingBox* box = getBoundingBox();
    if (box == NULL)
        return;
    int depth = getGenDepth();
    StartPiece* startPiece = static_cast<StartPiece*>(start);
    if (axis == 2)
        generateAndAddPiece(startPiece, pieces, random, box->x0 + 1, box->y0 + 1, box->z0 - 1, orientation, depth);
    else if (axis < 4)
        generateAndAddPiece(startPiece, pieces, random, box->x0 + 1, box->y0 + 1, box->z1 + 1, orientation, depth);
    else if (axis == 4)
        generateAndAddPiece(startPiece, pieces, random, box->x0 - 1, box->y0 + 1, box->z0 + 1, orientation, depth);
    else if (axis == 5)
        generateAndAddPiece(startPiece, pieces, random, box->x1 + 1, box->y0 + 1, box->z0 + 1, orientation, depth);
}
bool StrongholdPieces::StairsDown::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 4, 10, 4, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, static_cast<SmallDoorType>(mSmallDoor), 1, 7, 0);
    generateSmallDoor(level, random, box, OPENING, 1, 1, 4);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 2, 6, 1, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, 5, 1, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, 5, 2, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, 4, 3, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 2, 4, 3, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, 3, 3, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, 3, 2, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, 2, 1, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 2, 2, 1, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, 1, 1, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, 1, 2, box);
    return true;
}
StructurePiece* StrongholdPieces::StairsDown::Create() { return new StrongholdPieces::StairsDown(); }
EStructurePiece StrongholdPieces::StairsDown::GetType() { return eStructurePiece_StairsDown; }
void StrongholdPieces::Straight::addAdditionalSaveData(DataOutputStream* output) { StrongholdPiece::addAdditionalSaveData(output); output->writeInt(mData); }
void StrongholdPieces::Straight::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { StrongholdPiece::readAdditionalSaveData(input, manager); mData = input->readInt(); }
void StrongholdPieces::Straight::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random)
{
    StartPiece* startPiece = static_cast<StartPiece*>(start);
    generateSmallDoorChildForward(startPiece, pieces, random, 1, 1);
    if (mDoorFlags[0])
        generateSmallDoorChildLeft(startPiece, pieces, random, 1, 2);
    if (mDoorFlags[1])
        generateSmallDoorChildRight(startPiece, pieces, random, 1, 2);
}
bool StrongholdPieces::Straight::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 4, 4, 6, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, static_cast<SmallDoorType>(mSmallDoor), 1, 1, 0);
    generateSmallDoor(level, random, box, OPENING, 1, 1, 6);

    if (mDoorFlags[0])
    {
        generateBox(level, box, 0, 1, 2, 0, 3, 4,
            Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);
    }
    if (mDoorFlags[1])
    {
        generateBox(level, box, 4, 1, 2, 4, 3, 4,
            Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);
    }
    return true;
}
StructurePiece* StrongholdPieces::Straight::Create() { return new StrongholdPieces::Straight(); }
EStructurePiece StrongholdPieces::Straight::GetType() { return eStructurePiece_Straight; }
void StrongholdPieces::StraightStairsDown::addAdditionalSaveData(DataOutputStream* output) { StrongholdPiece::addAdditionalSaveData(output); output->writeInt(mData); }
void StrongholdPieces::StraightStairsDown::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { StrongholdPiece::readAdditionalSaveData(input, manager); mData = input->readInt(); }
void StrongholdPieces::StraightStairsDown::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random)
{
    generateSmallDoorChildForward(static_cast<StartPiece*>(start), pieces, random, 1, 1);
}
bool StrongholdPieces::StraightStairsDown::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 4, 10, 7, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, static_cast<SmallDoorType>(mSmallDoor), 1, 7, 0);
    generateSmallDoor(level, random, box, OPENING, 1, 1, 7);

    int firstY = 5;
    int secondY = 4;
    int count = 3;
    int index = 0;
    do
    {
        if (index < 5)
        {
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, firstY, index + 1, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 2, firstY, index + 1, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, firstY, index + 1, box);
        }
        if (index + 1 < 5)
        {
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, secondY, index + 2, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 2, secondY, index + 2, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, secondY, index + 2, box);
        }
        firstY -= 2;
        secondY -= 2;
        --count;
        index += 2;
    } while (count != 0);
    return true;
}
StructurePiece* StrongholdPieces::StraightStairsDown::Create() { return new StrongholdPieces::StraightStairsDown(); }
EStructurePiece StrongholdPieces::StraightStairsDown::GetType() { return eStructurePiece_StraightStairsDown; }

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

bool StrongholdPieces::PortalRoom::postProcess(Level* level, Random& random, BoundingBox* box)
{
    generateBox(level, box, 0, 0, 0, 10, 7, 15, false, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, GRATES, 4, 1, 0);
    generateBox(level, box, 9, 6, 1, 9, 6, 14, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 2, 6, 1, 8, 6, 2, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 2, 6, 14, 8, 6, 14, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 1, 1, 1, 2, 1, 4, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 8, 1, 1, 9, 1, 4, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 1, 1, 1, 1, 1, 3,
        Blocks::flowing_lava->defaultBlockState(), Blocks::flowing_lava->defaultBlockState(), false);
    generateBox(level, box, 3, 1, 8, 7, 1, 12, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 4, 1, 9, 6, 1, 11,
        Blocks::flowing_lava->defaultBlockState(), Blocks::flowing_lava->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 3, 0, 4, 3,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 10, 3, 3, 10, 4, 3,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 5, 0, 4, 5,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 10, 3, 5, 10, 4, 5,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 7, 0, 4, 7,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 10, 3, 7, 10, 4, 7,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 9, 0, 4, 9,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 10, 3, 9, 10, 4, 9,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 11, 0, 4, 11,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 10, 3, 11, 10, 4, 11,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 13, 0, 4, 13,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 10, 3, 13, 10, 4, 13,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 2, 3, 15, 2, 4, 15,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 4, 3, 15, 4, 4, 15,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 6, 3, 15, 6, 4, 15,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 8, 3, 15, 8, 4, 15,
        Blocks::iron_bars->defaultBlockState(), Blocks::iron_bars->defaultBlockState(), false);

    TypedBoxed<const Direction*> frameNorth(Direction::NORTH);
    const BlockState* frameNorthState =
        Blocks::end_portal_frame->defaultBlockState()->setBoxedValue(EndPortalFrameBlock::FACING, &frameNorth);
    TypedBoxed<const Direction*> frameSouth(Direction::SOUTH);
    const BlockState* frameSouthState =
        Blocks::end_portal_frame->defaultBlockState()->setBoxedValue(EndPortalFrameBlock::FACING, &frameSouth);
    TypedBoxed<const Direction*> frameEast(Direction::EAST);
    const BlockState* frameEastState =
        Blocks::end_portal_frame->defaultBlockState()->setBoxedValue(EndPortalFrameBlock::FACING, &frameEast);
    TypedBoxed<const Direction*> frameWest(Direction::WEST);
    const BlockState* frameWestState =
        Blocks::end_portal_frame->defaultBlockState()->setBoxedValue(EndPortalFrameBlock::FACING, &frameWest);

    placeBlock(level, frameNorthState, 4, 3, 8, box);
    placeBlock(level, frameNorthState, 5, 3, 8, box);
    placeBlock(level, frameNorthState, 6, 3, 8, box);
    placeBlock(level, frameSouthState, 4, 3, 12, box);
    placeBlock(level, frameSouthState, 5, 3, 12, box);
    placeBlock(level, frameSouthState, 6, 3, 12, box);
    placeBlock(level, frameEastState, 3, 3, 9, box);
    placeBlock(level, frameEastState, 3, 3, 10, box);
    placeBlock(level, frameEastState, 3, 3, 11, box);
    placeBlock(level, frameWestState, 7, 3, 9, box);
    placeBlock(level, frameWestState, 7, 3, 10, box);
    placeBlock(level, frameWestState, 7, 3, 11, box);

    if (!mFlag)
    {
        BlockPos pos(getWorldX(5, 6), getWorldY(3), getWorldZ(5, 6));
        if (box->isInside(pos))
        {
            mFlag = true;
            setDirty();
            level->setBlock(pos, Blocks::mob_spawner->defaultBlockState(), 2, false);
        }
    }
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
    if (edgesLiquid(level, box))
        return false;
    for (int z = 0; z < mData; ++z)
    {
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 0, 0, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, 0, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 2, 0, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, 0, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 4, 0, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 0, 1, z, box);
        placeBlock(level, Blocks::AIR->defaultBlockState(), 1, 1, z, box);
        placeBlock(level, Blocks::AIR->defaultBlockState(), 2, 1, z, box);
        placeBlock(level, Blocks::AIR->defaultBlockState(), 3, 1, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 4, 1, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 0, 2, z, box);
        placeBlock(level, Blocks::AIR->defaultBlockState(), 1, 2, z, box);
        placeBlock(level, Blocks::AIR->defaultBlockState(), 2, 2, z, box);
        placeBlock(level, Blocks::AIR->defaultBlockState(), 3, 2, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 4, 2, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 0, 3, z, box);
        placeBlock(level, Blocks::AIR->defaultBlockState(), 1, 3, z, box);
        placeBlock(level, Blocks::AIR->defaultBlockState(), 2, 3, z, box);
        placeBlock(level, Blocks::AIR->defaultBlockState(), 3, 3, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 4, 3, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 0, 4, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, 4, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 2, 4, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, 4, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 4, 4, z, box);
    }
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
StrongholdPieces::RightTurn::~RightTurn() = default;
void StrongholdPieces::RightTurn::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random)
{
    const Direction* orientation = getOrientation();
    if (orientation == Direction::NORTH || orientation == Direction::SOUTH)
        generateSmallDoorChildRight(static_cast<StartPiece*>(start), pieces, random, 1, 1);
    else
        generateSmallDoorChildLeft(static_cast<StartPiece*>(start), pieces, random, 1, 1);
}
bool StrongholdPieces::RightTurn::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 4, 4, 4, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, static_cast<SmallDoorType>(mSmallDoor), 1, 1, 0);
    const Direction* orientation = getOrientation();
    if (orientation == Direction::NORTH || orientation == Direction::EAST)
    {
        generateBox(level, box, 4, 1, 1, 4, 3, 3,
            Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);
    }
    else
    {
        generateBox(level, box, 0, 1, 1, 0, 3, 3,
            Blocks::AIR->defaultBlockState(), Blocks::AIR->defaultBlockState(), false);
    }
    return true;
}
StructurePiece* StrongholdPieces::RightTurn::Create() { return new RightTurn(); }
EStructurePiece StrongholdPieces::RightTurn::GetType() { return eStructurePiece_RightTurn; }

StrongholdPieces::StartPiece::StartPiece(int depth, Random& random, int x, int z, Level* level) : StairsDown(depth, random, x, z), mPortalRoom(NULL), mLevel(level), mLastPieceWeight(NULL) { }
StrongholdPieces::StartPiece::~StartPiece() = default;
BlockPos StrongholdPieces::StartPiece::getLocatorPosition()
{
    if (mPortalRoom != NULL)
        return mPortalRoom->getLocatorPosition();
    return StructurePiece::getLocatorPosition();
}
EStructurePiece StrongholdPieces::StartPiece::GetType() { return eStructurePiece_StrongholdStartPiece; }

StrongholdPieces::SmoothStoneSelector::~SmoothStoneSelector() = default;
void StrongholdPieces::SmoothStoneSelector::next(Random& random, int, int, int, bool edge)
{
    if (!edge) { mNext = Blocks::stone->defaultBlockState(); return; }
    const float roll = random.nextFloat();
    mNext = roll < 0.2f ? Blocks::stonebrick->defaultBlockState() : Blocks::stone->defaultBlockState();
}

#undef STRONGHOLD_PIECE_METHODS
