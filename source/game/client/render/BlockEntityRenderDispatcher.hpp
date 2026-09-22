#ifndef GAME_CLIENT_RENDER_BLOCKENTITYRENDERDISPATCHER_HPP
#define GAME_CLIENT_RENDER_BLOCKENTITYRENDERDISPATCHER_HPP

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

class BlockEntity;
class BlockEntityRenderer;
class Font;
class HitResult;
class Level;
class TexturePack;
class Textures;
class Entity;

enum class BlockEntityRendererType : unsigned long long
{
    Sign = 0x01000202ULL,
    MobSpawner = 0x01200201ULL,
    Piston = 0x01200205ULL,
    Chest = 0x01E00201ULL,
    EnderChest = 0x01200208ULL,
    EnchantTable = 0x01200206ULL,
    EndPortal = 0x01300200ULL,
    EndGateway = 0x01300201ULL,
    Skull = 0x01200207ULL,
    Furnace = 0x01E00202ULL,
    Beacon = 0x01E00204ULL,
    Banner = 0x0100020CULL,
    ShulkerBox = 0x0120020FULL,
    Bed = 0x01000210ULL,
    Conduit = 0x01200212ULL
};

class BlockEntityRenderDispatcher : public boost::unordered_map<unsigned long long, BlockEntityRenderer*>
{
public:
    static BlockEntityRenderDispatcher* instance;
    static double xOff;
    static double yOff;
    static double zOff;

    BlockEntityRenderDispatcher();

    Font* getFont();
    static void staticCtor();
    BlockEntityRenderer* getRenderer(unsigned long long type);
    BlockEntityRenderer* getRenderer(boost::shared_ptr<BlockEntity> blockEntity);
    bool hasRenderer(boost::shared_ptr<BlockEntity> blockEntity);
    void setLevel(Level* level);
    void prepare(Level* level, Textures* textures, Font* font, boost::shared_ptr<Entity> camera,
                 const HitResult* cameraHitResult, float partialTicks);
    void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList);
    void render(boost::shared_ptr<BlockEntity> blockEntity, float partialTicks, int breakProgress,
                bool bTranslucent);
    void render(boost::shared_ptr<BlockEntity> blockEntity, float partialTicks);
    void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                bool bTranslucent, float alpha, bool bUseDisplayList);
    void setupForTexturePack(TexturePack* texturePack);

    Font* mFont;
    Textures* mTextures;
    Level* mLevel;
    boost::shared_ptr<Entity> mCamera;
    float mCameraYRot;
    float mCameraXRot;
    const HitResult* mCameraHitResult;
    double mX;
    double mY;
    double mZ;
};

#endif
