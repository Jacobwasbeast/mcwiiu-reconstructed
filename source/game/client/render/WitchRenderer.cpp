#include "game/client/render/WitchRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/WitchModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/WitchItemLayer.hpp"

class Mob : public LivingEntity
{
public:
    virtual not_null_ptr<ItemInstance> getMainHandItem();
    virtual void setAggressive(bool aggressive);
};

static ResourceLocation WITCH_LOCATION;

WitchRenderer::WitchRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new WitchModel(0.0f, ModelDefinition()), 0.5f)
{
    addLayer(new WitchItemLayer(this));
}

WitchRenderer::~WitchRenderer()
{
}

void WitchRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                           float yaw, float partialTick)
{
    boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
    not_null_ptr<ItemInstance> item = mob->getMainHandItem();
    mob->setAggressive(!item->isEmpty());
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* WitchRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &WITCH_LOCATION;
}

void WitchRenderer::translateWeaponItem()
{
    GlStateManager::translatef(0.0f, 0.1875f, 0.0f);
}

void WitchRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::scalef(0.9375f, 0.9375f, 0.9375f);
}

Model* WitchRenderer::createModel(const ModelDefinition& definition)
{
    return new WitchModel(0.0f, definition);
}
