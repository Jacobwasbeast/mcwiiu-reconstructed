#include "game/world/generation/AbstractArmorLayer.hpp"

#include <map>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/ResourceLocation.hpp"

static std::map<std::wstring, ResourceLocation> sArmorLocations;
static ResourceLocation ENCHANT_LOCATION;

AbstractArmorLayer::AbstractArmorLayer(LivingEntityRenderer* renderer)
    : mRenderer(renderer)
{
    mAlpha = 1.0f;
    mRed = 1.0f;
    mGreen = 1.0f;
    mBlue = 1.0f;
    mColorSet = false;
}

AbstractArmorLayer::~AbstractArmorLayer()
{
}

bool AbstractArmorLayer::usesInnerModel(const EquipmentSlot* slot)
{
    return slot == EquipmentSlot::LEGS;
}

not_null_ptr<ItemInstance> AbstractArmorLayer::getSlot(boost::shared_ptr<LivingEntity>& entity, const EquipmentSlot* slot)
{
    return entity->getItemSlot(slot);
}

ResourceLocation* AbstractArmorLayer::getArmorLocation(ArmorItem* armor, bool inner, bool overlay)
{
    std::wstringstream name;
    name << L"armor/" << armor->GetArmorType() << L"_" << (inner ? 2 : 1);
    if (overlay)
        name << L"_b";
    std::wstring key = name.str();
    std::map<std::wstring, ResourceLocation>::iterator found = sArmorLocations.find(key);
    if (found == sArmorLocations.end())
        found = sArmorLocations.insert(std::make_pair(key, ResourceLocation(key))).first;
    return &found->second;
}

void AbstractArmorLayer::renderFoil(LivingEntityRenderer* renderer, boost::shared_ptr<LivingEntity>& entity, Model* model, float limbSwing, float limbSwingAmount, float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool bUseDisplayList)
{
    Minecraft::GetInstance()->mGameRenderer->resetFogColor(true);

    float ticks = (float)entity->mTickCount + partialTicks;

    renderer->bindTexture(&ENCHANT_LOCATION);
    GlStateManager::enableBlend();
    GlStateManager::depthFunc(2);
    GlStateManager::depthMask(false);
    GlStateManager::color4f(0.5f, 0.5f, 0.5f, 1.0f);
    GlStateManager::disableLighting();
    GlStateManager::blendFunc(2, 1);

    boost::shared_ptr<Entity> renderEntity = entity;
    for (int pass = 0; pass < 2; ++pass)
    {
        GlStateManager::color4f(0.38f, 0.19f, 0.608f, 1.0f);
        GlStateManager::matrixMode(2);
        GlStateManager::loadIdentity();
        GlStateManager::scalef(0.33333334f, 0.33333334f, 0.33333334f);
        GlStateManager::rotatef(30.0f - (float)pass * 60.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::translatef(0.0f, ticks * (0.001f + (float)pass * 0.003f) * 20.0f, 0.0f);
        GlStateManager::matrixMode(0);
        model->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch,
                      scale, bUseDisplayList);
    }

    GlStateManager::matrixMode(2);
    GlStateManager::loadIdentity();
    GlStateManager::matrixMode(0);
    GlStateManager::enableLighting();
    GlStateManager::depthMask(true);
    GlStateManager::depthFunc(3);
    GlStateManager::disableBlend();

    Minecraft::GetInstance()->mGameRenderer->resetFogColor(false);
}

void AbstractArmorLayer::renderArmorPiece(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount, float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale, const EquipmentSlot* slot, bool bUseDisplayList)
{
    not_null_ptr<ItemInstance> itemInstance = getSlot(entity, slot);
    boost::shared_ptr<Entity> renderEntity = entity;

    if (itemInstance->isEmpty())
    {
        return;
    }

    ArmorItem* armor = dynamic_cast<ArmorItem*>(itemInstance->getItem());
    if (armor == NULL)
    {
        return;
    }

    if ((entity->getSkinAdjustments().mAnimFlags & 0x100) != 0)
    {
        return;
    }

    if (armor->getSlot() != slot)
    {
        return;
    }

    Model* model = getArmorModel(slot);
    model->copyPropertiesFrom(mRenderer->getModel());
    model->prepareMobModel(entity, limbSwing, limbSwingAmount, partialTicks);
    setPartVisibility(model, slot);

    bool inner = usesInnerModel(slot);
    mRenderer->bindTexture(getArmorLocation(armor, inner, false));

    if (armor->getMaterial() == ArmorItem::ArmorMaterial::LEATHER)
    {
        int color = armor->getColor(itemInstance, 0);
        GlStateManager::color4f(mRed * ((float)((color >> 16) & 255) / 255.0f),
                                mGreen * ((float)((color >> 8) & 255) / 255.0f),
                                mBlue * ((float)(color & 255) / 255.0f),
                                mAlpha);
        model->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch,
                      scale, bUseDisplayList);
        mRenderer->bindTexture(getArmorLocation(armor, inner, true));
    }

    GlStateManager::color4f(mRed, mGreen, mBlue, mAlpha);
    model->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch,
                  scale, bUseDisplayList);

    if (!mColorSet && itemInstance->isEnchanted())
    {
        renderFoil(mRenderer, entity, model, limbSwing, limbSwingAmount, partialTicks, ageInTicks,
                   netHeadYaw, headPitch, scale, true);
    }
}

void AbstractArmorLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount, float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool bUseDisplayList)
{
    renderArmorPiece(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks, netHeadYaw, headPitch, scale, EquipmentSlot::CHEST, bUseDisplayList);
    renderArmorPiece(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks, netHeadYaw, headPitch, scale, EquipmentSlot::LEGS, bUseDisplayList);
    renderArmorPiece(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks, netHeadYaw, headPitch, scale, EquipmentSlot::FEET, bUseDisplayList);
    renderArmorPiece(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks, netHeadYaw, headPitch, scale, EquipmentSlot::HEAD, bUseDisplayList);
}

bool AbstractArmorLayer::colorsOnDamage()
{
    return false;
}

void AbstractArmorLayer::resetColor()
{
    setColor(1.0f, 1.0f, 1.0f, 1.0f);
    mColorSet = false;
}

void AbstractArmorLayer::setColor(float red, float green, float blue, float alpha)
{
    mRed = red;
    mGreen = green;
    mBlue = blue;
    mAlpha = alpha;
    mColorSet = true;
}

Model* AbstractArmorLayer::getArmorModel(const EquipmentSlot* slot)
{
    return usesInnerModel(slot) ? mModelArmorInner : mModelArmor;
}
