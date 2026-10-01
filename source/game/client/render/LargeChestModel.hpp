#ifndef GAME_CLIENT_RENDER_LARGECHESTMODEL_HPP
#define GAME_CLIENT_RENDER_LARGECHESTMODEL_HPP

#include "game/client/render/ChestModel.hpp"

class ModelDefinition;

class LargeChestModel : public ChestModel
{
public:
    LargeChestModel(const ModelDefinition& modelDefinition);
};

#endif
