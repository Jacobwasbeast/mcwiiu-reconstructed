#include "game/world/generation/VillagePieces.hpp"

#include <cstdlib>

#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Mth.hpp"

#include "game/world/biome/Biome.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/LogBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/SandStoneBlock.hpp"
#include "game/world/block/TorchBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/ZombieVillager.hpp"
static BoundingBox* villageBox(int x, int y, int z, int width, int height, int length, const Direction* direction)
{
    return BoundingBox::orientBox(x, y, z, 0, 0, 0, width, height, length, direction);
}
static const std::vector<Biome*>& villageBiomes()
{
    static std::vector<Biome*> biomes;
    if (biomes.empty())
    {
        biomes.push_back(Biome::PLAINS);
        biomes.push_back(Biome::DESERT);
        biomes.push_back(Biome::SAVANNA);
        biomes.push_back(Biome::TAIGA);
        biomes.push_back(Biome::COLD_TAIGA);
        biomes.push_back(Biome::ICE_PLAINS);
    }
    return biomes;
}
static const BlockState* villageLogState(Block* target, PlanksBlock::Variant* variant, const BlockState* source)
{
    TypedBoxed<PlanksBlock::Variant*> woodVariant(variant);
    return target->defaultBlockState()
        ->setBoxedValue(OldLogBlock::VARIANT, &woodVariant)
        ->setBoxedValue(LogBlock::AXIS, source->getBoxedValue(LogBlock::AXIS));
}
static const BlockState* villageStairState(Block* target, const BlockState* source)
{
    return target->defaultBlockState()->setBoxedValue(
        static_cast<const Property*>(HorizontalDirectionalBlock::FACING_PROPERTY),
        source->getBoxedValue(static_cast<const Property*>(HorizontalDirectionalBlock::FACING_PROPERTY)));
}
std::list<VillagePieces::PieceWeight*>* VillagePieces::createPieceSet(Random& random, int size)
{
    std::list<PieceWeight*>* pieces = new std::list<PieceWeight*>();
    pieces->push_back(new PieceWeight(ePieceClass_SimpleHouse, 4, random.nextInt(size + 2, size * 2 + 4)));
    pieces->push_back(new PieceWeight(ePieceClass_SmallTemple, 20, random.nextInt(size, size + 1)));
    pieces->push_back(new PieceWeight(ePieceClass_BookHouse, 20, random.nextInt(size, size + 2)));
    pieces->push_back(new PieceWeight(ePieceClass_SmallHut, 3, random.nextInt(size + 2, size * 3 + 5)));
    pieces->push_back(new PieceWeight(ePieceClass_PigHouse, 15, random.nextInt(size, size + 2)));
    pieces->push_back(new PieceWeight(ePieceClass_DoubleFarmland, 3, random.nextInt(size + 2, size * 2 + 4)));
    pieces->push_back(new PieceWeight(ePieceClass_Farmland, 3, random.nextInt(0, size + 1)));
    pieces->push_back(new PieceWeight(ePieceClass_Smithy, 15, random.nextInt(size, size * 2 + 3)));
    pieces->push_back(new PieceWeight(ePieceClass_TwoRoomHouse, 8, random.nextInt(size, size * 2 + 3)));
    for (std::list<PieceWeight*>::iterator it = pieces->begin(); it != pieces->end();)
    {
        if ((*it)->maxPlaceCount == 0)
        {
            delete *it;
            it = pieces->erase(it);
        }
        else
        {
            ++it;
        }
    }
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
    case ePieceClass_SimpleHouse: return SimpleHouse::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_SmallTemple: return SmallTemple::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_BookHouse: return BookHouse::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_SmallHut: return SmallHut::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_PigHouse: return PigHouse::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_Smithy: return Smithy::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_DoubleFarmland: return DoubleFarmland::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_Farmland: return Farmland::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_TwoRoomHouse: return TwoRoomHouse::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
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
                if (*it == startPiece->mLastPieceWeight && startPiece->mPieceWeights->size() > 1)
                {
                    break;
                }
                StructurePiece* piece = findAndCreatePieceFactory(startPiece, *it, pieces, random, x, y, z, direction, depth);
                if (piece != NULL)
                {
                    ++(*it)->placeCount;
                    startPiece->mLastPieceWeight = *it;
                    if (!(*it)->isValid()) startPiece->mPieceWeights->remove(*it);
                    return piece;
                }
            }
        }
    }
    BoundingBox* box = LightPost::findPieceBox(startPiece, pieces, random, x, y, z, direction);
    if (box != NULL)
    {
        return new LightPost(startPiece, depth, random, box, direction);
    }
    return NULL;
}
StructurePiece* VillagePieces::generateAndAddPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
    Random& random, int x, int y, int z, const Direction* direction, int depth)
{
    if (depth >= 51) return NULL;
    BoundingBox* startBox = startPiece->getBoundingBox();
    if (startBox == NULL || std::abs(x - startBox->x0) >= 113 || std::abs(z - startBox->z0) >= 113)
    {
        return NULL;
    }
    StructurePiece* piece = generatePieceFromSmallDoor(startPiece, pieces, random, x, y, z, direction, depth + 1);
    if (piece != NULL)
    {
        BoundingBox* pieceBox = piece->getBoundingBox();
        int width = pieceBox->x1 - pieceBox->x0;
        int length = pieceBox->z1 - pieceBox->z0;
        int radius = width > length ? width : length;
        if (startPiece->getBiomeSource()->containsOnly((pieceBox->x0 + pieceBox->x1) / 2,
                                                        (pieceBox->z0 + pieceBox->z1) / 2,
                                                        radius / 2 + 4, villageBiomes()))
        {
            pieces->push_back(piece);
            startPiece->mPendingPieces.push_back(piece);
            return piece;
        }
        delete piece;
    }
    return NULL;
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
    : pieceClass(kind), weight(chance), placeCount(0), maxPlaceCount(maximum)
{
}
VillagePieces::PieceWeight::PieceWeight() : pieceClass(ePieceClass_StraightRoad), weight(0), placeCount(0), maxPlaceCount(0)
{
}
bool VillagePieces::PieceWeight::doPlace(int depth)
{
    return maxPlaceCount == 0 || placeCount < maxPlaceCount;
}
bool VillagePieces::PieceWeight::isValid()
{
    return maxPlaceCount == 0 || placeCount < maxPlaceCount;
}
VillagePieces::VillagePiece::VillagePiece(StartPiece* startPiece, int depth)
    : StructurePiece(depth), mType(0), mWidth(-1), mHeight(0), mHasMadeChest(false),
      mCobblestoneSelector(NULL), mAverageGroundHeight(-1), mVillagerCount(0),
      mFirstBools(), mSecondBools(), mFirstInts(), mSecondInts(), mStartPiece(startPiece)
{
    if (startPiece != NULL)
    {
        mType = startPiece->mType;
        mHasMadeChest = startPiece->mHasMadeChest;
    }
    mCobblestoneSelector = new CobbleStoneSelector(mHasMadeChest, mType);
}
VillagePieces::VillagePiece::VillagePiece()
    : StructurePiece(), mType(0), mWidth(-1), mHeight(0), mHasMadeChest(false),
      mCobblestoneSelector(NULL), mAverageGroundHeight(-1), mVillagerCount(0),
      mFirstBools(), mSecondBools(), mFirstInts(), mSecondInts(), mStartPiece(NULL)
{
    mCobblestoneSelector = new CobbleStoneSelector(mHasMadeChest, mType);
}
VillagePieces::VillagePiece::~VillagePiece()
{
    delete mCobblestoneSelector;
}
void VillagePieces::VillagePiece::setStructureType(int type)
{
    mType = type;
}
bool VillagePieces::VillagePiece::isOkBox(BoundingBox* box, StartPiece* startPiece)
{
    if (box == NULL || startPiece == NULL || startPiece->mLevel == NULL || startPiece->mLevel->getLevelData() == NULL)
        return false;
    int half = (startPiece->mLevel->getLevelData()->getXZSize() << 4) >> 1;
    int lower = 1 - half;
    int upper = half - 1;
    return box->x1 < upper && lower < box->z0 && lower < box->x0 && box->y0 > 10 && box->z1 < upper;
}
void VillagePieces::VillagePiece::addAdditionalSaveData(DataOutputStream* output)
{
    if (mStartPiece != NULL) mType = mStartPiece->mType;
    output->writeInt(mWidth);
    output->writeInt(mHeight);
    output->writeByte(static_cast<unsigned char>(mType));
    output->writeBoolean(mHasMadeChest);
}
void VillagePieces::VillagePiece::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    mWidth = input->readInt();
    if (StructureFeatureIO::getVersion() == 1 && mStartPiece != NULL)
        mStartPiece->readStructurePieceSaveData(input, mFirstBools, mSecondBools, mFirstInts, mSecondInts);
    mHeight = input->readInt();
    if (StructureFeatureIO::getVersion() < 2)
    {
        if (input->readBoolean()) mType = 1;
    }
    else
    {
        mType = input->readByte();
        mHasMadeChest = input->readBoolean();
        delete mCobblestoneSelector;
        mCobblestoneSelector = new CobbleStoneSelector(mHasMadeChest, mType);
    }
    if (mStartPiece != NULL) mType = mStartPiece->mType;
}
unsigned int VillagePieces::VillagePiece::generateHouseNorthernLeft(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int z)
{
    const Direction* orientation = getOrientation();
    if (orientation == NULL) return false;
    if (orientation->get3DDataValue() > 3 && orientation->get3DDataValue() < 6)
        return generateAndAddPiece(startPiece, pieces, random, getBoundingBox()->x0 + z,
                                    getBoundingBox()->y0 + x, getBoundingBox()->z0 - 1,
                                    Direction::NORTH, getGenDepth()) != NULL;
    return generateAndAddPiece(startPiece, pieces, random, getBoundingBox()->x0 - 1,
                                getBoundingBox()->y0 + x, getBoundingBox()->z0 + z,
                                Direction::WEST, getGenDepth()) != NULL;
}
unsigned int VillagePieces::VillagePiece::generateHouseNorthernRight(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int z)
{
    const Direction* orientation = getOrientation();
    if (orientation == NULL) return false;
    if (orientation->get3DDataValue() > 3 && orientation->get3DDataValue() < 6)
        return generateAndAddPiece(startPiece, pieces, random, getBoundingBox()->x0 + z,
                                    getBoundingBox()->y0 + x, getBoundingBox()->z1 + 1,
                                    Direction::SOUTH, getGenDepth()) != NULL;
    return generateAndAddPiece(startPiece, pieces, random, getBoundingBox()->x1 + 1,
                                getBoundingBox()->y0 + x, getBoundingBox()->z0 + z,
                                Direction::EAST, getGenDepth()) != NULL;
}
int VillagePieces::VillagePiece::getAverageGroundHeight(Level* level, BoundingBox* box)
{
    if (level == NULL || box == NULL) return -1;
    int total = 0;
    int samples = 0;
    for (int localZ = box->z0; localZ <= box->z1; ++localZ)
    {
        for (int localX = box->x0; localX <= box->x1; ++localX)
        {
            BlockPos position(localX, 64, localZ);
            if (box->isInside(position))
            {
                BlockPos top = level->getTopSolidBlockPos(position);
                int ground = top.y - 1;
                if (ground < box->y0) ground = box->y0;
                total += ground;
                ++samples;
            }
        }
    }
    return samples == 0 ? -1 : total / samples;
}
void VillagePieces::VillagePiece::spawnVillagers(Level* level, BoundingBox* box, int x, int y, int z, int count)
{
    if (level == NULL || box == NULL || mVillagerCount >= count) return;
    int remaining = count - mVillagerCount;
    while (remaining-- > 0)
    {
        int worldX = getWorldX(x + mVillagerCount, z);
        int worldY = getWorldY(y);
        int worldZ = getWorldZ(x + mVillagerCount, z);
        BlockPos position(worldX, worldY, worldZ);
        if (!box->isInside(position)) break;
        ++mVillagerCount;
        if (mHasMadeChest)
        {
            boost::shared_ptr<ZombieVillager> zombie(new ZombieVillager(level));
            zombie->setPos(worldX - 0.5, worldY, worldZ - 0.5);
            zombie->finalizeMobSpawn(level->getCurrentDifficultyAt(position), NULL, std::wstring(L"unknown"));
            zombie->setProfession(getVillagerProfession(mVillagerCount - 1, 0));
            zombie->setPersistenceRequired(true);
            level->addEntity(zombie);
            continue;
        }
        else
        {
            boost::shared_ptr<Villager> villager(new Villager(level));
            villager->setPos(worldX - 0.5, worldY, worldZ - 0.5);
            villager->setProfession(getVillagerProfession(mVillagerCount - 1, level->mRandom->nextInt(6)));
            villager->finalizeMobSpawn(level->getCurrentDifficultyAt(position), NULL, false, std::wstring(L"unknown"));
            villager->setPersistenceRequired(true);
            level->addEntity(villager);
        }
    }
}
int VillagePieces::VillagePiece::getVillagerProfession(int x, int z)
{
    return z;
}
const BlockState* VillagePieces::VillagePiece::biomeBlock(const BlockState* state)
{
    Block* block = state->getBlock();
    if (mType == 1)
    {
        if (block == Blocks::log)
            return Blocks::sandstone->getBlockState(SandStoneBlock::SANDSTONE->getData());
        if (block == Blocks::log2)
            return Blocks::sandstone->defaultBlockState();
        if (block == Blocks::cobblestone) return Blocks::sandstone->getBlockState(0);
        if (block == Blocks::planks)
        {
            TypedBoxed<SandStoneBlock::Variant*> variant(SandStoneBlock::SMOOTH);
            return Blocks::sandstone->defaultBlockState()->setBoxedValue(SandStoneBlock::TYPE, &variant);
        }
        if (block == Blocks::oak_stairs)
            return villageStairState(Blocks::sandstone_stairs, state);
        if (block == Blocks::stone_brick_stairs)
            return villageStairState(Blocks::sandstone_stairs, state);
        if (block == Blocks::grass_path) return Blocks::sandstone->getBlockState(SandStoneBlock::SANDSTONE->getData());
    }
    else if (mType == 2)
    {
        if (block == Blocks::log) return villageLogState(Blocks::log2, PlanksBlock::Variant::ACACIA, state);
        if (block == Blocks::log2) return villageLogState(Blocks::log2, PlanksBlock::Variant::ACACIA, state);
        if (block == Blocks::planks)
        {
            TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::ACACIA);
            return Blocks::planks->defaultBlockState()->setBoxedValue(PlanksBlock::VARIANT, &variant);
        }
        if (block == Blocks::oak_stairs)
            return villageStairState(Blocks::acacia_stairs, state);
        if (block == Blocks::fence) return Blocks::acacia_fence->defaultBlockState();
    }
    else if (mType == 3)
    {
        if (block == Blocks::log)
            return villageLogState(Blocks::log, PlanksBlock::Variant::SPRUCE, state);
        if (block == Blocks::log2)
            return villageLogState(Blocks::log, PlanksBlock::Variant::SPRUCE, state);
        if (block == Blocks::planks)
        {
            TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::SPRUCE);
            return Blocks::planks->defaultBlockState()->setBoxedValue(PlanksBlock::VARIANT, &variant);
        }
        if (block == Blocks::oak_stairs)
            return villageStairState(Blocks::spruce_stairs, state);
        if (block == Blocks::fence) return Blocks::spruce_fence->defaultBlockState();
    }
    return state;
}
DoorBlock* VillagePieces::VillagePiece::biomeDoor()
{
    if (mType == 2) return static_cast<DoorBlock*>(Blocks::dark_oak_door);
    if (mType == 3) return static_cast<DoorBlock*>(Blocks::spruce_door);
    return static_cast<DoorBlock*>(Blocks::wooden_door);
}
void VillagePieces::VillagePiece::createVillageDoor(Level* level, BoundingBox* box, Random& random,
    int x, int y, int z, const Direction* direction)
{
    if (!mHasMadeChest)
    {
        createDoor(level, box, random, x, y, z, direction, biomeDoor());
    }
}
void VillagePieces::VillagePiece::placeTorch(Level* level, const Direction* direction,
    int x, int y, int z, BoundingBox* box)
{
    if (!mHasMadeChest)
    {
        int data = 0;
        if (direction == Direction::EAST) data = 1;
        else if (direction == Direction::WEST) data = 2;
        else if (direction == Direction::SOUTH) data = 3;
        else if (direction == Direction::NORTH) data = 4;
        const BlockState* torch = Blocks::torch->getBlockState(data);
        StructurePiece::placeBlock(level, torch, x, y, z, box);
    }
}
void VillagePieces::VillagePiece::placeBlock(Level* level, Random& random, const BlockState* state,
    StructurePiece::BlockSelector* selector, int x, int y, int z, BoundingBox* box)
{
    if (selector == NULL)
    {
        StructurePiece::placeBlock(level, state, x, y, z, box);
        return;
    }
    selector->next(random, x, y, z, false);
    StructurePiece::placeBlock(level, selector->getNext(), x, y, z, box);
}
VillagePieces::VillageRoadPiece::~VillageRoadPiece()
{
}
VillagePieces::Well::Well(StartPiece* startPiece, int depth, Random& random, int x, int z)
    : VillagePiece(startPiece, depth)
{
    setOrientation(Direction::Plane::HORIZONTAL->getRandomDirection(random));
    setBoundingBox(new BoundingBox(x, 64, z, x + 5, 78, z + 5));
}
VillagePieces::Well::Well() : VillagePiece()
{
}
VillagePieces::Well::~Well()
{
}
void VillagePieces::Well::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random)
{
    StartPiece* start = static_cast<StartPiece*>(startPiece);
    generateAndAddRoadPiece(start, pieces, random, getBoundingBox()->x0 - 1, getBoundingBox()->y0 - 4,
        getBoundingBox()->z0 + 1, Direction::WEST, getGenDepth());
    generateAndAddRoadPiece(start, pieces, random, getBoundingBox()->x1 + 1, getBoundingBox()->y0 - 4,
        getBoundingBox()->z0 + 1, Direction::EAST, getGenDepth());
    generateAndAddRoadPiece(start, pieces, random, getBoundingBox()->x0 + 1, getBoundingBox()->y0 - 4,
        getBoundingBox()->z0 - 1, Direction::NORTH, getGenDepth());
    generateAndAddRoadPiece(start, pieces, random, getBoundingBox()->x0 + 1, getBoundingBox()->y0 - 4,
        getBoundingBox()->z1 + 1, Direction::SOUTH, getGenDepth());
}
bool VillagePieces::Well::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0) return true;
        getBoundingBox()->move(0, mAverageGroundHeight - getBoundingBox()->y1 + 3, 0);
    }
    const BlockState* water = Blocks::FLOWING_WATER->defaultBlockState();
    const BlockState* cobblestone = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* fence = biomeBlock(Blocks::fence->defaultBlockState());
    const BlockState* path = biomeBlock(Blocks::grass_path->defaultBlockState());
    generateBox(level, box, 1, 0, 1, 4, 12, 4, cobblestone, water, false);
    for (int x = 2; x <= 3; ++x)
        for (int z = 2; z <= 3; ++z)
            placeBlock(level, water, x, 12, z, box);
    for (int x = 0; x < 6; ++x)
    {
        placeBlock(level, cobblestone, x, 11, 0, box);
        fillColumnDown(level, path, x, 12, 0, box);
        placeBlock(level, cobblestone, x, 11, 5, box);
        fillColumnDown(level, path, x, 12, 5, box);
    }
    for (int z = 1; z < 5; ++z)
    {
        placeBlock(level, cobblestone, 0, 11, z, box);
        fillColumnDown(level, path, 0, 12, z, box);
        placeBlock(level, cobblestone, 5, 11, z, box);
        fillColumnDown(level, path, 5, 12, z, box);
    }
    generateBox(level, box, 1, 15, 1, 4, 15, 4, false, random, mCobblestoneSelector);
    return true;
}
StructurePiece* VillagePieces::Well::Create()
{
    return new Well();
}
EStructurePiece VillagePieces::Well::GetType()
{
    return eStructurePiece_Well;
}
VillagePieces::StartPiece::StartPiece(BiomeSource* source, int depth, Random& random, int x, int z,
    std::list<PieceWeight*>* pieces, int size, Level* level)
    : Well(this, 0, random, x, z), mBiomeSource(source), mSize(size), mHasPlacedWell(false), mLastPieceWeight(NULL),
      mLevel(level), mPendingPieces(), mRoadPieces(), mPieceWeights(pieces)
{
    Biome* biome = source->getBiome(BlockPos(x, 0, z), Biome::DEFAULT);
    if (biome == Biome::DESERT || biome == Biome::DESERT_MUTATED || biome == Biome::DESERT_HILLS)
    {
        mType = 1;
    }
    else if (biome == Biome::SAVANNA || biome == Biome::SAVANNA_MUTATED ||
             biome == Biome::SAVANNA_PLATEAU || biome == Biome::SAVANNA_PLATEAU_MUTATED)
    {
        mType = 2;
    }
    else if (biome == Biome::TAIGA || biome == Biome::TAIGA_HILLS || biome == Biome::COLD_TAIGA ||
             biome == Biome::COLD_TAIGA_HILLS || biome == Biome::ICE_PLAINS ||
             biome == Biome::ICE_PLAINS_SPIKES)
    {
        mType = 3;
    }
    mHasMadeChest = random.nextInt(50) == 0;
    delete mCobblestoneSelector;
    mCobblestoneSelector = new CobbleStoneSelector(mHasMadeChest, mType);
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
BiomeSource* VillagePieces::StartPiece::getBiomeSource()
{
    return mBiomeSource;
}
EStructurePiece VillagePieces::StartPiece::GetType()
{
    return eStructurePiece_VillageStartPiece;
}
VillagePieces::StraightRoad::StraightRoad() : VillagePiece()
{
}
VillagePieces::StraightRoad::StraightRoad(StartPiece* startPiece, int depth, Random& random,
    BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth)
{
setBoundingBox(box);
setOrientation(direction);
} 
VillagePieces::StraightRoad::~StraightRoad()
{
}
StructurePiece* VillagePieces::StraightRoad::createPiece(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
BoundingBox* box = villageBox(x, y, z, 3, 3, 5, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
    return new StraightRoad(startPiece, depth, random, box, direction);
delete box;
return NULL;
} 
bool VillagePieces::StraightRoad::postProcess(Level* level, Random& random, BoundingBox* box)
{
    const BlockState* path = biomeBlock(Blocks::grass_path->defaultBlockState());
    const BlockState* planks = biomeBlock(Blocks::planks->defaultBlockState());
    const BlockState* air = Blocks::AIR->defaultBlockState();
    const BlockState* water = Blocks::FLOWING_WATER->defaultBlockState();
    const BlockState* cobblestone = biomeBlock(Blocks::cobblestone->defaultBlockState());

    for (int x = box->x0; x <= box->x1; ++x)
    {
        for (int z = box->z0; z <= box->z1; ++z)
        {
            BlockPos position(x, 64, z);
            if (!box->isInside(position)) continue;
            BlockPos top = level->getTopSolidBlockPos(position).below();
            if (top.y < level->getSeaLevel()) top = BlockPos(top.x, level->getSeaLevel() - 1, top.z);
            while (top.y >= level->getSeaLevel() - 1)
            {
                const BlockState* current = level->getBlockState(top);
                if (current->getBlock() == Blocks::WATER || current->getBlock() == Blocks::FLOWING_WATER)
                {
                    placeBlock(level, path, top.x - box->x0, top.y - box->y0, top.z - box->z0, box);
                    break;
                }
                if (current->isSolidRender())
                {
                    placeBlock(level, planks, top.x - box->x0, top.y - box->y0, top.z - box->z0, box);
                    break;
                }
                top = top.below();
            }
        }
    }
    generateBox(level, box, 0, 0, 0, 2, 0, 4, path, path, false);
    generateBox(level, box, 0, 1, 0, 2, 2, 4, air, air, false);
    placeBlock(level, cobblestone, 1, 0, 0, box);
    placeBlock(level, water, 1, 0, 4, box);
    return true;
}
int VillagePieces::StraightRoad::getVillagerProfession(int x, int z)
{
    return 0;
}
StructurePiece* VillagePieces::StraightRoad::Create()
{
    return new StraightRoad();
}
EStructurePiece VillagePieces::StraightRoad::GetType()
{
    return eStructurePiece_StraightRoad;
}
void VillagePieces::StraightRoad::addAdditionalSaveData(DataOutputStream* output)
{
    VillagePiece::addAdditionalSaveData(output);
}
void VillagePieces::StraightRoad::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    VillagePiece::readAdditionalSaveData(input, manager);
}
void VillagePieces::StraightRoad::addChildren(StructurePiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random)
{
    generateAndAddPiece(static_cast<StartPiece*>(startPiece), pieces, random,
        getBoundingBox()->x1 + 1, getBoundingBox()->y0, getBoundingBox()->z0,
        Direction::EAST, getGenDepth());
} 
BoundingBox* VillagePieces::StraightRoad::findPieceBox(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction)
{
BoundingBox* box = villageBox(x, y, z, 3, 3, 5, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL) return box;
delete box;
return NULL;
} 
VillagePieces::SimpleHouse::SimpleHouse() : VillagePiece(), mHasMadeDoor(false)
{
}
VillagePieces::SimpleHouse::SimpleHouse(StartPiece* startPiece, int depth, Random& random, BoundingBox* box, const Direction* direction)
    : VillagePiece(startPiece, depth), mHasMadeDoor(random.nextBoolean())
{
setBoundingBox(box);
setOrientation(direction);
} 
VillagePieces::SimpleHouse::~SimpleHouse()
{
}
StructurePiece* VillagePieces::SimpleHouse::createPiece(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
    BoundingBox* box = villageBox(x, y, z, 5, 6, 5, direction);
    if (VillagePiece::isOkBox(box, startPiece)
        && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new SimpleHouse(startPiece, depth, random, box, direction);
    delete box;
    return NULL;
}
bool VillagePieces::SimpleHouse::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0) return true;
        box->move(0, mAverageGroundHeight - box->y0 + 5, 0);
    }
    const BlockState* stone = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* wood = biomeBlock(Blocks::planks->defaultBlockState());
    const BlockState* log = biomeBlock(Blocks::log->defaultBlockState());
    const BlockState* air = Blocks::AIR->defaultBlockState();
    generateBox(level, box, 0, 0, 0, 4, 0, 4, false, random, mCobblestoneSelector);
    generateBox(level, box, 1, 1, 1, 3, 4, 3, wood, wood, false);
    generateBox(level, box, 0, 5, 0, 4, 5, 4, wood, wood, false);
    generateBox(level, box, 1, 5, 1, 3, 5, 3, log, log, false);
    fillBoxDown(level, stone, 0, 0, 4, 4, 0, box);
    generateBox(level, box, 0, 4, 0, 4, 4, 4, wood, wood, false);
    for (int x = 0; x <= 4; x += 4)
    {
        for (int z = 1; z <= 3; ++z)
            placeBlock(level, random, stone, mCobblestoneSelector, x, 1, z, box);
    }
    for (int z = 1; z <= 3; ++z)
    {
        placeBlock(level, random, stone, mCobblestoneSelector, 0, 1, z, box);
        placeBlock(level, random, stone, mCobblestoneSelector, 4, 1, z, box);
    }
    for (int x = 0; x <= 4; ++x)
    {
        placeBlock(level, random, stone, mCobblestoneSelector, x, 0, 0, box);
        placeBlock(level, random, stone, mCobblestoneSelector, x, 0, 4, box);
    }
    generateBox(level, box, 2, 2, 1, 2, 3, 1, air, air, false);
    generateBox(level, box, 2, 1, 3, 2, 2, 3, air, air, false);
    placeBlock(level, wood, 1, 1, 1, box);
    placeBlock(level, wood, 3, 1, 1, box);
    placeBlock(level, Blocks::crafting_table->defaultBlockState(), 1, 1, 3, box);
    placeBlock(level, Blocks::furnace->defaultBlockState(), 3, 1, 3, box);
    createVillageDoor(level, box, random, 2, 1, 0, Direction::NORTH);
    placeTorch(level, Direction::EAST, 1, 3, 1, box);
    placeTorch(level, Direction::WEST, 3, 3, 1, box);
    if (mHasMadeChest)
    {
        placeBlock(level, Blocks::glass->defaultBlockState(), Mth::nextInt(&random, 1, 3), 3, 3, box);
        placeBlock(level, Blocks::glass->defaultBlockState(), 1, 3, Mth::nextInt(&random, 1, 3), box);
    }
    if (mHasMadeDoor)
    {
        for (int x = 0; x <= 4; ++x)
        {
            placeBlock(level, wood, x, 5, 0, box);
            placeBlock(level, wood, x, 5, 4, box);
        }
    }
    spawnVillagers(level, box, 1, 1, 2, 1);
    return true;
}
int VillagePieces::SimpleHouse::getVillagerProfession(int x, int z)
{
    return 0;
}
StructurePiece* VillagePieces::SimpleHouse::Create()
{
    return new SimpleHouse();
}
EStructurePiece VillagePieces::SimpleHouse::GetType()
{
    return eStructurePiece_SimpleHouse;
}
void VillagePieces::SimpleHouse::addAdditionalSaveData(DataOutputStream* output)
{
    VillagePiece::addAdditionalSaveData(output);
    output->writeBoolean(mHasMadeDoor);
}
void VillagePieces::SimpleHouse::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    VillagePiece::readAdditionalSaveData(input, manager);
    mHasMadeDoor = input->readBoolean();
}
BoundingBox* VillagePieces::SimpleHouse::findPieceBox(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction)
{
BoundingBox* box = villageBox(x, y, z, 5, 6, 5, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL) return box;
delete box;
return NULL;
} 
VillagePieces::SmallTemple::SmallTemple() : VillagePiece()
{
}
VillagePieces::SmallTemple::SmallTemple(StartPiece* startPiece, int depth, Random& random,
    BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth)
{
setBoundingBox(box);
setOrientation(direction);
} 
VillagePieces::SmallTemple::~SmallTemple()
{
}
StructurePiece* VillagePieces::SmallTemple::createPiece(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
BoundingBox* box = villageBox(x, y, z, 5, 12, 9, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
    return new SmallTemple(startPiece, depth, random, box, direction);
delete box;
return NULL;
} 
bool VillagePieces::SmallTemple::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0) return true;
        box->move(0, mAverageGroundHeight - box->y0 + 11, 0);
    }
    const BlockState* stone = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* floor = biomeBlock(Blocks::planks->defaultBlockState());
    const BlockState* log = biomeBlock(Blocks::log->defaultBlockState());
    const BlockState* air = Blocks::AIR->defaultBlockState();
    generateBox(level, box, 1, 1, 1, 3, 3, 7, air, air, false);
    generateBox(level, box, 1, 5, 1, 3, 9, 3, air, air, false);
    generateBox(level, box, 1, 0, 0, 3, 0, 8, false, random, mCobblestoneSelector);
    generateBox(level, box, 1, 1, 0, 3, 10, 0, false, random, mCobblestoneSelector);
    generateBox(level, box, 0, 1, 1, 0, 10, 3, false, random, mCobblestoneSelector);
    generateBox(level, box, 4, 1, 1, 4, 10, 3, false, random, mCobblestoneSelector);
    generateBox(level, box, 4, 0, 4, 4, 4, 7, false, random, mCobblestoneSelector);
    generateBox(level, box, 1, 1, 8, 3, 4, 8, false, random, mCobblestoneSelector);
    generateBox(level, box, 1, 5, 4, 3, 10, 4, false, random, mCobblestoneSelector);
    generateBox(level, box, 1, 5, 5, 3, 5, 7, false, random, mCobblestoneSelector);
    generateBox(level, box, 0, 9, 0, 4, 9, 4, false, random, mCobblestoneSelector);
    generateBox(level, box, 0, 4, 0, 4, 4, 4, false, random, mCobblestoneSelector);
    for (int z = 0; z <= 8; ++z)
    {
        placeBlock(level, random, stone, mCobblestoneSelector, 0, 1, z, box);
        placeBlock(level, random, stone, mCobblestoneSelector, 4, 1, z, box);
    }
    fillBoxDown(level, stone, 0, 0, 4, 4, 0, box);
    generateBox(level, box, 2, 1, 0, 2, 2, 0, air, air, false);
    placeBlock(level, Blocks::brewing_stand->defaultBlockState(), 2, 2, 2, box);
    placeBlock(level, Blocks::cauldron->defaultBlockState(), 2, 2, 6, box);
    placeBlock(level, Blocks::chest->defaultBlockState(), 1, 2, 6, box);
    createVillageDoor(level, box, random, 2, 1, 0, Direction::NORTH);
    placeTorch(level, Direction::EAST, 1, 3, 2, box);
    placeTorch(level, Direction::WEST, 3, 3, 2, box);
    spawnVillagers(level, box, 2, 1, 2, 1);
    return true;
}
int VillagePieces::SmallTemple::getVillagerProfession(int x, int z)
{
    return 2;
}
StructurePiece* VillagePieces::SmallTemple::Create()
{
    return new SmallTemple();
}
EStructurePiece VillagePieces::SmallTemple::GetType()
{
    return eStructurePiece_SmallTemple;
}
BoundingBox* VillagePieces::SmallTemple::findPieceBox(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction)
{
BoundingBox* box = villageBox(x, y, z, 5, 12, 9, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL) return box;
delete box;
return NULL;
}
VillagePieces::BookHouse::BookHouse() : VillagePiece()
{
}
VillagePieces::BookHouse::BookHouse(StartPiece* startPiece, int depth, Random& random,
    BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth)
{
    setBoundingBox(box);
    setOrientation(direction);
}
VillagePieces::BookHouse::~BookHouse()
{
}
StructurePiece* VillagePieces::BookHouse::createPiece(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
BoundingBox* box = villageBox(x, y, z, 9, 9, 6, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
    return new BookHouse(startPiece, depth, random, box, direction);
delete box;
return NULL;
} 
bool VillagePieces::BookHouse::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0) return true;
        box->move(0, mAverageGroundHeight - box->y0 + 8, 0);
    }
    const BlockState* stone = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* wood = biomeBlock(Blocks::planks->defaultBlockState());
    const BlockState* stairs = biomeBlock(Blocks::oak_stairs->defaultBlockState());
    const BlockState* books = Blocks::bookshelf->defaultBlockState();
    const BlockState* glass = Blocks::glass->defaultBlockState();
    const BlockState* air = Blocks::AIR->defaultBlockState();
    generateBox(level, box, 2, 1, 0, 3, 3, 0, air, air, false);
    generateBox(level, box, 5, 1, 0, 6, 3, 0, air, air, false);
    generateBox(level, box, 1, 1, 1, 7, 5, 4, wood, wood, false);
    generateBox(level, box, 0, 0, 0, 8, 0, 5, stone, stone, false);
    generateBox(level, box, 0, 5, 0, 8, 5, 5, wood, wood, false);
    generateBox(level, box, 0, 6, 1, 8, 6, 4, stairs, stairs, false);
    generateBox(level, box, 0, 7, 2, 8, 7, 3, stairs, stairs, false);
    fillBoxDown(level, stone, 0, 0, 8, 5, 0, box);
    generateBox(level, box, 1, 2, 0, 7, 4, 0, stone, stone, false);
    generateBox(level, box, 1, 2, 5, 7, 4, 5, stone, stone, false);
    generateBox(level, box, 8, 2, 1, 8, 4, 4, stone, stone, false);
    generateBox(level, box, 1, 2, 1, 7, 4, 4, air, air, false);
    generateBox(level, box, 1, 4, 1, 7, 4, 1, wood, wood, false);
    generateBox(level, box, 1, 4, 4, 7, 4, 4, wood, wood, false);
    generateBox(level, box, 1, 3, 4, 7, 3, 4, wood, wood, false);
    generateBox(level, box, 0, 1, 0, 0, 4, 0, stone, stone, false);
    generateBox(level, box, 0, 1, 5, 0, 4, 5, stone, stone, false);
    generateBox(level, box, 8, 1, 0, 8, 4, 0, stone, stone, false);
    generateBox(level, box, 8, 1, 5, 8, 4, 5, stone, stone, false);
    if (mHasMadeChest)
    {
        placeBlock(level, random, stone, mCobblestoneSelector, 4, 4, 0, box);
        placeBlock(level, random, stone, mCobblestoneSelector, 2, 4, 6, box);
    }
    for (int z = 0; z < 5; ++z)
    {
        placeBlock(level, books, 1, 1, z, box);
        placeBlock(level, books, 7, 1, z, box);
    }
    for (int x = 2; x <= 6; ++x)
        placeBlock(level, books, x, 1, 5, box);
    placeBlock(level, Blocks::wool->getBlockState(DyeColor::YELLOW->getBlockData()), 6, 2, 3, box);
    placeBlock(level, Blocks::wool->getBlockState(DyeColor::YELLOW->getBlockData()), 4, 2, 3, box);
    placeBlock(level, Blocks::wool->getBlockState(DyeColor::YELLOW->getBlockData()), 4, 2, 4, box);
    generateBox(level, box, 1, 1, 1, 1, 3, 4, books, books, false);
    generateBox(level, box, 7, 1, 1, 7, 3, 4, books, books, false);
    placeBlock(level, glass, 4, 2, 5, box);
    placeBlock(level, Blocks::crafting_table->defaultBlockState(), 6, 1, 4, box);
    placeTorch(level, Direction::EAST, 1, 4, 4, box);
    placeTorch(level, Direction::WEST, 7, 4, 4, box);
    createVillageDoor(level, box, random, 4, 1, 0, Direction::NORTH);
    spawnVillagers(level, box, 2, 1, 2, 1);
    return true;
}
int VillagePieces::BookHouse::getVillagerProfession(int x, int z)
{
    return 1;
}
StructurePiece* VillagePieces::BookHouse::Create()
{
    return new BookHouse();
}
EStructurePiece VillagePieces::BookHouse::GetType()
{
    return eStructurePiece_BookHouse;
}
BoundingBox* VillagePieces::BookHouse::findPieceBox(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction)
{
BoundingBox* box = villageBox(x, y, z, 9, 9, 6, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL) return box;
delete box;
return NULL;
} 
VillagePieces::SmallHut::SmallHut() : VillagePiece(), mVillagersSpawned(0), mHasMadeDoor(false)
{
}
VillagePieces::SmallHut::SmallHut(StartPiece* startPiece, int depth, Random& random, BoundingBox* box, const Direction* direction)
    : VillagePiece(startPiece, depth), mVillagersSpawned(0), mHasMadeDoor(false)
{
    mHasMadeDoor = random.nextBoolean();
    mVillagersSpawned = random.nextInt(3);
    setBoundingBox(box);
    setOrientation(direction);
}
VillagePieces::SmallHut::~SmallHut()
{
}
StructurePiece* VillagePieces::SmallHut::createPiece(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
BoundingBox* box = villageBox(x, y, z, 4, 6, 5, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
    return new SmallHut(startPiece, depth, random, box, direction);
delete box;
return NULL;
} 
bool VillagePieces::SmallHut::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0) return true;
        box->move(0, mAverageGroundHeight - box->y0 + 5, 0);
    }
    const BlockState* wall = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* roof = biomeBlock(Blocks::planks->defaultBlockState());
    const BlockState* air = Blocks::AIR->defaultBlockState();
    generateBox(level, box, 1, 1, 1, 3, 5, 4, air, air, false);
    generateAirBox(level, box, 0, 0, 0, 3, 0, 4);
    generateBox(level, box, 1, 0, 1, 2, 0, 3, roof, roof, false);
    if (!mHasMadeDoor)
    {
        generateBox(level, box, 1, 5, 1, 2, 5, 3, wall, wall, false);
    }
    else
    {
        generateBox(level, box, 1, 5, 1, 2, 5, 3, wall, wall, false);
    }
    if (!mHasMadeDoor)
        generateBox(level, box, 1, 5, 1, 2, 5, 3, roof, roof, false);
    if (mHasMadeDoor)
    {
        int woolX = random.nextBoolean() ? 1 : 2;
        int woolZ = random.nextBoolean() ? 1 : 3;
        placeBlock(level, Blocks::wool->getBlockState(DyeColor::YELLOW->getBlockData()), woolX, 4, woolZ, box);
        int secondWoolX = random.nextBoolean() ? 1 : 2;
        int secondWoolZ = random.nextBoolean() ? 1 : 3;
        placeBlock(level, Blocks::wool->getBlockState(DyeColor::YELLOW->getBlockData()),
            secondWoolX, 3, secondWoolZ, box);
        int thirdWoolZ = random.nextBoolean() ? 1 : 3;
        placeBlock(level, Blocks::wool->getBlockState(DyeColor::YELLOW->getBlockData()),
            secondWoolX, 2, thirdWoolZ, box);
    }
    if (mVillagersSpawned > 0)
    {
        for (int i = 0; i < mVillagersSpawned; ++i)
            placeBlock(level, roof, 1 + i, 1, 3, box);
    }
    placeBlock(level, Blocks::crafting_table->defaultBlockState(), 1, 1, 2, box);
    placeBlock(level, Blocks::furnace->defaultBlockState(), 2, 1, 2, box);
    createVillageDoor(level, box, random, 1, 1, 0, Direction::NORTH);
    placeTorch(level, Direction::EAST, 1, 3, 1, box);
    spawnVillagers(level, box, 1, 1, 2, 1);
    return true;
}
int VillagePieces::SmallHut::getVillagerProfession(int x, int z)
{
    return 0;
}
StructurePiece* VillagePieces::SmallHut::Create()
{
    return new SmallHut();
}
EStructurePiece VillagePieces::SmallHut::GetType()
{
    return eStructurePiece_SmallHut;
}
void VillagePieces::SmallHut::addAdditionalSaveData(DataOutputStream* output)
{
    VillagePiece::addAdditionalSaveData(output);
    output->writeInt(mVillagersSpawned);
    output->writeBoolean(mHasMadeDoor);
}
void VillagePieces::SmallHut::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    VillagePiece::readAdditionalSaveData(input, manager);
    mVillagersSpawned = input->readInt();
    mHasMadeDoor = input->readBoolean();
}
BoundingBox* VillagePieces::SmallHut::findPieceBox(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction)
{
BoundingBox* box = villageBox(x, y, z, 4, 6, 5, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL) return box;
delete box;
return NULL;
} 
VillagePieces::PigHouse::PigHouse() : VillagePiece()
{
}
VillagePieces::PigHouse::PigHouse(StartPiece* startPiece, int depth, Random& random,
    BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth)
{
    setBoundingBox(box);
    setOrientation(direction);
}
VillagePieces::PigHouse::~PigHouse()
{
}
StructurePiece* VillagePieces::PigHouse::createPiece(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
BoundingBox* box = villageBox(x, y, z, 9, 7, 11, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
    return new PigHouse(startPiece, depth, random, box, direction);
delete box;
return NULL;
} 
bool VillagePieces::PigHouse::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0) return true;
        box->move(0, mAverageGroundHeight - box->y0 + 6, 0);
    }
    const BlockState* wall = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* wood = biomeBlock(Blocks::planks->defaultBlockState());
    const BlockState* fence = biomeBlock(Blocks::fence->defaultBlockState());
    const BlockState* air = Blocks::AIR->defaultBlockState();
    generateBox(level, box, 0, 0, 0, 8, 0, 10, wall, wall, false);
    generateBox(level, box, 1, 1, 1, 7, 4, 9, wood, wood, false);
    generateBox(level, box, 0, 5, 0, 8, 5, 10, wood, wood, false);
    generateBox(level, box, 2, 1, 0, 6, 3, 0, air, air, false);
    generateBox(level, box, 2, 1, 10, 6, 3, 10, air, air, false);
    generateBox(level, box, 1, 1, 1, 1, 3, 9, fence, fence, false);
    generateBox(level, box, 7, 1, 1, 7, 3, 9, fence, fence, false);
    generateBox(level, box, 1, 1, 1, 7, 4, 4, wood, wood, false);
    generateBox(level, box, 2, 1, 6, 8, 4, 10, wood, wood, false);
    generateBox(level, box, 2, 0, 6, 8, 0, 10, wall, wall, false);
    generateBox(level, box, 2, 1, 6, 2, 1, 10, fence, fence, false);
    generateBox(level, box, 8, 1, 6, 8, 1, 10, fence, fence, false);
    generateBox(level, box, 3, 1, 10, 7, 1, 10, fence, fence, false);
    generateBox(level, box, 1, 0, 1, 7, 0, 4, wall, wall, false);
    placeBlock(level, random, wall, mCobblestoneSelector, 6, 0, 6, box);
    for (int z = 6; z <= 10; ++z)
        placeBlock(level, random, fence, mCobblestoneSelector, 4, 1, z, box);
    placeBlock(level, fence, 0, 4, 2, box);
    placeBlock(level, fence, 0, 4, 3, box);
    placeBlock(level, fence, 8, 4, 2, box);
    placeBlock(level, fence, 8, 4, 3, box);
    placeBlock(level, fence, 0, 2, 1, box);
    placeBlock(level, fence, 0, 2, 4, box);
    placeBlock(level, fence, 8, 2, 1, box);
    placeBlock(level, fence, 8, 2, 4, box);
    placeBlock(level, fence, 2, 2, 5, box);
    placeBlock(level, fence, 3, 2, 5, box);
    placeBlock(level, fence, 5, 2, 0, box);
    placeBlock(level, fence, 6, 2, 5, box);
    placeBlock(level, fence, 2, 1, 3, box);
    placeBlock(level, Blocks::wool->getBlockState(DyeColor::YELLOW->getBlockData()), 2, 2, 3, box);
    placeBlock(level, wood, 1, 1, 4, box);
    placeBlock(level, wood, 2, 1, 4, box);
    placeBlock(level, wood, 1, 1, 3, box);
    generateBox(level, box, 5, 0, 1, 7, 0, 3, wood, wood, false);
    placeBlock(level, wood, 6, 1, 1, box);
    placeBlock(level, wood, 6, 1, 2, box);
    placeBlock(level, air, 2, 1, 0, box);
    placeBlock(level, air, 2, 2, 0, box);
    placeBlock(level, air, 6, 1, 5, box);
    placeBlock(level, air, 6, 2, 5, box);
    fillBoxDown(level, wall, 0, 0, 8, 10, 0, box);
    placeBlock(level, Blocks::hay_block->defaultBlockState(), 3, 1, 7, box);
    placeBlock(level, Blocks::hay_block->defaultBlockState(), 4, 1, 7, box);
    createVillageDoor(level, box, random, 2, 3, 1, Direction::NORTH);
    createVillageDoor(level, box, random, 6, 3, 4, Direction::SOUTH);
    placeTorch(level, Direction::EAST, 1, 4, 2, box);
    placeTorch(level, Direction::WEST, 7, 4, 2, box);
    for (int i = 0; i <= 4; ++i)
    {
        for (int x = 0; x <= 8; ++x)
        {
            placeBlock(level, random, fence, mCobblestoneSelector, x, 7, i, box);
            generateAirColumnUp(level, x, -1, i, box);
        }
    }
    spawnVillagers(level, box, 4, 1, 2, 2);
    return true;
}
int VillagePieces::PigHouse::getVillagerProfession(int x, int z)
{
    return z == 0 ? 4 : z;
}
StructurePiece* VillagePieces::PigHouse::Create()
{
    return new PigHouse();
}
EStructurePiece VillagePieces::PigHouse::GetType()
{
    return eStructurePiece_PigHouse;
}
BoundingBox* VillagePieces::PigHouse::findPieceBox(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction)
{
BoundingBox* box = villageBox(x, y, z, 9, 7, 11, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL) return box;
delete box;
return NULL;
} 
VillagePieces::TwoRoomHouse::TwoRoomHouse() : VillagePiece()
{
}
VillagePieces::TwoRoomHouse::TwoRoomHouse(StartPiece* startPiece, int depth, Random& random,
    BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth)
{
setBoundingBox(box);
setOrientation(direction);
} 
VillagePieces::TwoRoomHouse::~TwoRoomHouse()
{
}
StructurePiece* VillagePieces::TwoRoomHouse::createPiece(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
BoundingBox* box = villageBox(x, y, z, 9, 7, 12, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
    return new TwoRoomHouse(startPiece, depth, random, box, direction);
delete box;
return NULL;
} 
bool VillagePieces::TwoRoomHouse::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0) return true;
        box->move(0, mAverageGroundHeight - box->y0 + 6, 0);
    }
    const BlockState* wall = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* wood = biomeBlock(Blocks::planks->defaultBlockState());
    const BlockState* stairs = biomeBlock(Blocks::oak_stairs->defaultBlockState());
    const BlockState* air = Blocks::AIR->defaultBlockState();
    generateBox(level, box, 0, 0, 0, 8, 0, 11, wall, wall, false);
    generateBox(level, box, 1, 1, 1, 7, 4, 10, wood, wood, false);
    generateBox(level, box, 0, 5, 0, 8, 5, 11, wood, wood, false);
    generateBox(level, box, 4, 1, 1, 4, 3, 10, wall, wall, false);
    generateBox(level, box, 3, 2, 0, 5, 3, 0, air, air, false);
    generateBox(level, box, 3, 2, 11, 5, 3, 11, air, air, false);
    fillBoxDown(level, wall, 0, 0, 8, 11, 0, box);
    generateBox(level, box, 2, 0, 5, 8, 0, 10, wall, wall, false);
    generateBox(level, box, 1, 0, 1, 7, 0, 4, wall, wall, false);
    generateBox(level, box, 0, 0, 0, 8, 0, 11, wall, wall, false);
    generateBox(level, box, 0, 1, 0, 0, 4, 11, wall, wall, false);
    generateBox(level, box, 8, 1, 0, 8, 4, 11, wall, wall, false);
    generateBox(level, box, 1, 1, 0, 7, 2, 0, air, air, false);
    generateBox(level, box, 1, 1, 11, 7, 2, 11, air, air, false);
    generateBox(level, box, 1, 2, 1, 7, 3, 4, wood, wood, false);
    generateBox(level, box, 1, 2, 5, 2, 3, 5, wood, wood, false);
    generateBox(level, box, 6, 2, 5, 7, 3, 5, wood, wood, false);
    generateBox(level, box, 3, 4, 5, 5, 4, 10, wood, wood, false);
    generateBox(level, box, 4, 5, 4, 4, 5, 10, wood, wood, false);
    generateBox(level, box, 6, 5, 4, 6, 5, 10, wood, wood, false);
    generateBox(level, box, 5, 6, 3, 5, 6, 10, wood, wood, false);
    generateBox(level, box, 1, 1, 5, 7, 1, 5, stairs, stairs, false);
    for (int y = -1; y <= 2; ++y)
    {
        for (int x = 0; x < 12; x += 4)
        {
            for (int dx = 0; dx < 4; ++dx)
            {
                placeBlock(level, wood, x + dx, y + 4, y, box);
                if (y >= 1 || x + dx < 2)
                    placeBlock(level, wood, x + dx, y + 4, 6 - y, box);
            }
        }
    }
    placeBlock(level, wood, 6, 6, 3, box);
    placeBlock(level, wood, 7, 5, 4, box);
    for (int z = 4; z <= 10; ++z)
    {
        placeBlock(level, wood, 6, 6, z, box);
        placeBlock(level, wood, 7, 5, z, box);
        placeBlock(level, wood, 8, 4, z, box);
    }
    placeBlock(level, Blocks::crafting_table->defaultBlockState(), 2, 1, 3, box);
    placeBlock(level, Blocks::furnace->defaultBlockState(), 6, 1, 3, box);
    for (int z = 2; z <= 4; ++z)
    {
        placeBlock(level, wood, 1, 1, z, box);
        placeBlock(level, wood, 7, 1, z, box);
    }
    createChest(level, box, random, 7, 1, 8, &BuiltInLootTables::VILLAGE_TWO_ROOM_HOUSE);
    createVillageDoor(level, box, random, 2, 1, 0, Direction::NORTH);
    createVillageDoor(level, box, random, 6, 1, 11, Direction::SOUTH);
    placeTorch(level, Direction::EAST, 1, 3, 3, box);
    placeTorch(level, Direction::WEST, 7, 3, 3, box);
    BlockPos roomChest(getWorldX(5, 9), getWorldY(1), getWorldZ(5, 9));
    if (box->isInside(roomChest))
    {
        setDirty();
        placeBlock(level, Blocks::chest->defaultBlockState(), 5, 1, 9, box);
    }
    BlockPos roomCorner(getWorldX(7, 10), getWorldY(1), getWorldZ(7, 10));
    if (box->isInside(roomCorner))
    {
        setDirty();
        placeBlock(level, Blocks::chest->defaultBlockState(), 7, 1, 10, box);
    }
    spawnVillagers(level, box, 4, 1, 2, 2);
    return true;
}
int VillagePieces::TwoRoomHouse::getVillagerProfession(int x, int z)
{
    return 0;
}
StructurePiece* VillagePieces::TwoRoomHouse::Create()
{
    return new TwoRoomHouse();
}
EStructurePiece VillagePieces::TwoRoomHouse::GetType()
{
    return eStructurePiece_TwoRoomHouse;
}
BoundingBox* VillagePieces::TwoRoomHouse::findPieceBox(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction)
{
BoundingBox* box = villageBox(x, y, z, 9, 7, 12, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL) return box;
delete box;
return NULL;
} 
VillagePieces::Smithy::Smithy() : VillagePiece()
{
}
VillagePieces::Smithy::Smithy(StartPiece* startPiece, int depth, Random& random,
    BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth)
{
    setBoundingBox(box);
    setOrientation(direction);
}
VillagePieces::Smithy::~Smithy()
{
}
StructurePiece* VillagePieces::Smithy::createPiece(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction, int depth)
{
BoundingBox* box = villageBox(x, y, z, 10, 6, 7, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
    return new Smithy(startPiece, depth, random, box, direction);
delete box;
return NULL;
} 
bool VillagePieces::Smithy::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0) return true;
        box->move(0, mAverageGroundHeight - box->y0 + 5, 0);
    }
    const BlockState* stone = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* wood = biomeBlock(Blocks::planks->defaultBlockState());
    const BlockState* log = biomeBlock(Blocks::log->defaultBlockState());
    const BlockState* air = Blocks::AIR->defaultBlockState();
    generateBox(level, box, 0, 1, 0, 9, 4, 6, air, air, false);
    generateBox(level, box, 0, 0, 0, 9, 0, 6, stone, stone, false);
    generateBox(level, box, 1, 5, 1, 8, 5, 5, air, air, false);
    generateBox(level, box, 1, 1, 0, 2, 3, 0, wood, wood, false);
    generateBox(level, box, 0, 1, 0, 0, 4, 0, log, log, false);
    generateBox(level, box, 3, 1, 0, 3, 4, 0, log, log, false);
    generateBox(level, box, 0, 1, 6, 0, 4, 6, log, log, false);
    generateBox(level, box, 3, 1, 6, 3, 4, 6, log, log, false);
    generateBox(level, box, 4, 1, 3, 5, 3, 3, wood, wood, false);
    generateBox(level, box, 6, 1, 4, 8, 3, 6, wood, wood, false);
    placeBlock(level, random, stone, mCobblestoneSelector, 6, 1, 3, box);
    placeBlock(level, Blocks::stone->defaultBlockState(), 6, 2, 3, box);
    placeBlock(level, Blocks::stone->defaultBlockState(), 6, 3, 3, box);
    placeBlock(level, Blocks::stone->defaultBlockState(), 8, 1, 1, box);
    placeBlock(level, Blocks::stone->defaultBlockState(), 0, 2, 2, box);
    placeBlock(level, Blocks::stone->defaultBlockState(), 0, 2, 4, box);
    placeBlock(level, Blocks::stone->defaultBlockState(), 2, 2, 6, box);
    placeBlock(level, Blocks::stone->defaultBlockState(), 4, 2, 6, box);
    placeBlock(level, Blocks::furnace->defaultBlockState(), 5, 1, 1, box);
    placeBlock(level, Blocks::lit_furnace->defaultBlockState(), 6, 1, 1, box);
    placeBlock(level, Blocks::crafting_table->defaultBlockState(), 7, 1, 1, box);
    placeBlock(level, Blocks::anvil->defaultBlockState(), 5, 1, 4, box);
    placeBlock(level, Blocks::wool->getBlockState(DyeColor::YELLOW->getBlockData()), 5, 4, 2, box);
    placeBlock(level, Blocks::wool->getBlockState(DyeColor::YELLOW->getBlockData()), 6, 4, 2, box);
    for (int x = 2; x <= 4; ++x)
    {
        placeBlock(level, wood, x, 1, 6, box);
        placeBlock(level, wood, x, 2, 6, box);
    }
    if (!mHasMadeChest)
    {
        BlockPos chestPosition(getWorldX(5, 5), getWorldY(1), getWorldZ(5, 5));
        if (box->isInside(chestPosition))
        {
            setDirty();
            mHasMadeChest = true;
            createChest(level, box, random, 5, 1, 5, &BuiltInLootTables::VILLAGE_BLACKSMITH);
        }
    }
    BlockPos smithyChest(getWorldX(7, 4), getWorldY(2), getWorldZ(7, 4));
    if (box->isInside(smithyChest))
    {
        setDirty();
        placeBlock(level, Blocks::chest->defaultBlockState(), 7, 2, 4, box);
    }
    BlockPos smithyCorner(getWorldX(8, 0), getWorldY(0), getWorldZ(8, 0));
    if (box->isInside(smithyCorner))
    {
        setDirty();
        placeBlock(level, Blocks::chest->defaultBlockState(), 8, 0, 0, box);
    }
    for (int i = 0; i < 7; ++i)
    {
        for (int x = 0; x <= 9; ++x)
        {
            getBlock(level, x, 6, i, box);
            generateAirColumnUp(level, x, -1, i, box);
        }
    }
    spawnVillagers(level, box, 7, 1, 1, 1);
    return true;
}
int VillagePieces::Smithy::getVillagerProfession(int x, int z)
{
    return 3;
}
StructurePiece* VillagePieces::Smithy::Create()
{
    return new Smithy();
}
EStructurePiece VillagePieces::Smithy::GetType()
{
    return eStructurePiece_Smithy;
}
void VillagePieces::Smithy::addAdditionalSaveData(DataOutputStream* output)
{
    VillagePiece::addAdditionalSaveData(output);
    output->writeBoolean(mHasMadeChest);
}
void VillagePieces::Smithy::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    VillagePiece::readAdditionalSaveData(input, manager);
    mHasMadeChest = input->readBoolean();
}
BoundingBox* VillagePieces::Smithy::findPieceBox(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction)
{
BoundingBox* box = villageBox(x, y, z, 10, 6, 7, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL) return box;
delete box;
return NULL;
}
void VillagePieces::Smithy::staticCtor()
{
}
VillagePieces::Farmland::Farmland() : VillagePiece(), mCropA(0), mCropB(0)
{
}
VillagePieces::Farmland::Farmland(StartPiece* startPiece, int depth, Random& random, BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth), mCropA(0), mCropB(0) {
setBoundingBox(box);
setOrientation(direction);
selectCrops(random);
}
VillagePieces::Farmland::~Farmland()
{
}
unsigned int VillagePieces::Farmland::selectCrops(Random& random)
{
    mCropA = random.nextInt(4);
    mCropB = random.nextInt(4);
    return mCropA;
}
StructurePiece* VillagePieces::Farmland::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth) {
BoundingBox* box = villageBox(x, y, z, 7, 4, 9, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
    return new Farmland(startPiece, depth, random, box, direction);
delete box;
return NULL;
}
int VillagePieces::Farmland::selectOldCrops(int crop)
{
    return crop < 0 ? 0 : crop;
}
void VillagePieces::Farmland::addAdditionalSaveData(DataOutputStream* output)
{
    VillagePiece::addAdditionalSaveData(output);
    output->writeInt(mCropA);
    output->writeInt(mCropB);
}
void VillagePieces::Farmland::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    VillagePiece::readAdditionalSaveData(input, manager);
    mCropA = input->readInt();
    mCropB = input->readInt();
}
bool VillagePieces::Farmland::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0) return true;
        box->move(0, mAverageGroundHeight - box->y0 + 3, 0);
    }
    const BlockState* log = biomeBlock(Blocks::log->defaultBlockState());
    const BlockState* air = Blocks::AIR->defaultBlockState();
    const BlockState* farmland = Blocks::farmland->defaultBlockState();
    const BlockState* water = Blocks::FLOWING_WATER->defaultBlockState();
    const BlockState* cropA = Blocks::wheat->getBlockState(mCropA);
    const BlockState* cropB = Blocks::carrots->getBlockState(mCropB);
    generateBox(level, box, 0, 1, 0, 6, 4, 8, air, air, false);
    generateBox(level, box, 1, 0, 1, 2, 0, 7, farmland, farmland, false);
    generateBox(level, box, 4, 0, 1, 5, 0, 7, farmland, farmland, false);
    generateBox(level, box, 0, 0, 0, 0, 0, 8, log, log, false);
    generateBox(level, box, 1, 0, 0, 5, 0, 0, log, log, false);
    generateBox(level, box, 1, 0, 8, 5, 0, 8, log, log, false);
    generateBox(level, box, 6, 0, 0, 6, 0, 8, log, log, false);
    generateBox(level, box, 3, 0, 1, 3, 0, 7, water, water, false);
    if (!level->getBiome(box->getCenter())->isSnowCovered())
    {
        for (int z = 1; z <= 7; ++z)
        {
            placeBlock(level, cropA, 1, 1, z, box);
            placeBlock(level, cropA, 2, 1, z, box);
            placeBlock(level, cropB, 4, 1, z, box);
            placeBlock(level, cropB, 5, 1, z, box);
        }
    }
    return true;
}
StructurePiece* VillagePieces::Farmland::Create()
{
    return new Farmland();
}
EStructurePiece VillagePieces::Farmland::GetType()
{
    return eStructurePiece_Farmland;
}
VillagePieces::DoubleFarmland::DoubleFarmland() : Farmland(), mCropC(0), mCropD(0)
{
}
VillagePieces::DoubleFarmland::DoubleFarmland(StartPiece* startPiece, int depth, Random& random, BoundingBox* box, const Direction* direction) : Farmland(startPiece, depth, random, box, direction), mCropC(0), mCropD(0) {
selectCrops(random);
}
VillagePieces::DoubleFarmland::~DoubleFarmland()
{
}
unsigned int VillagePieces::DoubleFarmland::selectCrops(Random& random)
{
    mCropC = random.nextInt(4);
    mCropD = random.nextInt(4);
    return mCropC;
}
StructurePiece* VillagePieces::DoubleFarmland::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth) {
BoundingBox* box = villageBox(x, y, z, 13, 4, 9, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
    return new DoubleFarmland(startPiece, depth, random, box, direction);
delete box;
return NULL;
}
int VillagePieces::DoubleFarmland::selectOldCrops(int crop)
{
    return crop < 0 ? 0 : crop;
}
void VillagePieces::DoubleFarmland::addAdditionalSaveData(DataOutputStream* output)
{
    Farmland::addAdditionalSaveData(output);
    output->writeInt(mCropC);
    output->writeInt(mCropD);
}
void VillagePieces::DoubleFarmland::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    Farmland::readAdditionalSaveData(input, manager);
    mCropC = input->readInt();
    mCropD = input->readInt();
}
bool VillagePieces::DoubleFarmland::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (!Farmland::postProcess(level, random, box)) return false;
    const BlockState* cropC = Blocks::potatoes->getBlockState(mCropC);
    const BlockState* cropD = Blocks::beetroots->getBlockState(mCropD);
    if (!level->getBiome(box->getCenter())->isSnowCovered())
    {
        for (int z = 1; z <= 7; ++z)
        {
            placeBlock(level, cropC, 1, 1, z, box);
            placeBlock(level, cropC, 2, 1, z, box);
            placeBlock(level, cropD, 4, 1, z, box);
            placeBlock(level, cropD, 5, 1, z, box);
        }
    }
    return true;
}
StructurePiece* VillagePieces::DoubleFarmland::Create()
{
    return new DoubleFarmland();
}
EStructurePiece VillagePieces::DoubleFarmland::GetType()
{
    return eStructurePiece_DoubleFarmland;
}
VillagePieces::LightPost::LightPost() : VillagePiece()
{
}
VillagePieces::LightPost::LightPost(StartPiece* startPiece, int depth, Random& random,
    BoundingBox* box, const Direction* direction) : VillagePiece(startPiece, depth)
{
    setBoundingBox(box);
    setOrientation(direction);
}
VillagePieces::LightPost::~LightPost()
{
}
BoundingBox* VillagePieces::LightPost::findPieceBox(StartPiece* startPiece,
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
    const Direction* direction)
{
BoundingBox* box = villageBox(x, y, z, 3, 4, 3, direction);
if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL) return box;
delete box;
return NULL;
}
bool VillagePieces::LightPost::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0) return true;
        box->move(0, mAverageGroundHeight - box->y0 + 3, 0);
    }
    const BlockState* fence = biomeBlock(Blocks::fence->defaultBlockState());
    const BlockState* air = Blocks::AIR->defaultBlockState();
    const BlockState* wool = Blocks::wool->getBlockState(DyeColor::YELLOW->getBlockData());
    generateBox(level, box, 0, 0, 0, 2, 3, 1, air, air, false);
    placeBlock(level, fence, 1, 0, 0, box);
    placeBlock(level, fence, 1, 1, 0, box);
    placeBlock(level, fence, 1, 2, 0, box);
    placeBlock(level, wool, 1, 3, 0, box);
    placeTorch(level, Direction::EAST, 2, 3, 0, box);
    placeTorch(level, Direction::NORTH, 1, 3, 1, box);
    placeTorch(level, Direction::WEST, 0, 3, 0, box);
    placeTorch(level, Direction::SOUTH, 1, 3, -1, box);
    return true;
}
StructurePiece* VillagePieces::LightPost::Create()
{
    return new LightPost();
}
EStructurePiece VillagePieces::LightPost::GetType()
{
    return eStructurePiece_LightPost;
}
VillagePieces::CobbleStoneSelector::CobbleStoneSelector(bool mossy, int type)
    : mMossy(mossy), mType(type)
{
}
VillagePieces::CobbleStoneSelector::~CobbleStoneSelector()
{
}
void VillagePieces::CobbleStoneSelector::next(Random& random, int x, int y, int z, bool edge)
{
    if (mMossy && random.nextFloat() >= 0.4f)
    {
        mNext = mType == 1 ? Blocks::sandstone->getBlockState(SandStoneBlock::SANDSTONE->getData())
                           : Blocks::mossy_cobblestone->defaultBlockState();
    }
    else
    {
        mNext = mType == 1 ? Blocks::sandstone->getBlockState(SandStoneBlock::SANDSTONE->getData())
                           : Blocks::cobblestone->defaultBlockState();
    }
}
