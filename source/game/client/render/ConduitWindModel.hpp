#ifndef GAME_CLIENT_RENDER_CONDUITWINDMODEL_HPP
#define GAME_CLIENT_RENDER_CONDUITWINDMODEL_HPP

#include "game/client/render/Model.hpp"

class ModelPartSkin;

class ConduitWindModel : public Model
{
public:
    ConduitWindModel(int xTexOffs, int yTexOffs, int texWidth, int texHeight);

    virtual ~ConduitWindModel();

    void renderWind(bool useDisplayList);

    ModelPartSkin* mWind;
};

#endif
