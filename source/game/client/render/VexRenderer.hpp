#ifndef GAME_CLIENT_RENDER_VEXRENDERER_HPP
#define GAME_CLIENT_RENDER_VEXRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/VexModel.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class VexRenderer : public VexModel
{
public:
    VexRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~VexRenderer();

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                        float yaw, float partialTick);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTick);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
