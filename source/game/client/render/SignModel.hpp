#ifndef GAME_CLIENT_RENDER_SIGNMODEL_HPP
#define GAME_CLIENT_RENDER_SIGNMODEL_HPP

#include "game/client/render/Model.hpp"

class ModelDefinition;
class ModelPartSkin;

class SignModel : public Model
{
public:
    SignModel(const ModelDefinition& modelDefinition);

    virtual ~SignModel();

    void render(bool useDisplayList);

    ModelPartSkin* mSignBoard;
    ModelPartSkin* mSignStick;
};

#endif
