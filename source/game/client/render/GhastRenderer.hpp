#ifndef GAME_CLIENT_RENDER_GHASTRENDERER_HPP
#define GAME_CLIENT_RENDER_GHASTRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/GhastModel.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class GhastRenderer : public GhastModel
{
public:
    GhastRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~GhastRenderer();

    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
