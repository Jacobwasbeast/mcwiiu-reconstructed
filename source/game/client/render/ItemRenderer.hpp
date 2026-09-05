#ifndef GAME_CLIENT_RENDER_ITEMRENDERER_HPP
#define GAME_CLIENT_RENDER_ITEMRENDERER_HPP
#include <string>
#include <boost/shared_ptr.hpp>
#include "game/client/render/EntityRenderer.hpp"
#include "game/item/ItemTransforms.hpp"
#include "game/item/not_null_ptr.hpp"
class Entity; class EntityRenderDispatcher; class Font; class ItemColors; class ItemEntity; class ItemInstance; class LivingEntity; class Random; class ResourceLocation; class Tesselator; class TextureAtlasSprite; class Textures;
class ItemInHandRenderer;
class ItemRenderer : public EntityRenderer
{
public:
    ItemRenderer(EntityRenderDispatcher*, Textures*, ItemColors*); virtual ~ItemRenderer();
    ItemColors* getItemColors(); void renderFoilLayer(Tesselator*);
    virtual void render(boost::shared_ptr<Entity>&, double, double, double, float, float);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>&);
    const ResourceLocation* getTextureLocation(int);
    void renderItemBillboard(boost::shared_ptr<ItemEntity>&, TextureAtlasSprite*, int, float, float, float, float);
    void setupGuiItem(float, float, float, float, bool); bool isGui3d(not_null_ptr<ItemInstance>);
    void renderStatic(const not_null_ptr<ItemInstance>&, boost::shared_ptr<LivingEntity>, ItemTransforms::TransformType, float, bool, TextureAtlasSprite*);
    void renderGuiItem(not_null_ptr<ItemInstance>, float, float, float, float, float, bool, TextureAtlasSprite*);
    void renderGuiItem(not_null_ptr<ItemInstance>, float, float, float, float); void renderGuiItem(not_null_ptr<ItemInstance>, int, int);
    void renderAndDecorateItem(not_null_ptr<ItemInstance>, float, float, float, float, float, bool, bool, bool, TextureAtlasSprite*);
    void renderAndDecorateItem(not_null_ptr<ItemInstance>, float, float, float, float, bool); void renderAndDecorateItem(not_null_ptr<ItemInstance>, int, int);
    void fillRect(Tesselator*, int, int, int, int, int, int);
    void renderGuiItemDecorations(Font*, not_null_ptr<ItemInstance>, int, int, const std::wstring&, int, float);
    void renderGuiItemDecorations(Font*, not_null_ptr<ItemInstance>, int, int, float);
    void blit(float, float, TextureAtlasSprite*, float, float); void SetItemFrame(bool);
    void blitGlint(int, float, float, float, float); void blit(float, float, int, int, float, float);
private:
    friend class ItemInHandRenderer;
    ItemColors* mItemColors; Random* mRandom; bool mItemFrameEnabled; Textures* mTextures; bool mIsGui3d; float mDepth;
};
#endif
