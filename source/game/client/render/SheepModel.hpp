#ifndef GAME_CLIENT_RENDER_SHEEPMODEL_HPP
#define GAME_CLIENT_RENDER_SHEEPMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/QuadrupedModel.hpp"

class Entity;
class LivingEntity;
class ModelDefinition;

class SheepModel : public QuadrupedModel
{
public:
    SheepModel(const ModelDefinition& modelDefinition);
    virtual ~SheepModel();

    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks,
                           float netHeadYaw, float headPitch, float scale,
                           boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                                 float limbSwingAmount, float partialTicks);
    virtual std::wstring getName();

    float mHeadEatAngleScale;
};

#endif
