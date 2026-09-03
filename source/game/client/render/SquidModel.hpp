#ifndef GAME_CLIENT_RENDER_SQUIDMODEL_HPP
#define GAME_CLIENT_RENDER_SQUIDMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelDefinition;
class ModelPart;

class SquidModel : public Model
{
public:
    SquidModel(const ModelDefinition& modelDefinition);

    virtual ~SquidModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName();

    ModelPart* mBody;
    ModelPart* mTentacles[8];
};

#endif
