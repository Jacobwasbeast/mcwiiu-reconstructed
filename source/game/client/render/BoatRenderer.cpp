#include "game/client/render/BoatRenderer.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/render/BoatModel.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/Boat.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Culler.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation BOAT_TEXTURES[6] = {
    ResourceLocation((_TEXTURE_NAME)149),
    ResourceLocation((_TEXTURE_NAME)150),
    ResourceLocation((_TEXTURE_NAME)151),
    ResourceLocation((_TEXTURE_NAME)152),
    ResourceLocation((_TEXTURE_NAME)153),
    ResourceLocation((_TEXTURE_NAME)154)
};

BoatRenderer::BoatRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher)
{
    mShadowRadius = 0.5F;
    mModel = new BoatModel(ModelDefinition());
}

BoatRenderer::~BoatRenderer()
{
}

void BoatRenderer::setupTranslation(double x, double y, double z)
{
    GlStateManager::translatef((float)x, (float)y + 0.375F, (float)z);
}

void BoatRenderer::setupRotation(boost::shared_ptr<Entity> entity, float yaw, float partialTick)
{
    GlStateManager::rotatef(180.0F - yaw, 0.0F, 1.0F, 0.0F);
    boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(entity);
    float hurtTime = (float)((double)boat->getHurtTime() - (double)partialTick);
    float damage = boat->getDamage() - partialTick;
    if (damage < 0.0F)
        damage = 0.0F;
    if (hurtTime > 0.0F)
    {
        GlStateManager::rotatef(std::sin(hurtTime) * hurtTime * damage / 10.0F * boat->getHurtDir(),
                                1.0F, 0.0F, 0.0F);
    }
    if (!Mth::equal(boat->getBubbleAngle(partialTick), 0.0F))
        GlStateManager::rotatef(boat->getBubbleAngle(partialTick), 1.0F, 0.0F, 1.0F);
    GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
}

void BoatRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                          float yaw, float partialTick)
{
    boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(entity);
    GlStateManager::pushMatrix();
    setupTranslation(x, y, z);
    setupRotation(entity, yaw, partialTick);
    bindTexture(entity);
    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine((int)getTeamColor(entity));
    }
    mModel->render(entity, partialTick, 0.0F, -0.1F, 0.0F, 0.0F, 0.0625F, true);
    if (mSolidRender)
    {
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
    }
    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
    if (!mSolidRender)
    {
        if (entity->GetType() == EntityType::BOAT)
        {
            boost::shared_ptr<Boat> leashedBoat = boost::static_pointer_cast<Boat>(entity);
            renderLeash(leashedBoat, x, y, z, yaw, partialTick);
        }
    }
}

const ResourceLocation* BoatRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(entity);
    return &BOAT_TEXTURES[boat->getType()];
}

bool BoatRenderer::hasSecondPass()
{
    return true;
}

void BoatRenderer::renderSecondPass(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                    float yaw, float partialTick)
{
    GlStateManager::pushMatrix();
    setupTranslation(x, y, z);
    setupRotation(entity, yaw, partialTick);
    bindTexture(entity);
    static_cast<BoatModel*>(mModel)->renderSecondPass(entity, partialTick, 0.0F, -0.1F,
                                                      0.0F, 0.0F, 0.0625F, true);
    GlStateManager::popMatrix();
}

double BoatRenderer::lerp(double oldValue, double newValue, double partialTick)
{
    return oldValue + (newValue - oldValue) * partialTick;
}

void BoatRenderer::renderLeash(boost::shared_ptr<Boat>& boat, double x, double y, double z,
                               float yaw, float partialTick)
{
    boost::shared_ptr<Entity> holder = boat->getLeashHolder();
    if (!holder)
        return;
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    y -= boat->mBbHeight;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    const double radians = (double)0.01745329238474369F;
    double holderYaw = lerp(holder->mYRotO, holder->mYRot, partialTick * 0.5F) * radians;
    double holderPitch = lerp(holder->mXRotO, holder->mXRot, partialTick * 0.5F) * radians;
    double cosYaw = std::cos(holderYaw);
    double sinYaw = std::sin(holderYaw);
    double sinPitch = std::sin(holderPitch);
    if ((holder->GetType() & EntityType::HANGING_ENTITY) == EntityType::HANGING_ENTITY)
    {
        sinYaw = 0.0;
        sinPitch = -1.0;
        cosYaw = 0.0;
    }
    double cosPitch = std::cos(holderPitch);
    double holderX = lerp(holder->mXo, holder->mX, partialTick);
    double holderY = lerp(holder->mYo + holder->getEyeHeight() * 0.7,
                         holder->mY + holder->getEyeHeight() * 0.7, partialTick);
    double holderZ = lerp(holder->mZo, holder->mZ, partialTick);
    double boatX = lerp(boat->mXo, boat->mX, partialTick);
    double boatY = lerp(boat->mYo, boat->mY, partialTick);
    double boatZ = lerp(boat->mZo, boat->mZ, partialTick);
    double dx = (float)((holderX - cosYaw * 0.7 - sinYaw * 0.5 * cosPitch) - (boatX + cosYaw));
    double dy = (float)((holderY - sinPitch * 0.5 - 0.25) - boatY);
    double dz = (float)((holderZ - sinYaw * 0.7 + cosYaw * 0.5 * cosPitch) - (boatZ + sinYaw));
    GlStateManager::disableTexture();
    GlStateManager::disableLighting();
    GlStateManager::disableCull();
    unsigned int startColour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Leash_Light_Colour);
    unsigned int endColour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Leash_Dark_Colour);
    for (int strip = 0; strip < 2; ++strip)
    {
        builder->begin(1);
        for (int i = 0; i <= 24; ++i)
        {
            double t = (float)i / 24.0F;
            double px = x + dx * t;
            double py = dy * (float)(t * t + t) * 0.5 + y +
                        (double)((24.0F - (float)i) / 18.0F + 0.125F);
            double pz = z + dz * t;
            unsigned int colour = (i & 1) == 0 ? endColour : startColour;
            builder->color((float)((colour >> 16) & 255) / 255.0F,
                           (float)((colour >> 8) & 255) / 255.0F,
                           (float)(colour & 255) / 255.0F, 1.0F);
            if (strip == 0)
            {
                builder->vertex((float)px, (float)py, (float)pz);
                builder->vertex((float)(px + 0.025), (float)(py + 0.025), (float)pz);
            }
            else
            {
                builder->vertex((float)px, (float)(py + 0.025), (float)pz);
                builder->vertex((float)(px + 0.025), (float)py, (float)(pz + 0.025));
            }
        }
        builder->end();
    }
    GlStateManager::enableLighting();
    GlStateManager::enableTexture();
    GlStateManager::enableCull();
}

bool BoatRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler,
                                double x, double y, double z)
{
    if (EntityRenderer::shouldRender(entity, culler, x, y, z))
        return true;
    boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(entity);
    if (!boat || !boat->isLeashed())
        return false;
    boost::shared_ptr<Entity> holder = boat->getLeashHolder();
    return holder && culler->isVisible(holder->getBoundingBoxForCulling());
}

Model* BoatRenderer::createModel(const ModelDefinition& definition)
{
    return new BoatModel(definition);
}
