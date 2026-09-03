#ifndef GAME_CLIENT_RENDER_CHESTEDHORSEMODEL_HPP
#define GAME_CLIENT_RENDER_CHESTEDHORSEMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/AbstractHorseModel.hpp"

class Entity;
class ModelDefinition;

class ChestedHorseModel : public AbstractHorseModel
{
public:
    ChestedHorseModel(const ModelDefinition& modelDefinition);

    virtual ~ChestedHorseModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList);
};

#endif
