#include "game/client/render/EndermanModel.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/ModelPartsSkinned.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/block/MaterialTable.hpp"

EndermanModel::EndermanModel(float scale, const ModelDefinition& definition)
    : HumanoidModel(definition, 0.0f, -14.0f, 64, 32, false, true, true)
    , mModelParts(NULL)
    , mNumModelParts(0)
{
    mCarrying = false;
    mCreepy = false;
    ModelPart** modelParts = new ModelPart*[eModelPart_Count];
    mModelParts = modelParts;

    for (int i = 0; i < eModelPart_Count; ++i)
    {
        modelParts[i] = NULL;
    }

    mNumModelParts = eModelPart_Count;

    createModelParts(scale, definition);
    createInvisibleModelParts(scale, definition);
}

EndermanModel::~EndermanModel()
{
    delete[] mModelParts;

    mHead = NULL;
    mHat = NULL;
    mBody = NULL;
    mRightArm = NULL;
    mLeftArm = NULL;
    mRightLeg = NULL;
    mLeftLeg = NULL;
    mRightItem = NULL;
    mLeftItem = NULL;
}

void EndermanModel::createModelParts(float scale, const ModelDefinition& definition)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    if (definition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;

        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_Enderman);
    }
    else
    {
        delete mHat;
        mHat = new ModelPartSkin(this, 0, 16, NULL);
        mHat->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, scale - 0.5f);
        mHat->setPos(0.0f, -14.0f, 0.0f);

        delete mBody;
        mBody = new ModelPartSkin(this, 32, 16, NULL);
        mBody->addBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, scale);
        mBody->setPos(0.0f, -14.0f, 0.0f);

        delete mRightArm;
        mRightArm = new ModelPartSkin(this, 56, 0, NULL);
        mRightArm->addBox(-1.0f, -2.0f, -1.0f, 2, 30, 2, scale);
        mRightArm->setPos(-3.0f, -12.0f, 0.0f);

        delete mLeftArm;
        mLeftArm = new ModelPartSkin(this, 56, 0, NULL);
        mLeftArm->mMirror = true;
        mLeftArm->addBox(-1.0f, -2.0f, -1.0f, 2, 30, 2, scale);
        mLeftArm->setPos(5.0f, -12.0f, 0.0f);

        delete mRightLeg;
        mRightLeg = new ModelPartSkin(this, 56, 0, NULL);
        mRightLeg->addBox(-1.0f, 0.0f, -1.0f, 2, 30, 2, scale);
        mRightLeg->setPos(-2.0f, -2.0f, 0.0f);

        delete mLeftLeg;
        mLeftLeg = new ModelPartSkin(this, 56, 0, NULL);
        mLeftLeg->mMirror = true;
        mLeftLeg->addBox(-1.0f, 0.0f, -1.0f, 2, 30, 2, scale);
        mLeftLeg->setPos(2.0f, -2.0f, 0.0f);
    }

    mModelParts[eModelPart_Head] = mHead;
    mModelParts[eModelPart_Hat] = mHat;
    mModelParts[eModelPart_Body] = mBody;
    mModelParts[eModelPart_RightArm] = mRightArm;
    mModelParts[eModelPart_LeftArm] = mLeftArm;
    mModelParts[eModelPart_RightLeg] = mRightLeg;
    mModelParts[eModelPart_LeftLeg] = mLeftLeg;

    compileSkinnedDrawlist();

    builder->useDefaultShaders();
}

void EndermanModel::createInvisibleModelParts(float scale, const ModelDefinition& definition)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    builder->setVertexAndPixelShader(eMinecraftEntityMaterial_EndermanInvisible);

    if (definition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;

        mModelParts[eModelPart_InvisibleHead] = new ModelPartSkin(this, L"head", definition, NULL, 0.0f, &mInvisibleParts);
        mModelParts[eModelPart_InvisibleHat] = new ModelPartSkin(this, L"hat", definition, NULL, 0.0f, &mInvisibleParts);
        mModelParts[eModelPart_InvisibleBody] = new ModelPartSkin(this, L"body", definition, NULL, 0.0f, &mInvisibleParts);
        mModelParts[eModelPart_InvisibleRightArm] = new ModelPartSkin(this, L"rightArm", definition, NULL, 0.0f, &mInvisibleParts);
        mModelParts[eModelPart_InvisibleLeftArm] = new ModelPartSkin(this, L"leftArm", definition, NULL, 0.0f, &mInvisibleParts);
        mModelParts[eModelPart_InvisibleRightLeg] = new ModelPartSkin(this, L"rightLeg", definition, NULL, 0.0f, &mInvisibleParts);
        mModelParts[eModelPart_InvisibleLeftLeg] = new ModelPartSkin(this, L"leftLeg", definition, NULL, 0.0f, &mInvisibleParts);

        if (definition.hasModelPart(L"rightItem"))
        {
            mModelParts[eModelPart_InvisibleLeftItem] = new ModelPartSkin(this, L"rightItem", definition, mRightArm, 0.0f, &mInvisibleParts);
        }

        if (definition.hasModelPart(L"leftItem"))
        {
            mModelParts[eModelPart_InvisibleRightItem] = new ModelPartSkin(this, L"leftItem", definition, mLeftArm, 0.0f, &mInvisibleParts);
        }
    }
    else
    {
        ModelPart* part = new ModelPartSkin(this, 0, 0, &mInvisibleParts);
        mModelParts[eModelPart_InvisibleHead] = part;
        part->addHumanoidBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, scale);
        mModelParts[eModelPart_InvisibleHead]->setPos(0.0f, -14.0f, 0.0f);

        part = new ModelPartSkin(this, 0, 16, &mInvisibleParts);
        mModelParts[eModelPart_InvisibleHat] = part;
        part->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, scale - 0.5f);
        mModelParts[eModelPart_InvisibleHat]->setPos(0.0f, -14.0f, 0.0f);

        part = new ModelPartSkin(this, 32, 16, &mInvisibleParts);
        mModelParts[eModelPart_InvisibleBody] = part;
        part->addBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, scale);
        mModelParts[eModelPart_InvisibleBody]->setPos(0.0f, -14.0f, 0.0f);

        part = new ModelPartSkin(this, 56, 0, &mInvisibleParts);
        mModelParts[eModelPart_InvisibleRightArm] = part;
        part->addBox(-1.0f, -2.0f, -1.0f, 2, 30, 2, scale);
        mModelParts[eModelPart_InvisibleRightArm]->setPos(-3.0f, -12.0f, 0.0f);

        part = new ModelPartSkin(this, 56, 0, &mInvisibleParts);
        part->mMirror = true;
        mModelParts[eModelPart_InvisibleLeftArm] = part;
        part->addBox(-1.0f, -2.0f, -1.0f, 2, 30, 2, scale);
        mModelParts[eModelPart_InvisibleLeftArm]->setPos(5.0f, -12.0f, 0.0f);

        part = new ModelPartSkin(this, 56, 0, &mInvisibleParts);
        mModelParts[eModelPart_InvisibleRightLeg] = part;
        part->addBox(-1.0f, 0.0f, -1.0f, 2, 30, 2, scale);
        mModelParts[eModelPart_InvisibleRightLeg]->setPos(-2.0f, -2.0f, 0.0f);

        part = new ModelPartSkin(this, 56, 0, &mInvisibleParts);
        part->mMirror = true;
        mModelParts[eModelPart_InvisibleLeftLeg] = part;
        part->addBox(-1.0f, 0.0f, -1.0f, 2, 30, 2, scale);
        mModelParts[eModelPart_InvisibleLeftLeg]->setPos(2.0f, -2.0f, 0.0f);
    }

    mModelParts[eModelPart_InvisibleRightArm]->addChild(mRightItem);
    mModelParts[eModelPart_InvisibleLeftArm]->addChild(mLeftItem);

    mInvisibleParts.compile(0.0625f);

    builder->useDefaultShaders();
}

void EndermanModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    HumanoidModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, skinAdjustments);

    ModelPart* body = mBody;

    body->mY = -14.0f;
    body->mZ = -0.0f;
    body->mXRot = 0.0f;

    ModelPart* rightArm = mRightArm;
    ModelPart* leftArm = mLeftArm;
    ModelPart* rightLeg = mRightLeg;
    ModelPart* leftLeg = mLeftLeg;

    rightArm->mXRot *= 0.5f;
    leftArm->mXRot *= 0.5f;
    rightLeg->mXRot *= 0.5f;
    leftLeg->mXRot *= 0.5f;

    ModelPart* head = mHead;
    head->mShow = true;

    if (rightArm->mXRot > 0.4f)
    {
        rightArm->mXRot = 0.4f;
    }

    if (leftArm->mXRot > 0.4f)
    {
        leftArm->mXRot = 0.4f;
    }

    if (rightArm->mXRot < -0.4f)
    {
        rightArm->mXRot = -0.4f;
    }

    if (leftArm->mXRot < -0.4f)
    {
        leftArm->mXRot = -0.4f;
    }

    if (rightLeg->mXRot > 0.4f)
    {
        rightLeg->mXRot = 0.4f;
    }

    if (leftLeg->mXRot > 0.4f)
    {
        leftLeg->mXRot = 0.4f;
    }

    if (rightLeg->mXRot < -0.4f)
    {
        rightLeg->mXRot = -0.4f;
    }

    if (leftLeg->mXRot < -0.4f)
    {
        leftLeg->mXRot = -0.4f;
    }

    if (mCarrying)
    {
        rightArm->mXRot = -0.5f;
        leftArm->mXRot = -0.5f;
        rightArm->mZRot = 0.05f;
        leftArm->mZRot = -0.05f;
    }

    rightArm->mZ = -0.0f;
    leftArm->mZ = -0.0f;
    rightLeg->mZ = -0.0f;
    leftLeg->mZ = -0.0f;

    rightArm->mY = -12.0f;
    leftArm->mY = -12.0f;
    rightLeg->mY = -5.0f;
    leftLeg->mY = -5.0f;

    head->mZ = -0.0f;
    head->mY = -13.0f;

    ModelPart* hat = mHat;
    hat->mX = head->mX;
    hat->mY = head->mY;
    hat->mZ = head->mZ;
    hat->mXRot = head->mXRot;
    hat->mYRot = head->mYRot;
    hat->mZRot = head->mZRot;

    if (mCreepy)
    {
        head->mY -= 5.0f;
    }
}

void EndermanModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    if (entity->isInvisible())
    {
        mHead = mModelParts[eModelPart_InvisibleHead];
        mHat = mModelParts[eModelPart_InvisibleHat];
        mBody = mModelParts[eModelPart_InvisibleBody];
        mRightArm = mModelParts[eModelPart_InvisibleRightArm];
        mLeftArm = mModelParts[eModelPart_InvisibleLeftArm];
        mRightLeg = mModelParts[eModelPart_InvisibleRightLeg];
        mLeftLeg = mModelParts[eModelPart_InvisibleLeftLeg];
        mRightItem = mModelParts[eModelPart_InvisibleRightItem];
        mLeftItem = mModelParts[eModelPart_InvisibleLeftItem];
    }
    else
    {
        mHead = mModelParts[eModelPart_Head];
        mHat = mModelParts[eModelPart_Hat];
        mBody = mModelParts[eModelPart_Body];
        mRightArm = mModelParts[eModelPart_RightArm];
        mLeftArm = mModelParts[eModelPart_LeftArm];
        mRightLeg = mModelParts[eModelPart_RightLeg];
        mLeftLeg = mModelParts[eModelPart_LeftLeg];
        mRightItem = mModelParts[eModelPart_RightItem];
        mLeftItem = mModelParts[eModelPart_LeftItem];
    }

    setupBoneMatrices(entity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, useDisplayList);

    if (entity->isInvisible())
    {
        mInvisibleParts.render();
    }
    else
    {
        callSkinnedDrawlist();
    }
}

