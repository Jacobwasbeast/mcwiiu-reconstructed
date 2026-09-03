#ifndef GAME_CLIENT_RENDER_BEDMODEL_HPP
#define GAME_CLIENT_RENDER_BEDMODEL_HPP

#include <string>

#include "game/client/render/Model.hpp"

class ModelDefinition;
class ModelPartSkin;

class BedModel : public Model
{
public:
    BedModel(const ModelDefinition& modelDefinition);

    virtual ~BedModel();

    virtual std::wstring getName();

    int modelVersion();

    void render(bool useDisplayList);

    ModelPartSkin* mBed;
    ModelPartSkin* mLeg0;
    ModelPartSkin* mLeg1;
    ModelPartSkin* mLeg2;
    ModelPartSkin* mLeg3;
};

#endif
