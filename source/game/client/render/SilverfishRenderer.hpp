#ifndef GAME_CLIENT_RENDER_SILVERFISHRENDERER_HPP
#define GAME_CLIENT_RENDER_SILVERFISHRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/SilverfishModel.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class SilverfishRenderer : public SilverfishModel
{
public:
    SilverfishRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~SilverfishRenderer();

    virtual double getFlipDegrees(boost::shared_ptr<LivingEntity>& entity);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
