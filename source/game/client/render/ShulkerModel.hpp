#ifndef GAME_CLIENT_RENDER_SHULKERMODEL_HPP
#define GAME_CLIENT_RENDER_SHULKERMODEL_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"

class Entity;
class ModelDefinition;
class ModelPart;
class ModelPartsSkinned;

class ShulkerModel : public Model
{
public:
    ShulkerModel(const ModelDefinition& modelDefinition);
    virtual ~ShulkerModel();
    void renderWhenInvisible(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                             float ageInTicks, float netHeadYaw, float headPitch, float scale,
                             bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                           float headPitch, float scale, boost::shared_ptr<Entity> entity,
                           _SkinAdjustments skinAdjustments);
    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                        float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual std::wstring getName();

    ModelPart* mBase;
    ModelPart* mLid;
    ModelPart* mHead;
    ModelPart* mHeadSkin;
    ModelPartsSkinned mSkinnedParts;
};

#endif
