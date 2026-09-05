#include "game/client/render/SlimeRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SlimeModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/SlimeOuterLayer.hpp"

class Slime : public LivingEntity
{
public:
    virtual int getSize();

    float mSquish;
    float mOldSquish;
};

static ResourceLocation SLIME_LOCATION;

SlimeRenderer::SlimeRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new SlimeModel(16, ModelDefinition()), 0.25f)
{
    addLayer(new SlimeOuterLayer(this));
}

SlimeRenderer::~SlimeRenderer()
{
}

const ResourceLocation* SlimeRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &SLIME_LOCATION;
}

void SlimeRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                           float yaw, float partialTick)
{
    boost::shared_ptr<Slime> slime = boost::static_pointer_cast<Slime>(entity);
    mShadowRadius = 0.25f * static_cast<float>(slime->getSize());
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

void SlimeRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::scalef(0.999f, 0.999f, 0.999f);

    boost::shared_ptr<Slime> slime = boost::static_pointer_cast<Slime>(entity);
    float size = static_cast<float>(slime->getSize());
    float squish = slime->mOldSquish + (slime->mSquish - slime->mOldSquish) * partialTicks;
    float scale = 1.0f / (squish / (size * 0.5f + 1.0f) + 1.0f);
    float scaledSize = scale * size;

    GlStateManager::scalef(scaledSize, (1.0f / scale) * size, scaledSize);
}

Model* SlimeRenderer::createModel(const ModelDefinition& definition)
{
    return new SlimeModel(16, definition);
}
