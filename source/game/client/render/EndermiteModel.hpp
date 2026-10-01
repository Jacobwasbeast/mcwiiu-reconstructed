#ifndef GAME_CLIENT_RENDER_ENDERMITEMODEL_HPP
#define GAME_CLIENT_RENDER_ENDERMITEMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelDefinition;
class ModelPart;

class EndermiteModel : public Model
{
public:
    EndermiteModel(const ModelDefinition& modelDefinition);

    virtual ~EndermiteModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"endermite"; }

    ModelPart* mBodyParts[4];
};

#endif
