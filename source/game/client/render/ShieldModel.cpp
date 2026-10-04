#include "game/client/render/ShieldModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"

static int FACE_RIGHT = 1;
static int FACE_LEFT = 2;
static int FACE_TOP = 4;
static int FACE_BOTTOM = 8;
static int FACE_FRONT = 16;
static int FACE_BACK = 32;

ShieldModel::ShieldModel(const ModelDefinition& modelDefinition)
{
    mTexWidth = 64;
    mTexHeight = 64;

    mPlate = new ModelPart(this, 0, 0);
    mPlate->addBoxWithMask(-6.0f, -11.0f, -2.0f, 12, 22, 1,
                           FACE_RIGHT + FACE_LEFT + FACE_TOP + FACE_BOTTOM + FACE_BACK);

    mHandle = new ModelPart(this, 26, 0);
    mHandle->addBox(-1.0f, -3.0f, -1.0f, 2, 6, 6, 0.0f);

    mPlate->compile(0.0625f);
    mHandle->compile(0.0625f);

    mPlateFront = new ModelPart(this, 0, 0);
    mPlateFront->addBoxWithMask(-6.0f, -11.0f, -2.0f, 12, 22, 1, FACE_FRONT);
    mPlateFront->compile(0.0625f);
}

void ShieldModel::render(bool useDisplayList)
{
    mPlate->render(0.0625f, useDisplayList, false);
    mHandle->render(0.0625f, useDisplayList, false);
}
