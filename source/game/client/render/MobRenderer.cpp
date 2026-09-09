#include "game/client/render/MobRenderer.hpp"

#include <cmath>

#include "game/client/render/Model.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Culler.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"

MobRenderer::MobRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius)
    : LivingEntityRenderer(dispatcher, model, shadowRadius)
{
}

MobRenderer::~MobRenderer()
{
}

bool MobRenderer::shouldShowName(boost::shared_ptr<Entity>& entity)
{
    if (!LivingEntityRenderer::shouldShowName(entity))
        return false;
    boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
    if (!entity->isInvisible())
        return true;
    return mob->isCustomNameVisible();
}

bool MobRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler,
                               double x, double y, double z)
{
    if (EntityRenderer::shouldRender(entity, culler, x, y, z))
        return true;
    boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
    if (!mob || !mob->isLeashed())
        return false;
    boost::shared_ptr<Entity> leashHolder = mob->getLeashHolder();
    return leashHolder && culler->isVisible(leashHolder->getBoundingBoxForCulling());
}

void MobRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                         float yaw, float partialTick)
{
    LivingEntityRenderer::render(entity, x, y, z, yaw, partialTick);
    if (mItemFrame)
        return;
    if ((entity->GetType() & ENTITY_TYPE_MOB) != ENTITY_TYPE_MOB)
        return;
    boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
    setLightColor(entity);
    mModel->render(entity, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0625F, false);
}

void MobRenderer::setLightColor(boost::shared_ptr<Entity>& entity)
{
    int lightColor = entity->getLightColor();
    GlStateManager::multiTexCoord2f((float)(lightColor & 65535),
                                    (float)((unsigned int)lightColor >> 16));
}

double MobRenderer::lerp(double first, double second, double fraction)
{
    return (second - first) * fraction + first;
}

void MobRenderer::renderLeash(boost::shared_ptr<Mob>& mob, double x, double y, double z,
                              float yaw, float partialTick)
{
    boost::shared_ptr<Entity> holder = mob->getLeashHolder();
    if (!holder)
        return;

    const double radians = 0.01745329238474369;
    const double leashOffset = 0.7;
    const double leashWidth = 0.025;
    const double segmentCount = 24.0;
    const double sag = 0.25;

    double mobYaw = lerp((double)mob->mYRotO, (double)mob->mYRot, partialTick) * radians;
    double mobPitch = lerp((double)mob->mXRotO, (double)mob->mXRot, partialTick) * radians;
    double mobCosYaw = std::cos(mobYaw);
    double mobSinYaw = std::sin(mobYaw);
    double mobSinPitch = std::sin(mobPitch);
    double mobX = lerp(mob->mXo, mob->mX, partialTick) + mobCosYaw * leashOffset;
    double mobY = lerp(mob->mYo, mob->mY, partialTick) + mobSinPitch * leashOffset;
    double mobZ = lerp(mob->mZo, mob->mZ, partialTick) + mobSinYaw * leashOffset;
    double holderX = lerp(holder->mXo, holder->mX, partialTick);
    double holderY = lerp(holder->mYo, holder->mY, partialTick) + holder->getEyeHeight();
    double holderZ = lerp(holder->mZo, holder->mZ, partialTick);
    double dx = holderX - mobX;
    double dy = holderY - mobY;
    double dz = holderZ - mobZ;

    GlStateManager::disableTexture();
    GlStateManager::disableLighting();
    GlStateManager::disableCull();
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    unsigned int lightColour = Minecraft::GetInstance()->getColourTable()->getColour(
        eMinecraftColour_Leash_Light_Colour);
    unsigned int darkColour = Minecraft::GetInstance()->getColourTable()->getColour(
        eMinecraftColour_Leash_Dark_Colour);
    const float lightRed = (float)((lightColour >> 16) & 255) / 255.0F;
    const float lightGreen = (float)((lightColour >> 8) & 255) / 255.0F;
    const float lightBlue = (float)(lightColour & 255) / 255.0F;
    const float darkRed = (float)((darkColour >> 16) & 255) / 255.0F;
    const float darkGreen = (float)((darkColour >> 8) & 255) / 255.0F;
    const float darkBlue = (float)(darkColour & 255) / 255.0F;

    for (int pass = 0; pass < 2; ++pass)
    {
        builder->begin(1);
        for (int i = 0; i <= 24; ++i)
        {
            double fraction = (double)i / segmentCount;
            double px = x + dx * fraction;
            double pz = z + dz * fraction;
            double py = y + dy * fraction + sag * (fraction * fraction + fraction);
            bool light = (i & 1) != 0;
            if (pass != 0)
            {
                px += leashWidth;
                pz += leashWidth;
            }
            builder->color(light ? lightRed : darkRed, light ? lightGreen : darkGreen,
                           light ? lightBlue : darkBlue, 1.0F);
            if (pass == 0)
            {
                builder->vertex((float)px, (float)py, (float)pz);
                builder->vertex((float)(px + leashWidth), (float)(py + leashWidth),
                                (float)(pz + leashWidth));
            }
            else
            {
                builder->vertex((float)px, (float)(py + leashWidth), (float)pz);
                builder->vertex((float)(px + leashWidth), (float)py,
                                (float)(pz + leashWidth));
            }
        }
        builder->end();
    }
    GlStateManager::enableLighting();
    GlStateManager::enableTexture();
    GlStateManager::enableCull();
}
