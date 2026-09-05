#include "game/client/render/EvocationIllagerRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/IllagerModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/util/ResourceLocation.hpp"

class SpellcasterIllager
{
public:
    virtual bool isCastingSpell();
};

static ResourceLocation evokerLocation;

EvocationIllagerRenderer::EvocationIllagerRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new IllagerModel(0.0f, 0.0f, 64, 64, ModelDefinition()), 0.5f)
{
    addLayer(new EvocationIllagerRenderer_ItemInHandLayer(this));
}

EvocationIllagerRenderer::~EvocationIllagerRenderer()
{
}

const ResourceLocation* EvocationIllagerRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &evokerLocation;
}

void EvocationIllagerRenderer::scale(boost::shared_ptr<LivingEntity>&, float)
{
    GlStateManager::scalef(0.9375f, 0.9375f, 0.9375f);
}

std::wstring EvocationIllagerRenderer::getModelName()
{
    return L"evoker";
}

Model* EvocationIllagerRenderer::createModel(const ModelDefinition& definition)
{
    return new IllagerModel(0.0f, 0.0f, 64, 64, definition);
}

EvocationIllagerRenderer::EvocationIllagerRenderer_ItemInHandLayer::EvocationIllagerRenderer_ItemInHandLayer(
    EvocationIllagerRenderer* renderer)
    : ItemInHandLayer(renderer)
{
}

EvocationIllagerRenderer::EvocationIllagerRenderer_ItemInHandLayer::~EvocationIllagerRenderer_ItemInHandLayer()
{
}

void EvocationIllagerRenderer::EvocationIllagerRenderer_ItemInHandLayer::render(
    boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount, float partialTicks,
    float ageInTicks, float netHeadYaw, float headPitch, float scale, bool bUseDisplayList)
{
    SpellcasterIllager* illager = (SpellcasterIllager*)entity.get();
    if (illager->isCastingSpell())
    {
        ItemInHandLayer::render(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks, netHeadYaw,
                                headPitch, scale, bUseDisplayList);
    }
}

void EvocationIllagerRenderer::EvocationIllagerRenderer_ItemInHandLayer::translateToHand(const HumanoidArm* arm)
{
    IllagerModel* model = static_cast<IllagerModel*>(mLivingEntityRenderer->getModel());
    model->getArm(arm)->translateTo(0.0625f);
}
