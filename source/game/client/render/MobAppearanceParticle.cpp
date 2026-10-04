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
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    xd = 0.0;
    yd = 0.0;
    zd = 0.0;
    rCol = 1.0f;
    gCol = 1.0f;
    bCol = 1.0f;
    lifetime = 30;
    gravity = 0.0f;
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
        guardian = newGuardian;
    }
}

bool MobAppearanceParticle::isAffected()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    int hasCamera = guardian != NULL && minecraft != NULL && minecraft->getCameraEntity();
    MobEffect* slowdown = MobEffects::DIG_SLOWDOWN;
    if (hasCamera)
    {
        if ((minecraft->getCameraEntity()->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
        {
            boost::shared_ptr<LivingEntity> living =
                boost::static_pointer_cast<LivingEntity>(minecraft->getCameraEntity());
            if (living && living->hasEffect(slowdown))
                return true;
        }
    }
    return false;
}

void MobAppearanceParticle::render(BufferBuilder*,
                                   const boost::shared_ptr<Entity>& camera,
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
    dispatcher->setPosition(Particle::xOff, Particle::yOff, Particle::zOff);

    float scale = 1.0f / 2.35f;
    float progress = ((float)age + partialTicks) / (float)lifetime;

    GlStateManager::depthMask(true);
    GlStateManager::enableBlend();
    GlStateManager::enableDepthTest();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::multiTexCoord2f(240.0f, 240.0f);
    GlStateManager::pushMatrix();
    float pulse = sinf(progress * 3.1415927f);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 0.5f * pulse + 0.05f);
    GlStateManager::translatef(0.0f, 1.8f, 0.0f);
    GlStateManager::rotatef(180.0f - camera->mYRot, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(-(150.0f * progress - 60.0f) - camera->mXRot, 1.0f, 0.0f, 0.0f);
    GlStateManager::translatef(0.0f, -0.4f, -1.5f);
    GlStateManager::scalef(scale, scale, scale);
    guardian->mYRot = guardian->mYRotO = 0.0f;
    guardian->mYHeadRot = guardian->mYHeadRotO = 0.0f;
    boost::shared_ptr<Entity> renderEntity = guardian;
    dispatcher->render(renderEntity, 0.0, 0.0, 0.0, 0.0f, partialTicks,
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
