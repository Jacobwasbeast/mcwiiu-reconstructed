#include "game/client/render/ChickenRenderer.hpp"

#include "game/client/render/ChickenModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/Chicken.hpp"
#include "game/core/Mth.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation CHICKEN_LOCATION((_TEXTURE_NAME)21);

ChickenRenderer::ChickenRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius)
    : MobRenderer(dispatcher, model, shadowRadius)
{
}

ChickenRenderer::~ChickenRenderer()
{
}

const ResourceLocation* ChickenRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &CHICKEN_LOCATION;
}

double ChickenRenderer::getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    boost::shared_ptr<Chicken> chicken = boost::static_pointer_cast<Chicken>(entity);
    float flap = (chicken->mFlapTime - chicken->mOFlap) * partialTicks + chicken->mOFlap;
    float flapSpeed = (chicken->mFlapSpeed - chicken->mOFlapSpeed) * partialTicks + chicken->mOFlapSpeed;
    return (Mth::sin(flap) + 1.0F) * flapSpeed;
}

Model* ChickenRenderer::createModel(const ModelDefinition& definition)
{
    return new ChickenModel(definition);
}
