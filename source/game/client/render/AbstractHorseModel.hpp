#ifndef GAME_CLIENT_RENDER_ABSTRACTHORSEMODEL_HPP
#define GAME_CLIENT_RENDER_ABSTRACTHORSEMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"

class Entity;
class LivingEntity;
class ModelDefinition;
class ModelPart;

class AbstractHorseModel : public Model
{
public:
    AbstractHorseModel(bool horse, const ModelDefinition& modelDefinition);

    virtual ~AbstractHorseModel() {}

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount, float partialTicks);
    virtual std::wstring getName() { return L"horse.v2"; }

    float rotlerp(float from, float to, float delta);

    ModelPart* mBody;
    ModelPart* mHead;
    ModelPart* mUMouth;
    ModelPart* mNeck;
    ModelPart* mMane;
    ModelPart* mEar1;
    ModelPart* mEar2;
    ModelPart* mMuleEarL;
    ModelPart* mMuleEarR;
    ModelPart* mBag1;
    ModelPart* mBag2;
    ModelPart* mSaddle;
    ModelPart* mSaddleMouthL;
    ModelPart* mSaddleMouthR;
    ModelPart* mSaddleMouthLine;
    ModelPart* mSaddleMouthLineR;
    ModelPart* mHeadSaddle;
    ModelPart* mLeg1A;
    ModelPart* mLeg2A;
    ModelPart* mLeg3A;
    ModelPart* mLeg4A;
    ModelPart* mTailA;
    float mLeg3ADefaultY;
    float mLeg3ADefaultZ;
    float mBag2DefaultY;
    float mBag2DefaultZ;
};

#endif
