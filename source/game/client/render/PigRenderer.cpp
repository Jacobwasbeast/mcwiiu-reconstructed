#include "game/client/render/PigRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/PigModel.hpp"
#include "game/core/Pig.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/PigSaddleLayer.hpp"

static ResourceLocation PIG_LOCATION;

PigRenderer::PigRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new PigModel(ModelDefinition()), 0.7F)
{
    mSaddleLayer = new PigSaddleLayer(this);
    addLayer(mSaddleLayer);
}

PigRenderer::~PigRenderer()
{
}

const ResourceLocation* PigRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    if (mModel->hasOverriddenParts())
    {
        boost::shared_ptr<Pig> pig = boost::static_pointer_cast<Pig>(entity);
        if (pig->hasSaddle())
            return mSaddleLayer->getSaddleTextureLocation();
    }
    return &PIG_LOCATION;
}

void PigRenderer::setupForTexturePack(TexturePack* pack)
{
    LivingEntityRenderer::setupForTexturePack(pack);
    if (mModel->hasOverriddenParts())
    {
        removeLayer(mSaddleLayer);
        return;
    }

    std::vector<RenderLayer*>::iterator layer = mLayers.begin();
    while (layer != mLayers.end())
    {
        if (*layer == mSaddleLayer)
            break;
        ++layer;
    }
    if (layer == mLayers.end())
        addLayer(mSaddleLayer);
}

Model* PigRenderer::createModel(const ModelDefinition& definition)
{
    return new PigModel(definition);
}
