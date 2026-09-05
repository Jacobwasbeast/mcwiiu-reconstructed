#ifndef GAME_CLIENT_RENDER_ENDERDRAGONRENDERER_HPP
#define GAME_CLIENT_RENDER_ENDERDRAGONRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class EnderDragonEyesLayer;
class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;
class TexturePack;

class EnderDragonRenderer : public MobRenderer
{
public:
    EnderDragonRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~EnderDragonRenderer();
    static void renderCrystalBeams(double, double, double, float, double, double, double, int, double, double, double);
    virtual void setupRotations(boost::shared_ptr<LivingEntity>&, float, float, float);
    virtual void renderModel(boost::shared_ptr<LivingEntity>&, float, float, float, float, float, float);
    virtual void render(boost::shared_ptr<Entity>&, double, double, double, float, float);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>&);
    virtual void setupForTexturePack(TexturePack*);
    virtual Model* createModel(const ModelDefinition&);

private:
    EnderDragonEyesLayer* mEyesLayer;
};

#endif
