#ifndef GAME_CLIENT_RENDER_GHASTMODEL_HPP
#define GAME_CLIENT_RENDER_GHASTMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelDefinition;
class ModelPart;

class GhastModel : public Model
{
public:
    GhastModel(const ModelDefinition& modelDefinition);

    virtual ~GhastModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName();

    ModelPart* mBody;
    ModelPart* mTentacles[9];
};

#endif
