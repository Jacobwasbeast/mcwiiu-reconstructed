#include "game/client/render/BoatRenderer.hpp"

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

static ResourceLocation BOAT_TEXTURES[6];

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
    float damage = (float)(boat->getDamage() - (double)partialTick);
    if (hurtTime > 0.0F)
    {
        GlStateManager::rotatef(std::sin(hurtTime) * hurtTime * damage / 10.0F,
                                1.0F, 0.0F, 0.0F);
    }
    float bubbleAngle = boat->getBubbleAngle(partialTick);
    if (!Mth::equal(bubbleAngle, 0.0F))
        GlStateManager::rotatef(bubbleAngle, 1.0F, 0.0F, 1.0F);
    GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
}

void BoatRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                          float yaw, float partialTick)
{
    GlStateManager::pushMatrix();
    setupTranslation(x, y, z);
    setupRotation(entity, yaw, partialTick);
    bindTexture(entity);
    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        getTeamColor(entity);
        GlStateManager::setupSolidRenderingTextureCombine(0);
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
        boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(entity);
        if (boat->GetType() == ENTITY_TYPE_BOAT)
            renderSecondPass(entity, x, y, z, yaw, partialTick);
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
    double boatX = lerp(boat->mXo, boat->mX, partialTick);
    double boatY = lerp(boat->mYo, boat->mY, partialTick) - boat->mBbHeight;
    double boatZ = lerp(boat->mZo, boat->mZ, partialTick);
    double holderX = lerp(holder->mXo, holder->mX, partialTick);
    double holderY = lerp(holder->mYo, holder->mY, partialTick) + holder->getEyeHeight();
    double holderZ = lerp(holder->mZo, holder->mZ, partialTick);
    double dx = holderX - boatX;
    double dy = holderY - boatY;
    double dz = holderZ - boatZ;
    double length = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (length == 0.0)
        return;
    GlStateManager::disableTexture();
    GlStateManager::disableLighting();
    GlStateManager::disableCull();
    unsigned int startColour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Leash_Light_Colour);
    unsigned int endColour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Leash_Dark_Colour);
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin(1);
    for (int i = 0; i <= 24; ++i)
    {
        double t = (double)i / 24.0;
        double px = x + dx * t;
        double py = y + dy * t;
        double pz = z + dz * t;
        unsigned int colour = (i & 1) == 0 ? startColour : endColour;
        builder->color((float)((colour >> 16) & 255) / 255.0F,
                       (float)((colour >> 8) & 255) / 255.0F,
                       (float)(colour & 255) / 255.0F, 1.0F);
        builder->vertex((float)px, (float)py, (float)pz);
        builder->vertex((float)(px + 0.025), (float)(py + 0.025), (float)(pz + 0.025));
    }
    builder->end();
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
    if (!boat || !boat->hasPlayerPassenger())
        return false;
    boost::shared_ptr<Entity> passenger = boat->getControllingPassenger();
    return passenger && culler->isVisible(passenger->getBoundingBoxForCulling());
}

Model* BoatRenderer::createModel(const ModelDefinition& definition)
{
    return new BoatModel(definition);
}
