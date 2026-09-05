#ifndef GAME_CLIENT_RENDER_ITEMRENDERER_HPP
#define GAME_CLIENT_RENDER_ITEMRENDERER_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"
#include "game/item/not_null_ptr.hpp"

class Entity;
class EntityRenderDispatcher;
class Font;
class ItemColors;
class ItemEntity;
class ItemInstance;
class LivingEntity;
class Random;
class ResourceLocation;
class Tesselator;
class TextureAtlasSprite;
class Textures;

class ItemTransforms
{
public:
    typedef int TransformType;
};

class ItemRenderer : public EntityRenderer
{
public:
    ItemRenderer(EntityRenderDispatcher* dispatcher, Textures* textures, ItemColors* itemColors);
    virtual ~ItemRenderer();

    ItemColors* getItemColors();
    void renderFoilLayer(Tesselator* tesselator);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    const ResourceLocation* getTextureLocation(int itemId);
    void renderItemBillboard(boost::shared_ptr<ItemEntity>& entity, TextureAtlasSprite* sprite, int count, float x, float y, float z, float partialTick);
    void setupGuiItem(float x, float y, float width, float height, bool enableLighting);
    bool isGui3d(not_null_ptr<ItemInstance> item);
    void renderStatic(const not_null_ptr<ItemInstance>& item, boost::shared_ptr<LivingEntity> entity, ItemTransforms::TransformType transformType, float partialTick, bool leftHand, TextureAtlasSprite* sprite);
    void renderGuiItem(not_null_ptr<ItemInstance> item, float x, float y, float scale, float rotation, float alpha, bool flat, TextureAtlasSprite* sprite);
    void renderGuiItem(not_null_ptr<ItemInstance> item, float x, float y, float scale, float alpha);
    void renderGuiItem(not_null_ptr<ItemInstance> item, int x, int y);
    void renderAndDecorateItem(not_null_ptr<ItemInstance> item, float x, float y, float scale, float alpha, float partialTick, bool flat, bool drawDecorations, bool drawOverlay, TextureAtlasSprite* sprite);
    void renderAndDecorateItem(not_null_ptr<ItemInstance> item, float x, float y, float scale, float alpha, bool flat);
    void renderAndDecorateItem(not_null_ptr<ItemInstance> item, int x, int y);
    void fillRect(Tesselator* tesselator, int x, int y, int width, int height, int color, int alpha);
    void renderGuiItemDecorations(Font* font, not_null_ptr<ItemInstance> item, int x, int y, const std::wstring& text, int color, float scale);
    void renderGuiItemDecorations(Font* font, not_null_ptr<ItemInstance> item, int x, int y, float scale);
    void blit(float x, float y, TextureAtlasSprite* sprite, float width, float height);
    void SetItemFrame(bool itemFrame);
    void blitGlint(int texture, float x, float y, float width, float height);
    void blit(float x, float y, int u, int v, float width, float height);

private:
    ItemColors* mItemColors;
    Random* mRandom;
    bool mItemFrameEnabled;
    Textures* mTextures;
    bool mIsGui3d;
    float mDepth;
};

#endif
