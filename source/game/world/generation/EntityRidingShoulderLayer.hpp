#ifndef GAME_WORLD_GENERATION_ENTITYRIDINGSHOULDERLAYER_HPP
#define GAME_WORLD_GENERATION_ENTITYRIDINGSHOULDERLAYER_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"
#include "game/util/ResourceLocation.hpp"

class CompoundTag;
class EntityRenderDispatcher;
class LivingEntity;
class LivingEntityRenderer;
class Model;
class ParrotRenderer;
class TexturePack;

class EntityRidingShoulderLayer : public RenderLayer
{
public:
    class DataContainer
    {
    public:
        DataContainer(std::wstring name, LivingEntityRenderer* renderer, Model* model,
                      ResourceLocation texture, unsigned long long entityType);

        std::wstring name;
        LivingEntityRenderer* renderer;
        Model* model;
        ResourceLocation texture;
        unsigned long long entityType;
    };

    EntityRidingShoulderLayer(EntityRenderDispatcher* dispatcher);
    virtual ~EntityRidingShoulderLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                        float limbSwingAmount, float partialTicks, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale,
                        bool useDisplayList);
    virtual bool colorsOnDamage();
    virtual void setupForTexturePack(TexturePack* texturePack);

    void renderShoulder(boost::shared_ptr<LivingEntity>& entity, std::wstring name,
                        CompoundTag* tag, LivingEntityRenderer* renderer, Model* model,
                        ResourceLocation texture, unsigned long long entityType,
                        float limbSwing, float limbSwingAmount, float partialTicks,
                        float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool left, bool useDisplayList);

private:
    EntityRenderDispatcher* dispatcher;
    LivingEntityRenderer* leftRenderer;
    Model* leftModel;
    ResourceLocation leftTexture;
    std::wstring leftName;
    unsigned long long leftEntityType;
    LivingEntityRenderer* rightRenderer;
    Model* rightModel;
    ResourceLocation rightTexture;
    std::wstring rightName;
    unsigned long long rightEntityType;
    boost::shared_ptr<ParrotRenderer> parrotRenderer;
};

#endif
