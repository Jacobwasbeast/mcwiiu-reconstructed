#include "game/minigame/ApplySchematicRuleDefinition.hpp"

#include <cmath>

#include "game/core/ConsoleSchematicFile.hpp"
#include "game/core/StringConversion.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/core/PIXNamedEvents.hpp"

ApplySchematicRuleDefinition::ApplySchematicRuleDefinition(LevelGenerationOptions* options)
    : mLevelGenerationOptions(options),
      mSchematicName(),
      mSchematicFile(NULL),
      mOrigin(Vec3::newPermanent(0.0, 0.0, 0.0)),
      mLocationBox(NULL),
      mRotation(ConsoleSchematicFile::eSchematicRotation_0),
      mDimension(0),
      mBlocksApplied(0),
      mLightingApplied(0),
      mComplete(false)
{
}

ApplySchematicRuleDefinition::~ApplySchematicRuleDefinition()
{
    mSchematicFile = NULL;
    delete mOrigin;
}

void ApplySchematicRuleDefinition::reset()
{
    mComplete = false;
    mBlocksApplied = 0;
    mLightingApplied = 0;
}

bool ApplySchematicRuleDefinition::isComplete()
{
    return mComplete;
}

void ApplySchematicRuleDefinition::updateLocationBox()
{
    if (mSchematicFile == NULL)
    {
        mSchematicFile = mLevelGenerationOptions->getSchematicFile(mSchematicName);
    }

    double width = static_cast<double>(mSchematicFile->mWidth);
    double height = static_cast<double>(mSchematicFile->mHeight);
    double depth = static_cast<double>(mSchematicFile->mDepth);

    if (mRotation == ConsoleSchematicFile::eSchematicRotation_90 ||
        mRotation == ConsoleSchematicFile::eSchematicRotation_270)
    {
        double dimension = width;
        width = depth;
        depth = dimension;
    }

    mLocationBox = AABB::newPermanent(mOrigin->x, mOrigin->y, mOrigin->z,
                                      mOrigin->x + width, mOrigin->y + height,
                                      mOrigin->z + depth);
}

void ApplySchematicRuleDefinition::processSchematic(AABB* chunkBox, LevelChunk* chunk)
{
    if (mComplete || chunk->level->mDimension->getType() != DimensionType::getById(mDimension))
    {
        return;
    }

    PIXBeginNamedEvent(0.0f, "Processing ApplySchematicRuleDefinition");

    if (mSchematicFile == NULL)
    {
        mSchematicFile = mLevelGenerationOptions->getSchematicFile(mSchematicName);
    }
    if (mLocationBox == NULL)
    {
        updateLocationBox();
    }

    if (chunkBox->intersects(mLocationBox->mMinX, mLocationBox->mMinY,
                             mLocationBox->mMinZ, mLocationBox->mMaxX,
                             mLocationBox->mMaxY, mLocationBox->mMaxZ))
    {
        if (mLocationBox->mMaxY > 256.0)
        {
            mLocationBox->mMaxY = 256.0;
        }

        PIXBeginNamedEvent(0.0f, "Applying blocks and data");
        mBlocksApplied += mSchematicFile->applyBlocksAndData(
            chunk, mLocationBox, mRotation, ConsoleSchematicFile::eSchematicMirror_None);
        PIXEndNamedEvent();
        PIXBeginNamedEvent(0.0f, "Applying block entities");
        mSchematicFile->applyBlockEntities(chunk, chunkBox, mLocationBox, mRotation,
                                            ConsoleSchematicFile::eSchematicMirror_None, NULL);
        PIXEndNamedEvent();

        int volume = static_cast<int>((mLocationBox->mMaxX - mLocationBox->mMinX) *
                                      (mLocationBox->mMaxY - mLocationBox->mMinY) *
                                      (mLocationBox->mMaxZ - mLocationBox->mMinZ));
        if (mBlocksApplied == volume && mLightingApplied == volume)
        {
            mComplete = true;
        }
    }

    PIXEndNamedEvent();
}

void ApplySchematicRuleDefinition::processSchematicLighting(AABB* chunkBox, LevelChunk* chunk)
{
    if (mComplete || chunk->level->mDimension->getType() != DimensionType::getById(mDimension))
    {
        return;
    }

    PIXBeginNamedEvent(0.0f, "Processing ApplySchematicRuleDefinition (lighting)");

    if (mSchematicFile == NULL)
    {
        mSchematicFile = mLevelGenerationOptions->getSchematicFile(mSchematicName);
    }
    if (mLocationBox == NULL)
    {
        updateLocationBox();
    }

    if (chunkBox->intersects(mLocationBox->mMinX, mLocationBox->mMinY,
                             mLocationBox->mMinZ, mLocationBox->mMaxX,
                             mLocationBox->mMaxY, mLocationBox->mMaxZ))
    {
        if (mLocationBox->mMaxY > 256.0)
        {
            mLocationBox->mMaxY = 256.0;
        }

        PIXBeginNamedEvent(0.0f, "Patching lighting");
        mLightingApplied += mSchematicFile->applyLighting(chunk, chunkBox, mLocationBox, mRotation);
        PIXEndNamedEvent();

        int volume = static_cast<int>((mLocationBox->mMaxX - mLocationBox->mMinX) *
                                      (mLocationBox->mMaxY - mLocationBox->mMinY) *
                                      (mLocationBox->mMaxZ - mLocationBox->mMinZ));
        if (mBlocksApplied == volume && mLightingApplied == volume)
        {
            mComplete = true;
        }
    }

    PIXEndNamedEvent();
}

int ApplySchematicRuleDefinition::getMinY()
{
    if (mLocationBox == NULL)
    {
        updateLocationBox();
    }
    return static_cast<int>(mLocationBox->mMinY);
}

bool ApplySchematicRuleDefinition::checkIntersects(const BlockPos& min, const BlockPos& max)
{
    if (mLocationBox == NULL)
    {
        updateLocationBox();
    }
    Vec3* from = Vec3::newTemp(&min);
    Vec3* to = Vec3::newTemp(&max);
    return mLocationBox->intersects(from, to);
}

std::wstring ApplySchematicRuleDefinition::getSchematicName()
{
    return mSchematicName;
}

ConsoleGameRules::EGameRuleType ApplySchematicRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_ApplySchematic;
}

void ApplySchematicRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface,
                                                    unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 5);
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_filename, mSchematicName);
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_x, _toString(mOrigin->x));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_y, _toString(mOrigin->y));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_z, _toString(mOrigin->z));
    int rotation = 0;
    if (mRotation >= ConsoleSchematicFile::eSchematicRotation_90 &&
        mRotation <= ConsoleSchematicFile::eSchematicRotation_270)
    {
        rotation = static_cast<int>(mRotation) * 90;
    }
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_rot, _toString(rotation));
}

void ApplySchematicRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name == L"filename")
    {
        mSchematicName = ConsoleSchematicFile::formatFilenameWithExt(value);
        if (!mSchematicName.empty())
        {
            mSchematicFile = mLevelGenerationOptions->getSchematicFile(mSchematicName);
        }
        return;
    }
    if (name == L"x")
    {
        double x = static_cast<double>(_fromString<int>(value));
        if (static_cast<int>(fabs(x)) & 1)
        {
            x -= 1.0;
        }
        mOrigin->x = x;
        return;
    }
    if (name == L"y")
    {
        double y = static_cast<double>(_fromString<int>(value));
        if (static_cast<int>(fabs(y)) & 1)
        {
            y -= 1.0;
        }
        if (y < 0.0)
        {
            y = 0.0;
        }
        mOrigin->y = y;
        return;
    }
    if (name == L"z")
    {
        double z = static_cast<double>(_fromString<int>(value));
        if (static_cast<int>(fabs(z)) & 1)
        {
            z -= 1.0;
        }
        mOrigin->z = z;
        return;
    }
    if (name == L"rot")
    {
        int rotation = _fromString<int>(value);
        while (rotation < 0) rotation += 360;
        while (rotation >= 360) rotation -= 360;
        int rotationIndex = static_cast<int>(static_cast<float>(rotation) / 90.0f + 0.5f);
        if (rotationIndex == 0 || rotationIndex == 1 || rotationIndex == 2)
        {
            mRotation = static_cast<ConsoleSchematicFile::ESchematicRotation>(rotationIndex);
        }
        else if (rotationIndex == 3 || rotationIndex == 4)
        {
            mRotation = ConsoleSchematicFile::eSchematicRotation_270;
        }
        else
        {
            mRotation = ConsoleSchematicFile::eSchematicRotation_0;
        }
        return;
    }
    if (name == L"dim")
    {
        int dimension = _fromString<int>(value);
        mDimension = static_cast<unsigned int>(dimension + 1) < 3 ? dimension : 0;
        return;
    }
    GameRuleDefinition::addAttribute(name, value);
}
