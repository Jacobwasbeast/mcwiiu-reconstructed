#include "game/client/render/ParrotRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ParrotModel.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Parrot.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation PARROT_LOCATIONS[5];

ParrotRenderer::ParrotRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new ParrotModel(ModelDefinition()), 0.3F)
{
}

ParrotRenderer::~ParrotRenderer()
{
}

const ResourceLocation* ParrotRenderer::getSpectateTextureLocation(boost::shared_ptr<MultiplayerLocalPlayer> player)
{
    return &PARROT_LOCATIONS[player->getId() % 5];
}

const ResourceLocation* ParrotRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    if (Minecraft::InMiniGame(eMiniGameId_Any, true)
        && (entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer =
            boost::static_pointer_cast<MultiplayerLocalPlayer>(entity);
        return getSpectateTextureLocation(localPlayer);
    }

    boost::shared_ptr<Parrot> parrot = boost::static_pointer_cast<Parrot>(entity);
    return &PARROT_LOCATIONS[parrot->getVariant()];
}

double ParrotRenderer::getCustomBob(boost::shared_ptr<Parrot> parrot, float partialTicks)
{
    float flap = (parrot->mFlapTime - parrot->mOFlapSpeed) * partialTicks + parrot->mOFlapSpeed;
    float flapSpeed = (parrot->mFlapSpeed - parrot->mOFlap) * partialTicks + parrot->mOFlap;
    return (Mth::sin(flap) + 1.0F) * flapSpeed;
}

double ParrotRenderer::getCustomBob(boost::shared_ptr<Player> player, float partialTicks)
{
    float flap = (player->mFlap - player->mOFlap) * partialTicks + player->mOFlap;
    float flapSpeed = (player->mFlapSpeed - player->mOFlapSpeed) * partialTicks + player->mOFlapSpeed;
    return (Mth::sin(flap) + 1.0F) * flapSpeed;
}

double ParrotRenderer::getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    if (entity->GetType() == 0x2D6C0003ULL)
    {
        return getCustomBob(boost::static_pointer_cast<Parrot>(entity), partialTicks);
    }

    if (Minecraft::InMiniGame(eMiniGameId_Any, true)
        && (entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        return getCustomBob(boost::static_pointer_cast<Player>(entity), partialTicks);
    }

    return 0.0;
}

Model* ParrotRenderer::createModel(const ModelDefinition& definition)
{
    return new ParrotModel(definition);
}
