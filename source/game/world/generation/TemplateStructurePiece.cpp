#include "game/world/generation/TemplateStructurePiece.hpp"

#include <boost/unordered_map.hpp>

#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/generation/StructureTemplate.hpp"

TemplateStructurePiece::TemplateStructurePiece()
    : StructurePiece(), mTemplate(NULL), mPlaceSettings(new StructurePlaceSettings()), mTemplatePosition()
{
    mPlaceSettings->setIgnoreEntities(true);
    mPlaceSettings->setIgnoreBlock(Blocks::air);
}

TemplateStructurePiece::TemplateStructurePiece(int genDepth)
    : StructurePiece(genDepth), mTemplate(NULL), mPlaceSettings(new StructurePlaceSettings()), mTemplatePosition()
{
    mPlaceSettings->setIgnoreEntities(true);
    mPlaceSettings->setIgnoreBlock(Blocks::air);
}

TemplateStructurePiece::~TemplateStructurePiece()
{
    delete mPlaceSettings;
}

void TemplateStructurePiece::setBoundingBoxFromTemplate()
{
    BlockPos size = mTemplate->getSize(mPlaceSettings->getRotation());
    delete mBoundingBox;
    mBoundingBox = new BoundingBox(0, 0, 0, size.x, size.y - 1, size.z);

    Rotation* rotation = mPlaceSettings->getRotation();
    Mirror* mirror = mPlaceSettings->getMirror();

    if (rotation == Rotation::CLOCKWISE_90)
        mBoundingBox->move(-size.x, 0, 0);
    else if (rotation == Rotation::CLOCKWISE_180)
        mBoundingBox->move(-size.x, 0, -size.z);
    else if (rotation == Rotation::COUNTERCLOCKWISE_90)
        mBoundingBox->move(0, 0, -size.z);

    if (mirror == Mirror::LEFT_RIGHT)
    {
        if (rotation == Rotation::NONE || rotation == Rotation::CLOCKWISE_180)
        {
            BlockPos offset = mTemplatePosition.relative(rotation->rotate(Direction::NORTH), size.x);
            mBoundingBox->move(offset.x, 0, offset.z);
        }
        else if (rotation == Rotation::CLOCKWISE_90)
        {
            BlockPos offset = mTemplatePosition.relative(Direction::NORTH);
            mBoundingBox->move(offset.x, 0, offset.z);
        }
        else
        {
            BlockPos offset = mTemplatePosition.relative(Direction::SOUTH, size.z);
            mBoundingBox->move(offset.x, 0, offset.z);
        }
    }
    else if (mirror == Mirror::FRONT_BACK)
    {
        if (rotation == Rotation::NONE || rotation == Rotation::CLOCKWISE_180)
        {
            BlockPos offset = mTemplatePosition.relative(rotation->rotate(Direction::EAST), size.z);
            mBoundingBox->move(offset.x, 0, offset.z);
        }
        else if (rotation == Rotation::CLOCKWISE_90)
        {
            BlockPos offset = mTemplatePosition.relative(Direction::EAST, size.x);
            mBoundingBox->move(offset.x, 0, offset.z);
        }
        else
        {
            BlockPos offset = mTemplatePosition.relative(Direction::WEST);
            mBoundingBox->move(offset.x, 0, offset.z);
        }
    }

    mBoundingBox->move(mTemplatePosition.x, mTemplatePosition.y, mTemplatePosition.z);
}

void TemplateStructurePiece::setup(const StructureTemplate* templateObject, const BlockPos& templatePosition,
    StructurePlaceSettings* placeSettings)
{
    mTemplate = const_cast<StructureTemplate*>(templateObject);
    setOrientation(Direction::NORTH);
    mTemplatePosition = templatePosition;
    mPlaceSettings = placeSettings;
    setBoundingBoxFromTemplate();
}

void TemplateStructurePiece::addAdditionalSaveData(DataOutputStream* output)
{
    output->writeInt(mTemplatePosition.x);
    output->writeInt(mTemplatePosition.y);
    output->writeInt(mTemplatePosition.z);
}

void TemplateStructurePiece::readAdditionalSaveData(DataInputStream* input, StructureManager*)
{
    mTemplatePosition = BlockPos(input->readInt(), input->readInt(), input->readInt());
}

bool TemplateStructurePiece::postProcess(Level* level, Random& random, BoundingBox* box)
{
    mPlaceSettings->setBoundingBox(box);
    mTemplate->placeInWorld(level, mTemplatePosition, mPlaceSettings, 18);
    StructureTemplate::MarkerMap markers = mTemplate->getMarkers(mTemplatePosition, mPlaceSettings);

    for (StructureTemplate::MarkerMap::const_iterator it = markers.begin(); it != markers.end(); ++it)
        handleDataMarker(it->second, it->first, level, random, box);

    return true;
}

void TemplateStructurePiece::move(int x, int y, int z)
{
    StructurePiece::move(x, y, z);
    mTemplatePosition = mTemplatePosition.offset(x, y, z);
}

Rotation* TemplateStructurePiece::getRotation()
{
    return mPlaceSettings->getRotation();
}

BlockPos TemplateStructurePiece::getTemplatePosition()
{
    return mTemplatePosition;
}

StructureTemplate* TemplateStructurePiece::getStructureTemplate()
{
    return mTemplate;
}

StructurePlaceSettings* TemplateStructurePiece::getPlaceSettings()
{
    return mPlaceSettings;
}
