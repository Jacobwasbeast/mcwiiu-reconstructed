#include "game/world/generation/EntityRidingShoulderLayer.hpp"

#include <cwchar>

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ParrotRenderer.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"

EntityRidingShoulderLayer::DataContainer::DataContainer(std::wstring nameValue,
                                                         LivingEntityRenderer* rendererValue,
                                                         Model* modelValue,
                                                         ResourceLocation textureValue,
                                                         unsigned long long entityTypeValue)
    : name(nameValue), renderer(rendererValue), model(modelValue), texture(textureValue),
      entityType(entityTypeValue)
{
}

EntityRidingShoulderLayer::EntityRidingShoulderLayer(EntityRenderDispatcher* dispatcherValue)
    : dispatcher(dispatcherValue), leftRenderer(NULL), leftModel(NULL), leftTexture(),
      leftName(), leftEntityType(0), rightRenderer(NULL), rightModel(NULL), rightTexture(),
      rightName(), rightEntityType(0), parrotRenderer()
{
    parrotRenderer.reset(new ParrotRenderer(dispatcherValue));
}

EntityRidingShoulderLayer::~EntityRidingShoulderLayer()
{
}

void EntityRidingShoulderLayer::renderShoulder(boost::shared_ptr<LivingEntity>& entity,
                                               std::wstring name, CompoundTag* tag,
                                               LivingEntityRenderer* renderer, Model* model,
                                               ResourceLocation texture,
                                               unsigned long long entityType,
                                               float limbSwing, float limbSwingAmount,
                                               float partialTicks, float ageInTicks,
                                               float netHeadYaw, float headPitch, float scale,
                                               bool left, bool useDisplayList)
{
    if (tag->getString(std::wstring(L"UUID")) != name)
    {
        name = tag->getString(std::wstring(L"UUID"));
    }

    if (EntityIO::byString(tag->getString(std::wstring(L"id"))) == 0x2D6C0003ULL)
    {
        texture = ResourceLocation();
        texture = ResourceLocation(texture);
        entityType = EntityIO::byString(tag->getString(std::wstring(L"id")));
        texture = ResourceLocation(texture);
    }

    renderer->bindTexture(&texture);
    GlStateManager::pushMatrix();

    if (entity->isBaby())
    {
        GlStateManager::translatef(left ? -0.4f : 0.4f, 0.0f, 0.0f);
    }
    else
    {
        GlStateManager::translatef(left ? -0.4f : 0.4f, -1.3f, 0.0f);
    }

    renderer->renderModel(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks,
                          netHeadYaw, headPitch);
    renderer->renderLayers(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks,
                           netHeadYaw, headPitch, scale, useDisplayList);
    GlStateManager::popMatrix();

    DataContainer data(name, renderer, model, texture, entityType);
    (void)data;
}

void EntityRidingShoulderLayer::render(boost::shared_ptr<LivingEntity>& entity,
                                       float limbSwing, float limbSwingAmount,
                                       float partialTicks, float ageInTicks,
                                       float netHeadYaw, float headPitch, float scale,
                                       bool useDisplayList)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    CompoundTag* leftTag = player->getShoulderEntityLeft();
    CompoundTag* rightTag = player->getShoulderEntityRight();

    GlStateManager::enableRescaleNormal();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);

    if (leftTag)
    {
        renderShoulder(entity, leftName, leftTag, leftRenderer,
                       leftModel, leftTexture, leftEntityType, limbSwing, limbSwingAmount,
                       partialTicks, ageInTicks, netHeadYaw, headPitch, scale, true,
                       useDisplayList);
    }
    if (rightTag)
    {
        renderShoulder(entity, rightName, rightTag, rightRenderer,
                       rightModel, rightTexture, rightEntityType, limbSwing, limbSwingAmount,
                       partialTicks, ageInTicks, netHeadYaw, headPitch, scale, false,
                       useDisplayList);
    }

    GlStateManager::disableRescaleNormal();
}

bool EntityRidingShoulderLayer::colorsOnDamage()
{
    return false;
}

void EntityRidingShoulderLayer::setupForTexturePack(TexturePack* texturePack)
{
    parrotRenderer->setupForTexturePack(texturePack);
}
