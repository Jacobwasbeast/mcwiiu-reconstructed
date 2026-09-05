#ifndef GAME_CLIENT_RENDER_ARMORSTANDMODEL_HPP
#define GAME_CLIENT_RENDER_ARMORSTANDMODEL_HPP

#include "game/client/render/ArmorStandArmorModel.hpp"

class ModelPartSkin;
class ModelDefinition;
class HumanoidArm;

class ArmorStandModel : public ArmorStandArmorModel
{
public:
    ArmorStandModel(float scale, const ModelDefinition& definition);

    virtual ~ArmorStandModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                           float headPitch, float scale, boost::shared_ptr<Entity> entity,
                           _SkinAdjustments skinAdjustments);
    virtual void translateToHandItem(float scale, const HumanoidArm* arm);

    ModelPartSkin* mArmorStandBody;
    ModelPartSkin* mArmorStandRightArm;
    ModelPartSkin* mArmorStandLeftArm;
    ModelPartSkin* mArmorStandBase;
};

#endif
