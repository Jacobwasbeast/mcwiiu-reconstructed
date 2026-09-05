#ifndef GAME_CLIENT_RENDER_VEXMODEL_HPP
#define GAME_CLIENT_RENDER_VEXMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidModel.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelDefinition;
class ModelPart;

class VexModel : public HumanoidModel
{
public:
    VexModel(const ModelDefinition& modelDefinition);
    VexModel(float scale, const ModelDefinition& modelDefinition);

    virtual ~VexModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName();

    void _init(float scale, const ModelDefinition& modelDefinition);
    unsigned int getModelVersion();

    ModelPart* mRightWing;
    ModelPart* mLeftWing;
};

#endif
