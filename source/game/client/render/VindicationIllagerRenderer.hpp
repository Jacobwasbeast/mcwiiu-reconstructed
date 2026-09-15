#ifndef GAME_CLIENT_RENDER_VINDICATIONILLAGERRENDERER_HPP
#define GAME_CLIENT_RENDER_VINDICATIONILLAGERRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"
#include "game/world/generation/ItemInHandLayer.hpp"

class Entity;
class EntityRenderDispatcher;
class IllagerModel;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class VindicationIllagerRenderer : public MobRenderer
{
public:
    class VindicationIllagerRenderer_AnonymousItemInHandLayer : public ItemInHandLayer
    {
    public:
        explicit VindicationIllagerRenderer_AnonymousItemInHandLayer(VindicationIllagerRenderer* renderer);
        virtual ~VindicationIllagerRenderer_AnonymousItemInHandLayer();

        virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                            float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                            bool bUseDisplayList);
        virtual void translateToHand(const HumanoidArm* arm);
    };

    VindicationIllagerRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~VindicationIllagerRenderer();

    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                        float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
