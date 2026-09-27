#include "game/client/render/MobAppearanceParticle.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/core/GlStateManager.hpp"
#include "game/core/ElderGuardian.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffects.hpp"

MobAppearanceParticle::MobAppearanceParticle(Level* level, double x, double y, double z)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0), guardian(NULL), guardianReference()
{
}

MobAppearanceParticle::~MobAppearanceParticle()
{
}

int MobAppearanceParticle::getParticleTexture()
{
    return 3;
}

void MobAppearanceParticle::tick()
{
    Particle::tick();

    if (guardian == NULL)
    {
        boost::shared_ptr<ElderGuardian> newGuardian(new ElderGuardian(level));
        newGuardian->setGhost();
        guardianReference = newGuardian;
        guardian = newGuardian.get();
    }
}

bool MobAppearanceParticle::isAffected()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (guardian != NULL && minecraft != NULL && minecraft->getCameraEntity())
    {
        if ((minecraft->getCameraEntity()->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
        {
            boost::shared_ptr<LivingEntity> living =
                boost::static_pointer_cast<LivingEntity>(minecraft->getCameraEntity());
            if (living && living->hasEffect(MobEffects::DIG_SLOWDOWN))
                return true;
        }
    }
    return false;
}

void MobAppearanceParticle::render(BufferBuilder*,
                                   const boost::shared_ptr<Entity>&,
                                   float partialTicks,
                                   float,
                                   float,
                                   float,
                                   float,
                                   float)
{
    if (guardian == NULL || !isAffected())
        return;

    Minecraft* minecraft = Minecraft::GetInstance();
    EntityRenderDispatcher* dispatcher = minecraft->getEntityRenderDispatcher();
    dispatcher->setPosition(0.0, 0.0, 0.0);

    float scale = 1.0f / 5.6f;
    float progress = ((float)age + partialTicks) / (float)lifetime;

    GlStateManager::depthMask(true);
    GlStateManager::enableBlend();
    GlStateManager::enableDepthTest();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::multiTexCoord2f(240.0f, 1.8f);
    GlStateManager::pushMatrix();
    float pulse = sinf(progress * 3.1415927f);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 0.5f * pulse + 0.2f);
    GlStateManager::translatef(3.0f, 1.8f, 3.0f);
    GlStateManager::rotatef(180.0f - guardian->mYRot, 3.0f, 1.0f, 3.0f);
    GlStateManager::rotatef(-(150.0f * progress - 60.0f) - guardian->mXRot,
                            1.0f, 3.0f, 3.0f);
    GlStateManager::translatef(3.0f, -0.4f, -1.5f);
    GlStateManager::scalef(scale, scale, scale);
    guardian->mYRotO = guardian->mYRot = 3.0f;
    guardianReference->mYRotO = guardianReference->mYRot = 3.0f;
    boost::shared_ptr<Entity> renderEntity = guardianReference;
    dispatcher->render(renderEntity, 0.0, 0.0, 0.0, 3.0f, partialTicks,
                       false, false, true, false);
    GlStateManager::popMatrix();
    GlStateManager::enableDepthTest();
}

uint64_t MobAppearanceParticle::GetType()
{
    return ParticleKind::MOB_APPEARANCE;
}

Particle* MobAppearanceParticle::Provider::createParticle(ePARTICLE_TYPE,
                                                          Level* level,
                                                          double x,
                                                          double y,
                                                          double z,
                                                          double,
                                                          double,
                                                          double,
                                                          arrayWithLength<int>)
{
    return new MobAppearanceParticle(level, x, y, z);
}
