#include "game/world/generation/AbstractArmorLayer.hpp"

#include <map>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/ResourceLocation.hpp"

static std::map<std::wstring, ResourceLocation> sArmorLocations;

AbstractArmorLayer::AbstractArmorLayer(LivingEntityRenderer* renderer)
{
    mRenderer = renderer;
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
    name << L"armor_" << armor->GetArmorType() << L"_" << (inner ? 2 : 1);
    if (overlay)
        name << L"_b";
    std::wstring key = name.str();
    std::map<std::wstring, ResourceLocation>::iterator found = sArmorLocations.find(key);
    if (found == sArmorLocations.end())
        found = sArmorLocations.insert(std::make_pair(key, ResourceLocation(key))).first;
    return &found->second;
}

void AbstractArmorLayer::renderFoil(LivingEntityRenderer*, boost::shared_ptr<LivingEntity>& entity, Model* model, float limbSwing, float limbSwingAmount, float, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool bUseDisplayList)
{
    GlStateManager::enableBlend();
    GlStateManager::depthFunc(514);
    GlStateManager::depthMask(false);
    GlStateManager::color4f(0.38f, 0.19f, 0.32f, 1.0f);
    GlStateManager::disableLighting();
    GlStateManager::blendFunc(1, 1);
    boost::shared_ptr<Entity> renderEntity = boost::static_pointer_cast<Entity>(entity);
    model->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, bUseDisplayList);
    GlStateManager::enableLighting();
    GlStateManager::disableBlend();
    GlStateManager::depthMask(true);
    GlStateManager::depthFunc(515);
}

void AbstractArmorLayer::renderArmorPiece(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount, float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale, const EquipmentSlot* slot, bool bUseDisplayList)
{
    not_null_ptr<ItemInstance> itemInstance = getSlot(entity, slot);
    if (itemInstance->isEmpty())
        return;
    ArmorItem* armor = dynamic_cast<ArmorItem*>(itemInstance->getItem());
    if (armor == NULL || armor->getSlot() != slot)
        return;
    Model* model = getArmorModel(slot);
    if (model == NULL)
        return;
    model->SetSkinAdjustments(entity->getSkinAdjustments());
    setPartVisibility(model, slot);
    mRenderer->bindTexture(getArmorLocation(armor, usesInnerModel(slot), false));
    boost::shared_ptr<Entity> renderEntity = boost::static_pointer_cast<Entity>(entity);
    model->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, bUseDisplayList);
    if (armor->getMaterial() == ArmorItem::ArmorMaterial::LEATHER)
    {
        int color = armor->getColor(itemInstance, 0);
        setColor((float)((color >> 16) & 255) / 255.0f, (float)((color >> 8) & 255) / 255.0f, (float)(color & 255) / 255.0f, mAlpha);
        model->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, bUseDisplayList);
        mRenderer->bindTexture(getArmorLocation(armor, usesInnerModel(slot), true));
        resetColor();
    }
    if (itemInstance->isEnchanted())
        renderFoil(mRenderer, entity, model, limbSwing, limbSwingAmount, partialTicks, ageInTicks, netHeadYaw, headPitch, scale, bUseDisplayList);
}

void AbstractArmorLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount, float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool bUseDisplayList)
{
    renderArmorPiece(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks, netHeadYaw, headPitch, scale, EquipmentSlot::FEET, bUseDisplayList);
    renderArmorPiece(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks, netHeadYaw, headPitch, scale, EquipmentSlot::LEGS, bUseDisplayList);
    renderArmorPiece(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks, netHeadYaw, headPitch, scale, EquipmentSlot::CHEST, bUseDisplayList);
    renderArmorPiece(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks, netHeadYaw, headPitch, scale, EquipmentSlot::HEAD, bUseDisplayList);
}

bool AbstractArmorLayer::colorsOnDamage()
{
    return false;
}

void AbstractArmorLayer::resetColor()
{
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
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
