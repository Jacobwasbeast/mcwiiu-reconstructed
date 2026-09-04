#ifndef GAME_CLIENT_RENDER_COWMODEL_HPP
#define GAME_CLIENT_RENDER_COWMODEL_HPP

#include <string>

#include "game/client/render/QuadrupedModel.hpp"

class ModelDefinition;

class CowModel : public QuadrupedModel
{
public:
    CowModel(const ModelDefinition& modelDefinition);
    virtual ~CowModel();

    virtual std::wstring getName();
};

#endif
