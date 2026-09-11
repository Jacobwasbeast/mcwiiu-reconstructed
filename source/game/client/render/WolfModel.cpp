#include "game/client/render/WolfModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Wolf.hpp"
WolfModel::WolfModel(const ModelDefinition& modelDefinition):mHead(NULL),mBody(NULL),mLeg1(NULL),mLeg2(NULL),mLeg3(NULL),mLeg4(NULL),mTail(NULL){
 mHead=new ModelPart(this); mHead->texOffs(0,0)->addBox(-3,-3,-2,6,6,4); mHead->setPos(-1,13,-7);
 mBody=new ModelPart(this); mBody->texOffs(18,14)->addBox(-4,-2,-3,8,12,6); mBody->setPos(0,11,2);
 mLeg1=new ModelPart(this); mLeg2=new ModelPart(this); mLeg3=new ModelPart(this); mLeg4=new ModelPart(this);
 mLeg1->setPos(-2.5f,16,7); mLeg2->setPos(0.5f,16,7); mLeg3->setPos(-2.5f,16,-4); mLeg4->setPos(0.5f,16,-4);
 mTail=new ModelPart(this); mTail->texOffs(0,18)->addBox(-1,0,-1,2,8,2); mTail->setPos(-1,16,7); compileSkinnedDrawlist(); }
WolfModel::~WolfModel(){}
void WolfModel::render(boost::shared_ptr<Entity>& e, float a, float b, float c, float d, float f, float s, bool u)
{
    setupAnim(a, b, c, d, f, s, e, _SkinAdjustments());
    mHead->render(s, u, false);
    mBody->render(s, u, false);
    mLeg1->render(s, u, false);
    mLeg2->render(s, u, false);
    mLeg3->render(s, u, false);
    mLeg4->render(s, u, false);
    mTail->render(s, u, false);
    callSkinnedDrawlist();
}
void WolfModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                                float limbSwingAmount, float partialTicks)
{
    boost::shared_ptr<Wolf> wolf = boost::dynamic_pointer_cast<Wolf>(entity);
    ModelPart* upperBody = *reinterpret_cast<ModelPart**>(reinterpret_cast<char*>(this) + 0x80);

    if (wolf->isAngry())
        mTail->mYRot = 0.0f;
    else
        mTail->mYRot = Mth::cos(limbSwing * 0.6661999821662903f) * 1.399999976158142f * limbSwingAmount;

    if (!wolf->isSitting())
    {
        mBody->setPos(0.0f, 14.0f, 2.0f);
        mBody->mXRot = 90.0f * 0.01745329238474369f;
        upperBody->setPos(-1.0f, 14.0f, -3.0f);
        upperBody->mXRot = mBody->mXRot;
        mTail->setPos(-1.0f, 12.0f, 8.0f);
        mLeg1->setPos(-2.5f, 16.0f, 7.0f);
        mLeg2->setPos(0.5f, 16.0f, 7.0f);
        mLeg3->setPos(-2.5f, 16.0f, -4.0f);
        mLeg4->setPos(0.5f, 16.0f, -4.0f);

        float f = limbSwing * 0.6661999821662903f;
        mLeg1->mXRot = Mth::cos(f) * 1.399999976158142f * limbSwingAmount;
        mLeg2->mXRot = Mth::cos(f + 3.1415927410125732f) * 1.399999976158142f * limbSwingAmount;
        mLeg3->mXRot = Mth::cos(f + 3.1415927410125732f) * 1.399999976158142f * limbSwingAmount;
        mLeg4->mXRot = Mth::cos(f) * 1.399999976158142f * limbSwingAmount;
    }
    else
    {
        upperBody->setPos(-1.0f, 16.0f, -3.0f);
        upperBody->mXRot = 1.2566370964050293f;
        upperBody->mYRot = 0.0f;
        mBody->setPos(0.0f, 18.0f, 0.0f);
        mBody->mXRot = 0.7853981852531433f;
        mTail->setPos(-1.0f, 5.0f, 6.0f);
        mLeg1->setPos(-2.5f, 22.0f, 2.0f);
        mLeg1->mXRot = 4.71238899230957f;
        mLeg2->setPos(0.5f, 22.0f, 2.0f);
        mLeg2->mXRot = 4.71238899230957f;
        mLeg3->setPos(-2.49f, 17.0f, -4.0f);
        mLeg3->mXRot = 5.811946868896484f;
        mLeg4->setPos(0.5099999904632568f, 17.0f, -4.0f);
        mLeg4->mXRot = 5.811946868896484f;
    }

    mHead->mZRot = (float)(wolf->getHeadRollAngle(partialTicks)
                         + wolf->getBodyRollAngle(partialTicks, 0.0f));
    upperBody->mZRot = (float)wolf->getBodyRollAngle(partialTicks, -0.07999999821186066f);
    mBody->mZRot = (float)wolf->getBodyRollAngle(partialTicks, -0.1599999964237213f);
    mTail->mZRot = (float)wolf->getBodyRollAngle(partialTicks, -0.20000000298023224f);
}
void WolfModel::setupAnim(float a, float b, float c, float d, float f, float s,
                           boost::shared_ptr<Entity> e, _SkinAdjustments x)
{
    Model::setupAnim(a, b, c, d, f, s, e, x);
    mHead->mYRot = d * 0.017453292f;
    mHead->mXRot = f * 0.017453292f;
    mLeg1->mXRot = 0.6662f * a;
    mLeg2->mXRot = 0.6662f * a + 3.1415927f;
    mLeg3->mXRot = 0.6662f * a + 3.1415927f;
    mLeg4->mXRot = 0.6662f * a;
    mTail->mXRot = 0.3f;
}
std::wstring WolfModel::getName(){return L"wolf";}
