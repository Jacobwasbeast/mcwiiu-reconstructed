#ifndef GAME_CLIENT_RENDER_SPIDERMODEL_HPP
#define GAME_CLIENT_RENDER_SPIDERMODEL_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelPartsSkinned.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelDefinition;
class ModelPartSkin;

class SpiderModel : public Model
{
public:
    SpiderModel(const ModelDefinition& modelDefinition);
    virtual ~SpiderModel();

    void createModelParts(const ModelDefinition& modelDefinition);
    void createInvisibleModelParts(const ModelDefinition& modelDefinition);
    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                        float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks,
                           float netHeadYaw, float headPitch, float scale,
                           boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"spider"; }

private:
    ModelPartSkin** mSpiderParts;
    int mSpiderPartCount;
    ModelPartsSkinned mInvisibleParts;
};

#endif
