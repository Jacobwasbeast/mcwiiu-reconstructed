#include "game/client/render/EndermanRenderer.hpp"

#include "game/client/render/EndermanModel.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/RenderLayer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/EnderEyesLayer.hpp"

class CarriedBlockLayer : public RenderLayer
{
public:
    CarriedBlockLayer(EndermanRenderer* renderer);
    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                        float limbSwingAmount, float partialTicks, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();
    EndermanRenderer* mEndermanRenderer;
};

class BlockState;
class EnderMan : public Entity
{
public:
    virtual BlockState* getCarryingBlock();
    virtual bool isCreepy();
};

static ResourceLocation ENDERMAN_LOCATION;

EndermanRenderer::EndermanRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new EndermanModel(0.5F, ModelDefinition()), 1.0F)
{
    mEyesLayer = new EnderEyesLayer(this);
    addLayer(mEyesLayer);
    addLayer(new CarriedBlockLayer(this));
}

EndermanRenderer::~EndermanRenderer()
{
}

void EndermanRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                              float yaw, float partialTick)
{
    boost::shared_ptr<EnderMan> enderman = boost::static_pointer_cast<EnderMan>(entity);
    EndermanModel* model = static_cast<EndermanModel*>(getModel());
    model->mCarrying = enderman->getCarryingBlock() != NULL;
    model->mCreepy = enderman->isCreepy();

    if (model->mCreepy)
    {
        const double offset = 0.02;
        x += mRandom.nextGaussian() * offset;
        z += mRandom.nextGaussian() * offset;
    }
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* EndermanRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &ENDERMAN_LOCATION;
}

void EndermanRenderer::setupForTexturePack(TexturePack* pack)
{
    LivingEntityRenderer::setupForTexturePack(pack);
    if (mModel->hasOverriddenParts())
    {
        removeLayer(mEyesLayer);
        return;
    }

    for (std::vector<RenderLayer*>::iterator it = mLayers.begin(); it != mLayers.end(); ++it)
    {
        if (*it == mEyesLayer)
            return;
    }
    addLayer(mEyesLayer);
}

void EndermanRenderer::renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                                   float limbSwingAmount, float ageInTicks, float netHeadYaw,
                                   float headPitch, float scale)
{
    bool translucent = false;
    if (!isVisible(entity))
    {
        boost::shared_ptr<Player> player = Minecraft::GetInstance()->mPlayer;
        if (entity->isInvisibleTo(player))
            return;
        translucent = true;
        GlStateManager::setProfile();
    }

    Model* model = getModel(entity);
    boost::shared_ptr<Entity> renderEntity = boost::static_pointer_cast<Entity>(entity);
    model->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, true);

    if (translucent)
        GlStateManager::unsetProfile();
}

Model* EndermanRenderer::createModel(const ModelDefinition& definition)
{
    return new EndermanModel(0.5F, definition);
}
