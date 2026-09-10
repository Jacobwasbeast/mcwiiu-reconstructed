#include "game/core/MineShaftPieces.hpp"

#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

MineShaftPieces::MineShaftPiece::MineShaftPiece(int genDepth, MineShaftFeature::MineShaftType type)
    : StructurePiece(genDepth), mType(type)
{
}

MineShaftPieces::MineShaftPiece::MineShaftPiece()
    : StructurePiece(), mType(MineShaftFeature::eMineShaftType_Normal)
{
}

MineShaftPieces::MineShaftPiece::~MineShaftPiece()
{
}

const BlockState* MineShaftPieces::MineShaftPiece::getPlanksBlock()
{
    Block* block = Block::byId(5);
    if (block == NULL) return NULL;
    const BlockState* state = block->defaultBlockState();
    if (mType != MineShaftFeature::eMineShaftType_Mesa) return state;
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::DARK_OAK);
    return state->setBoxedValue(PlanksBlock::VARIANT, &variant);
}

const BlockState* MineShaftPieces::MineShaftPiece::getFenceBlock()
{
    Block* block = mType == MineShaftFeature::eMineShaftType_Mesa
        ? Blocks::dark_oak_fence : Blocks::fence;
    return block == NULL ? NULL : block->defaultBlockState();
}

bool MineShaftPieces::MineShaftPiece::isSupportingBox(Level* level, BoundingBox* box,
                                                       int x0, int x1, int y, int z)
{
    for (int x = x0; x <= x1; ++x)
    {
        const BlockState* state = getBlock(level, x, y + 1, z, box);
        if (state == NULL || state->getMaterial() == Material::AIR) return false;
    }
    return true;
}

void MineShaftPieces::MineShaftPiece::addAdditionalSaveData(DataOutputStream* output)
{
    output->writeInt(static_cast<int>(mType));
}

void MineShaftPieces::MineShaftPiece::readAdditionalSaveData(DataInputStream* input,
                                                              StructureManager* structureManager)
{
    if (StructureFeatureIO::getVersion() < 2)
        mType = MineShaftFeature::eMineShaftType_Normal;
    else
        mType = static_cast<MineShaftFeature::MineShaftType>(input->readInt());
}

MineShaftPieces::MineShaftCorridor::MineShaftCorridor()
    : MineShaftPiece(), mHasRails(false), mHasSpiderWebs(false), mHasPlacedSupport(false), mNumSections(0)
{
}

MineShaftPieces::MineShaftCorridor::MineShaftCorridor(int genDepth, Random& random, BoundingBox* box,
                                                       const Direction* direction,
                                                       MineShaftFeature::MineShaftType type)
    : MineShaftPiece(genDepth, type), mHasRails(random.nextInt(3) == 0),
      mHasSpiderWebs(false), mHasPlacedSupport(false), mNumSections(0)
{
    setOrientation(direction);
    setBoundingBox(box);
    mHasSpiderWebs = !mHasRails && random.nextInt(23) == 0;
    mNumSections = (direction->getAxis() == Direction::Axis::Z) ? box->getZSpan() / 5 : box->getXSpan() / 5;
}

MineShaftPieces::MineShaftCorridor::~MineShaftCorridor()
{
}

BoundingBox* MineShaftPieces::MineShaftCorridor::findCorridorSize(std::vector<StructurePiece*>* pieces,
                                                                   Random& random, int x, int y, int z,
                                                                   const Direction* direction)
{
    int length = (random.nextInt(3) + 2) * 5;
    BoundingBox* box = BoundingBox::orientBox(x, y, z, 0, 0, 0, 3, 3, length, direction);
    if (box != NULL && StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return box;
}

void MineShaftPieces::MineShaftCorridor::addAdditionalSaveData(DataOutputStream* output)
{
    MineShaftPiece::addAdditionalSaveData(output);
    output->writeBoolean(mHasRails);
    output->writeBoolean(mHasSpiderWebs);
    output->writeBoolean(mHasPlacedSupport);
    output->writeInt(mNumSections);
}

void MineShaftPieces::MineShaftCorridor::readAdditionalSaveData(DataInputStream* input,
                                                                 StructureManager* structureManager)
{
    MineShaftPiece::readAdditionalSaveData(input, structureManager);
    mHasRails = input->readBoolean();
    mHasSpiderWebs = input->readBoolean();
    mHasPlacedSupport = input->readBoolean();
    mNumSections = input->readInt();
}

void MineShaftPieces::MineShaftCorridor::addChildren(StructurePiece* startPiece,
                                                     std::vector<StructurePiece*>* pieces, Random& random)
{
    for (int i = 0; i < 2; ++i)
    {
        generateAndAddPiece(startPiece, pieces, random, getBoundingBox()->x0 + i * 3,
                            getBoundingBox()->y0, getBoundingBox()->z0, getOrientation(), getGenDepth());
    }
}

bool MineShaftPieces::MineShaftCorridor::createChest(Level* level, BoundingBox* box, Random& random,
                                                      int x, int y, int z, ResourceLocation* lootTable)
{
    return StructurePiece::createChest(level, box, random, x, y, z, lootTable);
}

void MineShaftPieces::MineShaftCorridor::placeSupport(Level* level, BoundingBox* box, int x0, int y0,
                                                      int z0, int x1, int y1, Random& random)
{
    const BlockState* planks = getPlanksBlock();
    if (planks == NULL) return;
    generateBox(level, box, x0, y0, z0, x0, y1, z0, planks, planks, false);
    generateBox(level, box, x1, y0, z0, x1, y1, z0, planks, planks, false);
    generateBox(level, box, x0, y1, z0, x1, y1, z0, planks, planks, false);
}

void MineShaftPieces::MineShaftCorridor::placeCobWeb(Level* level, BoundingBox* box, Random& random,
                                                      float chance, int x, int y, int z)
{
    if (random.nextFloat() < chance) maybeGenerateBlock(level, box, random, 1.0f, x, y, z, getPlanksBlock());
}

bool MineShaftPieces::MineShaftCorridor::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box)) return false;
    const BlockState* planks = getPlanksBlock();
    if (planks == NULL) return false;
    generateAirBox(level, box, 0, 0, 0, 2, 2, mNumSections * 5);
    for (int section = 0; section < mNumSections; ++section)
    {
        int z = section * 5;
        if (mHasPlacedSupport || section == 0 || random.nextInt(4) == 0)
            placeSupport(level, box, 0, 0, z, 2, 2, random);
        if (mHasSpiderWebs) placeCobWeb(level, box, random, 0.1f, 1, 1, z + 2);
        if (mHasRails) generateBox(level, box, 1, 0, z, 1, 0, z + 4, planks, planks, false);
    }
    return true;
}

StructurePiece* MineShaftPieces::MineShaftCorridor::Create()
{
    return new MineShaftCorridor();
}

EStructurePiece MineShaftPieces::MineShaftCorridor::GetType()
{
    return eStructurePiece_MineShaftCorridor;
}

MineShaftPieces::MineShaftCrossing::MineShaftCrossing()
    : MineShaftPiece(), mDirection(NULL), mIsTwoFloored(false)
{
}

MineShaftPieces::MineShaftCrossing::MineShaftCrossing(int genDepth, Random& random, BoundingBox* box,
                                                       const Direction* direction,
                                                       MineShaftFeature::MineShaftType type)
    : MineShaftPiece(genDepth, type), mDirection(direction), mIsTwoFloored(random.nextBoolean())
{
    setOrientation(direction);
    setBoundingBox(box);
}

MineShaftPieces::MineShaftCrossing::~MineShaftCrossing()
{
}

BoundingBox* MineShaftPieces::MineShaftCrossing::findCrossing(std::vector<StructurePiece*>* pieces,
                                                              Random& random, int x, int y, int z,
                                                              const Direction* direction)
{
    BoundingBox* box = new BoundingBox(x, y, z, x, y + 2, z);
    if (random.nextInt(4) == 0)
        box->y1 += 4;

    int dir = direction->get3DDataValue();
    if (dir > 2)
    {
        if (dir == 3)
        {
            box->x1 = x + 3;
            box->z1 = z + 4;
            box->x0 = x - 1;
        }
        else if (dir < 5)
        {
            box->z1 = z + 3;
            box->z0 = z - 1;
            box->x0 = x - 4;
        }
        else if (dir == 5)
        {
            box->x1 = x + 4;
            box->z1 = z + 3;
            box->z0 = z - 1;
        }
    }
    else
    {
        box->x1 = x + 3;
        box->z0 = z - 4;
        box->x0 = x - 1;
    }

    if (StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return box;
}

void MineShaftPieces::MineShaftCrossing::addAdditionalSaveData(DataOutputStream* output)
{
    MineShaftPiece::addAdditionalSaveData(output);
    output->writeInt(mDirection == NULL ? 0 : mDirection->get3DDataValue());
    output->writeBoolean(mIsTwoFloored);
}

void MineShaftPieces::MineShaftCrossing::readAdditionalSaveData(DataInputStream* input,
                                                                 StructureManager* structureManager)
{
    MineShaftPiece::readAdditionalSaveData(input, structureManager);
    mDirection = Direction::from3DDataValue(input->readInt());
    mIsTwoFloored = input->readBoolean();
}

void MineShaftPieces::MineShaftCrossing::addChildren(StructurePiece* startPiece,
                                                     std::vector<StructurePiece*>* pieces, Random& random)
{
    BoundingBox* box = getBoundingBox();
    generateAndAddPiece(startPiece, pieces, random, box->x0, box->y0, box->z0, mDirection, getGenDepth());
    generateAndAddPiece(startPiece, pieces, random, box->x1, box->y0, box->z1, mDirection, getGenDepth());
}

void MineShaftPieces::MineShaftCrossing::placeSupportPillar(Level* level, BoundingBox* box,
                                                             int x0, int y0, int z0, int y1)
{
    const BlockState* planks = getPlanksBlock();
    if (planks != NULL) generateBox(level, box, x0, y0, z0, x0, y1, z0, planks, planks, false);
}

bool MineShaftPieces::MineShaftCrossing::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box)) return false;
    const BlockState* planks = getPlanksBlock();
    if (planks == NULL) return false;
    generateAirBox(level, box, 0, 0, 0, 4, mIsTwoFloored ? 4 : 2, 4);
    placeSupportPillar(level, box, 0, 0, 0, 2);
    placeSupportPillar(level, box, 4, 0, 4, 2);
    return true;
}

StructurePiece* MineShaftPieces::MineShaftCrossing::Create()
{
    return new MineShaftCrossing();
}

EStructurePiece MineShaftPieces::MineShaftCrossing::GetType()
{
    return eStructurePiece_MineShaftCrossing;
}

MineShaftPieces::MineShaftRoom::MineShaftRoom(int genDepth, Random& random, int x, int z,
                                               MineShaftFeature::MineShaftType type)
    : MineShaftPiece(genDepth, type)
{
    setBoundingBox(new BoundingBox(x, 50, z, x + 7 + random.nextInt(6), 54 + random.nextInt(6),
                                   z + 7 + random.nextInt(6)));
}

MineShaftPieces::MineShaftRoom::MineShaftRoom()
    : MineShaftPiece()
{
}

MineShaftPieces::MineShaftRoom::~MineShaftRoom()
{
    for (std::list<BoundingBox*>::iterator it = mChildPieces.begin(); it != mChildPieces.end(); ++it)
        delete *it;
}

void MineShaftPieces::MineShaftRoom::addChildren(StructurePiece* startPiece,
                                                 std::vector<StructurePiece*>* pieces, Random& random)
{
    BoundingBox* box = getBoundingBox();
    generateAndAddPiece(startPiece, pieces, random, box->x0, box->y0, box->z0, Direction::NORTH, getGenDepth());
}

void MineShaftPieces::MineShaftRoom::move(int x, int y, int z)
{
    StructurePiece::move(x, y, z);
    for (std::list<BoundingBox*>::iterator it = mChildPieces.begin(); it != mChildPieces.end(); ++it)
        (*it)->move(x, y, z);
}

bool MineShaftPieces::MineShaftRoom::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box)) return false;
    const BlockState* planks = getPlanksBlock();
    if (planks == NULL) return false;
    generateAirBox(level, box, 0, 0, 0, box->getXSpan() - 1, box->getYSpan() - 1, box->getZSpan() - 1);
    for (std::list<BoundingBox*>::iterator it = mChildPieces.begin(); it != mChildPieces.end(); ++it)
        generateBox(level, box, *it, planks, planks, false);
    return true;
}

void MineShaftPieces::MineShaftRoom::addAdditionalSaveData(DataOutputStream* output)
{
    MineShaftPiece::addAdditionalSaveData(output);
    output->writeInt(static_cast<int>(mChildPieces.size()));
    for (std::list<BoundingBox*>::const_iterator it = mChildPieces.begin(); it != mChildPieces.end(); ++it)
        (*it)->write(output);
}

void MineShaftPieces::MineShaftRoom::readAdditionalSaveData(DataInputStream* input,
                                                             StructureManager* structureManager)
{
    MineShaftPiece::readAdditionalSaveData(input, structureManager);
    int count = input->readInt();
    for (int i = 0; i < count; ++i)
    {
        BoundingBox* box = new BoundingBox();
        box->read(input);
        mChildPieces.push_back(box);
    }
}

StructurePiece* MineShaftPieces::MineShaftRoom::Create()
{
    return new MineShaftRoom();
}

EStructurePiece MineShaftPieces::MineShaftRoom::GetType()
{
    return eStructurePiece_MineShaftRoom;
}

MineShaftPieces::MineShaftStairs::MineShaftStairs()
    : MineShaftPiece()
{
}

MineShaftPieces::MineShaftStairs::MineShaftStairs(int genDepth, Random& random, BoundingBox* box,
                                                  const Direction* direction,
                                                  MineShaftFeature::MineShaftType type)
    : MineShaftPiece(genDepth, type)
{
    setOrientation(direction);
    setBoundingBox(box);
}

MineShaftPieces::MineShaftStairs::~MineShaftStairs()
{
}

BoundingBox* MineShaftPieces::MineShaftStairs::findStairs(std::vector<StructurePiece*>* pieces,
                                                          Random& random, int x, int y, int z,
                                                          const Direction* direction)
{
    BoundingBox* box = new BoundingBox(x, y - 5, z, x, y + 2, z);
    int dir = direction->get3DDataValue();
    if (dir > 2)
    {
        if (dir == 3)
        {
            box->x1 = x + 2;
            box->z1 = z + 8;
        }
        else if (dir < 5)
        {
            box->x0 = x - 8;
            box->z1 = z + 2;
        }
        else if (dir == 5)
        {
            box->x1 = x + 8;
            box->z1 = z + 2;
        }
    }
    else
    {
        box->x1 = x + 2;
        box->z0 = z - 8;
    }

    if (StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return box;
}

void MineShaftPieces::MineShaftStairs::addChildren(StructurePiece* startPiece,
                                                   std::vector<StructurePiece*>* pieces, Random& random)
{
    BoundingBox* box = getBoundingBox();
    generateAndAddPiece(startPiece, pieces, random, box->x0, box->y0, box->z0, getOrientation(), getGenDepth());
}

bool MineShaftPieces::MineShaftStairs::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box)) return false;
    const BlockState* planks = getPlanksBlock();
    if (planks == NULL) return false;
    for (int y = 0; y < 7; ++y)
        generateBox(level, box, 0, y, y, 2, y, y + 1, planks, planks, false);
    return true;
}

StructurePiece* MineShaftPieces::MineShaftStairs::Create()
{
    return new MineShaftStairs();
}

EStructurePiece MineShaftPieces::MineShaftStairs::GetType()
{
    return eStructurePiece_MineShaftStairs;
}

void MineShaftPieces::loadStatic()
{
    StructureFeatureIO::setPieceId(eStructurePiece_MineShaftCorridor, &MineShaftCorridor::Create, L"MSCorridor");
    StructureFeatureIO::setPieceId(eStructurePiece_MineShaftCrossing, &MineShaftCrossing::Create, L"MSCrossing");
    StructureFeatureIO::setPieceId(eStructurePiece_MineShaftRoom, &MineShaftRoom::Create, L"MSRoom");
    StructureFeatureIO::setPieceId(eStructurePiece_MineShaftStairs, &MineShaftStairs::Create, L"MSStairs");
}

StructurePiece* MineShaftPieces::createRandomShaftPiece(std::vector<StructurePiece*>* pieces, Random& random,
                                                        int x, int y, int z, const Direction* direction,
                                                        int genDepth, MineShaftFeature::MineShaftType type)
{
    int choice = random.nextInt(100);
    BoundingBox* box = NULL;
    if (choice < 70)
    {
        box = MineShaftCorridor::findCorridorSize(pieces, random, x, y, z, direction);
        return box == NULL ? NULL : new MineShaftCorridor(genDepth, random, box, direction, type);
    }
    if (choice < 80)
    {
        box = MineShaftStairs::findStairs(pieces, random, x, y, z, direction);
        return box == NULL ? NULL : new MineShaftStairs(genDepth, random, box, direction, type);
    }
    box = MineShaftCrossing::findCrossing(pieces, random, x, y, z, direction);
    return box == NULL ? NULL : new MineShaftCrossing(genDepth, random, box, direction, type);
}

int MineShaftPieces::generateAndAddPiece(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces,
                                         Random& random, int x, int y, int z, const Direction* direction,
                                         int genDepth)
{
    if (startPiece == NULL || pieces == NULL || genDepth >= 9) return 0;
    BoundingBox* parent = startPiece->getBoundingBox();
    if (parent == NULL || x - parent->x0 >= 81 || z - parent->z0 >= 81) return 0;
    MineShaftPiece* shaft = dynamic_cast<MineShaftPiece*>(startPiece);
    if (shaft == NULL) return 0;
    StructurePiece* piece = createRandomShaftPiece(pieces, random, x, y, z, direction, genDepth + 1, shaft->mType);
    if (piece == NULL) return 0;
    pieces->push_back(piece);
    piece->addChildren(startPiece, pieces, random);
    return piece == NULL ? 0 : 1;
}
