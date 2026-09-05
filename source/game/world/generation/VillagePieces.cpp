#include "game/world/generation/VillagePieces.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
static BoundingBox* villageBox(int x, int y, int z, int width, int height, int length, const Direction* direction)
{
    return BoundingBox::orientBox(x, y, z, 0, 0, 0, width, height, length, direction);
}
static bool villageFits(std::vector<StructurePiece*>* pieces, BoundingBox* box)
{
    if (box == NULL) return false;
    if (StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return false;
    }
    return true;
}
std::list<VillagePieces::PieceWeight*>* VillagePieces::createPieceSet(Random& random, int size)
{
    std::list<PieceWeight*>* pieces = new std::list<PieceWeight*>();
    pieces->push_back(new PieceWeight(ePieceClass_StraightRoad, 3, size + 2));
    pieces->push_back(new PieceWeight(ePieceClass_SimpleHouse, 3, size + 2));
    pieces->push_back(new PieceWeight(ePieceClass_SmallTemple, 2, size + 1));
    pieces->push_back(new PieceWeight(ePieceClass_BookHouse, 20, size + 1));
    pieces->push_back(new PieceWeight(ePieceClass_SmallHut, 3, size + 2));
    pieces->push_back(new PieceWeight(ePieceClass_PigHouse, 3, size + 2));
    pieces->push_back(new PieceWeight(ePieceClass_TwoRoomHouse, 1, size + 1));
    pieces->push_back(new PieceWeight(ePieceClass_Smithy, 3, size + 1));
    pieces->push_back(new PieceWeight(ePieceClass_Farmland, 3, size + 2));
    pieces->push_back(new PieceWeight(ePieceClass_DoubleFarmland, 1, size + 1));
    pieces->push_back(new PieceWeight(ePieceClass_LightPost, 1, size + 1));
    return pieces;
}
void VillagePieces::loadStatic()
{
    StructureFeatureIO::setPieceId(eStructurePiece_StraightRoad, StraightRoad::Create, L"ViSR");
    StructureFeatureIO::setPieceId(eStructurePiece_SimpleHouse, SimpleHouse::Create, L"ViSH");
    StructureFeatureIO::setPieceId(eStructurePiece_SmallTemple, SmallTemple::Create, L"ViST");
    StructureFeatureIO::setPieceId(eStructurePiece_BookHouse, BookHouse::Create, L"ViBH");
    StructureFeatureIO::setPieceId(eStructurePiece_SmallHut, SmallHut::Create, L"ViSHT");
    StructureFeatureIO::setPieceId(eStructurePiece_PigHouse, PigHouse::Create, L"ViPH");
    StructureFeatureIO::setPieceId(eStructurePiece_TwoRoomHouse, TwoRoomHouse::Create, L"ViTRH");
    StructureFeatureIO::setPieceId(eStructurePiece_Smithy, Smithy::Create, L"ViS");
    StructureFeatureIO::setPieceId(eStructurePiece_Farmland, Farmland::Create, L"ViF");
    StructureFeatureIO::setPieceId(eStructurePiece_DoubleFarmland, DoubleFarmland::Create, L"ViDF");
    StructureFeatureIO::setPieceId(eStructurePiece_LightPost, LightPost::Create, L"ViLP");
    StructureFeatureIO::setPieceId(eStructurePiece_Well, Well::Create, L"ViW");
}
int VillagePieces::updatePieceWeight(std::list<PieceWeight*>* pieces)
{
    bool available = false;
    int total = 0;
    std::list<PieceWeight*>::iterator it;
    for (it = pieces->begin(); it != pieces->end(); ++it)
    {
        if ((*it)->isValid()) available = true;
        total += (*it)->weight;
    }
    return available ? total : -1;
}
StructurePiece* VillagePieces::findAndCreatePieceFactory(StartPiece* startPiece, PieceWeight* weight,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth)
{
    switch (weight->pieceClass)
    {
    case ePieceClass_StraightRoad: return StraightRoad::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_SimpleHouse: return SimpleHouse::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_SmallTemple: return SmallTemple::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_BookHouse: return BookHouse::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_SmallHut: return SmallHut::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_PigHouse: return PigHouse::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_TwoRoomHouse: return TwoRoomHouse::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_Smithy: return Smithy::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_Farmland: return Farmland::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_DoubleFarmland: return DoubleFarmland::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    default: return NULL;
    }
}
StructurePiece* VillagePieces::generatePieceFromSmallDoor(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
    Random& random, int x, int y, int z, const Direction* direction, int depth)
{
    int total = updatePieceWeight(startPiece->mPieceWeights);
    if (total < 1) return NULL;
    for (int attempt = 0; attempt < 5; ++attempt)
    {
        int choice = random.nextInt(total);
        std::list<PieceWeight*>::iterator it;
        for (it = startPiece->mPieceWeights->begin(); it != startPiece->mPieceWeights->end(); ++it)
        {
            choice -= (*it)->weight;
            if (choice < 0 && (*it)->doPlace(depth))
            {
                StructurePiece* piece = findAndCreatePieceFactory(startPiece, *it, pieces, random, x, y, z, direction, depth);
                if (piece != NULL)
                {
                    ++(*it)->placeCount;
                    startPiece->mLastPieceWeight = (*it)->placeCount;
                    if (!(*it)->isValid()) startPiece->mPieceWeights->remove(*it);
                    return piece;
                }
                break;
            }
        }
    }
    return NULL;
}
StructurePiece* VillagePieces::generateAndAddPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
    Random& random, int x, int y, int z, const Direction* direction, int depth)
{
    if (depth >= 50) return NULL;
    StructurePiece* piece = generatePieceFromSmallDoor(startPiece, pieces, random, x, y, z, direction, depth + 1);
    if (piece != NULL)
    {
        pieces->push_back(piece);
        startPiece->mPendingPieces.push_back(piece);
    }
    return piece;
}
StructurePiece* VillagePieces::generateAndAddRoadPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
    Random& random, int x, int y, int z, const Direction* direction, int depth)
{
    if (depth > startPiece->mSize + 3 || depth >= 50) return NULL;
    BoundingBox* box = StraightRoad::findPieceBox(startPiece, pieces, random, x, y, z, direction);
    if (box == NULL) return NULL;
    StructurePiece* road = new StraightRoad(startPiece, depth, random, box, direction);
    pieces->push_back(road);
    startPiece->mRoadPieces.push_back(road);
    return road;
}
VillagePieces::PieceWeight::PieceWeight(EPieceClass kind, int chance, int maximum)
    : pieceClass(kind), weight(chance), placeCount(0), maxPlaceCount(maximum) {}
VillagePieces::PieceWeight::PieceWeight() : pieceClass(ePieceClass_StraightRoad), weight(0), placeCount(0), maxPlaceCount(0) {}
bool VillagePieces::PieceWeight::doPlace(int depth) { return maxPlaceCount == 0 || placeCount < maxPlaceCount; }
bool VillagePieces::PieceWeight::isValid() { return maxPlaceCount == 0 || placeCount < maxPlaceCount; }
VillagePieces::VillagePiece::VillagePiece(StartPiece* startPiece, int depth)
    : StructurePiece(depth), mType(0), mWidth(0), mHeight(0), mHasMadeChest(false), mCobblestoneSelector(NULL), mAverageGroundHeight(-1), mVillagerCount(0), mStartPiece(startPiece) {}
VillagePieces::VillagePiece::VillagePiece()
    : StructurePiece(), mType(0), mWidth(0), mHeight(0), mHasMadeChest(false), mCobblestoneSelector(NULL), mAverageGroundHeight(-1), mVillagerCount(0), mStartPiece(NULL) {}
VillagePieces::VillagePiece::~VillagePiece() { delete mCobblestoneSelector; }
void VillagePieces::VillagePiece::setStructureType(int type) { mType = type; }
bool VillagePieces::VillagePiece::isOkBox(BoundingBox* box, StartPiece* startPiece) { return box != NULL && startPiece != NULL; }
void VillagePieces::VillagePiece::addAdditionalSaveData(DataOutputStream* output) { output->writeInt(mType); output->writeInt(mAverageGroundHeight); }
void VillagePieces::VillagePiece::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { mType = input->readInt(); mAverageGroundHeight = input->readInt(); }
unsigned int VillagePieces::VillagePiece::generateHouseNorthernLeft(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int z) {
return generateAndAddPiece(startPiece, pieces, random, getBoundingBox()->x0 + z, getBoundingBox()->y0 + x, getBoundingBox()->z1 + 1, Direction::NORTH, getGenDepth()) != NULL;
}
unsigned int VillagePieces::VillagePiece::generateHouseNorthernRight(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int z) {
return generateAndAddPiece(startPiece, pieces, random, getBoundingBox()->x1 + 1, getBoundingBox()->y0 + x, getBoundingBox()->z0 + z, Direction::EAST, getGenDepth()) != NULL;
}
int VillagePieces::VillagePiece::getAverageGroundHeight(Level* level, BoundingBox* box) { return box == NULL ? -1 : box->y0; }
void VillagePieces::VillagePiece::spawnVillagers(Level* level, BoundingBox* box, int x, int y, int z, int count) { mVillagerCount += count; }
int VillagePieces::VillagePiece::getVillagerProfession(int x, int z) { return 0; }
const BlockState* VillagePieces::VillagePiece::biomeBlock(const BlockState* state) { return state; }
const BlockState* VillagePieces::VillagePiece::biomeDoor() { Block* block = Block::byId(64); return block == NULL ? NULL : block->defaultBlockState(); }
void VillagePieces::VillagePiece::createVillageDoor(Level* level, BoundingBox* box, Random& random, int x, int y, int z, const Direction* direction) { createDoor(level, box, random, x, y, z, direction, NULL); }
void VillagePieces::VillagePiece::placeTorch(Level* level, const Direction* direction, int x, int y, int z, BoundingBox* box) { Random random; placeBlock(level, random, biomeBlock(NULL), NULL, x, y, z, box); }
void VillagePieces::VillagePiece::placeBlock(Level* level, Random& random, const BlockState* state, StructurePiece::BlockSelector* selector, int x, int y, int z, BoundingBox* box) {
if (state != NULL) StructurePiece::placeBlock(level, state, x, y, z, box);
}
VillagePieces::VillageRoadPiece::~VillageRoadPiece() {}
VillagePieces::Well::Well(StartPiece* startPiece, int depth, Random& random, int x, int z) : VillagePiece(startPiece, depth) { setBoundingBox(villageBox(x, 0, z, 4, 4, 4, Direction::NORTH)); }
VillagePieces::Well::Well() : VillagePiece() {}
VillagePieces::Well::~Well() {}
void VillagePieces::Well::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random) {
generateAndAddRoadPiece(static_cast<StartPiece*>(startPiece), pieces, random, getBoundingBox()->x0, getBoundingBox()->y0, getBoundingBox()->z1 + 1, Direction::SOUTH, getGenDepth());
}
bool VillagePieces::Well::postProcess(Level* level, Random& random, BoundingBox* box) {
const BlockState* state = Block::byId(4)->defaultBlockState();
generateBox(level, box, 0, 0, 0, 3, 3, 3, state, state, false);
return true;
}
StructurePiece* VillagePieces::Well::Create() { return new Well(); }
EStructurePiece VillagePieces::Well::GetType() { return eStructurePiece_Well; }
VillagePieces::StartPiece::StartPiece(BiomeSource* source, int depth, Random& random, int x, int z,
    std::list<PieceWeight*>* pieces, int size, Level* level)
    : Well(), mBiomeSource(source), mSize(size), mHasPlacedWell(false), mLastPieceWeight(-1),
      mNextPieceWeight(0), mLevel(level), mPendingPieces(), mRoadPieces(), mPieceWeights(pieces) {
setGenDepth(depth);
setBoundingBox(villageBox(x, 0, z, 4, 4, 4, Direction::NORTH));
}
VillagePieces::StartPiece::~StartPiece()
{
    std::list<PieceWeight*>::iterator it;
    for (it = mPieceWeights->begin(); it != mPieceWeights->end(); ++it)
    {
        delete *it;
    }
    delete mPieceWeights;
}
BiomeSource* VillagePieces::StartPiece::getBiomeSource() { return mBiomeSource; }
EStructurePiece VillagePieces::StartPiece::GetType() { return eStructurePiece_VillageStartPiece; }
static bool villageProcess(StructurePiece* piece, Level* level, Random& random, BoundingBox* box, int width, int height, int length)
{
    Block* block = Block::byId(4);
    if (block == NULL) return false;
    const BlockState* state = block->defaultBlockState();
    piece->generateBox(level, box, 0, 0, 0, width - 1, height - 1, length - 1, state, state, false);
    return true;
}
VillagePieces::StraightRoad::StraightRoad() : VillagePiece() {} 
VillagePieces::StraightRoad::StraightRoad(StartPiece* startPiece, int depth, Random& random, BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth) {
setBoundingBox(box);
setOrientation(direction);
} 
VillagePieces::StraightRoad::~StraightRoad() {} 
StructurePiece* VillagePieces::StraightRoad::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth) {
BoundingBox* box = villageBox(x, y, z, 3, 3, 5, direction);
return villageFits(pieces, box) ? new StraightRoad(startPiece, depth, random, box, direction) : NULL;
} 
bool VillagePieces::StraightRoad::postProcess(Level* level, Random& random, BoundingBox* box) { return villageProcess(this, level, random, box, 3, 3, 5); } 
int VillagePieces::StraightRoad::getVillagerProfession(int x, int z) { return 0; } 
StructurePiece* VillagePieces::StraightRoad::Create() { return new StraightRoad(); } 
EStructurePiece VillagePieces::StraightRoad::GetType() { return eStructurePiece_StraightRoad; } 
void VillagePieces::StraightRoad::addAdditionalSaveData(DataOutputStream* output) { VillagePiece::addAdditionalSaveData(output); } 
void VillagePieces::StraightRoad::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { VillagePiece::readAdditionalSaveData(input, manager); } 
void VillagePieces::StraightRoad::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random) {
generateAndAddPiece(static_cast<StartPiece*>(startPiece), pieces, random, getBoundingBox()->x1 + 1, getBoundingBox()->y0, getBoundingBox()->z0, Direction::EAST, getGenDepth());
} 
BoundingBox* VillagePieces::StraightRoad::findPieceBox(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction) {
BoundingBox* box = villageBox(x, y, z, 3, 3, 5, direction);
return villageFits(pieces, box) ? box : NULL;
} 
void VillagePieces::StraightRoad::staticCtor() {}
VillagePieces::SimpleHouse::SimpleHouse() : VillagePiece() {} 
VillagePieces::SimpleHouse::SimpleHouse(StartPiece* startPiece, int depth, Random& random, BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth) {
setBoundingBox(box);
setOrientation(direction);
} 
VillagePieces::SimpleHouse::~SimpleHouse() {} 
StructurePiece* VillagePieces::SimpleHouse::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth) {
BoundingBox* box = villageBox(x, y, z, 5, 6, 5, direction);
return villageFits(pieces, box) ? new SimpleHouse(startPiece, depth, random, box, direction) : NULL;
} 
bool VillagePieces::SimpleHouse::postProcess(Level* level, Random& random, BoundingBox* box) { return villageProcess(this, level, random, box, 5, 6, 5); } 
int VillagePieces::SimpleHouse::getVillagerProfession(int x, int z) { return 0; } 
StructurePiece* VillagePieces::SimpleHouse::Create() { return new SimpleHouse(); } 
EStructurePiece VillagePieces::SimpleHouse::GetType() { return eStructurePiece_SimpleHouse; } 
void VillagePieces::SimpleHouse::addAdditionalSaveData(DataOutputStream* output) { VillagePiece::addAdditionalSaveData(output); } 
void VillagePieces::SimpleHouse::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { VillagePiece::readAdditionalSaveData(input, manager); } 
void VillagePieces::SimpleHouse::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random) {
generateAndAddPiece(static_cast<StartPiece*>(startPiece), pieces, random, getBoundingBox()->x1 + 1, getBoundingBox()->y0, getBoundingBox()->z0, Direction::EAST, getGenDepth());
} 
BoundingBox* VillagePieces::SimpleHouse::findPieceBox(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction) {
BoundingBox* box = villageBox(x, y, z, 5, 6, 5, direction);
return villageFits(pieces, box) ? box : NULL;
} 
void VillagePieces::SimpleHouse::staticCtor() {}
VillagePieces::SmallTemple::SmallTemple() : VillagePiece() {} 
VillagePieces::SmallTemple::SmallTemple(StartPiece* startPiece, int depth, Random& random, BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth) {
setBoundingBox(box);
setOrientation(direction);
} 
VillagePieces::SmallTemple::~SmallTemple() {} 
StructurePiece* VillagePieces::SmallTemple::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth) {
BoundingBox* box = villageBox(x, y, z, 5, 6, 5, direction);
return villageFits(pieces, box) ? new SmallTemple(startPiece, depth, random, box, direction) : NULL;
} 
bool VillagePieces::SmallTemple::postProcess(Level* level, Random& random, BoundingBox* box) { return villageProcess(this, level, random, box, 5, 6, 5); } 
int VillagePieces::SmallTemple::getVillagerProfession(int x, int z) { return 0; } 
StructurePiece* VillagePieces::SmallTemple::Create() { return new SmallTemple(); } 
EStructurePiece VillagePieces::SmallTemple::GetType() { return eStructurePiece_SmallTemple; } 
void VillagePieces::SmallTemple::addAdditionalSaveData(DataOutputStream* output) { VillagePiece::addAdditionalSaveData(output); } 
void VillagePieces::SmallTemple::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { VillagePiece::readAdditionalSaveData(input, manager); } 
void VillagePieces::SmallTemple::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random) {
generateAndAddPiece(static_cast<StartPiece*>(startPiece), pieces, random, getBoundingBox()->x1 + 1, getBoundingBox()->y0, getBoundingBox()->z0, Direction::EAST, getGenDepth());
} 
BoundingBox* VillagePieces::SmallTemple::findPieceBox(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction) {
BoundingBox* box = villageBox(x, y, z, 5, 6, 5, direction);
return villageFits(pieces, box) ? box : NULL;
} 
void VillagePieces::SmallTemple::staticCtor() {}
VillagePieces::BookHouse::BookHouse() : VillagePiece() {} 
VillagePieces::BookHouse::BookHouse(StartPiece* startPiece, int depth, Random& random, BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth) { setBoundingBox(box); setOrientation(direction); } 
VillagePieces::BookHouse::~BookHouse() {} 
StructurePiece* VillagePieces::BookHouse::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth) {
BoundingBox* box = villageBox(x, y, z, 9, 6, 6, direction);
return villageFits(pieces, box) ? new BookHouse(startPiece, depth, random, box, direction) : NULL;
} 
bool VillagePieces::BookHouse::postProcess(Level* level, Random& random, BoundingBox* box) { return villageProcess(this, level, random, box, 9, 6, 6); } 
int VillagePieces::BookHouse::getVillagerProfession(int x, int z) { return 0; } 
StructurePiece* VillagePieces::BookHouse::Create() { return new BookHouse(); } 
EStructurePiece VillagePieces::BookHouse::GetType() { return eStructurePiece_BookHouse; } 
void VillagePieces::BookHouse::addAdditionalSaveData(DataOutputStream* output) { VillagePiece::addAdditionalSaveData(output); } 
void VillagePieces::BookHouse::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { VillagePiece::readAdditionalSaveData(input, manager); } 
void VillagePieces::BookHouse::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random) {
generateAndAddPiece(static_cast<StartPiece*>(startPiece), pieces, random, getBoundingBox()->x1 + 1, getBoundingBox()->y0, getBoundingBox()->z0, Direction::EAST, getGenDepth());
} 
BoundingBox* VillagePieces::BookHouse::findPieceBox(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction) {
BoundingBox* box = villageBox(x, y, z, 9, 6, 6, direction);
return villageFits(pieces, box) ? box : NULL;
} 
void VillagePieces::BookHouse::staticCtor() {}
VillagePieces::SmallHut::SmallHut() : VillagePiece() {} 
VillagePieces::SmallHut::SmallHut(StartPiece* startPiece, int depth, Random& random, BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth) { setBoundingBox(box); setOrientation(direction); } 
VillagePieces::SmallHut::~SmallHut() {} 
StructurePiece* VillagePieces::SmallHut::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth) {
BoundingBox* box = villageBox(x, y, z, 4, 5, 4, direction);
return villageFits(pieces, box) ? new SmallHut(startPiece, depth, random, box, direction) : NULL;
} 
bool VillagePieces::SmallHut::postProcess(Level* level, Random& random, BoundingBox* box) { return villageProcess(this, level, random, box, 4, 5, 4); } 
int VillagePieces::SmallHut::getVillagerProfession(int x, int z) { return 0; } 
StructurePiece* VillagePieces::SmallHut::Create() { return new SmallHut(); } 
EStructurePiece VillagePieces::SmallHut::GetType() { return eStructurePiece_SmallHut; } 
void VillagePieces::SmallHut::addAdditionalSaveData(DataOutputStream* output) { VillagePiece::addAdditionalSaveData(output); } 
void VillagePieces::SmallHut::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { VillagePiece::readAdditionalSaveData(input, manager); } 
void VillagePieces::SmallHut::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random) {
generateAndAddPiece(static_cast<StartPiece*>(startPiece), pieces, random, getBoundingBox()->x1 + 1, getBoundingBox()->y0, getBoundingBox()->z0, Direction::EAST, getGenDepth());
} 
BoundingBox* VillagePieces::SmallHut::findPieceBox(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction) {
BoundingBox* box = villageBox(x, y, z, 4, 5, 4, direction);
return villageFits(pieces, box) ? box : NULL;
} 
void VillagePieces::SmallHut::staticCtor() {}
VillagePieces::PigHouse::PigHouse() : VillagePiece() {} 
VillagePieces::PigHouse::PigHouse(StartPiece* startPiece, int depth, Random& random, BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth) { setBoundingBox(box); setOrientation(direction); } 
VillagePieces::PigHouse::~PigHouse() {} 
StructurePiece* VillagePieces::PigHouse::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth) {
BoundingBox* box = villageBox(x, y, z, 5, 5, 5, direction);
return villageFits(pieces, box) ? new PigHouse(startPiece, depth, random, box, direction) : NULL;
} 
bool VillagePieces::PigHouse::postProcess(Level* level, Random& random, BoundingBox* box) { return villageProcess(this, level, random, box, 5, 5, 5); } 
int VillagePieces::PigHouse::getVillagerProfession(int x, int z) { return 0; } 
StructurePiece* VillagePieces::PigHouse::Create() { return new PigHouse(); } 
EStructurePiece VillagePieces::PigHouse::GetType() { return eStructurePiece_PigHouse; } 
void VillagePieces::PigHouse::addAdditionalSaveData(DataOutputStream* output) { VillagePiece::addAdditionalSaveData(output); } 
void VillagePieces::PigHouse::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { VillagePiece::readAdditionalSaveData(input, manager); } 
void VillagePieces::PigHouse::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random) {
generateAndAddPiece(static_cast<StartPiece*>(startPiece), pieces, random, getBoundingBox()->x1 + 1, getBoundingBox()->y0, getBoundingBox()->z0, Direction::EAST, getGenDepth());
} 
BoundingBox* VillagePieces::PigHouse::findPieceBox(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction) {
BoundingBox* box = villageBox(x, y, z, 5, 5, 5, direction);
return villageFits(pieces, box) ? box : NULL;
} 
void VillagePieces::PigHouse::staticCtor() {}
VillagePieces::TwoRoomHouse::TwoRoomHouse() : VillagePiece() {} 
VillagePieces::TwoRoomHouse::TwoRoomHouse(StartPiece* startPiece, int depth, Random& random, BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth) {
setBoundingBox(box);
setOrientation(direction);
} 
VillagePieces::TwoRoomHouse::~TwoRoomHouse() {} 
StructurePiece* VillagePieces::TwoRoomHouse::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth) {
BoundingBox* box = villageBox(x, y, z, 9, 6, 6, direction);
return villageFits(pieces, box) ? new TwoRoomHouse(startPiece, depth, random, box, direction) : NULL;
} 
bool VillagePieces::TwoRoomHouse::postProcess(Level* level, Random& random, BoundingBox* box) { return villageProcess(this, level, random, box, 9, 6, 6); } 
int VillagePieces::TwoRoomHouse::getVillagerProfession(int x, int z) { return 0; } 
StructurePiece* VillagePieces::TwoRoomHouse::Create() { return new TwoRoomHouse(); } 
EStructurePiece VillagePieces::TwoRoomHouse::GetType() { return eStructurePiece_TwoRoomHouse; } 
void VillagePieces::TwoRoomHouse::addAdditionalSaveData(DataOutputStream* output) { VillagePiece::addAdditionalSaveData(output); } 
void VillagePieces::TwoRoomHouse::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { VillagePiece::readAdditionalSaveData(input, manager); } 
void VillagePieces::TwoRoomHouse::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random) {
generateAndAddPiece(static_cast<StartPiece*>(startPiece), pieces, random, getBoundingBox()->x1 + 1, getBoundingBox()->y0, getBoundingBox()->z0, Direction::EAST, getGenDepth());
} 
BoundingBox* VillagePieces::TwoRoomHouse::findPieceBox(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction) {
BoundingBox* box = villageBox(x, y, z, 9, 6, 6, direction);
return villageFits(pieces, box) ? box : NULL;
} 
void VillagePieces::TwoRoomHouse::staticCtor() {}
VillagePieces::Smithy::Smithy() : VillagePiece() {} 
VillagePieces::Smithy::Smithy(StartPiece* startPiece, int depth, Random& random, BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth) { setBoundingBox(box); setOrientation(direction); } 
VillagePieces::Smithy::~Smithy() {} 
StructurePiece* VillagePieces::Smithy::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth) {
BoundingBox* box = villageBox(x, y, z, 7, 6, 9, direction);
return villageFits(pieces, box) ? new Smithy(startPiece, depth, random, box, direction) : NULL;
} 
bool VillagePieces::Smithy::postProcess(Level* level, Random& random, BoundingBox* box) { return villageProcess(this, level, random, box, 7, 6, 9); } 
int VillagePieces::Smithy::getVillagerProfession(int x, int z) { return 0; } 
StructurePiece* VillagePieces::Smithy::Create() { return new Smithy(); } 
EStructurePiece VillagePieces::Smithy::GetType() { return eStructurePiece_Smithy; } 
void VillagePieces::Smithy::addAdditionalSaveData(DataOutputStream* output) { VillagePiece::addAdditionalSaveData(output); } 
void VillagePieces::Smithy::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { VillagePiece::readAdditionalSaveData(input, manager); } 
void VillagePieces::Smithy::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random) {
generateAndAddPiece(static_cast<StartPiece*>(startPiece), pieces, random, getBoundingBox()->x1 + 1, getBoundingBox()->y0, getBoundingBox()->z0, Direction::EAST, getGenDepth());
} 
BoundingBox* VillagePieces::Smithy::findPieceBox(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction) {
BoundingBox* box = villageBox(x, y, z, 7, 6, 9, direction);
return villageFits(pieces, box) ? box : NULL;
} 
void VillagePieces::Smithy::staticCtor() {}
VillagePieces::Farmland::Farmland() : VillagePiece(), mCropA(0), mCropB(0) {}
VillagePieces::Farmland::Farmland(StartPiece* startPiece, int depth, Random& random, BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth), mCropA(0), mCropB(0) {
setBoundingBox(box);
setOrientation(direction);
selectCrops(random);
}
VillagePieces::Farmland::~Farmland() {}
unsigned int VillagePieces::Farmland::selectCrops(Random& random) { mCropA = random.nextInt(4); mCropB = random.nextInt(4); return mCropA; }
StructurePiece* VillagePieces::Farmland::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth) {
BoundingBox* box = villageBox(x, y, z, 7, 4, 9, direction);
return villageFits(pieces, box) ? new Farmland(startPiece, depth, random, box, direction) : NULL;
}
int VillagePieces::Farmland::selectOldCrops(int crop) { return crop < 0 ? 0 : crop; }
void VillagePieces::Farmland::addAdditionalSaveData(DataOutputStream* output) { VillagePiece::addAdditionalSaveData(output); output->writeInt(mCropA); output->writeInt(mCropB); }
void VillagePieces::Farmland::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { VillagePiece::readAdditionalSaveData(input, manager); mCropA = input->readInt(); mCropB = input->readInt(); }
bool VillagePieces::Farmland::postProcess(Level* level, Random& random, BoundingBox* box) { return villageProcess(this, level, random, box, 7, 4, 9); }
StructurePiece* VillagePieces::Farmland::Create() { return new Farmland(); }
EStructurePiece VillagePieces::Farmland::GetType() { return eStructurePiece_Farmland; }
VillagePieces::DoubleFarmland::DoubleFarmland() : Farmland(), mCropC(0), mCropD(0) {}
VillagePieces::DoubleFarmland::DoubleFarmland(StartPiece* startPiece, int depth, Random& random, BoundingBox* box, const Direction* direction) : Farmland(startPiece, depth, random, box, direction), mCropC(0), mCropD(0) {
selectCrops(random);
}
VillagePieces::DoubleFarmland::~DoubleFarmland() {}
unsigned int VillagePieces::DoubleFarmland::selectCrops(Random& random) { mCropC = random.nextInt(4); mCropD = random.nextInt(4); return mCropC; }
StructurePiece* VillagePieces::DoubleFarmland::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth) {
BoundingBox* box = villageBox(x, y, z, 7, 4, 9, direction);
return villageFits(pieces, box) ? new DoubleFarmland(startPiece, depth, random, box, direction) : NULL;
}
int VillagePieces::DoubleFarmland::selectOldCrops(int crop) { return crop < 0 ? 0 : crop; }
void VillagePieces::DoubleFarmland::addAdditionalSaveData(DataOutputStream* output) { Farmland::addAdditionalSaveData(output); output->writeInt(mCropC); output->writeInt(mCropD); }
void VillagePieces::DoubleFarmland::readAdditionalSaveData(DataInputStream* input, StructureManager* manager) { Farmland::readAdditionalSaveData(input, manager); mCropC = input->readInt(); mCropD = input->readInt(); }
bool VillagePieces::DoubleFarmland::postProcess(Level* level, Random& random, BoundingBox* box) { return villageProcess(this, level, random, box, 7, 4, 9); }
StructurePiece* VillagePieces::DoubleFarmland::Create() { return new DoubleFarmland(); }
EStructurePiece VillagePieces::DoubleFarmland::GetType() { return eStructurePiece_DoubleFarmland; }
VillagePieces::LightPost::LightPost() : VillagePiece() {}
VillagePieces::LightPost::LightPost(StartPiece* startPiece, int depth, Random& random, BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth) { setBoundingBox(box); setOrientation(direction); }
VillagePieces::LightPost::~LightPost() {}
BoundingBox* VillagePieces::LightPost::findPieceBox(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction) {
BoundingBox* box = villageBox(x, y, z, 3, 4, 3, direction);
return villageFits(pieces, box) ? box : NULL;
}
bool VillagePieces::LightPost::postProcess(Level* level, Random& random, BoundingBox* box) { return villageProcess(this, level, random, box, 3, 4, 3); }
StructurePiece* VillagePieces::LightPost::Create() { return new LightPost(); }
EStructurePiece VillagePieces::LightPost::GetType() { return eStructurePiece_LightPost; }
VillagePieces::CobbleStoneSelector::CobbleStoneSelector(bool mossy, int type) : mMossy(mossy), mType(type) {}
VillagePieces::CobbleStoneSelector::~CobbleStoneSelector() {}
void VillagePieces::CobbleStoneSelector::next(Random& random, int x, int y, int z, bool edge) { mNext = Block::byId(edge && mMossy ? 48 : mType)->defaultBlockState(); }
