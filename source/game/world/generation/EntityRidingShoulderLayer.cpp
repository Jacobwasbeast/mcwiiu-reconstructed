#include "game/world/generation/EntityRidingShoulderLayer.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ParrotRenderer.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"

extern ResourceLocation PARROT_LOCATIONS[5];

EntityRidingShoulderLayer::DataContainer::DataContainer(std::wstring name,
                                                        LivingEntityRenderer* renderer,
                                                        Model* model,
                                                        ResourceLocation texture,
                                                        unsigned long long entityType)
    : mName(name), mRenderer(renderer), mModel(model), mTexture(texture), mEntityType(entityType)
{
}

EntityRidingShoulderLayer::EntityRidingShoulderLayer(EntityRenderDispatcher* dispatcher)
    : mDispatcher(dispatcher), mLeftModel(NULL), mLeftEntityType(0), mRightModel(NULL),
      mRightEntityType(0), mParrotRenderer(new ParrotRenderer(dispatcher))
{
}

EntityRidingShoulderLayer::~EntityRidingShoulderLayer()
{
    delete mParrotRenderer;
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
    if (name.empty() || name.compare(0, name.size(), tag->getString(std::wstring(L"UUID")).c_str(),
                                     tag->getString(std::wstring(L"UUID")).size()) != 0)
    {
        name = tag->getString(std::wstring(L"UUID"));
        entityType = EntityIO::byString(tag->getString(std::wstring(L"id")));
        if (entityType == EntityType::PARROT)
        {
            renderer = mParrotRenderer;
            texture = PARROT_LOCATIONS[tag->getInt(std::wstring(L"Variant"))];
        }
    }

    renderer->bindTexture(&texture);
    GlStateManager::pushMatrix();
    GlStateManager::translatef(left ? 0.4f : -0.4f, entity->isSneaking() ? -1.3f : -1.5f, 0.0f);

    renderer->renderModel(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks,
                          netHeadYaw, headPitch);
    renderer->renderLayers(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks,
                           netHeadYaw, headPitch, scale, useDisplayList);
    GlStateManager::popMatrix();
}

void EntityRidingShoulderLayer::render(boost::shared_ptr<LivingEntity>& entity,
                                       float limbSwing, float limbSwingAmount,
                                       float partialTicks, float ageInTicks,
                                       float netHeadYaw, float headPitch, float scale,
                                       bool useDisplayList)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    if (player->getShoulderEntityLeft() == NULL && player->getShoulderEntityRight() == NULL)
    {
        return;
    }

    GlStateManager::enableRescaleNormal();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);

    CompoundTag* leftTag = player->getShoulderEntityLeft();
    if (!leftTag->isEmpty())
    {
        renderShoulder(entity, mLeftName, leftTag, mLeftRenderer, mLeftModel, mLeftTexture,
                       mLeftEntityType, limbSwing, limbSwingAmount, partialTicks, ageInTicks,
                       netHeadYaw, headPitch, scale, true, useDisplayList);
    }

    CompoundTag* rightTag = player->getShoulderEntityRight();
    if (!rightTag->isEmpty())
    {
        renderShoulder(entity, mRightName, rightTag, mRightRenderer, mRightModel, mRightTexture,
                       mRightEntityType, limbSwing, limbSwingAmount, partialTicks, ageInTicks,
                       netHeadYaw, headPitch, scale, false, useDisplayList);
    }

    GlStateManager::disableRescaleNormal();
}

bool EntityRidingShoulderLayer::colorsOnDamage()
{
    return false;
}

void EntityRidingShoulderLayer::setupForTexturePack(TexturePack* texturePack)
{
    mParrotRenderer->setupForTexturePack(texturePack);
}
