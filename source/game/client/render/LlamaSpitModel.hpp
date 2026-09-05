#ifndef GAME_CLIENT_RENDER_LLAMASPITMODEL_HPP
#define GAME_CLIENT_RENDER_LLAMASPITMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"

class ModelDefinition;

class LlamaSpitModel : public Model
{
public:
    LlamaSpitModel(const ModelDefinition& modelDefinition);
    LlamaSpitModel(float delta, const ModelDefinition& modelDefinition);

    virtual ~LlamaSpitModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                        float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool useDisplayList);

    void _init(float delta, const ModelDefinition& modelDefinition);

    ModelPart* mSpit;
};

#endif
