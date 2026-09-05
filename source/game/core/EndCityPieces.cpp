#include "game/core/EndCityPieces.hpp"

#include <map>

#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructurePiece.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/generation/StructureTemplate.hpp"

static std::map<Rotation*, BlockPos> gTowerBridges;
static std::map<Rotation*, BlockPos> gFatTowerBridges;
static StructurePlaceSettings* gIgnoreEntitiesSettings = NULL;
static StructurePlaceSettings* gIgnoreEntitiesAndBlocksSettings = NULL;
static EndCityPieces::TowerSectionGenerator* gTowerGenerator = NULL;
static EndCityPieces::HouseTowerSectionGenerator* gHouseGenerator = NULL;
static EndCityPieces::TowerBridgeSectionGenerator* gTowerBridgeGenerator = NULL;
static EndCityPieces::FatTowerSectionGenerator* gFatTowerGenerator = NULL;

EndCityPieces::EndCityPiece* EndCityPieces::addHelper(std::vector<StructurePiece*>& pieces,
    EndCityPieces::EndCityPiece* piece)
{
    pieces.push_back(piece);
    return piece;
}

EndCityPieces::EndCityPiece* EndCityPieces::addPiece(StructureManager* structureManager,
    EndCityPieces::EndCityPiece* previous, const BlockPos& position,
    eMinecraftStructureTemplate templateType, Rotation* rotation, bool overwrite)
{
    BlockPos previousPosition = previous->getTemplatePosition();
    EndCityPiece* piece = new EndCityPiece(structureManager, templateType, previousPosition,
        rotation, overwrite);
    StructureTemplate* previousTemplate = previous->getStructureTemplate();
    StructurePlaceSettings* previousSettings = previous->getPlaceSettings();
    StructurePlaceSettings* pieceSettings = piece->getPlaceSettings();
    BlockPos connected = previousTemplate->calculateConnectedPosition(previousSettings,
        position, pieceSettings, BlockPos::ZERO);
    piece->move(connected.x, connected.y, connected.z);
    return piece;
}

bool EndCityPieces::recursiveChildren(StructureManager* structureManager, SectionGenerator* generator,
    int genDepth, EndCityPiece* previous, const BlockPos& position,
    std::vector<StructurePiece*>& pieces, Random& random)
{
    if (genDepth > 8)
        return false;
    std::vector<StructurePiece*> children;
    if (!generator->generate(structureManager, genDepth, previous, position, children, random))
        return false;
    int generationDepth = random.nextInt();
    std::vector<StructurePiece*>::iterator it;
    for (it = children.begin(); it != children.end(); ++it)
    {
        (*it)->setGenDepth(generationDepth);
        if (StructurePiece::findCollisionPiece(&pieces, (*it)->getBoundingBox()) != NULL)
            return false;
    }
    pieces.insert(pieces.end(), children.begin(), children.end());
    return true;
}

void EndCityPieces::startHouseTower(StructureManager* structureManager, const BlockPos& position,
    Rotation* rotation, std::vector<StructurePiece*>& pieces, Random& random)
{
    EndCityPiece* piece = addHelper(pieces, new EndCityPiece(structureManager,
        static_cast<eMinecraftStructureTemplate>(0x13), position, rotation, true));
    piece = addPiece(structureManager, piece, BlockPos(-1, 0, -1),
        static_cast<eMinecraftStructureTemplate>(0x1c), rotation, false);
    addHelper(pieces, piece);
    piece = addPiece(structureManager, piece, BlockPos(-1, 4, -1),
        static_cast<eMinecraftStructureTemplate>(0x20), rotation, false);
    addHelper(pieces, piece);
    piece = addPiece(structureManager, piece, BlockPos(-1, 8, -1),
        static_cast<eMinecraftStructureTemplate>(0x23), rotation, true);
    addHelper(pieces, piece);
    recursiveChildren(structureManager, gHouseGenerator, 1, piece, BlockPos::ZERO, pieces, random);
}

void EndCityPieces::createTowerBridgesMap()
{
    gTowerBridges.clear();
    gTowerBridges[Rotation::NONE] = BlockPos(1, -1, 0);
    gTowerBridges[Rotation::CLOCKWISE_90] = BlockPos(6, -1, 1);
    gTowerBridges[Rotation::COUNTERCLOCKWISE_90] = BlockPos(0, -1, 5);
    gTowerBridges[Rotation::CLOCKWISE_180] = BlockPos(5, -1, 6);
}

void EndCityPieces::createFatTowerBridgesMap()
{
    gFatTowerBridges.clear();
    gFatTowerBridges[Rotation::NONE] = BlockPos(4, -1, 0);
    gFatTowerBridges[Rotation::CLOCKWISE_90] = BlockPos(12, -1, 4);
    gFatTowerBridges[Rotation::COUNTERCLOCKWISE_90] = BlockPos(0, -1, 8);
    gFatTowerBridges[Rotation::CLOCKWISE_180] = BlockPos(8, -1, 12);
}

void EndCityPieces::loadStatic()
{
    createTowerBridgesMap();
    createFatTowerBridgesMap();
    if (gIgnoreEntitiesSettings == NULL)
    {
        gIgnoreEntitiesSettings = new StructurePlaceSettings();
        gIgnoreEntitiesSettings->setIgnoreEntities(true);
        gIgnoreEntitiesAndBlocksSettings = new StructurePlaceSettings();
        gIgnoreEntitiesAndBlocksSettings->setIgnoreEntities(true);
    }
    if (gTowerGenerator == NULL)
    {
        gTowerGenerator = new TowerSectionGenerator();
        gHouseGenerator = new HouseTowerSectionGenerator();
        gTowerBridgeGenerator = new TowerBridgeSectionGenerator();
        gFatTowerGenerator = new FatTowerSectionGenerator();
    }
}

EndCityPieces::EndCityPiece::EndCityPiece(StructureManager* structureManager,
    eMinecraftStructureTemplate templateType, const BlockPos& position,
    Rotation* rotation, bool overwrite)
    : TemplateStructurePiece(0), mTemplateType(templateType), mRotation(rotation), mOverwrite(overwrite)
{
    mTemplatePosition = position;
    loadTemplate(structureManager);
}

EndCityPieces::EndCityPiece::EndCityPiece()
    : TemplateStructurePiece(), mTemplateType(static_cast<eMinecraftStructureTemplate>(0xbc)),
      mRotation(Rotation::NONE), mOverwrite(false)
{
}

EndCityPieces::EndCityPiece::~EndCityPiece()
{
}

void EndCityPieces::EndCityPiece::loadTemplate(StructureManager* structureManager)
{
    const StructureTemplate* structure = structureManager->getOrCreate(mTemplateType);
    StructurePlaceSettings* settings = (mOverwrite ? gIgnoreEntitiesAndBlocksSettings :
        gIgnoreEntitiesSettings)->copy();
    settings->setRotation(mRotation);
    setup(structure, mTemplatePosition, settings);
}

void EndCityPieces::EndCityPiece::addAdditionalSaveData(DataOutputStream* output)
{
    TemplateStructurePiece::addAdditionalSaveData(output);
}

void EndCityPieces::EndCityPiece::readAdditionalSaveData(DataInputStream* input,
    StructureManager* structureManager)
{
    TemplateStructurePiece::readAdditionalSaveData(input, structureManager);
}

void EndCityPieces::EndCityPiece::handleDataMarker(const std::wstring& marker,
    const BlockPos& position, Level*, Random&, BoundingBox* box)
{
    if (marker == L"Chest")
        setBoundingBox(box);
    else if (marker == L"Sentry")
        setBoundingBox(box);
    else if (marker == L"Elytra")
        setBoundingBox(box);
}

StructurePiece* EndCityPieces::EndCityPiece::Create()
{
    return new EndCityPiece();
}

EStructurePiece EndCityPieces::EndCityPiece::GetType()
{
    return eStructurePiece_EndCityPiece;
}

EndCityPieces::HouseTowerSectionGenerator::HouseTowerSectionGenerator()
{
    init();
}

void EndCityPieces::HouseTowerSectionGenerator::init()
{
    int i;
    for (i = 0; i < 8; ++i)
        mGenerated[i] = false;
}

bool EndCityPieces::HouseTowerSectionGenerator::generate(StructureManager* structureManager, int genDepth,
    EndCityPiece* previous, const BlockPos&, std::vector<StructurePiece*>& pieces, Random& random)
{
    if (genDepth > 8)
        return false;
    Rotation* rotation = previous->getPlaceSettings()->getRotation();
    EndCityPiece* piece = addHelper(pieces, EndCityPieces::addPiece(structureManager, previous,
        BlockPos(-1, 4, -1), static_cast<eMinecraftStructureTemplate>(0x13), rotation, true));
    int branch = random.nextInt(3);
    if (!mGenerated[genDepth])
        mGenerated[genDepth] = true;
    else if (branch == 0)
        addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 4, -1),
            static_cast<eMinecraftStructureTemplate>(0x14), rotation, true));
    else if (branch == 1)
    {
        piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 0, -1),
            static_cast<eMinecraftStructureTemplate>(0x1d), rotation, false));
        piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 8, -1),
            static_cast<eMinecraftStructureTemplate>(0x1e), rotation, false));
        recursiveChildren(structureManager, gHouseGenerator, genDepth + 1, piece,
            BlockPos::ZERO, pieces, random);
    }
    else
    {
        piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 0, -1),
            static_cast<eMinecraftStructureTemplate>(0x1d), rotation, false));
        piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 4, -1),
            static_cast<eMinecraftStructureTemplate>(0x22), rotation, false));
        piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 8, -1),
            static_cast<eMinecraftStructureTemplate>(0x23), rotation, true));
        recursiveChildren(structureManager, gTowerGenerator, genDepth + 1, piece,
            BlockPos::ZERO, pieces, random);
    }
    return true;
}

void EndCityPieces::FatTowerSectionGenerator::init()
{
}

bool EndCityPieces::FatTowerSectionGenerator::generate(StructureManager* structureManager, int genDepth,
    EndCityPiece* previous, const BlockPos&, std::vector<StructurePiece*>& pieces, Random& random)
{
    Rotation* rotation = previous->getPlaceSettings()->getRotation();
    EndCityPiece* piece = addHelper(pieces, addPiece(structureManager, previous,
        BlockPos(-3, 4, -3), static_cast<eMinecraftStructureTemplate>(0x19), rotation, true));
    piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(0, 4, 0),
        static_cast<eMinecraftStructureTemplate>(0x1a), rotation, true));
    piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(0, 8, 0),
        static_cast<eMinecraftStructureTemplate>(0x1a), rotation, true));
    piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(0, 8, 0),
        static_cast<eMinecraftStructureTemplate>(0x1a), rotation, true));
    std::map<Rotation*, BlockPos>::iterator it;
    for (it = gFatTowerBridges.begin(); it != gFatTowerBridges.end(); ++it)
        if (random.nextInt(10) > 2)
            recursiveChildren(structureManager, gTowerBridgeGenerator, genDepth + 1,
                addHelper(pieces, addPiece(structureManager, piece, it->second,
                    static_cast<eMinecraftStructureTemplate>(0x15), rotation->getRotated(it->first), true)),
                BlockPos::ZERO, pieces, random);
    addHelper(pieces, addPiece(structureManager, piece, BlockPos(-2, 8, -2),
        static_cast<eMinecraftStructureTemplate>(0x1b), rotation, true));
    return true;
}

EndCityPieces::TowerBridgeSectionGenerator::TowerBridgeSectionGenerator()
    : mGenerated(false)
{
}

void EndCityPieces::TowerBridgeSectionGenerator::init()
{
    mGenerated = false;
}

bool EndCityPieces::TowerBridgeSectionGenerator::generate(StructureManager* structureManager, int genDepth,
    EndCityPiece* previous, const BlockPos&, std::vector<StructurePiece*>& pieces, Random& random)
{
    Rotation* rotation = previous->getPlaceSettings()->getRotation();
    int count = random.nextInt(4) + 1;
    EndCityPiece* piece = addHelper(pieces, addPiece(structureManager, previous,
        BlockPos(0, 0, -4), static_cast<eMinecraftStructureTemplate>(0x17), rotation, true));
    piece->setGenDepth(-1);
    int y = 0;
    int i;
    for (i = 0; i < count; ++i)
    {
        if (random.nextBoolean())
            piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(0, y, -4),
                static_cast<eMinecraftStructureTemplate>(0x17), rotation, true));
        else
            piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(0, y, -8),
                static_cast<eMinecraftStructureTemplate>(0x16), rotation, true));
        y += 4;
    }
    if (!mGenerated)
    {
        addHelper(pieces, addPiece(structureManager, piece, BlockPos(random.nextInt(8) - 8, y, -48),
            static_cast<eMinecraftStructureTemplate>(0x1f), rotation, true));
        mGenerated = true;
    }
    else if (!recursiveChildren(structureManager, gHouseGenerator, genDepth + 1, piece,
        BlockPos(-3, y + 1, -21), pieces, random))
        return false;
    piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(4, y, 0),
        static_cast<eMinecraftStructureTemplate>(0x15), rotation->getRotated(Rotation::COUNTERCLOCKWISE_90), true));
    piece->setGenDepth(-1);
    return true;
}

void EndCityPieces::TowerSectionGenerator::init()
{
}

bool EndCityPieces::TowerSectionGenerator::generate(StructureManager* structureManager, int genDepth,
    EndCityPiece* previous, const BlockPos&, std::vector<StructurePiece*>& pieces, Random& random)
{
    Rotation* rotation = previous->getPlaceSettings()->getRotation();
    EndCityPiece* piece = addHelper(pieces, addPiece(structureManager, previous,
        BlockPos(4, -3, 4), static_cast<eMinecraftStructureTemplate>(0x24), rotation, true));
    piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(0, 7, 0),
        static_cast<eMinecraftStructureTemplate>(0x26), rotation, true));
    int count = random.nextInt(3) + 1;
    int i;
    for (i = 0; i < count; ++i)
        piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(0, 4, 0),
            static_cast<eMinecraftStructureTemplate>(0x26), rotation, true));
    if (genDepth == 7)
        addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 4, -1),
            static_cast<eMinecraftStructureTemplate>(0x27), rotation, true));
    else if (!recursiveChildren(structureManager, gTowerBridgeGenerator, genDepth + 1,
        piece, BlockPos::ZERO, pieces, random))
        return false;
    addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 4, -1),
        static_cast<eMinecraftStructureTemplate>(0x27), rotation, true));
    return true;
}
