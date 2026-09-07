#include "game/client/render/VillagerRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/VillagerModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/CustomHeadLayer.hpp"
#include "game/world/generation/Villager.hpp"

static ResourceLocation sVillagerTexture;
static ResourceLocation sFarmerTexture;
static ResourceLocation sLibrarianTexture;
static ResourceLocation sPriestTexture;
static ResourceLocation sBlacksmithTexture;
static ResourceLocation sButcherTexture;
static ResourceLocation sItemTexture;

VillagerRenderer::VillagerRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new VillagerModel(0.0F, ModelDefinition()), 0.5F)
{
    addLayer(new CustomHeadLayer(static_cast<VillagerModel*>(getModel())->mHead));
}

VillagerRenderer::~VillagerRenderer()
{
}

Model* VillagerRenderer::getModel()
{
    return LivingEntityRenderer::getModel();
}

void VillagerRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                              float yaw, float partialTick)
{
    MobRenderer::render(entity, x, y, z, yaw, partialTick);

    boost::shared_ptr<Villager> villager = boost::static_pointer_cast<Villager>(entity);
    if (villager->getTrading() == 0)
        return;

    not_null_ptr<ItemInstance> item(new ItemInstance(Items::emerald, 1));
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    Item* itemType = item->getItem();
    TextureAtlasSprite* sprite = itemType->getIcon(item);

    Minecraft* minecraft = Minecraft::GetInstance();
    EntityRenderDispatcher* dispatcher = minecraft->getEntityRenderDispatcher();
    bindTexture(&sItemTexture);

    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x,
                               (float)y + villager->getEyeHeight() + 0.5F,
                               (float)z);
    GlStateManager::normal3f(0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(180.0F - dispatcher->mCameraYRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(-dispatcher->mCameraXRot, 1.0F, 0.0F, 0.0F);
    GlStateManager::scalef(0.5F, 0.5F, 0.5F);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);

    builder->begin();
    builder->color(0xFFFFFF, 0x80);
    builder->normal(0.0F, 1.0F, 0.0F);
    builder->vertexUV(-0.5F, -0.25F, 0.0F, (float)sprite->getU0(false), (float)sprite->getV1(false));
    builder->vertexUV(0.5F, -0.25F, 0.0F, (float)sprite->getU1(false), (float)sprite->getV1(false));
    builder->vertexUV(0.5F, 0.75F, 0.0F, (float)sprite->getU1(false), (float)sprite->getV0(false));
    builder->vertexUV(-0.5F, 0.75F, 0.0F, (float)sprite->getU0(false), (float)sprite->getV0(false));
    builder->end();

    GlStateManager::disableBlend();
    GlStateManager::popMatrix();
}

const ResourceLocation* VillagerRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<Villager> villager = boost::static_pointer_cast<Villager>(entity);
    int profession = villager->getProfession();
    if (profession == 0)
        return &sFarmerTexture;
    if (profession == 1)
        return &sLibrarianTexture;
    if (profession < 3)
        return &sPriestTexture;
    if (profession == 3)
        return &sBlacksmithTexture;
    if (profession == 4)
        return &sButcherTexture;
    return &sVillagerTexture;
}

void VillagerRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    boost::shared_ptr<Villager> villager = boost::static_pointer_cast<Villager>(entity);
    if (villager->isBaby())
    {
        mShadowRadius = 0.25F;
        GlStateManager::scalef(0.46875F, 0.46875F, 0.46875F);
    }
    else
    {
        mShadowRadius = 0.5F;
        GlStateManager::scalef(0.9375F, 0.9375F, 0.9375F);
    }
    (void)partialTicks;
}

Model* VillagerRenderer::createModel(const ModelDefinition& definition)
{
    return new VillagerModel(0.0F, definition);
}
