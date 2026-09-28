#ifndef GAME_CLIENT_RENDER_CHESTMODEL_HPP
#define GAME_CLIENT_RENDER_CHESTMODEL_HPP

#include "game/client/render/Model.hpp"

class ModelDefinition;

class ChestModel : public Model
{
public:
    ChestModel(const ModelDefinition& modelDefinition);

    virtual ~ChestModel() {}

    void render(bool useDisplayList);

    ModelPart* mLid;
    ModelPart* mBottom;
    ModelPart* mLock;
};

#endif
