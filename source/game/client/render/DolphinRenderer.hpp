#ifndef GAME_CLIENT_RENDER_DOLPHINRENDERER_HPP
#define GAME_CLIENT_RENDER_DOLPHINRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class DolphinRenderer : public MobRenderer
{
public:
    DolphinRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~DolphinRenderer();

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTick);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
