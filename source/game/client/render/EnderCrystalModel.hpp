#ifndef GAME_CLIENT_RENDER_ENDERCRYSTALMODEL_HPP
#define GAME_CLIENT_RENDER_ENDERCRYSTALMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"

class Entity;
class ModelDefinition;
class ModelPartSkin;

class EnderCrystalModel : public Model
{
public:
    EnderCrystalModel(float scale, bool renderBase, const ModelDefinition& modelDefinition);
    virtual ~EnderCrystalModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                        float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool useDisplayList);

    ModelPartSkin* mGlass;
    ModelPartSkin* mCube;
    ModelPartSkin* mBase;
};

#endif
