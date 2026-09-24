#ifndef GAME_CLIENT_RENDER_GAMERENDERER_HPP
#define GAME_CLIENT_RENDER_GAMERENDERER_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/util/Random.hpp"
#include "game/core/SmoothFloat.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class Minecraft;
class ResourceManager;
class AbstractClientPlayer;
class Font;
class Level;
class BlockPos;
class LevelRenderer;
class Culler;
class Entity;
class LivingEntity;
class Player;
class ItemInstance;
class ResourceLocation;
class MapRenderer;
class ItemInHandRenderer;
class PostChain;
class SparseLightStorage;
class LevelChunk;
class ElementStorage;
class CompressedBlockStorage;
class SparseDataStorage;
class FloatBuffer;

class GameRenderer
{
public:
    struct PositionStruct
    {
        float x;
        float y;
        float z;
    };

    GameRenderer(Minecraft* minecraft, ResourceManager* resourceManager);
    ~GameRenderer();

    static void renderNameTagInWorld(Font*, const std::wstring&, float, float, float, int, float, float, bool, bool, int, float);
    unsigned int getLightTexture(int, Level*);
    static int runUpdate(void*);
    unsigned int getTickCount();
    unsigned int postEffectActive();
    void shutdownEffect();
    void checkEntityPostEffect(boost::shared_ptr<Entity>);
    void cycleEffect();
    void onResourceManagerReload(ResourceManager*);
    double GetFovVal();
    static void UpdateElementStorageDeleteTicks();
    void tickFov();
    void tickLightTexture();
    boost::shared_ptr<Entity> getCamera();
    void tickRain();
    void tick(bool);
    void pick(float);
    double getFov(float, bool);
    void bobHurt(float);
    void bobView(float);
    unsigned int getCameraThirdPerson();
    double getCameraYRot(float);
    double getCameraXRot(float);
    void moveCameraToPlayer(float);
    void getFovAndAspect(float&, float&, float, bool);
    void setupCamera(float, int);
    void turnOnLightLayer(double);
    void turnOffLightLayer(double);
    void renderItemInHand(float, int);
    double getNightVisionScale(boost::shared_ptr<LivingEntity>, float);
    double getUnderWaterVisionScale(const boost::shared_ptr<Player>&, float);
    void updateLightTexture(float);
    static unsigned int fjGetFpsCap(int);
    const BlockPos* getFogOffsets();
    double getTargetFogDistance(const BlockPos&);
    void getTargetFogColor(const BlockPos&, float&, float&, float&);
    double modifyFogDistance(const BlockPos&, float, float);
    void setupClearColor(float);
    FloatBuffer* getBuffer(float, float, float, float);
    void resetFogColor(bool);
    void setupFog(int, float);
    void prepareAndRenderClouds(LevelRenderer*, float, int, double, double, double);
    void renderThermal(Culler*, float);
    void renderGhost(float, boost::shared_ptr<Entity>);
    void renderGhostPath(float, boost::shared_ptr<Entity>);
    void renderSnowAndRain(float);
    void renderEyePass(int, float, long long, bool);
    void renderLevel(float, long long);
    void setupGuiScreen(int);
    void renderItemActivationAnimation(int, int, float);
    void render(float, long long, bool);
    int shouldRenderBlockOutline();
    void resetData();
    MapRenderer* getMapRenderer();
    void setItemInHandRenderer(ItemInHandRenderer* renderer);
    static void AddForDelete(SparseLightStorage*);
    static void AddForDelete(LevelChunk*);
    static void AddForDelete(ElementStorage*);
    static void FinishedReassigning();
    void EnableUpdateThread(int);
    void DisableUpdateThread(int);
    static void EnterDeleteThreadCS();
    static void LeaveDeleteThreadCS();
    unsigned long long displayItemActivation(not_null_ptr<ItemInstance>, int);

    void setRenderHand(bool);
    bool shouldRenderHand();
    void setRenderBlockOutline(bool);
    void setPanoramicMode(bool);
    void setPanoramicSection(int);
    void togglePostEffect();
    void loadEffect(ResourceLocation*);
    void SetFovVal(float);
    PostChain* currentEffect();
    void resize(int, int);
    void zoomRegion(double, double, double);
    void unZoomRegion();
    void renderLevel(float);
    void updateAllChunks();
    static void AddForDelete(unsigned char*);
    static void AddForDelete(CompressedBlockStorage*);
    static void AddForDelete(SparseDataStorage*);
    bool IsUpdateThreadEnabled();

private:
    OSFastMutex mUpdateThreadMutex;
    unsigned int mUpdateThreadDisableMask;
    Minecraft* mMinecraft;
    ResourceManager* mResourceManager;
    Random mRandom;
    float mFogDistance;
    ItemInHandRenderer* mItemInHandRenderer;
    MapRenderer* mMapRenderer;
    unsigned int mTickCount;
    boost::shared_ptr<Entity> mPointedEntity;
    SmoothFloat mFov;
    SmoothFloat mFog;
    SmoothFloat mCamera;
    SmoothFloat mZoom;
    SmoothFloat mView;
    SmoothFloat mLight;
    unsigned char mData[120];
    unsigned int mLightTextures[4];
    arrayWithLength<int> mLightPixels[4];
    unsigned char mDataAfterLight[8452];
    not_null_ptr<ItemInstance> mActivationItems[4];
    unsigned char mDataTail[108];
    FloatBuffer* mBuffer;
    bool mRenderBlockOutline;
};

#endif
