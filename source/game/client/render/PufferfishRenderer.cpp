#include "game/client/render/PufferfishRenderer.hpp"

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/FishModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/Pufferfish.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"

PufferfishRenderer::PufferfishRenderer(EntityRenderDispatcher* dispatcher)
    : FishRenderer(dispatcher, FISH_TYPE_PUFFERFISH, 0.2f),
      mPuffState(Pufferfish::PUFFED_STATE_SMALL),
      mModelMedium(new FishModel(ModelDefinition(), FISH_TYPE_PUFFERFISH, 1)),
      mModelLarge(new FishModel(ModelDefinition(), FISH_TYPE_PUFFERFISH, 2)),
      mOriginalModelMedium(mModelMedium),
      mOriginalModelLarge(mModelLarge)
{
}

PufferfishRenderer::~PufferfishRenderer()
{
}

void PufferfishRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                float yaw, float partialTick)
{
    boost::shared_ptr<Pufferfish> pufferfish = boost::static_pointer_cast<Pufferfish>(entity);
    mPuffState = pufferfish->mPuffState;
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

Model* PufferfishRenderer::getModel(boost::shared_ptr<LivingEntity>&)
{
    if (mPuffState == Pufferfish::PUFFED_STATE_MEDIUM)
        return mModelMedium;
    if (mPuffState != Pufferfish::PUFFED_STATE_LARGE)
        return mModel;
    return mModelLarge;
}

void PufferfishRenderer::setupForTexturePack(TexturePack* pack)
{
    LivingEntityRenderer::setupForTexturePack(pack);

    std::wstring modelName = mModelMedium->getName();
    if (mModelMedium != mOriginalModelMedium)
    {
        if (mModelMedium != NULL)
            delete mModelMedium;
        mModelMedium = NULL;
    }
    if (!pack->HasCustomModel(modelName))
    {
        mModelMedium = mOriginalModelMedium;
    }
    else
    {
        mModelMedium = new FishModel(*pack->GetCustomModel(modelName), FISH_TYPE_PUFFERFISH, 1);
    }

    modelName = mModelLarge->getName();
    if (mModelLarge != mOriginalModelLarge)
    {
        if (mModelLarge != NULL)
            delete mModelLarge;
        mModelLarge = NULL;
    }
    if (!pack->HasCustomModel(modelName))
    {
        mModelLarge = mOriginalModelLarge;
    }
    else
    {
        mModelLarge = new FishModel(*pack->GetCustomModel(modelName), FISH_TYPE_PUFFERFISH, 2);
    }
}

Model* PufferfishRenderer::createModel(const ModelDefinition& definition)
{
    return new FishModel(definition, FISH_TYPE_PUFFERFISH, 0);
}
