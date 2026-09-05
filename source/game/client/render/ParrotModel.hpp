#ifndef GAME_CLIENT_RENDER_PARROTMODEL_HPP
#define GAME_CLIENT_RENDER_PARROTMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class LivingEntity;
class ModelDefinition;
class ModelPart;

class ParrotModel : public Model
{
public:
    ParrotModel(const ModelDefinition& modelDefinition);
    virtual ~ParrotModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                        float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks,
                           float netHeadYaw, float headPitch, float scale,
                           boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                                 float limbSwingAmount, float partialTicks);
    virtual std::wstring getName();

    ModelPart* mBody;
    ModelPart* mTail;
    ModelPart* mWing0;
    ModelPart* mWing1;
    ModelPart* mHead;
    ModelPart* mHead2;
    ModelPart* mBeak1;
    ModelPart* mBeak2;
    ModelPart* mFeather;
    ModelPart* mLeg0;
    ModelPart* mLeg1;
    int mAnimationState;
    float mBodyX;
    float mBodyY;
    float mBodyZ;
    float mTailX;
    float mTailY;
    float mTailZ;
    float mWing0X;
    float mWing0Y;
    float mWing0Z;
    float mWing1X;
    float mWing1Y;
    float mWing1Z;
    float mHeadX;
    float mHeadY;
    float mHeadZ;
    float mHead2X;
    float mHead2Y;
    float mHead2Z;
    float mLeg0X;
    float mLeg0Y;
    float mLeg1X;
    float mLeg1Y;
    float mLeg1Z;
};

#endif
