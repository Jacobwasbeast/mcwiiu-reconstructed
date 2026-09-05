#include "game/client/render/ChickenRenderer.hpp"

#include "game/client/render/ChickenModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"

class Chicken : public LivingEntity
{
public:
    float mFlapTime;
    float mOFlap;
    float mFlapSpeed;
    float mOFlapSpeed;
};

static ResourceLocation CHICKEN_LOCATION;

ChickenRenderer::ChickenRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius)
    : MobRenderer(dispatcher, model, shadowRadius)
{
}

ChickenRenderer::~ChickenRenderer()
{
}

const ResourceLocation* ChickenRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &CHICKEN_LOCATION;
}

double ChickenRenderer::getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    boost::shared_ptr<Chicken> chicken = boost::static_pointer_cast<Chicken>(entity);
    float flap = (chicken->mFlapTime - chicken->mOFlapSpeed) * partialTicks + chicken->mOFlapSpeed;
    float flapSpeed = (chicken->mFlapSpeed - chicken->mOFlap) * partialTicks + chicken->mOFlap;
    return (Mth::sin(flap) + 1.0F) * flapSpeed;
}

Model* ChickenRenderer::createModel(const ModelDefinition& definition)
{
    return new ChickenModel(definition);
}
