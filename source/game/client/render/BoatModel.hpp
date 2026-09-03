#ifndef GAME_CLIENT_RENDER_BOATMODEL_HPP
#define GAME_CLIENT_RENDER_BOATMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/TwoPassModel.hpp"

class Boat;
class Entity;
class ModelDefinition;

class BoatModel : public Model, public TwoPassModel
{
public:
    BoatModel(const ModelDefinition& definition);

    virtual ~BoatModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float partialTicks, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual std::wstring getName();
    virtual void renderSecondPass(boost::shared_ptr<Entity> entity, float partialTicks, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList);

    ModelPartSkin* makePaddle(bool left);
    void animatePaddle(ModelPartSkin* paddle, boost::shared_ptr<Boat> boat, int side, float scale, float partialTicks, bool useDisplayList);

    ModelPartSkin* mBottom;
    ModelPartSkin* mBack;
    ModelPartSkin* mFront;
    ModelPartSkin* mRight;
    ModelPartSkin* mLeft;
    ModelPartSkin* mPaddleLeft;
    ModelPartSkin* mPaddleRight;
    ModelPart* mSecondPassBottom;
};

#endif
