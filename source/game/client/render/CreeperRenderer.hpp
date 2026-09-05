#ifndef GAME_CLIENT_RENDER_CREEPERRENDERER_HPP
#define GAME_CLIENT_RENDER_CREEPERRENDER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/CreeperModel.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class CreeperRenderer : public CreeperModel
{
public:
    CreeperRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~CreeperRenderer();

    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    virtual unsigned int getOverlayColor(boost::shared_ptr<LivingEntity>& entity, float u, float v);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
