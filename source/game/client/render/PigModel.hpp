#ifndef GAME_CLIENT_RENDER_PIGMODEL_HPP
#define GAME_CLIENT_RENDER_PIGMODEL_HPP

#include <string>

#include "game/client/render/QuadrupedModel.hpp"

class ModelDefinition;

class PigModel : public QuadrupedModel
{
public:
    PigModel(const ModelDefinition& modelDefinition);
    PigModel(float scale, const ModelDefinition& modelDefinition);

    virtual ~PigModel();

    virtual std::wstring getName();
};

#endif
