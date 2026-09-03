#include "game/client/render/WolfModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
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
void WolfModel::prepareMobModel(boost::shared_ptr<LivingEntity>& e,float a,float b,float c){Model::prepareMobModel(e,a,b,c);}
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
