#ifndef GAME_CLIENT_RENDER_SQUIDMODEL_HPP
#define GAME_CLIENT_RENDER_SQUIDMODEL_HPP

#include "game/client/render/Model.hpp"

class ModelDefinition;
class ModelPartSkin;

class SquidModel : public Model
{
public:
    explicit SquidModel(const ModelDefinition& definition);
    virtual ~SquidModel();
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                           float headPitch, float scale, boost::shared_ptr<Entity> entity,
                           _SkinAdjustments skinAdjustments);
    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                        float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool useDisplayList);
    virtual std::wstring getName();

    ModelPartSkin* mBody;
    ModelPartSkin* mTentacle1;
    ModelPartSkin* mTentacle2;
    ModelPartSkin* mTentacle3;
    ModelPartSkin* mTentacle4;
    ModelPartSkin* mTentacle5;
    ModelPartSkin* mTentacle6;
    ModelPartSkin* mTentacle7;
    ModelPartSkin* mTentacle8;
};

#endif
