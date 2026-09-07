#ifndef GAME_CLIENT_RENDER_BLAZEMODEL_HPP
#define GAME_CLIENT_RENDER_BLAZEMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelDefinition;

class BlazeModel : public Model
{
public:
    BlazeModel(const ModelDefinition& modelDefinition);

    virtual ~BlazeModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName();

    ModelPartSkin** mUpperBodyParts;
    unsigned int mNumUpperBodyParts;
    ModelPartSkin* mHead;
};

#endif
