#ifndef GAME_CLIENT_RENDER_WITHERBOSSMODEL_HPP
#define GAME_CLIENT_RENDER_WITHERBOSSMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class LivingEntity;
class ModelPartSkin;

class WitherBossModel : public Model
{
public:
    WitherBossModel(float scale, const ModelDefinition& modelDefinition = ModelDefinition());


    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                           float headPitch, float scale, boost::shared_ptr<Entity> entity,
                           _SkinAdjustments skinAdjustments);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTicks);
    virtual std::wstring getName() { return L"witherBoss"; }

    ModelPartSkin** mUpperBodyParts;
    unsigned int mNumUpperBodyParts;
    ModelPartSkin** mHeads;
    unsigned int mNumHeads;
};

#endif
