#include "game/client/render/ModelPartDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartCubeDefinition.hpp"
#include "game/core/DataInputStream.hpp"

ModelPartDefinition::ModelPartDefinition()
    : mX(0.0f), mY(0.0f), mZ(0.0f), mXRot(0.0f), mYRot(0.0f), mZRot(0.0f),
      mCubeXRot(0.0f), mCubeYRot(0.0f), mCubeZRot(0.0f), mCubeCount(0), mCubes(0)
{
}

ModelPartDefinition::~ModelPartDefinition()
{
    delete[] mCubes;
}

void ModelPartDefinition::SetPartPosition(ModelPart* part, ModelPart* parent) const
{
    part->setWorldPos(mX, mY, mZ);
    if (parent == 0)
        part->setPos(mX, mY, mZ);
    else
        part->setPos(mX - parent->mZ, mY - parent->mWorldX, mZ - parent->mWorldY);
}

void ModelPartDefinition::SetPartRotation(ModelPart* part) const
{
    part->mXRot = mXRot;
    part->mYRot = mYRot;
    part->mZRot = mZRot;
    part->mCubeXRot = mCubeXRot;
    part->mCubeYRot = mCubeYRot;
    part->mCubeZRot = mCubeZRot;
}

void ModelPartDefinition::AddCubes(ModelPart* part, float scale) const
{
    int i;
    for (i = 0; i < mCubeCount; ++i)
    {
        const ModelPartCubeDefinition& cube = mCubes[i];
        part->addBoxMapTex(cube.x, cube.y, cube.z, cube.width, cube.height, cube.depth,
                           static_cast<int>(cube.texU), static_cast<int>(cube.texV),
                           cube.mirror, cube.inflate + scale);
    }
}

void ModelPartDefinition::SetupModelPart(ModelPart* part, ModelPart* parent, float scale) const
{
    SetPartPosition(part, parent);
    SetPartRotation(part);
    AddCubes(part, scale);
    if (parent != 0)
        parent->addChild(part);
}

void ModelPartDefinition::readModelPart(DataInputStream* input, int hasCubeRotation)
{
    const float degreesToRadians = 0.017453292f;
    mX = input->readFloat();
    mY = input->readFloat();
    mZ = input->readFloat();
    mXRot = input->readFloat() * degreesToRadians;
    mYRot = input->readFloat() * degreesToRadians;
    mZRot = input->readFloat() * degreesToRadians;
    if (hasCubeRotation > 0)
    {
        mCubeXRot = input->readFloat() * degreesToRadians;
        mCubeYRot = input->readFloat() * degreesToRadians;
        mCubeZRot = input->readFloat() * degreesToRadians;
    }
    mCubeCount = input->readInt();
    mCubes = new ModelPartCubeDefinition[mCubeCount];
    int i;
    for (i = 0; i < mCubeCount; ++i)
        mCubes[i].readModelPartCube(input);
}
