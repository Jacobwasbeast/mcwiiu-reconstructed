#include "game/client/render/SilverfishModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/Mth.hpp"

static const int BODY_PART_COUNT = 7;
static const int BODY_LAYER_COUNT = 3;

static const int BODY_PART_TEX_OFFS[BODY_PART_COUNT][2] =
{
    { 0, 0 },
    { 0, 4 },
    { 0, 9 },
    { 0, 16 },
    { 0, 22 },
    { 11, 0 },
    { 13, 4 }
};

static const int BODY_PART_SIZES[BODY_PART_COUNT][3] =
{
    { 3, 2, 2 },
    { 4, 3, 2 },
    { 6, 4, 3 },
    { 3, 3, 3 },
    { 2, 2, 3 },
    { 2, 1, 2 },
    { 1, 1, 2 }
};

SilverfishModel::SilverfishModel(const ModelDefinition& modelDefinition)
    : mBodyParts(NULL)
    , mNumBodyParts(0)
    , mBodyLayers(NULL)
    , mNumBodyLayers(0)
{
    mBodyParts = new ModelPart*[BODY_PART_COUNT]();
    mNumBodyParts = BODY_PART_COUNT;

    mBodyLayers = new ModelPart*[BODY_LAYER_COUNT]();
    mNumBodyLayers = BODY_LAYER_COUNT;

    if (modelDefinition.DoesOverrideModel())
    {
        mBodyParts[0] = new ModelPartSkin(this, L"bodyPart_0", modelDefinition, NULL, 0.0f, NULL);
        mBodyParts[1] = new ModelPartSkin(this, L"bodyPart_1", modelDefinition, NULL, 0.0f, NULL);
        mBodyParts[2] = new ModelPartSkin(this, L"bodyPart_2", modelDefinition, NULL, 0.0f, NULL);
        mBodyParts[3] = new ModelPartSkin(this, L"bodyPart_3", modelDefinition, NULL, 0.0f, NULL);
        mBodyParts[4] = new ModelPartSkin(this, L"bodyPart_4", modelDefinition, NULL, 0.0f, NULL);
        mBodyParts[5] = new ModelPartSkin(this, L"bodyPart_5", modelDefinition, NULL, 0.0f, NULL);
        mBodyParts[6] = new ModelPartSkin(this, L"bodyPart_6", modelDefinition, NULL, 0.0f, NULL);

        mBodyLayers[0] = new ModelPartSkin(this, L"bodyLayer_0", modelDefinition, NULL, 0.0f, NULL);
        mBodyLayers[1] = new ModelPartSkin(this, L"bodyLayer_1", modelDefinition, NULL, 0.0f, NULL);
        mBodyLayers[2] = new ModelPartSkin(this, L"bodyLayer_2", modelDefinition, NULL, 0.0f, NULL);

        compileSkinnedDrawlist();
    }
    else
    {
        float zPos = -3.5f;

        for (unsigned int i = 0; i < mNumBodyParts; ++i)
        {
            mBodyParts[i] = new ModelPartSkin(this, BODY_PART_TEX_OFFS[i][0], BODY_PART_TEX_OFFS[i][1], NULL);
            mBodyParts[i]->addBox(BODY_PART_SIZES[i][0] * -0.5f, 0.0f, BODY_PART_SIZES[i][2] * -0.5f,
                                  BODY_PART_SIZES[i][0], BODY_PART_SIZES[i][1], BODY_PART_SIZES[i][2]);
            mBodyParts[i]->setPos(0.0f, 24.0f - BODY_PART_SIZES[i][1], zPos);
            mBodyPartZ[i] = zPos;

            if (i < mNumBodyParts - 1)
            {
                zPos += (BODY_PART_SIZES[i][2] + BODY_PART_SIZES[i + 1][2]) * 0.5f;
            }
        }

        mBodyLayers[0] = new ModelPartSkin(this, 20, 0, NULL);
        mBodyLayers[0]->addBox(-5.0f, 0.0f, -1.5f, 10, 8, 3);
        mBodyLayers[0]->setPos(0.0f, 16.0f, mBodyPartZ[2]);

        mBodyLayers[1] = new ModelPartSkin(this, 20, 11, NULL);
        mBodyLayers[1]->addBox(-3.0f, 0.0f, -1.5f, 6, 4, 3);
        mBodyLayers[1]->setPos(0.0f, 20.0f, mBodyPartZ[4]);

        mBodyLayers[2] = new ModelPartSkin(this, 20, 18, NULL);
        mBodyLayers[2]->addBox(-3.0f, 0.0f, -1.5f, 6, 5, 2);
        mBodyLayers[2]->setPos(0.0f, 19.0f, mBodyPartZ[1]);

        compileSkinnedDrawlist();
    }
}

SilverfishModel::~SilverfishModel()
{
    for (unsigned int i = 0; i < mNumBodyParts; ++i)
    {
        delete mBodyParts[i];
    }

    for (unsigned int i = 0; i < mNumBodyLayers; ++i)
    {
        delete mBodyLayers[i];
    }
}

void SilverfishModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    for (unsigned int i = 0; i < mNumBodyParts; ++i)
    {
        mBodyParts[i]->render(scale, useDisplayList, false);
    }

    for (unsigned int i = 0; i < mNumBodyLayers; ++i)
    {
        mBodyLayers[i]->render(scale, useDisplayList, false);
    }

    callSkinnedDrawlist();
}

void SilverfishModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    for (unsigned int i = 0; i < mNumBodyParts; ++i)
    {
        float angle = (float)i * 0.15f * 3.1415927f + ageInTicks * 0.9f;

        mBodyParts[i]->mYRot = Mth::cos(angle) * 3.1415927f * 0.05f * (float)(Mth::abs((int)i - 2) + 1);
        mBodyParts[i]->mX = Mth::sin(angle) * 3.1415927f * 0.2f * (float)Mth::abs((int)i - 2);
    }

    mBodyLayers[0]->mYRot = mBodyParts[2]->mYRot;

    mBodyLayers[1]->mYRot = mBodyParts[4]->mYRot;
    mBodyLayers[1]->mX = mBodyParts[4]->mX;

    mBodyLayers[2]->mYRot = mBodyParts[1]->mYRot;
    mBodyLayers[2]->mX = mBodyParts[1]->mX;
}

std::wstring SilverfishModel::getName()
{
    std::wstring result;
    result.assign(L"silverfish");
    return result;
}
