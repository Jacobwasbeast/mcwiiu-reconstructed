#ifndef GAME_CLIENT_RENDER_ILLUSIONILLAGERRENDERER_HPP
#define GAME_CLIENT_RENDER_ILLUSIONILLAGERRENDERER_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"
#include "game/world/generation/ItemInHandLayer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;
class Vec3;

class IllusionIllagerRenderer : public MobRenderer
{
public:
    IllusionIllagerRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~IllusionIllagerRenderer();

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTick);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual void renderName(boost::shared_ptr<Entity>& entity, double x, double y, double z);
    virtual bool isVisible(boost::shared_ptr<LivingEntity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);

    class IllusionIllagerRenderer_ItemInHandLayer : public ItemInHandLayer
    {
    public:
        IllusionIllagerRenderer_ItemInHandLayer(IllusionIllagerRenderer* renderer);
        virtual ~IllusionIllagerRenderer_ItemInHandLayer();
        virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                            float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                            bool bUseDisplayList);
        virtual void translateToHand(const HumanoidArm* arm);
    };
};

#endif
