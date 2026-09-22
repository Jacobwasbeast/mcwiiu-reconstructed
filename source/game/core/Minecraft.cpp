#include "game/core/Minecraft.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"

#include <wchar.h>

Textures* Minecraft::getTextureManager()
{
    return mTextures;
}

#include "game/client/audio/ConsoleSoundEngine.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/client/render/BlockEntityRenderDispatcher.hpp"
#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/EntityBlockRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/GuiParticles.hpp"
#include "game/client/render/ItemInHandRenderer.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/TextureAtlas.hpp"
#include "game/client/render/TextureManager.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BossHealthOverlay.hpp"
#include "game/core/AbstractIllager.hpp"
#include "game/core/ArmorStand.hpp"
#include "game/core/BannerPattern.hpp"
#include "game/core/Bat.hpp"
#include "game/core/Blaze.hpp"
#include "game/core/Boat.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Chicken.hpp"
#include "game/core/CommonStats.hpp"
#include "game/core/CameraController.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/DefaultVertexFormat.hpp"
#include "game/core/DispenserBootstrap.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/core/EnderMan.hpp"
#include "game/core/FaceInfo.hpp"
#include "game/core/FontCharacterWidths.hpp"
#include "game/core/Ghast.hpp"
#include "game/core/Guardian.hpp"
#include "game/core/Llama.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/Parrot.hpp"
#include "game/core/Pig.hpp"
#include "game/core/PolarBear.hpp"
#include "game/core/PrimedTnt.hpp"
#include "game/core/Salmon.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/core/Sheep.hpp"
#include "game/core/Shulker.hpp"
#include "game/core/Slime.hpp"
#include "game/core/SnowMan.hpp"
#include "game/core/SpellcasterIllager.hpp"
#include "game/core/Spider.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/core/TropicalFish.hpp"
#include "game/core/Turtle.hpp"
#include "game/core/Vex.hpp"
#include "game/core/Witch.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/core/WitherSkull.hpp"
#include "game/core/Wolf.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "vendor/_ConnectionProgressParams.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/FoliageColor.hpp"
#include "game/core/FrameTimer.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/GhostController.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/GrassColor.hpp"
#include "game/core/Gui.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Input.hpp"
#include "game/core/InfoBarManager.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Options.hpp"
#include "game/core/Stats.hpp"
#include "game/core/StatsCounter.hpp"
#include "game/core/StatsUID.hpp"
#include "game/core/System.hpp"
#include "game/core/Timer.hpp"
#include "game/core/TrialMode.hpp"
#include "game/core/User.hpp"
#include "game/core/UnityClient13.hpp"
#include "game/core/UnityClient6.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixers.hpp"
#include "game/command/CheckpointCommand.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/McRegionLevelStorageSource.hpp"
#include "game/data/SparseDataStorage.hpp"
#include "game/data/SparseLightStorage.hpp"
#include "game/data/StringTag.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemColors.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/minigame/MiniGameMedals.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/PlayerReadyPacket.hpp"
#include "game/ui/Screen.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"
#include "game/ui/TitleScreen.hpp"
#include "game/ui/Tooltips.hpp"
#include "game/ui/UIScene_CreativeMenu.hpp"
#include "game/ui/UIScene_LeaderboardsMenu.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockColors.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/entity/mob/AbstractChestedHorse.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/TamableAnimal.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/network/Packet.hpp"
#include "game/entity/projectile/AbstractRotatedItemProjectile.hpp"
#include "game/entity/projectile/FishingHook.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/AxeItem.hpp"
#include "game/item/PickaxeItem.hpp"
#include "game/item/ShovelItem.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/FurnaceRecipes.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemIdFix.hpp"
#include "game/item/ItemPotionFix.hpp"
#include "game/item/MapExtendingRecipe.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionBrewing.hpp"
#include "game/item/Potions.hpp"
#include "game/item/Recipes.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/ui/InventoryMenu.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/JungleBiome.hpp"
#include "game/world/biome/MesaBiome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/chunk/ChunkInfoMapManager.hpp"
#include "game/world/chunk/McRegionChunkStorage.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/generation/BirchFeature.hpp"
#include "game/world/generation/BuriedTreasureFeature.hpp"
#include "game/world/generation/DesertWellFeature.hpp"
#include "game/world/generation/MegaPineTreeFeature.hpp"
#include "game/world/generation/OceanMonumentFeature.hpp"
#include "game/core/OceanMonumentPieces.hpp"
#include "game/world/generation/PineFeature.hpp"
#include "game/world/generation/RandomScatteredLargeFeature.hpp"
#include "game/world/generation/RoofTreeFeature.hpp"
#include "game/world/generation/SavannaTreeFeature.hpp"
#include "game/world/generation/ScatteredFeaturePieces.hpp"
#include "game/world/generation/SpruceFeature.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/world/generation/SwampTreeFeature.hpp"
#include "game/world/generation/TreeFeature.hpp"
#include "game/world/generation/VillageFeature.hpp"
#include "game/world/generation/VillagePieces.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"
#include "game/world/generation/WoodlandMansionFeature.hpp"
#include "game/world/generation/ZombieVillager.hpp"
#include "game/core/Creeper.hpp"
#include "game/core/Ocelot.hpp"
#include "game/core/StrongholdPieces.hpp"
#include "game/entity/mob/EnderDragon.hpp"
#include "game/core/MinecartFurnace.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/util/Rabbit.hpp"
#include "game/entity/mob/Horse.hpp"
#include "game/world/block/BeaconBlockEntity.hpp"
#include "game/world/block/BlockEntityIdFix.hpp"
#include "game/world/block/EatBlockGoal.hpp"
#include "game/world/block/FallingBlockEntity.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/world/block/ItemInstanceBlockEntityTagWalker.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/MinecartCommandBlock.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"
#include "game/server/ServerLevel.hpp"

void MemSect(int section);
void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();
void Sleep(unsigned int milliseconds);
int XEnableGuestSignin(int enable);
void MinecraftWorld_RunStaticCtors();
void stripWhitespaceForHtml(std::wstring& text, bool collapse);
std::wstring convStringToWstring(wchar_t* text);

template <typename T>
std::wstring _toString(T value);

static Minecraft* sInstance = NULL;
static int sAverageFps = 0;
static bool sStaticsInitialised = false;
static long long sFrameStartTime = 0;
static int sPressStartHoldOffFrames = 0;
static bool sCheckForNewLocalPlayers = true;
bool gFancyGraphics = true;
static std::wstring sWorkingDirectory;
static OSFastMutex sTextureRequestLock;
GenericStats* gStats = NULL;

Minecraft::Minecraft(Component* parent, Canvas* canvas, MinecraftApplet* applet, int width, int height,
                     bool fullscreen)
    : mFrameTimer(new FrameTimer())
{
    mFixerUpper = DataFixers::createFixerUpper();
    mHasCrashed = false;
    mGameMode = NULL;
    mTimer = new Timer(20.0f);
    mPendingLevelChange = 0;
    mLevel = NULL;
    mLevelRenderer = NULL;
    mPlayer.reset();
    mLevels = new MultiPlayerLevel*[3];
    mLevels[0] = NULL;
    mLevels[1] = NULL;
    mLevels[2] = NULL;
    mLevelCount = 3;
    mCameraEntity.reset();
    mMissTime = 0;
    mRightClickDelay = 0;
    mLastClickTick = 0;
    mLevelId = 0;
    mFrameNumber = 0;
    mPaused = false;

    InitializeCriticalSection(&sTextureRequestLock);
    InitializeCriticalSection(&mLevelLock);

    mNoRender = false;
    mScreen = NULL;
    mHitResult = NULL;
    mGui = NULL;
    mOptions = NULL;
    mMouseGrabTicks = 0;
    mProgressRenderer = NULL;
    mGameRenderer = NULL;
    mDefaultHeight = 0;
    mTextures = NULL;

    mSoundEngine = new SoundEngine();
    mTexturePackRepository = NULL;
    mSoundReloadDelay = 0;
    mWorkingDirectory = File(L"");
    sWorkingDirectory = File(L"").getPath();
    mLevelSource = NULL;
    mStats[0] = NULL;
    mStats[1] = NULL;
    mStats[2] = NULL;
    mStats[3] = NULL;
    mLevelId = 0;
    mEmptyViewport = -1;
    mLastRenderTime = -1;
    mRunning = true;
    mFrameNumber = 0;
    Stats::init();

    mMouseGrabTicks = 0;
    mServerPort = 0;
    mItemColors = NULL;
    mBlockColors = NULL;
    mDefaultHeight = height;
    mFullscreen = fullscreen;

    sInstance = this;

    mPaused = false;
    mFullHeight = height;
    mFullWidth = width;
    mHeight = height;
    if (gRenderer->IsWidescreen())
        mWidth = width;
    else
        mWidth = (width * 3) / 4;
    TextureManager::createInstance();

    mConnectionStartTime[0] = 0;
    mConnectionStartTime[1] = 0;
    mConnectionStartTime[2] = 0;
    mConnectionStartTime[3] = 0;
    mTutorialPlayers = 0;
    mLastTextureRequestTick = 0;
    mConnectionFailed[0] = false;
    mConnectionFailed[1] = false;
    mConnectionFailed[2] = false;
    mConnectionFailed[3] = false;
    mDisconnectReasons[0] = DisconnectPacket::eDisconnectReason_None;
    mDisconnectReasons[1] = DisconnectPacket::eDisconnectReason_None;
    mDisconnectReasons[2] = DisconnectPacket::eDisconnectReason_None;
    mDisconnectReasons[3] = DisconnectPacket::eDisconnectReason_None;
    mReloadTextures = false;
    mLocalPlayerIdx = 0;
    mPendingConnections[0] = NULL;
    mPendingConnections[1] = NULL;
    mPendingConnections[2] = NULL;
    mPendingConnections[3] = NULL;
    mSoundEngine->init(NULL);

    mClientMasterGameMode = new ClientMasterGameMode();
    mGhostController = NULL;
    mMiniGame = NULL;
}

Minecraft* Minecraft::GetInstance()
{
    return sInstance;
}

long long Minecraft::currentTimeMillis()
{
    return System::processTimeInMilliSecs();
}

DataFixerUpper* Minecraft::getFixerUpper()
{
    return mFixerUpper;
}

BlockRenderDispatcher* Minecraft::getBlockRenderer()
{
    return mBlockRenderer;
}

ItemRenderer* Minecraft::getItemRenderer()
{
    return mItemRenderer;
}

ItemInHandRenderer* Minecraft::getItemInHandRenderer()
{
    return mItemInHandRenderer;
}

EntityRenderDispatcher* Minecraft::getEntityRenderDispatcher()
{
    return mEntityRenderDispatcher;
}

BlockColors* Minecraft::getBlockColors()
{
    return mBlockColors;
}

ItemColors* Minecraft::getItemColors()
{
    return mItemColors;
}


double Minecraft::getFrameTime()
{
    return mTimer->renderPartialTicks;
}

ColourTable* Minecraft::getColourTable()
{
    ColourTable* colourTable = mTexturePackRepository->getSelected()->getColourTable();
    if (colourTable == NULL)
        colourTable = TexturePackRepository::defaultTexturePack->getColourTable();
    return colourTable;
}

MaterialTable* Minecraft::getMaterialTable()
{
    MaterialTable* materialTable = mTexturePackRepository->getSelected()->getMaterialTable();
    if (materialTable == NULL)
        materialTable = TexturePackRepository::defaultTexturePack->getMaterialTable();
    return materialTable;
}

StructureManager* Minecraft::getStructureManager()
{
    StructureManager* structureManager = mTexturePackRepository->getSelected()->getStructureManager();
    if (structureManager == NULL)
        structureManager = TexturePackRepository::defaultTexturePack->getStructureManager();
    return structureManager;
}

unsigned int Minecraft::getCurrentTexturePackId()
{
    return mTexturePackRepository->getSelected()->getId();
}

bool Minecraft::isUsingDefaultSkin()
{
    return GetInstance()->mTexturePackRepository->getSelected() == TexturePackRepository::defaultTexturePack;
}

void Minecraft::setScreen(Screen* screen)
{
    if (mScreen != NULL)
        mScreen->removed();

    if (screen == NULL && mLevel == NULL)
    {
        screen = new TitleScreen();
    }
    else if (mPlayer && !gConsoleUIController.GetMenuDisplayed(mPlayer->mUserIndex))
    {
        if (mPlayer->getHealth() <= 0.0f)
        {
            if (mLastClickTick == 0)
                mPlayer->respawn();
            else
                gConsoleUIController.NavigateToScene(mPlayer->mUserIndex, (EUIScene)0x18, NULL, (EUILayer)6,
                                                      (EUIGroup)7);
        }
    }

    if (screen != NULL && dynamic_cast<TitleScreen*>(screen) != NULL)
    {
        mOptions->mRenderDebug = 0;
        mGui->clearMessages(-1);
    }

    mScreen = screen;
    if (screen != NULL)
    {
        ScreenSizeCalculator sizeCalculator(mOptions, mWidth, mHeight, -1);
        screen->init(this, sizeCalculator.getWidth(), sizeCalculator.getHeight());
        mNoRender = false;
    }
}

void Minecraft::init()
{
    mWorkingDirectory = File(L"");
    mLevelSource = new McRegionLevelStorageSource(File(mWorkingDirectory, L"saves"), getFixerUpper());
    mOptions = new Options(this, mWorkingDirectory);
    mTexturePackRepository = new TexturePackRepository(mWorkingDirectory, this);
    mTexturePackRepository->addDebugPacks();
    mTextures = new Textures(mTexturePackRepository, mOptions);

    mFont = new Font(mOptions, L"font/Default", mTextures, false, &UnityClient13TextureLocation_0x7A, 0x17, 0x1c, 0x10,
                     0x10, fontCharacterWidthTable);
    mAlternateFont =
        new Font(mOptions, L"font/alternate", mTextures, false, &UnityClient13TextureLocation_0x7B, 0x10, 0x10, 8, 8, NULL);

    GrassColor::init(mTextures->loadTexturePixels(static_cast<_TEXTURE_NAME>(0), L"misc/grasscolor"));
    FoliageColor::init(mTextures->loadTexturePixels(static_cast<_TEXTURE_NAME>(1), L"misc/foliagecolor"));
    Biome::generateColoursDebugOutput();

    mTerrainAtlas = new TextureAtlas(0, L"terrain", L"textures/blocks/", NULL, true);
    mBlockColors = BlockColors::createDefault();
    mItemColors = ItemColors::createDefault(mBlockColors);
    mItemRenderer = new ItemRenderer(mEntityRenderDispatcher, mTextures, mItemColors);
    mEntityRenderDispatcher = new EntityRenderDispatcher(mTextures, mItemRenderer);

    GlStateManager::enableLighting();
    mItemInHandRenderer = new ItemInHandRenderer(this);
    GlStateManager::disableLighting();

    mGameRenderer = new GameRenderer(this, NULL);
    mBlockRenderer = new BlockRenderDispatcher(mBlockColors);
    BlockEntityRenderDispatcher::instance->mTextures = mTextures;
    EntityBlockRenderer::instance = new EntityBlockRenderer();

    mStats[0] = new StatsCounter();
    mStats[1] = new StatsCounter();
    mStats[2] = new StatsCounter();
    mStats[3] = new StatsCounter();

    MemSect(0x1f);
    MemSect(0);

    GlStateManager::enableTexture();
    GlStateManager::shadeModel(0);
    GlStateManager::clearDepth(1.0);
    GlStateManager::enableDepthTest();
    GlStateManager::depthFunc(3);
    GlStateManager::enableAlphaTest();
    GlStateManager::alphaFunc(4, 0.1f);
    GlStateManager::cullFace(1);
    GlStateManager::matrixMode(1);
    GlStateManager::loadIdentity();
    GlStateManager::matrixMode(0);

    MemSect(0x1f);
    MemSect(0);

    mLevelRenderer = new LevelRenderer(this, mTextures);
    mTextures->stitch();
    GlStateManager::viewport(0, 0, mWidth, mHeight);
    mParticleEngine = new ParticleEngine(mLevel, mTextures);

    MemSect(0x1f);
    MemSect(0);

    mGui = new Gui(this);
    if (mLevelName.compare(0, mLevelName.size(), L"", wcslen(L"")) == 0)
        setScreen(new TitleScreen());

    mProgressRenderer = new ProgressRenderer(this);
    gRenderer->CBuffLockStaticCreations();
}

void Minecraft::run()
{
    mRunning = true;
    init();
}

void Minecraft::stop()
{
    mRunning = false;
}

void Minecraft::destroy()
{
    setLevel(NULL, -1, boost::shared_ptr<Player>(), true, false);
    MemoryTracker::release();
    mSoundEngine->destroy();
}

void Minecraft::emergencySave()
{
    mLevelRenderer->clear();
    AABB::clearPool();
    Vec3::clearPool();
    setLevel(NULL, -1, boost::shared_ptr<Player>(), true, false);
}

void Minecraft::reloadSound()
{
    mSoundEngine = new SoundEngine();
    mSoundEngine->init(mOptions);
}

bool Minecraft::isClientSide()
{
    return mLevel != NULL && mLevel->mIsClientSide;
}

bool Minecraft::isTutorial()
{
    return mTutorialPlayers != 0;
}

bool Minecraft::showOnlyReducedInfo()
{
    return mPlayer && mPlayer->isReducedDebugInfo();
}

LevelStorageSource* Minecraft::getLevelSource()
{
    return mLevelSource;
}

MultiPlayerLevel* Minecraft::getLevel(int dimension)
{
    if (dimension == -1)
        return mLevels[1];
    if (dimension == 1)
        return mLevels[2];
    return mLevels[0];
}

void Minecraft::forceaddLevel(MultiPlayerLevel* level)
{
    const DimensionType* type = level->mDimension->getType();
    if (type == DimensionType::NETHER)
        mLevels[1] = level;
    else if (type == DimensionType::THE_END)
        mLevels[2] = level;
    else
        mLevels[0] = level;
}

void Minecraft::releaseLevel(int dimension)
{
    setLevel(NULL, dimension, boost::shared_ptr<Player>(), true, false);
}

void Minecraft::prepareLevel(int dimension)
{
    if (mProgressRenderer != NULL)
    {
        mProgressRenderer->progressStart(dimension);
        mProgressRenderer->progressStage(-1590511448);
    }

    int radius = mGameMode->isCutScene() ? 0x40 : 0x80;
    int step = (radius * 2) / 16 + 1;
    int total = step * step;
    int done = 0;
    for (int x = -radius; x <= radius; x += 16)
    {
        for (int z = -radius; z <= radius; z += 16)
        {
            if (mProgressRenderer != NULL)
            {
                mProgressRenderer->progressStagePercentage((done * 100) / total);
                ++done;
            }
        }
    }

    if (!mGameMode->isCutScene() && mProgressRenderer != NULL)
        mProgressRenderer->progressStage(-262824192);
}

void Minecraft::setLevel(MultiPlayerLevel* level, int dimension, boost::shared_ptr<Player> player, bool disconnect,
                         bool saveStats)
{
    EnterCriticalSection(&mLevelLock);

    mCameraEntity.reset();

    if (mProgressRenderer != NULL)
    {
        mProgressRenderer->progressStart(dimension);
        mProgressRenderer->progressStage(-1);
    }

    mGameRenderer->DisableUpdateThread(0);

    for (int i = 0; i < mLevelCount; ++i)
    {
        if (mLevels[i] != NULL && level == NULL)
        {
            if (saveStats && mPlayer)
                forceStatsSave(mPlayer->mUserIndex);

            if (mLevelRenderer != NULL)
            {
                mLevelRenderer->setLevel(0, NULL);
                mLevelRenderer->setLevel(1, NULL);
                mLevelRenderer->setLevel(2, NULL);
                mLevelRenderer->setLevel(3, NULL);
            }
            if (mParticleEngine != NULL)
                mParticleEngine->setLevel(NULL);
        }
    }

    if (level == NULL)
    {
        mGui->onDisconnected();
        mSoundEngine->stopAll();

        if (mLevels[0] != NULL)
        {
            delete mLevels[0];
            mLevels[0] = NULL;
            if (mLevels[1] != NULL)
                mLevels[1]->setSavedDataStorage(NULL);
            if (mLevels[2] != NULL)
                mLevels[2]->setSavedDataStorage(NULL);
        }
        if (mLevels[1] != NULL)
            delete mLevels[1];
        mLevels[1] = NULL;
        if (mLevels[2] != NULL)
            delete mLevels[2];
        mLevels[2] = NULL;

        for (int i = 0; i < 4; ++i)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mLocalPlayers[i];
            if (localPlayer && localPlayer->mPacketListener != NULL)
            {
                delete localPlayer->mPacketListener;
                localPlayer->mPacketListener = NULL;
            }
            if (mGameModes[i] != NULL)
            {
                delete mGameModes[i];
                mGameModes[i] = NULL;
            }
            if (mPendingConnections[i] != NULL)
            {
                delete mPendingConnections[i];
                mPendingConnections[i] = NULL;
            }
            mLocalPlayers[i].reset();
        }

        mPlayer.reset();
        mGameMode = NULL;
        mCameraEntity.reset();
        mEntityRenderDispatcher->mCamera.reset();
        BlockEntityRenderDispatcher::instance->mCamera.reset();
    }

    mLevel = level;

    if (level == NULL)
    {
        mLevelSource->requiresConversion(NULL, std::wstring());
        mPlayer.reset();

        for (int i = 0; i < 4; ++i)
        {
            if (mPendingConnections[i] != NULL)
                mPendingConnections[i]->close();
            mPendingConnections[i] = NULL;
            mLocalPlayers[i].reset();
            mGameModes[i] = NULL;
        }

        LeaveCriticalSection(&mLevelLock);
        return;
    }

    boost::shared_ptr<MultiplayerLocalPlayer> newPlayer =
        boost::static_pointer_cast<MultiplayerLocalPlayer>(player);
    const DimensionType* type = level->mDimension->getType();
    if (type == DimensionType::NETHER)
        mLevels[1] = level;
    else if (type == DimensionType::THE_END)
        mLevels[2] = level;
    else
        mLevels[0] = level;
    if (type != DimensionType::NETHER && type != DimensionType::THE_END && !newPlayer)
    {
        int userIndex = gProfile->GetPrimaryPad();
        newPlayer = boost::static_pointer_cast<MultiplayerLocalPlayer>(mGameMode->createPlayer(level));
        mPlayer = newPlayer;

        PlayerUID xuid(0);
        PlayerUID suid(0);
        gProfile->GetXUID(userIndex, &xuid, false);
        gProfile->GetXUID(userIndex, &suid, true);
        newPlayer->setXuid(xuid);
        newPlayer->mSuid = suid;

        newPlayer->setName(convStringToWstring(gProfile->GetGamertag(userIndex)));
        newPlayer->mDisplayName = gProfile->GetDisplayName(userIndex);
        if (!newPlayer->mIsGuest)
            newPlayer->setUUID(newPlayer->mDisplayName);

        newPlayer->mUserIndex = userIndex;
        newPlayer->resetPos();
        mGameMode->initPlayer(newPlayer);

        for (int i = 0; i < 4; ++i)
        {
            mPendingConnections[i] = NULL;
            if (userIndex != i)
                mGameModes[i] = NULL;
        }
    }

    newPlayer->resetPos();
    level->addEntity(newPlayer);

    if (newPlayer->mInput != NULL)
        delete newPlayer->mInput;
    newPlayer->mInput = new Input();

    if (mLevelRenderer != NULL)
        mLevelRenderer->setLevel(newPlayer->mUserIndex, level);
    if (mParticleEngine != NULL)
        mParticleEngine->setLevel(level);

    mGameMode->adjustPlayer(newPlayer);

    mPendingConnections[0] = NULL;
    mPendingConnections[1] = NULL;
    mPendingConnections[2] = NULL;
    mPendingConnections[3] = NULL;

    updatePlayerViewportAssignments();
    mCameraEntity = mPlayer;
    mGameRenderer->EnableUpdateThread(0);

    LeaveCriticalSection(&mLevelLock);
}

void Minecraft::levelTickUpdateFunc(void* level)
{
    ((MultiPlayerLevel*)level)->tick();
}

void Minecraft::checkViewportAssignment(int playerCount)
{
    int primary = gProfile->GetPrimaryPad();
    if (playerCount < 2)
        return;

    if (playerCount == 2)
    {
        bool verticalSplit = GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), (eGameSetting)0xb) != 0;
        if (mLocalPlayers[primary]->mViewport == 3 || !verticalSplit)
        {
            if (mLocalPlayers[primary]->mViewport == 1)
                return;
            if (verticalSplit)
                return;
        }

        for (int i = 0; i < 4; ++i)
        {
            if (mLocalPlayers[i] && i != primary)
            {
                int viewport = mLocalPlayers[primary]->mViewport;
                mLocalPlayers[primary]->mViewport = mLocalPlayers[i]->mViewport;
                mLocalPlayers[i]->mViewport = viewport;
                return;
            }
        }
        return;
    }

    if (playerCount > 4)
        return;

    if (mLocalPlayers[primary]->mViewport == 5)
        return;

    for (int i = 0; i < 4; ++i)
    {
        if (mLocalPlayers[i] && i != primary && mLocalPlayers[i]->mViewport == 5)
        {
            int viewport = mLocalPlayers[primary]->mViewport;
            mLocalPlayers[primary]->mViewport = mLocalPlayers[i]->mViewport;
            mLocalPlayers[i]->mViewport = viewport;
            return;
        }
    }
}

void Minecraft::updatePlayerViewportAssignments()
{
    mEmptyViewport = -1;

    int playerCount = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (mLocalPlayers[i])
            ++playerCount;
    }

    if (playerCount == 1)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (mLocalPlayers[i])
                mLocalPlayers[i]->mViewport = 0;
        }
    }
    else if (playerCount == 2)
    {
        int viewport = 0;
        for (int i = 0; i < 4; ++i)
        {
            if (!mLocalPlayers[i])
                continue;
            if (GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), (eGameSetting)0xb) == 0)
            {
                ++viewport;
                mLocalPlayers[i]->mViewport = viewport;
            }
            else
            {
                mLocalPlayers[i]->mViewport = viewport + 3;
                ++viewport;
            }
        }
    }
    else if (playerCount > 2)
    {
        bool quadrantUsed[4];
        quadrantUsed[0] = false;
        quadrantUsed[1] = false;
        quadrantUsed[2] = false;
        quadrantUsed[3] = false;

        for (int i = 0; i < 4; ++i)
        {
            if (!mLocalPlayers[i])
                continue;
            if (!gApp.GetGameStarted())
            {
                mLocalPlayers[i]->mViewport = 0;
                continue;
            }
            int viewport = mLocalPlayers[i]->mViewport;
            if (viewport >= 5 && viewport <= 8)
                quadrantUsed[viewport - 5] = true;
        }

        for (int i = 0; i < 4; ++i)
        {
            if (!mLocalPlayers[i])
                continue;
            int viewport = mLocalPlayers[i]->mViewport;
            if (viewport >= 5 && viewport <= 8)
                continue;
            for (int quadrant = 0; quadrant < 4; ++quadrant)
            {
                if (!quadrantUsed[quadrant])
                {
                    quadrantUsed[quadrant] = true;
                    mLocalPlayers[i]->mViewport = quadrant + 5;
                    break;
                }
            }
        }

        for (int quadrant = 0; quadrant < 4; ++quadrant)
        {
            if (!quadrantUsed[quadrant])
                mEmptyViewport = quadrant;
        }
    }

    checkViewportAssignment(playerCount);

    if (gApp.GetGameStarted())
        gConsoleUIController.UpdatePlayerBasePositions();
}

bool Minecraft::addLocalPlayer(int playerIdx)
{
    if (mPendingConnections[playerIdx] != NULL)
        mPendingConnections[playerIdx]->close();
    mConnectionFailed[playerIdx] = false;
    mPendingConnections[playerIdx] = NULL;

    bool added = CGameNetworkManager::sInstance.AddLocalPlayerByUserIndex(playerIdx);
    gApp.SetDRCCheckPlayerCountEnabled(true);
    if (!added)
    {
        gProfile->SignOutUser(playerIdx);
        return false;
    }

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer(
        new MultiplayerLocalPlayer(this, mLevel, mUser, NULL));
    mLocalPlayers[playerIdx] = localPlayer;
    mGameModes[playerIdx] = NULL;
    updatePlayerViewportAssignments();

    _ConnectionProgressParams* params = new _ConnectionProgressParams();
    params->m_connectionId = playerIdx;
    params->m_userIndex = -0x3AD04E98;
    params->m_isJoining = true;
    params->m_progress = 500;
    gConsoleUIController.NavigateToScene(playerIdx, (EUIScene)0x1f, params, (EUILayer)6, (EUIGroup)7);
    return true;
}

void Minecraft::addPendingLocalConnection(int playerIdx, ClientPacketListener* connection)
{
    mPendingConnections[playerIdx] = connection;
}

const MiniGameDef* Minecraft::GetMiniGame()
{
    return mMiniGame;
}

ClientMasterGameMode* Minecraft::GetClientMasterGameMode()
{
    return mClientMasterGameMode;
}

bool Minecraft::InMiniGame(EMiniGameId miniGameId, bool includeNextGameMode)
{
    if (miniGameId == (EMiniGameId)15)
        return gApp.GetGameHostOption((eGameHostOption)0x1f) != 0;

    const MiniGameDef* miniGame = GetInstance()->GetMiniGame();
    if (includeNextGameMode && miniGameId != (EMiniGameId)0 && miniGame->GetId() == (EMiniGameId)0)
        return GetInstance()->GetClientMasterGameMode()->GetNextGameMode() == miniGameId;

    return miniGame->GetId() == miniGameId;
}

EMiniGameId Minecraft::GetMiniGameId()
{
    EMiniGameId id = static_cast<EMiniGameId>(GetInstance()->GetMiniGame()->GetId());
    if (id == eMiniGameId_Lobby)
        return static_cast<EMiniGameId>(GetInstance()->GetClientMasterGameMode()->GetNextGameMode());
    return id;
}

void Minecraft::SetupMiniGameInstance(const MiniGameDef& miniGame, int variant)
{
    mMiniGame = &miniGame;
}

const MiniGameDef* Minecraft::GetFirstMiniGame()
{
    return MiniGameDef::GetCustomGameModeById(gApp.GetFirstMiniGameType(), true);
}

void Minecraft::playerStartedTutorial(int playerIdx)
{
    if (gApp.GetTutorialMode(boost::shared_ptr<Player>()))
        mTutorialPlayers = (unsigned char)(mTutorialPlayers | (1 << playerIdx));
}

void Minecraft::playerLeftTutorial(int playerIdx)
{
    if (mTutorialPlayers == 0)
    {
        gApp.SetTutorialMode(false, boost::shared_ptr<Player>());
        return;
    }

    mTutorialPlayers = (unsigned char)(mTutorialPlayers & ~(1 << playerIdx));
    if (mTutorialPlayers != 0)
        return;

    gApp.SetTutorialMode(false, boost::shared_ptr<Player>());

    for (int i = 0; i < 4; ++i)
    {
        if (!mLocalPlayers[i])
            continue;
        telemetryManagerInstance->RecordLevelStart(i, eSen_FriendOrMatch_Friend, eSen_CompeteOrCoop_Both,
                                                   mLevel->getDifficulty()->getId(), gApp.GetLocalPlayerCount(),
                                                   CGameNetworkManager::sInstance.GetOnlinePlayerCount());
    }
}

boost::shared_ptr<Entity> Minecraft::getCameraEntity()
{
    return mCameraEntity;
}

void Minecraft::setCameraEntity(boost::shared_ptr<Entity> entity)
{
    mCameraEntity = entity;
}

boost::shared_ptr<MultiplayerLocalPlayer> Minecraft::createExtraLocalPlayer(int entityId, const std::wstring& name,
                                                                           int playerIdx, int userIndex,
                                                                           ClientPacketListener* connection,
                                                                           MultiPlayerLevel* level, int dimension,
                                                                           int gameTypeId)
{
    if (connection == NULL)
        return boost::shared_ptr<MultiplayerLocalPlayer>();

    if (connection != mPendingConnections[playerIdx])
        return mLocalPlayers[playerIdx];

    int viewport = 0;
    if (mLocalPlayers[playerIdx] && mGameModes[playerIdx] == NULL)
        viewport = mLocalPlayers[playerIdx]->mViewport;

    std::wstring previousName = mUser->name;
    mUser->name = name;
    mPendingConnections[playerIdx] = NULL;

    PlayerUID xuid(0);
    PlayerUID suid(0);
    gProfile->GetXUID(playerIdx, &xuid, false);
    gProfile->GetXUID(playerIdx, &suid, true);

    bool tutorialForUid = gApp.GetTutorialMode(suid != PlayerUID(0) ? suid : xuid);
    bool tutorialMode = gApp.GetTutorialMode(boost::shared_ptr<Player>());

    if (level == NULL)
    {
        mLevel = getLevel(dimension);
        getLevel(dimension)->mConnections.push_back(connection);
    }
    else
    {
        mLevel = level;
    }

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer;
    if (tutorialMode && !tutorialForUid)
    {
        mGameModes[playerIdx] = new MultiPlayerGameMode(playerIdx, connection, userIndex, true, false);
        localPlayer = boost::static_pointer_cast<MultiplayerLocalPlayer>(
            mGameModes[playerIdx]->createPlayer(mLevel));
    }
    else if (!gProfile->IsFullVersion())
    {
        mGameModes[playerIdx] = new TrialMode(playerIdx, connection, userIndex);
        localPlayer = boost::static_pointer_cast<MultiplayerLocalPlayer>(
            mGameModes[playerIdx]->createPlayer(mLevel));
    }
    else
    {
        mGameModes[playerIdx] =
            new MultiPlayerGameMode(playerIdx, connection, userIndex, false, GameType::byId(gameTypeId) == GameType::CREATIVE);
        localPlayer = boost::static_pointer_cast<MultiplayerLocalPlayer>(
            mGameModes[playerIdx]->createPlayer(mLevel));
    }

    mLocalPlayers[playerIdx] = localPlayer;

    localPlayer->mIsGuest = gProfile->IsGuest(playerIdx);
    localPlayer->setXuid(xuid);
    localPlayer->mSuid = suid;
    localPlayer->setName(convStringToWstring(gProfile->GetGamertag(playerIdx)));
    localPlayer->mDisplayName = gProfile->GetDisplayName(playerIdx);
    if (!localPlayer->mIsGuest && suid == PlayerUID(0))
        localPlayer->setUUID(localPlayer->mDisplayName);

    localPlayer->mViewport = viewport;

    if (level == NULL)
        mLevel->addEntity(localPlayer);

    localPlayer->mUserIndex = userIndex;

    if (localPlayer->mInput != NULL)
        delete localPlayer->mInput;
    localPlayer->mInput = new Input();

    if (GetInstance()->GetClientMasterGameMode()->IsInActiveRoundState())
        localPlayer->SetGameType(GameType::ADVENTURE);

    localPlayer->resetPos();
    mLevelRenderer->setLevel(playerIdx, mLevel);
    localPlayer->mLevel = mLevel;

    mUser->name = previousName;
    updatePlayerViewportAssignments();

    if (tutorialMode && !tutorialForUid)
    {
        if (mGameModes[playerIdx]->getGameModeConstraints()->canMoveToPosition(
                localPlayer->mX, localPlayer->mY, localPlayer->mZ, localPlayer->mX, localPlayer->mY, localPlayer->mZ,
                NULL))
        {
            boost::shared_ptr<MultiplayerLocalPlayer> primary =
                GetInstance()->mLocalPlayers[gProfile->GetPrimaryPad()];
            localPlayer->teleportTo(primary->mX, primary->mY, primary->mZ);
        }
    }

    return mLocalPlayers[playerIdx];
}

void Minecraft::createPrimaryLocalPlayer(int playerIdx)
{
    mGameModes[playerIdx] = mGameMode;
    mLocalPlayers[playerIdx] = boost::dynamic_pointer_cast<MultiplayerLocalPlayer>(mPlayer);
    mLocalItemInHandRenderers[playerIdx]->Reset();

    if (gProfile->IsSignedInLive(gProfile->GetPrimaryPad()))
        mUser->name = convStringToWstring(gProfile->GetGamertag(gProfile->GetPrimaryPad()));
}

bool Minecraft::setLocalPlayerIdx(int playerIdx)
{
    mLocalPlayerIdx = playerIdx;
    if (!mLocalPlayers[playerIdx] || mGameModes[playerIdx] == NULL)
        return false;

    mGameMode = mGameModes[playerIdx];
    mPlayer = mLocalPlayers[playerIdx];
    mCameraEntity = mLocalPlayers[playerIdx]->GetCamera();
    mGameRenderer->setItemInHandRenderer(mLocalItemInHandRenderers[playerIdx]);
    mLevel = getLevel(mLocalPlayers[playerIdx]->mDimension);
    mParticleEngine->setLevel(mLevel);
    return true;
}

int Minecraft::getLocalPlayerIdx()
{
    return mLocalPlayerIdx;
}

void Minecraft::removeLocalPlayerIdx(int playerIdx)
{
    if (mGameModes[playerIdx] == NULL)
    {
        if (mPendingConnections[playerIdx] != NULL)
        {
            mPendingConnections[playerIdx]->sendAndDisconnect(
                boost::shared_ptr<Packet>(new DisconnectPacket(DisconnectPacket::eDisconnectReason_Quit)));

            if (gApp.GetGameStarted())
            {
                CGameNetworkManager::sInstance.RemoveCreatedConnection(mPendingConnections[playerIdx]);
                delete mPendingConnections[playerIdx];
            }
            mPendingConnections[playerIdx] = NULL;
            CGameNetworkManager::sInstance.RemoveLocalPlayerByUserIndex(playerIdx);
        }
    }
    else
    {
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mLocalPlayers[playerIdx];
        MultiPlayerLevel* level = getLevel(localPlayer->mDimension);
        if (level->mIsClientSide)
        {
            getLevel(localPlayer->mDimension)->removeClientPacketListener(localPlayer->mPacketListener, true);
            CGameNetworkManager::sInstance.RemoveCreatedConnection(localPlayer->mPacketListener);
            delete localPlayer->mPacketListener;
            localPlayer->mPacketListener = NULL;
            CGameNetworkManager::sInstance.RemoveLocalPlayerByUserIndex(playerIdx);
        }

        getLevel(localPlayer->mDimension)->removeEntity(localPlayer);
        playerLeftTutorial(playerIdx);
        delete mGameModes[playerIdx];
        mGameModes[playerIdx] = NULL;
    }

    mLocalPlayers[playerIdx].reset();

    if (playerIdx != gProfile->GetPrimaryPad())
    {
        mGameRenderer->DisableUpdateThread(5);
        mLevelRenderer->setLevel(playerIdx, NULL);
        mGameRenderer->EnableUpdateThread(5);
        gConsoleUIController.CloseUIScenes(playerIdx, true);
        updatePlayerViewportAssignments();
        mGui->getBossOverlay(playerIdx)->reset();
    }
}

void Minecraft::storeExtraLocalPlayer(int playerIdx)
{
    mLocalPlayers[playerIdx] = boost::dynamic_pointer_cast<MultiplayerLocalPlayer>(mPlayer);

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mLocalPlayers[playerIdx];
    if (localPlayer->mInput != NULL)
        delete localPlayer->mInput;
    localPlayer->mInput = new Input();

    if (gProfile->IsSignedIn(playerIdx))
        localPlayer->setName(convStringToWstring(gProfile->GetGamertag(playerIdx)));

    mLocalItemInHandRenderers[playerIdx]->Reset();
}

void Minecraft::clearConnectionFailed()
{
    mConnectionFailedTitle = 0;
    mConnectionFailed[3] = false;
    mConnectionFailed[1] = false;
    mConnectionFailed[0] = false;
    mConnectionFailedMessage = 0;
    mConnectionFailedPlayerIdx = 0;
    mConnectionFailed[2] = false;
    mConnectionFailedParameter = 0;
    mDisconnectReasons[0] = DisconnectPacket::eDisconnectReason_None;
    mDisconnectReasons[1] = DisconnectPacket::eDisconnectReason_None;
    mDisconnectReasons[2] = DisconnectPacket::eDisconnectReason_None;
    mDisconnectReasons[3] = DisconnectPacket::eDisconnectReason_None;
    gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_None);
}

void Minecraft::forceStatsSave(int playerIdx)
{
    mStats[playerIdx]->save(playerIdx, true, NULL);
    if (gProfile->IsSignedInLive(playerIdx))
    {
        int lockedProfile = gProfile->GetLockedProfile();
        gProfile->SetLockedProfile(playerIdx);
        mStats[playerIdx]->saveLeaderboards();
        gProfile->SetLockedProfile(lockedProfile);
    }
}

void Minecraft::forceLeaderboardSave(int playerIdx)
{
    if (gProfile->IsSignedInLive(playerIdx))
    {
        int lockedProfile = gProfile->GetLockedProfile();
        gProfile->SetLockedProfile(playerIdx);
        mStats[playerIdx]->saveLeaderboards();
        gProfile->SetLockedProfile(lockedProfile);
    }
}

int Minecraft::InGame_SignInReturned(void* userData, bool signedIn, int userIndex, int playerIdx)
{
    Minecraft* minecraft = (Minecraft*)userData;

    if (CGameNetworkManager::sInstance.IsInSession())
        XEnableGuestSignin(0);

    if (!signedIn || !CGameNetworkManager::sInstance.IsInSession())
    {
        gApp.SetDRCCheckPlayerCountEnabled(true);
        return 0;
    }

    if (minecraft->mLocalPlayers[playerIdx])
    {
        gApp.SetDRCCheckPlayerCountEnabled(true);
        return 0;
    }

    if (gConsoleUIController.IsIgnorePlayerJoinMenuDisplayed(gProfile->GetPrimaryPad()))
    {
        gApp.SetDRCCheckPlayerCountEnabled(true);
        return 0;
    }

    if (!gProfile->IsSignedIn(playerIdx))
        return 0;

    if (!CGameNetworkManager::sInstance.SessionHasSpace(1))
    {
        int messageParameters[1];
        messageParameters[0] = 0xB9A1BF4D;
        gConsoleUIController.RequestErrorMessage(0x1AFA54B5, 0xF18FCCD0, messageParameters, 1, 0xff, NULL, NULL,
                                                  NULL, true);
        gProfile->SignOutUser(playerIdx);
        gApp.SetDRCCheckPlayerCountEnabled(true);
        return 0;
    }

    if (CGameNetworkManager::sInstance.SessionHasMii(playerIdx))
    {
        int messageParameters[1];
        messageParameters[0] = 0xB9A1BF4D;
        gConsoleUIController.RequestErrorMessage(0x624AEAD3, 0x3103BE06, messageParameters, 1, 0xff, NULL, NULL,
                                                  NULL, true);
        gProfile->SignOutUser(playerIdx);
        gApp.SetDRCCheckPlayerCountEnabled(true);
        return 0;
    }

    if (!CGameNetworkManager::sInstance.IsLocalGame() &&
        (!gProfile->IsSignedInLive(playerIdx) || !gProfile->AllowedToPlayMultiplayer(playerIdx)))
    {
        if (!gProfile->IsSignedInLive(gProfile->GetPrimaryPad()))
            return 0;
        if (gProfile->AllowedToPlayMultiplayer(playerIdx))
            return 0;

        int messageParameters[1];
        messageParameters[0] = 0xB9A1BF4D;
        gConsoleUIController.RequestErrorMessage(0x252AB13F, 0xCC5A590F, messageParameters, 1, playerIdx, NULL,
                                                  NULL, NULL, true);
        gProfile->SignOutUser(playerIdx);
        gApp.SetDRCCheckPlayerCountEnabled(true);
        return 0;
    }

    if (minecraft->mLevel->mIsClientSide)
    {
        minecraft->addLocalPlayer(playerIdx);
        return 0;
    }

    minecraft->createExtraLocalPlayer(playerIdx, convStringToWstring(gProfile->GetGamertag(playerIdx)), playerIdx,
                                      playerIdx, NULL, NULL,
                                      minecraft->mLevel->mDimension->getType()->getId(), 0);
    return 0;
}

boost::shared_ptr<ItemInstance> Minecraft::addCustomNbtData(boost::shared_ptr<ItemInstance> item,
                                                            boost::shared_ptr<BlockEntity> blockEntity)
{
    CompoundTag* data = new CompoundTag();
    blockEntity->save(data);

    if (item->getItem() == Items::skull && data->contains(L"Owner"))
    {
        CompoundTag* owner = data->getCompound(L"Owner");
        CompoundTag* tag = new CompoundTag();
        tag->put(L"SkullOwner", owner);
        item->setTag(tag);
        return item;
    }

    item->addTagElement(L"BlockEntityTag", data);

    CompoundTag* display = new CompoundTag();
    ListTag* lore = new ListTag(0);
    lore->add(new StringTag(L"(+NBT)"));
    display->put(L"Lore", lore);
    item->addTagElement(L"display", display);
    return item;
}

void Minecraft::tickAllConnections()
{
    int previousIdx = getLocalPlayerIdx();

    for (int i = 0; i < 4; ++i)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mLocalPlayers[i];
        if (localPlayer && localPlayer->mPacketListener != NULL)
        {
            setLocalPlayerIdx(i);
            localPlayer->mPacketListener->tick();
        }
    }

    setLocalPlayerIdx(previousIdx);
}

void Minecraft::tick(bool handleInput, bool lastTickOfFrame)
{
    int playerIdx = mPlayer->mUserIndex;

    mStats[playerIdx]->tick(playerIdx);
    gApp.TickOpacityTimer(playerIdx);

    if (handleInput)
        mLevelRenderer->mDestroyedBlocks->tick();

    for (std::list<GhostController*>::iterator it = mFinishedGhostControllers.begin();
         it != mFinishedGhostControllers.end();)
    {
        if ((*it)->canBeDeleted())
        {
            delete *it;
            it = mFinishedGhostControllers.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (InMiniGame(eMiniGameId_Glide, true) && mGhostController != NULL)
        mGhostController->tick();

    mGui->tick();
    mGameRenderer->pick(1.0f);

    if (!mAppPaused && mLevel != NULL)
        mGameMode->tick();

    MemSect(0x1f);
    GlStateManager::bindTexture(mTextures->loadTexture(0xf1));
    MemSect(0);

    if (handleInput)
    {
        PIXBeginNamedEvent(0.0f, "Texture tick");
        mTextures->tick(lastTickOfFrame, mAppPaused, true);
        PIXEndNamedEvent();
    }

    if (mScreen == NULL)
    {
        if (mPlayer && mPlayer->getHealth() <= 0.0f)
            setScreen(NULL);
    }
    else
    {
        mMissTime = mTicks + 10000;
        mScreen->mGuiParticles->tick();
        mScreen->tick();
        if (mScreen == NULL)
            setScreen(NULL);
    }

    if (mScreen == NULL && !gConsoleUIController.GetMenuDisplayed(playerIdx) && !gApp.DebugUIFocussed())
    {
        if ((mPlayer->mButtonsDown & 0x200) != 0 &&
            mGameMode->isInputAllowed((EControllerActions)0x29) && mPlayer->CanChangeThirdPersonView())
        {
            mPlayer->SetThirdPersonView((mPlayer->GetThirdPersonView(false) + 1) % 3);
        }

        if (mGameMode->isInputAllowed((EControllerActions)0x22))
        {
            if ((mPlayer->mButtonsDown & 4) != 0)
            {
                mPlayer->handleMouseClick(0);
                mPlayer->mLastAttackTick = mTicks;
            }
            else if (g_pInput->ButtonDown(playerIdx, 0x22) &&
                     (float)(mTicks - mPlayer->mLastAttackTick) >= mTimer->ticksPerSecond * 0.25f)
            {
                mPlayer->handleMouseClick(0);
                mPlayer->mLastAttackTick = mTicks;
            }

            mPlayer->handleMouseDown(0, g_pInput->ButtonDown(playerIdx, 0x22));
        }

        if ((mPlayer->mButtonsPressed & 0x400000) != 0)
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
            gConsoleUIController.NavigateToScene(playerIdx, (EUIScene)5, NULL, (EUILayer)6, (EUIGroup)7);
        }

        bool waitingForScoreboard = mPlayer->IsWaitingForScoreboard();
        bool showScoreboard = false;

        if ((!mPlayer->mHasOpenedInventory || gConsoleUIController.getCleanupOnReload()) &&
            (mClientMasterGameMode == NULL ||
             (!mClientMasterGameMode->IsInScoringState() && !mClientMasterGameMode->IsWaitingToStart() &&
              !mClientMasterGameMode->IsRoundStarting())))
        {
            if (!gConsoleUIController.IsSceneInStack(0xff, (EUIScene)4))
            {
                if (waitingForScoreboard)
                    showScoreboard = true;
                else if ((mPlayer->mButtonsPressed & 0x800000) != 0 &&
                         mGameMode->isInputAllowed((EControllerActions)0x17))
                    showScoreboard = true;
            }
        }

        if (showScoreboard)
        {
            mPlayer->SetWaitingForScoreboard(false);
            if (!InMiniGame(eMiniGameId_Any, true))
            {
                gConsoleUIController.NavigateToScene(playerIdx, (EUIScene)0x1e, NULL, (EUILayer)6, (EUIGroup)7);
            }
            else if (!gConsoleUIController.IsSceneInStack(playerIdx, (EUIScene)0x3c))
            {
                int* sceneData = new int[2];
                sceneData[0] = 0;
                sceneData[1] = (mPlayer->GetGameType() == GameType::CREATIVE) ? 1 : 0;
                gConsoleUIController.NavigateToScene(playerIdx, (EUIScene)0x3c, sceneData, (EUILayer)6,
                                                      (EUIGroup)7);

                if (mGameModes[playerIdx]->getTutorial() != NULL)
                    mGameModes[playerIdx]->getTutorial()->onGameModeChange((eTutorial_GameMode)9);

                if (InMiniGame(eMiniGameId_Glide, true))
                {
                    std::wstring winner = mLevel->GetClientScoreboard()->GetWinnerName(-1);
                    if (winner.compare(0, winner.size(), mPlayer->getDisplayName().c_str(),
                                       mPlayer->getDisplayName().size()) == 0 && waitingForScoreboard)
                    {
                        mSoundEngine->playUI(SoundEvents::MG03_WINNER_ANNOUNCE, 1.0f, 1.0f);
                    }
                }
                gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
            }
            else
            {
                gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
            }
        }

        if (mPlayer->isSpectator())
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mLocalPlayers[playerIdx];
            MultiPlayerLevel* level = localPlayer->GetLevel();
            if (level != NULL && level->GetClientScoreboard() != NULL &&
                level->GetClientScoreboard()->HasWinner(-1))
            {
                Tooltips::ResetAllTooltips(playerIdx);
            }
            else
            {
                int activePlayers =
                    GetInstance()->mLevel->GetClientScoreboard()->GetActivePlayerCount(eMiniGameId_Any);
                if (!GetClientMasterGameMode()->IsRoundEnding() && activePlayers > 1)
                {
                    Tooltips::OverwriteTooltip(playerIdx, (EControllerActions)0x30, 0x2ECE48BA);
                    Tooltips::OverwriteTooltip(playerIdx, (EControllerActions)0x31, 0x600DA5D4);

                    if (g_pInput->GetValue(playerIdx, 0x30, true) &&
                        mGameMode->isInputAllowed((EControllerActions)0x30))
                    {
                        mPlayer->SpectatePlayerPrev();
                    }
                    else if (g_pInput->GetValue(playerIdx, 0x31, true) &&
                             mGameMode->isInputAllowed((EControllerActions)0x31))
                    {
                        mPlayer->SpectatePlayerNext();
                    }
                }
                else
                {
                    Tooltips::ResetTooltip(playerIdx, (EControllerActions)0x30);
                    Tooltips::ResetTooltip(playerIdx, (EControllerActions)0x31);
                }

                if (mPlayer->IsSpectatingOtherPlayer() && !GetMiniGame()->ForceSpectateModeFollow())
                {
                    Tooltips::OverwriteTooltip(playerIdx, (EControllerActions)0x33, 0x4F6BCF5E);
                    Tooltips::ResetTooltip(playerIdx, (EControllerActions)0x22);
                    Tooltips::ResetTooltip(playerIdx, (EControllerActions)0x32);
                    if (g_pInput->GetValue(playerIdx, 0x33, true) &&
                        mGameMode->isInputAllowed((EControllerActions)0x33))
                    {
                        mGui->clearSingleMessages(playerIdx);
                        mPlayer->StopSpectatingPlayer();
                    }
                }
                else if (mPlayer->SpectateAsOtherEntity() || mPlayer->SpectateAsInvisible() ||
                         mPlayer->SpectateAsFloatingHead())
                {
                    Tooltips::ResetTooltip(playerIdx, (EControllerActions)0x33);

                    if (GetInstance()->GetMiniGame()->HasSpectatorParticipation() && mHitResult != NULL &&
                        mHitResult->mType == 1 &&
                        GetInstance()->mGameModes[playerIdx]->MayDestroy(mHitResult->getBlockPos()) &&
                        mPlayer->isAllowedToMine())
                    {
                        Tooltips::OverwriteTooltip(playerIdx, (EControllerActions)0x22, 0x5FB7A25A);
                        Tooltips::ResetTooltip(playerIdx, (EControllerActions)0x32);
                    }
                    else if (GetInstance()->mGameModes[playerIdx]->SpectateSoundEnabled())
                    {
                        Tooltips::ResetTooltip(playerIdx, (EControllerActions)0x22);
                        unsigned long long spectateEntity = mPlayer->GetSpectateEntity();
                        if (spectateEntity == 0x5D540048ULL)
                            Tooltips::OverwriteTooltip(playerIdx, (EControllerActions)0x32, 0x9D34B5A5);
                        else if (spectateEntity == 0x2D6C0003ULL)
                            Tooltips::OverwriteTooltip(playerIdx, (EControllerActions)0x32, 0xC1781F1C);
                        else if (spectateEntity == 0x1D000101ULL)
                            Tooltips::OverwriteTooltip(playerIdx, (EControllerActions)0x32, 0x162E9829);
                        else
                            Tooltips::ResetTooltip(playerIdx, (EControllerActions)0x32);

                        if (GetInstance()->mGameModes[playerIdx]->CanMakeSpectateSound() &&
                            g_pInput->GetValue(playerIdx, 0x32, true) &&
                            mGameMode->isInputAllowed((EControllerActions)0x32))
                        {
                            GetInstance()->mGameModes[playerIdx]->MakeSpectateSound();
                        }
                    }
                }
                else
                {
                    Tooltips::ResetTooltip(playerIdx, (EControllerActions)0x33);
                    Tooltips::ResetTooltip(playerIdx, (EControllerActions)0x22);
                    Tooltips::ResetTooltip(playerIdx, (EControllerActions)0x32);
                }
            }
            Tooltips::DisplayTooltips(playerIdx);
        }
        else
        {
            Tooltips::ResetTooltip(playerIdx, (EControllerActions)0x33);
            Tooltips::ResetTooltip(playerIdx, (EControllerActions)0x31);
            Tooltips::ResetTooltip(playerIdx, (EControllerActions)0x30);
            Tooltips::ResetTooltip(playerIdx, (EControllerActions)0x32);

            InteractionResult::EInteractionResult interaction = InteractionResult::eInteractionResult_Pass;
            if (mHitResult == NULL || mHitResult->mType == 0)
            {
                interaction = mGameMode->useItem(mPlayer, mLevel, InteractionHand::eInteractionHand_MainHand, true);
            }
            else if (mHitResult->mType == 1)
            {
                const BlockPos blockPos = mHitResult->getBlockPos();
                const BlockState* blockState = mLevel->getBlockState(blockPos);
                interaction = mGameMode->useItemOn(mPlayer, mLevel, blockPos, mHitResult->mDirection,
                                                   mHitResult->mLocation,
                                                   InteractionHand::eInteractionHand_MainHand, true, NULL);

                if (mGameMode != NULL && mGameMode->getTutorial() != NULL)
                {
                    if (blockState->getBlock() == Blocks::skull)
                        mGameMode->getTutorial()->onLookAtSkull(mLevel, blockPos, blockState);
                    else
                        mGameMode->getTutorial()->onLookAt(blockState);
                }
            }
            else if (mHitResult->mType == 2)
            {
                if (mGameMode != NULL && mGameMode->getTutorial() != NULL)
                    mGameMode->getTutorial()->onLookAtEntity(mHitResult->mEntity);
                interaction = mGameMode->useItem(mPlayer, mLevel, InteractionHand::eInteractionHand_MainHand, true);
            }

            Tooltips::SetInteractTooltips(mLocalPlayers[playerIdx], mLevel, mHitResult, interaction);

            int hotbarScroll = 0;
            if (g_pInput->GetValue(playerIdx, 0x23, true) && mGameMode->isInputAllowed((EControllerActions)0x23))
                hotbarScroll = 1;
            else if (g_pInput->GetValue(playerIdx, 0x24, true) &&
                     mGameMode->isInputAllowed((EControllerActions)0x24))
                hotbarScroll = -1;

            if (hotbarScroll != 0)
            {
                mPlayer->mInventory->swapPaint(hotbarScroll);

                if (mGameMode != NULL && mGameMode->getTutorial() != NULL)
                    mGameMode->getTutorial()->onSelectedItemChanged(mPlayer->mInventory->getSelected());

                mPlayer->updateRichPresence();

                if (mOptions->mScrollHotbar)
                    mOptions->mHotbarScroll += (float)hotbarScroll * 0.25f;
            }

            if (g_pInput->ButtonDown(playerIdx, 0x2f))
                mPlayer->handleMouseClick(2);

            if (!mPlayer->isUsingItem())
            {
                if (mGameMode->isInputAllowed((EControllerActions)0x21) &&
                    !g_pInput->ButtonDown(playerIdx, 0x22))
                {
                    if (!mPlayer->mAbilities.mInstabuild)
                    {
                        bool repeat = g_pInput->ButtonDown(playerIdx, 0x21);
                        if (!repeat)
                        {
                            mPlayer->mLastUseTick = 0;
                        }
                        else
                        {
                            if (mPlayer->isSleeping())
                                mPlayer->mLastUseTick = mTicks + (int)(mTimer->ticksPerSecond * 2.0f);

                            mPlayer->handleMouseClick(1);
                            if (!mPlayer->isSleeping())
                                mPlayer->mLastUseTick = mTicks;
                            else
                                mPlayer->mLastUseTick = mTicks + (int)(mTimer->ticksPerSecond * 2.0f);
                        }
                    }
                    else
                    {
                        bool used = mPlayer->creativeModeHandleMouseClick(1, g_pInput->ButtonDown(playerIdx, 0x21));
                        if (mPlayer->mCreativeRepeatState == 4 && !used && g_pInput->ButtonDown(playerIdx, 0x21) &&
                            (float)(mTicks - mPlayer->mLastUseTick) >= mTimer->ticksPerSecond * 0.25f)
                        {
                            mPlayer->handleMouseClick(1);
                            mPlayer->mLastUseTick = mTicks;
                        }
                    }
                }
            }
            else if (!g_pInput->ButtonDown(playerIdx, 0x21))
            {
                mGameMode->releaseUsingItem(mPlayer);
            }

            if (mPlayer->mDestroyTicks > 0)
                --mPlayer->mDestroyTicks;

            bool openInventory = false;
            if ((mPlayer->mButtonsDown & 0x20) == 0)
            {
                openInventory = gConsoleUIController.GetHoldActionCancelled(playerIdx);
            }
            else if (!gConsoleUIController.GetHoldToShowAchievementsActive(true) &&
                     !gConsoleUIController.GetAchievementToastShowing(playerIdx))
            {
                openInventory = !gConsoleUIController.GetHoldActionCancelled(playerIdx);
            }

            if (openInventory && !(mGameMode->isInputAllowed((EControllerActions)0x21) &&
                                   g_pInput->ButtonDown(playerIdx, 0x21)))
            {
                if (mPlayer->mHasOpenedInventory && mGameMode->isInputAllowed((EControllerActions)0x25))
                {
                    gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
                    if (mGameMode->isServerControlledInventory())
                        gApp.LoadInventoryMenu(playerIdx, mPlayer, false);
                    else
                        mPlayer->sendOpenInventory();
                }
            }

            bool canCraft = InMiniGame(eMiniGameId_Any, true) ||
                            GetInstance()->GetMiniGame()->CanCraft();
            if (canCraft && (mPlayer->mButtonsDown & 0x100) != 0 &&
                mGameMode->isInputAllowed((EControllerActions)0x28))
            {
                if (mGameMode->hasInfiniteItems())
                {
                    gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
                    gApp.LoadCreativeMenu(playerIdx, mPlayer, false);
                }
                else if (mHitResult != NULL && mHitResult->mType == 1 &&
                         mLevel->getBlockState(mHitResult->getBlockPos())->getBlock() == Blocks::crafting_table)
                {
                    bool cancelled = false;
                    mGameMode->useItemOn(mPlayer, mLevel, mHitResult->getBlockPos(), Direction::UP,
                                         mHitResult->mLocation, InteractionHand::eInteractionHand_MainHand, false,
                                         &cancelled);
                }
                else
                {
                    gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
                    gApp.LoadCrafting2x2Menu(playerIdx, mPlayer);
                }
            }

            if ((mPlayer->mButtonsDown & 0x100000) != 0 &&
                mGameMode->isInputAllowed((EControllerActions)0x34) &&
                mPlayer->GetGameType() == GameType::ADVENTURE)
            {
                mPlayer->mPacketListener->send(boost::shared_ptr<Packet>(
                    new PlayerReadyPacket(mPlayer->mPlayerIndex,
                                          !GetClientMasterGameMode()->IsPlayerReady(mPlayer->mPlayerIndex))));
            }

            if (!gConsoleUIController.GetMenuDisplayed(playerIdx) &&
                !GetInstance()->GetClientMasterGameMode()->IsInScoringState() &&
                (mPlayer->mButtonsDown & 0x200000) != 0 &&
                mGameMode->isInputAllowed((EControllerActions)0x35))
            {
                std::vector<ClientMasterGameMode::MapSelectInfo> playlist;
                GetClientMasterGameMode()->GetUniquePlaylist(playlist);

                bool canVote = mPlayer->GetGameType() == GameType::ADVENTURE;
                if (canVote && (playlist.empty() || playlist.size() < 2))
                    canVote = !InMiniGame(eMiniGameId_Battle, true) && GetMiniGame()->HasMultipleVariants();

                if (canVote)
                {
                    if (CGameNetworkManager::sInstance.IsInPublicJoinableGame())
                    {
                        if (gProfile->GetPrimaryPad() == playerIdx &&
                            CGameNetworkManager::sInstance.IsHost() &&
                            GetInstance()->mLocalPlayers[playerIdx]->GetGameType() == GameType::ADVENTURE &&
                            GetMiniGame()->HostOption_Bool((eCustomGameModeHostOption)7))
                        {
                            gConsoleUIController.NavigateToScene(playerIdx, (EUIScene)0x3e, (void*)1,
                                                                  (EUILayer)6, (EUIGroup)7);
                        }
                    }
                    else if (GetMiniGame()->HostOption_Bool((eCustomGameModeHostOption)7))
                    {
                        gConsoleUIController.NavigateToScene(playerIdx, (EUIScene)0x3e, NULL, (EUILayer)6,
                                                              (EUIGroup)7);
                    }
                }
            }

            if ((mPlayer->mButtonsDown & 0x40) != 0 && mGameMode->isInputAllowed((EControllerActions)0x26) &&
                !gConsoleUIController.GetMenuDisplayed(playerIdx))
            {
                mPlayer->drop();
            }

            int touchX = 0;
            int touchY = 0;
            bool touching = false;
            unsigned short* touchData = g_pInput->GetTouchPadData(playerIdx, (C_4JInput::ETouchType)0, true);
            if (touchData != NULL && touchData[2] == 1 && touchData[3] == 0)
            {
                touching = gConsoleUIController.GetDRCEnabled();
                touchX = touchData[0];
                touchY = touchData[1];
            }

            InMiniGame(eMiniGameId_Tumble, true);
            if (touching && gConsoleUIController.GetQuickSelectVisible(playerIdx))
            {
                int hotBarX = gConsoleUIController.GetHotBarX(playerIdx);
                int hotBarY = gConsoleUIController.GetHotBarY(playerIdx);
                int hotBarWidth = gConsoleUIController.GetHotBarWidth(playerIdx);
                int hotBarHeight = gConsoleUIController.GetHotBarHeight(playerIdx);
                if (touchX > hotBarX && touchX < hotBarX + hotBarWidth && touchY > hotBarY &&
                    touchY < hotBarY + hotBarHeight)
                {
                    int slot = (touchX - hotBarX) / (hotBarWidth / 9);
                    if (slot > 8)
                        slot = 8;
                    mPlayer->mInventory->m_selected = slot;
                }
            }

            std::wstring itemName(L"");
            boost::shared_ptr<ItemInstance> mainHandItem = mPlayer->getMainHandItem();
            if (mainHandItem && !mainHandItem->isEmpty())
            {
                if ((mPlayer->mButtonsDown & 0x40) == 0 || mainHandItem->getCount() != 1)
                    itemName = mainHandItem->getHoverName();

                std::vector<HtmlString>* tooltipLines = mainHandItem->getTooltipLines(mPlayer, false);
                if (!tooltipLines->empty())
                {
                    tooltipLines->at(0).mColour = static_cast<eMinecraftColour>(mainHandItem->getRarity());
                    if (mainHandItem->hasCustomHoverName())
                        tooltipLines->at(0).mColour = eMinecraftColour_Color_RenamedItemTitle;
                }

                for (int line = (int)tooltipLines->size() - 1; line >= 0; --line)
                {
                    tooltipLines->at(line).mIndent = false;
                    stripWhitespaceForHtml(tooltipLines->at(line).mText, true);
                    if (tooltipLines->at(line).mText == L"")
                        tooltipLines->erase(tooltipLines->begin() + line);
                }

                if (tooltipLines->size() > 4)
                    tooltipLines->push_back(HtmlString(L"...", eMinecraftColour_NOTSET, false, false, 0));

                gConsoleUIController.SetSelectedItem(playerIdx, itemName, HtmlString::Compose(tooltipLines));
                delete tooltipLines;
            }
            else
            {
                gConsoleUIController.SetSelectedItem(playerIdx, L"", L"");
            }
        }
    }

    if (mScreen != NULL)
        mScreen->tick();

    mGameMode->stopDestroyBlock();

    if (mPlayer && !mPlayer->isUsingItem())
        mLevel->ensureAdded(mPlayer);

    PIXBeginNamedEvent(0.0f, "Game renderer tick");
    if (!mAppPaused)
        mGameRenderer->tick(handleInput);
    PIXEndNamedEvent();

    SparseLightStorage::tick();
    CompressedBlockStorage::tick();
    SparseDataStorage::tick();
    mClientMasterGameMode->Tick();

    for (int i = 0; i < mLevelCount; ++i)
    {
        if (mLevels[i] == NULL)
            continue;

        PIXBeginNamedEvent(0.0f, "Level renderer tick");
        if (!mAppPaused)
            mLevelRenderer->tick();
        PIXEndNamedEvent();

        if (mGameRenderer->postEffectActive())
            mGameRenderer->shutdownEffect();

        if (!mAppPaused)
        {
            PIXBeginNamedEvent(0.0f, "Level entity tick");
            mLevels[i]->tickEntities();
            PIXEndNamedEvent();

            mLevels[i]->setSpawnSettings(mLevel->getDifficulty() != Difficulty::PEACEFUL, true);
            PIXBeginNamedEvent(0.0f, "Level tick");
            mLevels[i]->tick();
            PIXEndNamedEvent();
        }
    }

    PIXBeginNamedEvent(0.0f, "Particle tick");
    if (!mAppPaused)
        mParticleEngine->tick();
    PIXEndNamedEvent();

    if (mAppPaused)
        tickAllConnections();

    if (mPlayer && InMiniGame(eMiniGameId_Glide, false))
    {
        MultiPlayerLevel* level = mLocalPlayers[playerIdx]->GetLevel();
        ClientScoreboard* scoreboard = level->GetClientScoreboard();
        int checkpointPlayers = 0;
        for (int i = 0; i < 4; ++i)
        {
            if (mLocalPlayers[i] &&
                (mLocalPlayers[i]->GetScoreboard() == NULL ||
                 scoreboard->GetLastCheckpointID(mLocalPlayers[i]->getSUID(), -1) != 0))
            {
                ++checkpointPlayers;
            }
        }

        if (checkpointPlayers == 1 && GetMiniGame()->GetHostOption((eCustomGameModeHostOption)0xe) != 0 &&
            GetClientMasterGameMode()->getScoreboard() != NULL &&
            scoreboard->GetLastCheckpointID(mPlayer->getSUID(), -1) == 0)
        {
            bool resetTraversal = false;
            if ((mPlayer->mButtonsPressed & 0x400000) != 0)
            {
                mPlayer->mPacketListener->send(CheckpointCommand::preparePacket(mPlayer->getSUID(), -1));
                resetTraversal = true;
            }
            if ((mPlayer->mButtonsPressed & 0x800000) != 0)
            {
                mPlayer->mPacketListener->send(CheckpointCommand::preparePacket(mPlayer->getSUID(), 0));
                resetTraversal = true;
            }
            if (resetTraversal)
            {
                mPlayer->ResetPercentageTraversed();
                mPlayer->ResetThermalVariables();
            }
        }
    }

    ++mTicks;
    Tooltips::DisplayTooltips(playerIdx);
}

void Minecraft::renderInfoBars()
{
    InfoBarManager::Render((float)mWidth, (float)mHeight);
}

void Minecraft::renderChunkInfoMap()
{
    ChunkInfoMapManager::Render((float)mWidth, (float)mHeight);
}

void Minecraft::run_middle()
{
    if (sFrameStartTime == 0)
        sFrameStartTime = System::processTimeInNanoSecs();

    EnterCriticalSection(&mLevelLock);
    if (!mRunning)
    {
        LeaveCriticalSection(&mLevelLock);
        return;
    }

    if (mReloadTextures)
    {
        mReloadTextures = false;
        mTextures->reloadAll();
    }

    AABB::resetPool();
    Vec3::resetPool();

    if (mLevel != NULL && gProfile->IsFullVersion() && CGameNetworkManager::sInstance.IsHost() &&
        !InMiniGame(eMiniGameId_Any, true) && !gStorage->GetSaveDisabled())
    {
        int primary = gProfile->GetPrimaryPad();
        if (gApp.GetXuiAction(primary) == 0 && !gConsoleUIController.IsPauseMenuDisplayed(primary))
        {
            if (gConsoleUIController.IsIgnoreAutosaveMenuDisplayed(primary))
            {
                gConsoleUIController.ShowAutosaveCountdownTimer(false);
            }
            else
            {
                bool autosaveAllowed =
                    GameSettings::GetGameSettings(primary, (eGameSetting)0xd) != 0;
                TexturePack* selected =
                    GetInstance()->mTexturePackRepository->getSelected();
                if (selected != TexturePackRepository::defaultTexturePack &&
                    selected->getDLCPack() != NULL && !selected->getDLCPack()->hasPurchasedPack())
                {
                    autosaveAllowed = false;
                }

                if (autosaveAllowed)
                {
                    if (gApp.AutosaveDue())
                    {
                        unsigned int seconds = gApp.SecondsToAutosave();
                        if (seconds <= 5)
                        {
                            gConsoleUIController.ShowAutosaveCountdownTimer(true);
                            gConsoleUIController.UpdateAutosaveCountdownTimer(seconds);
                        }
                    }
                    else
                    {
                        gConsoleUIController.ShowAutosaveCountdownTimer(false);
                        gApp.SetAction(primary, (eXuiAction)0x1e, NULL);
                    }
                }
            }
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        if (mLocalPlayers[i] && !gApp.GetBanListCheck(i) && !GetInstance()->isTutorial() &&
            gProfile->IsSignedInLive(i) && !gProfile->IsGuest(i) && !gProfile->IsSystemUIDisplayed())
        {
            gApp.SetBanListCheck(i, true);
        }
    }

    if (!gProfile->IsSystemUIDisplayed() && gApp.DLCInstallProcessCompleted() && !gApp.DLCInstallPending() &&
        gApp.mDLCManager.m_bCorruptDLCFound)
    {
        gApp.mDLCManager.checkForCorruptDLCAndAlert(true);
    }

    if (!gApp.IsDrcPreventingSplitscreen() && mLevel != NULL && sCheckForNewLocalPlayers)
    {
        if (!CGameNetworkManager::sInstance.SessionHasSpace(1))
        {
            if (sPressStartHoldOffFrames != 0)
            {
                --sPressStartHoldOffFrames;
            }
            else
            {
                sCheckForNewLocalPlayers = false;
                if (gApp.IsLocalMultiplayerAvailable())
                {
                    for (int i = 0; i < 4; ++i)
                    {
                        if (!mLocalPlayers[i] && g_pInput->IsPadConnected(i) &&
                            !gConsoleUIController.PressStartPlaying(i))
                        {
                            gConsoleUIController.ShowPressStart(i);
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        if (!mLocalPlayers[i])
        {
            if (!mAppPaused &&
                !gConsoleUIController.IsIgnorePlayerJoinMenuDisplayed(gProfile->GetPrimaryPad()) &&
                CGameNetworkManager::sInstance.SessionHasSpace(1) && gRenderer->IsHiDef() &&
                g_pInput->GetValue(i, 0xff, false) && gRenderer->IsWidescreen())
            {
                if (!gConsoleUIController.PressStartPlaying(i))
                {
                    gConsoleUIController.ShowPressStart(i);
                }
                else if (g_pInput->GetValue(i, 0x16, false) && !gApp.IsDrcPreventingSplitscreen())
                {
                    if (gProfile->IsSignedIn(i))
                    {
                        if (!CGameNetworkManager::sInstance.IsLocalGame() &&
                            (!gProfile->IsSignedInLive(i) || !gProfile->AllowedToPlayMultiplayer(i)))
                        {
                            if (gProfile->IsSignedInLive(gProfile->GetPrimaryPad()) &&
                                !gProfile->AllowedToPlayMultiplayer(i))
                            {
                                gProfile->RequestSignInUI(false, false, false, false, false,
                                                        InGame_SignInReturned, this, i);
                                gConsoleUIController.HidePressStart();
                                int messageParameters[1];
                                messageParameters[0] = 0xB9A1BF4D;
                                gConsoleUIController.RequestErrorMessage(0x252AB13F, 0xCC5A590F,
                                                                          messageParameters, 1, i, NULL, NULL,
                                                                          NULL, true);
                            }
                            gApp.SetDRCIconEnabled(false);
                            gApp.SetDRCCheckPlayerCountEnabled(false);
                            gProfile->RequestSignInUI(0, CGameNetworkManager::sInstance.IsLocalGame(), 1, 0, 1,
                                                     InGame_SignInReturned, this, i);
                        }
                        else
                        {
                            gApp.SetDRCIconEnabled(false);
                            gApp.SetDRCCheckPlayerCountEnabled(false);
                            if (!mLevel->mIsClientSide)
                            {
                                createExtraLocalPlayer(i, convStringToWstring(gProfile->GetGamertag(i)), i, i,
                                                       NULL, NULL, mLevel->mDimension->getType()->getId(), 0);
                            }
                            else if (!addLocalPlayer(i))
                            {
                                gProfile->RequestSignInUI(0, CGameNetworkManager::sInstance.IsLocalGame(), 1, 0,
                                                         1, InGame_SignInReturned, this, i);
                            }
                        }
                    }
                    else
                    {
                        gApp.SetDRCIconEnabled(false);
                        gApp.SetDRCCheckPlayerCountEnabled(false);
                        gProfile->RequestSignInUI(0, CGameNetworkManager::sInstance.IsLocalGame(), 1, 0, 1,
                                                 InGame_SignInReturned, this, i);
                    }
                }
            }
            continue;
        }

        gApp.ProcessFakePadValueA();

        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mLocalPlayers[i];
        if (g_pInput->GetValue(i, 0x20, GameSettings::GetInputButton(i, true, (EControllerActions)0x20)))
            localPlayer->mButtonsDown |= 1;
        if (g_pInput->GetValue(i, 0x21, GameSettings::GetInputButton(i, true, (EControllerActions)0x21)))
            localPlayer->mButtonsDown |= 2;
        if (g_pInput->GetValue(i, 0x25, GameSettings::GetInputButton(i, true, (EControllerActions)0x25)))
            localPlayer->mButtonsDown |= 0x20;
        if (g_pInput->GetValue(i, 0x22, GameSettings::GetInputButton(i, true, (EControllerActions)0x22)))
            localPlayer->mButtonsDown |= 4;
        if (g_pInput->GetValue(i, 0x28, GameSettings::GetInputButton(i, true, (EControllerActions)0x28)))
            localPlayer->mButtonsDown |= 0x100;
        if (g_pInput->GetValue(i, 0x16, false))
            localPlayer->mButtonsPressed |= 0x400000;
        if (g_pInput->GetValue(i, 0x26, GameSettings::GetInputButton(i, true, (EControllerActions)0x26)))
            localPlayer->mButtonsDown |= 0x40;
        if ((!localPlayer->mAbilities.mFlying &&
             (!localPlayer->headInWater() || localPlayer->mOnGround))
                ? g_pInput->GetValue(i, 0x27,
                                     GameSettings::GetInputButton(i, true, (EControllerActions)0x27))
                : g_pInput->ButtonDown(i, 0x27))
        {
            localPlayer->mButtonsDown |= 0x80;
        }
        if (g_pInput->GetValue(i, 0x29, GameSettings::GetInputButton(i, true, (EControllerActions)0x29)))
            localPlayer->mButtonsDown |= 0x200;
        if (g_pInput->GetValue(i, 0x17, false))
            localPlayer->mButtonsPressed |= 0x800000;
        if (g_pInput->GetValue(i, 0x34, GameSettings::GetInputButton(i, true, (EControllerActions)0x34)))
            localPlayer->mButtonsDown |= 0x100000;
        if (g_pInput->GetValue(i, 0x35, GameSettings::GetInputButton(i, true, (EControllerActions)0x35)))
            localPlayer->mButtonsDown |= 0x200000;
        if (g_pInput->GetValue(i, 0x36, GameSettings::GetInputButton(i, true, (EControllerActions)0x36)))
            localPlayer->mButtonsDown |= 0x400000;
        if (g_pInput->GetValue(i, 0x37, GameSettings::GetInputButton(i, true, (EControllerActions)0x37)))
            localPlayer->mButtonsDown |= 0x800000;

        if (GetMiniGame()->HasFeatureLookBack() && localPlayer->mThirdPersonView != 2)
            localPlayer->mDRCTouching = g_pInput->ButtonDown(i, 0x38);

        int moveButtons = 0;
        localPlayer->mLookInputX = 0;
        localPlayer->mLookInputY = 0;
        if (g_pInput->ButtonDown(i, 0x2a))
        {
            ++moveButtons;
            localPlayer->mLookInputX |= 0x400;
        }
        if (g_pInput->ButtonDown(i, 0x2b))
        {
            ++moveButtons;
            localPlayer->mLookInputX |= 0x800;
        }
        if (g_pInput->ButtonDown(i, 0x2c))
        {
            ++moveButtons;
            localPlayer->mLookInputX |= 0x1000;
        }
        if (g_pInput->ButtonDown(i, 0x2d))
        {
            ++moveButtons;
            localPlayer->mLookInputX |= 0x2000;
        }

        if (moveButtons < 2)
        {
            localPlayer->mMoveInputX = localPlayer->mLookInputX;
            localPlayer->mMoveInputY = localPlayer->mLookInputY;
            localPlayer->mReserved0x928 = localPlayer->mLookInputX;
            localPlayer->mReserved0x92c = localPlayer->mLookInputY;
        }
        else
        {
            localPlayer->mReserved0x928 = localPlayer->mMoveInputX;
            localPlayer->mReserved0x92c = localPlayer->mMoveInputY;
        }

        if (g_pInput->GetValue(i, 0x23, false) || g_pInput->GetValue(i, 0x24, false))
            gApp.SetOpacityTimer(i);
    }

    if (!mAppPaused && !gConsoleUIController.IsIgnorePlayerJoinMenuDisplayed(gProfile->GetPrimaryPad()) &&
        CGameNetworkManager::sInstance.SessionHasSpace(1) && gRenderer->IsHiDef())
    {
        int freeSlot = 0;
        if (mLocalPlayers[0])
        {
            if (!mLocalPlayers[1])
                freeSlot = 1;
            else if (!mLocalPlayers[2])
                freeSlot = 2;
            else if (!mLocalPlayers[3])
                freeSlot = 3;
        }

        for (int pad = 4; pad < 9; ++pad)
        {
            if (!g_pInput->IsPadLocked(pad) && g_pInput->IsPadConnected(pad) &&
                g_pInput->GetValue(pad, 0xff, false))
            {
                if (!gConsoleUIController.PressStartPlaying(freeSlot))
                {
                    gConsoleUIController.ShowPressStart(freeSlot);
                }
                else if (!gApp.IsDrcPreventingSplitscreen() && g_pInput->GetValue(pad, 0x16, false))
                {
                    gApp.SetDRCIconEnabled(false);
                    gApp.SetDRCCheckPlayerCountEnabled(false);
                    gProfile->RequestSignInUI(0, CGameNetworkManager::sInstance.IsLocalGame(), 1, 0, 1,
                                             InGame_SignInReturned, this, pad);
                    break;
                }
            }
        }
    }

    long long tickStartTime = System::processTimeInNanoSecs();

    if (mAppPaused && mLevel != NULL)
    {
        float partialTicks = mTimer->renderPartialTicks;
        mTimer->advanceTime();
        mTimer->renderPartialTicks = partialTicks;
    }
    else
    {
        mTimer->advanceTime();
    }

    for (int step = 0; step < mTimer->elapsedTicks; ++step)
    {
        if (step != 0)
        {
            g_pInput->Tick();
            gApp.HandleButtonPresses();
        }

        ++mTicks;
        bool handleInput = true;
        for (int i = 0; i < 4; ++i)
        {
            if (mPendingConnections[i] != NULL)
                mPendingConnections[i]->tick();

            bool active = true;
            if (mLocalPlayers[i])
            {
                boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mLocalPlayers[i];
                if (localPlayer->mButtonsDown == 0 && localPlayer->mButtonsPressed == 0 &&
                    g_pInput->GetJoypadStick_LX(i, false) == 0.0f && g_pInput->GetJoypadStick_LY(i, false) == 0.0f &&
                    g_pInput->GetJoypadStick_RX(i, false) == 0.0f && g_pInput->GetJoypadStick_RY(i, false) == 0.0f)
                {
                    if (localPlayer->mIdleTicks < 0xff)
                        ++localPlayer->mIdleTicks;
                }
                else
                {
                    localPlayer->mIdleTicks = 0;
                }

                if (localPlayer->mIdleTicks < 0xc9)
                {
                    if (localPlayer->isIdle())
                        localPlayer->setIsIdle(false);
                }
                else if (!localPlayer->isIdle() && localPlayer->mOnGround)
                {
                    localPlayer->setIsIdle(true);
                }
            }

            active = setLocalPlayerIdx(i);
            if (active)
            {
                tick(handleInput, step == mTimer->elapsedTicks - 1);
                handleInput = false;
                mPlayer->mButtonsDown = 0;
                mPlayer->mButtonsPressed = 0;
            }
        }

        gConsoleUIController.HandleGameTick();
        setLocalPlayerIdx(gProfile->GetPrimaryPad());

        for (int i = 0; i < mLevelCount; ++i)
        {
            if (mLevels[i] != NULL)
                mLevels[i]->animateTickDoWork();
        }
    }

    MemSect(0x1f);
    MemSect(0);

    gFancyGraphics = mOptions->mFancyGraphics;

    PIXBeginNamedEvent(0.0f, "Sound engine update");
    mSoundEngine->tick(&mLocalPlayers[0], mTimer->renderPartialTicks);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Light update");
    GlStateManager::enableTexture();
    PIXEndNamedEvent();

    for (int i = 0; i < 4; ++i)
    {
        if (!setLocalPlayerIdx(i))
            continue;
        if (mPlayer->isInWall())
            mPlayer->SetThirdPersonView(0);
        if (mPlayer->GetCameraController() != NULL)
        {
            mPlayer->GetCameraController()->update((float)mTimer->elapsedTicks + mTimer->renderPartialTicks);
        }
    }

    if (!mNoRender)
    {
        int primary = gProfile->GetPrimaryPad();
        bool clearScreen = true;
        for (int i = 0; i < 4; ++i)
        {
            if (!setLocalPlayerIdx(i))
                continue;

            PIXBeginNamedEvent(0.0f, "Game render player idx: %d", i);
            gRenderer->StateSetViewport((C4JRender::eViewportType)mPlayer->mViewport);
            mGameRenderer->render(mTimer->renderPartialTicks, sFrameStartTime, clearScreen);
            clearScreen = false;
            PIXEndNamedEvent();

            if (primary == i)
            {
                int action = gApp.GetXuiAction(i);
                if (action == 2 || action == 4 || action == 0x1f)
                    gApp.CaptureSaveThumbnail();
            }
        }

        if (mEmptyViewport >= 0)
        {
            gRenderer->StateSetViewport((C4JRender::eViewportType)(mEmptyViewport + 5));
            GlStateManager::clearColor(0.0f, 0.0f, 0.0f, 0.0f);
            GlStateManager::clear(2);
            gConsoleUIController.SetEmptyQuadrantLogo(mEmptyViewport + 5);
        }

        setLocalPlayerIdx(primary);
        gRenderer->StateSetViewport((C4JRender::eViewportType)0);
    }

    GlStateManager::flush();

    if (mOptions->mShowInfoBars)
        renderInfoBars();
    if (mOptions->mShowChunkInfoMap)
        renderChunkInfoMap();

    if (mOptions->mRenderDebug != 0)
        CGameNetworkManager::sInstance.renderQueueMeter();
    else
        mLastRenderTime = System::processTimeInNanoSecs();

    PIXBeginNamedEvent(0.0f, "Sleeping");
    Sleep(0);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Display update");
    PIXEndNamedEvent();

    MemSect(0x1f);
    MemSect(0);

    mAppPaused = gApp.IsAppPaused();

    long long now = System::processTimeInNanoSecs();
    mFrameTimer->logFrameDuration(now - mLastFrameTime);
    mLastFrameTime = now;

    LeaveCriticalSection(&mLevelLock);
}

void Minecraft::respawnPlayer(int playerIdx, int dimension, int entityId, bool alive)
{
    mGameRenderer->DisableUpdateThread(6);

    boost::shared_ptr<MultiplayerLocalPlayer> oldPlayer = mLocalPlayers[playerIdx];
    mLevel->validateSpawn();
    mLevel->removeAllPendingEntityRemovals();
    if (oldPlayer)
        mLevel->removeEntity(oldPlayer);

    boost::shared_ptr<MultiplayerLocalPlayer> newPlayer;
    newPlayer = boost::static_pointer_cast<MultiplayerLocalPlayer>(
        mGameModes[playerIdx]->createPlayer(mLevel));
    mPlayer = newPlayer;

    newPlayer->setName(convStringToWstring(gProfile->GetGamertag(playerIdx)));
    if (alive)
        newPlayer->getEntityData()->assignValues(oldPlayer->getEntityData()->getAll());

    newPlayer->setSharedFlag(0, false);
    if (InMiniGame(eMiniGameId_Glide, false) && gApp.getLevelGenerationOptions() != NULL)
    {
        LevelRuleset* ruleset = gApp.getLevelGenerationOptions()->getRequiredGameRules();
        if (ruleset != NULL)
        {
            AABB* jumpOffArea = ruleset->getNamedArea(L"JumpOffPoint");
            bool inJumpOffArea = jumpOffArea != NULL &&
                                 jumpOffArea->containsIncludingLowerBound(newPlayer->getPos(1.0f));
            if (!inJumpOffArea)
                newPlayer->setSharedFlag(9, true);
        }
    }

    PlayerUID xuid(0);
    PlayerUID suid(0);
    gProfile->GetXUID(playerIdx, &xuid, false);
    gProfile->GetXUID(playerIdx, &suid, true);
    newPlayer->setXuid(xuid);
    newPlayer->mSuid = suid;

    if (oldPlayer->IsSpectatingOtherPlayer())
        newPlayer->SetCamera(oldPlayer->GetCamera());

    newPlayer->setCustomSkin(oldPlayer->mField0x7dc);
    newPlayer->setPlayerDefaultSkin(oldPlayer->mField0x7d8);
    newPlayer->setCustomCape(oldPlayer->mField0x7e0);
    newPlayer->setPlayerGamePrivilege((Player::EPlayerGamePrivileges)0x21,
                                      oldPlayer->getPlayerGamePrivilege((Player::EPlayerGamePrivileges)0x21));
    newPlayer->mDimension = dimension;
    mCameraEntity = newPlayer;

    if (playerIdx == gProfile->GetPrimaryPad())
    {
        createPrimaryLocalPlayer(playerIdx);
        GameSettings::SetGameSettingsDebugMask(playerIdx,
                                               GameSettings::GetGameSettingsDebugMask(playerIdx, true));
    }
    else
    {
        storeExtraLocalPlayer(playerIdx);
    }

    oldPlayer->resetPos();
    mLevel->addEntity(oldPlayer);

    mGameModes[playerIdx]->initPlayer(newPlayer);

    if (InMiniGame(eMiniGameId_Glide, true) && CommonMasterGameMode::GetMiniGame()->IsCheckpointBasedSpawn())
    {
        MultiPlayerLevel* level = newPlayer->GetLevel();
        if (level != NULL && level->GetClientScoreboard() != NULL)
        {
            int checkpoint = level->GetClientScoreboard()->GetLastCheckpointID(newPlayer->getSUID(), -1);
            LevelGenerationOptions* generationOptions = gApp.getLevelGenerationOptions();
            if (generationOptions != NULL)
            {
                LevelRuleset* ruleset = generationOptions->getRequiredGameRules();
                const std::vector<CheckpointRuleDefinition*> checkpoints = *ruleset->getCheckpoints();
                if (checkpoint >= 0 && static_cast<unsigned int>(checkpoint) < checkpoints.size())
                {
                    double speedBoost = ruleset->getCheckpoints()->at(checkpoint)->getElytraBoost();
                    if (speedBoost > 0.0)
                        newPlayer->activateElytraSpeedBoost(speedBoost);
                }
            }
        }
    }

    mGameModes[playerIdx]->adjustPlayer(newPlayer);
    newPlayer->setId(entityId);

    if (!mLevel->mIsClientSide)
        prepareLevel((int)0xf5ba56aa);

    if (gConsoleUIController.IsSceneInStack(playerIdx, (EUIScene)0x1f) ||
        gApp.GetXuiAction(playerIdx) == 7)
    {
        if (gConsoleUIController.IsSceneInStack(playerIdx, (EUIScene)0x30))
        {
            gConsoleUIController.NavigateBack(playerIdx, false, (EUIScene)0x30, (EUILayer)8);
        }
        else if (gApp.GetXuiAction(playerIdx) != 0xf)
        {
            gConsoleUIController.CloseUIScenes(playerIdx, false);
            gConsoleUIController.UpdatePlayerBasePositions();
        }
    }

    newPlayer->SetWaitingForScoreboard(oldPlayer->IsWaitingForScoreboard());

    if (mScreen != NULL)
        setScreen(NULL);

    mGameRenderer->EnableUpdateThread(6);
}

void Minecraft::startAndConnectTo(const std::wstring& userName, const std::wstring& sessionId,
                                  const std::wstring& server)
{
    Minecraft* minecraft = new Minecraft(NULL, NULL, NULL, 0x500, 0x2d0, false);
    minecraft->mServer = L"www.minecraft.net";

    if (userName.compare(0, userName.size(), L"", wcslen(L"")) != 0 &&
        sessionId.compare(0, sessionId.size(), L"", wcslen(L"")) != 0)
    {
        minecraft->mUser = new User(userName, sessionId);
        minecraft->run();
        return;
    }

    std::wstring generatedName = std::wstring(L"Player") + _toString(System::processTimeInMilliSecs() % 1000);
    minecraft->mUser = new User(generatedName, L"");
    minecraft->run();
}

void Minecraft::start(const std::wstring& userName, const std::wstring& sessionId)
{
    startAndConnectTo(userName, sessionId, L"");
}

void MinecraftWorld_RunStaticCtors()
{
    Direction::staticCtor();
    Packet::staticCtor();
    SoundEvents::staticCtor();
    SoundType::StaticCtor();
    MaterialColor::staticCtor();
    DyeColor::staticCtor();
    MaterialColor::staticCtorAfterDyeColor();
    Material::staticCtor();
    Block::staticCtor();
    Blocks::staticInit();
    AxeItem::staticCtor();
    PickaxeItem::staticCtor();
    ShovelItem::staticCtor();
    JungleBiome::staticCtor();
    MesaBiome::staticCtor();
    Biome::staticCtor();
    MobEffect::staticCtor();
    MobEffects::staticCtor();
    EquipmentSlot::staticCtor();
    InventoryMenu::staticCtor();
    ArmorItem::staticCtor();
    Item::staticCtor();
    FurnaceRecipes::staticCtor();
    MapExtendingRecipe::StaticCtor();
    BannerPattern::staticCtor();
    Potion::StaticCtor();
    Potions::StaticCtor();
    Recipes::staticCtor();
    gStats = new CommonStats();
    Stats::staticCtor();
    BlockEntity::staticCtor();
    EntityIO::bootStrap();
    MobCategory::staticCtor();
    BlockEntityIdFix::staticCtor();
    ItemPotionFix::staticCtor();
    Item::staticInit();
    LevelChunk::staticCtor();
    LevelType::staticCtor();
    StructureFeatureIO::staticCtor();
    VillagePieces::Smithy::staticCtor();
    VillageFeature::staticCtor();
    OceanMonumentFeature::staticCtor();
    RandomScatteredLargeFeature::staticCtor();
    OceanMonumentPieces::OceanMonumentPiece::staticCtor();
    DesertWellFeature::staticCtor();
    ScatteredFeaturePieces::JunglePyramidPiece::staticCtor();
    SwampTreeFeature::staticCtor();
    StrongholdPieces::staticCtor();
    WoodlandMansionFeature::staticCtor();
    BuriedTreasureFeature::staticCtor();
    Entity::staticCtor();
    LivingEntity::staticCtor();
    ArmorStand::staticCtor();
    Mob::staticCtor();

    SynchedEntityData::defineId<int>(0x000000000D400000ULL, 0x000000000D000000ULL, NULL);
    AgableMob::staticCtor();
    Animal::staticCtor();
    Villager::staticCtor();
    TamableAnimal::staticCtor();
    Ocelot::staticCtor();
    Wolf::staticCtor();

    SynchedEntityData::defineId<int>(0x000000000D690000ULL, 0x000000000D680000ULL, NULL);
    AbstractHorse::staticCtor();
    SynchedEntityData::defineId<int>(0x000000000D698000ULL, 0x000000000D690000ULL, NULL);
    AbstractChestedHorse::staticCtor();
    Horse::staticCtor();
    Llama::staticCtor();

    SynchedEntityData::defineId<int>(0x000000000D6C0000ULL, 0x000000000D680000ULL, NULL);
    Parrot::staticCtor();
    Sheep::staticCtor();
    Pig::staticCtor();
    Chicken::staticCtor();
    Rabbit::staticCtor();
    PolarBear::staticCtor();
    Turtle::staticCtor();

    SynchedEntityData::defineId<int>(0x000000000D580000ULL, 0x000000000D400000ULL, NULL);
    SnowMan::staticCtor();
    VillagerGolem::staticCtor();
    Shulker::staticCtor();

    SynchedEntityData::defineId<int>(0x000000004D540000ULL, 0x000000000D400000ULL, NULL);
    Spider::staticCtor();
    Zombie::staticCtor();
    ZombieVillager::staticCtor();
    Creeper::staticCtor();
    AbstractSkeleton::staticCtor();
    EnderMan::staticCtor();
    Blaze::staticCtor();
    Witch::staticCtor();
    WitherBoss::staticCtor();
    Guardian::staticCtor();
    Vex::staticCtor();

    SynchedEntityData::defineId<int>(0x000000004D541040ULL, 0x000000004D540000ULL, NULL);
    AbstractIllager::staticCtor();
    SynchedEntityData::defineId<int>(0x000000004D541840ULL, 0x000000004D541040ULL, NULL);
    SpellcasterIllager::staticCtor();
    SynchedEntityData::defineId<int>(0x000000000D502000ULL, 0x000000000D000000ULL, NULL);
    TropicalFish::staticCtor();
    Salmon::staticCtor();
    SynchedEntityData::defineId<int>(0x000000000D000100ULL, 0x000000000D000000ULL, NULL);
    Bat::staticCtor();
    SynchedEntityData::defineId<int>(0x000000000D000080ULL, 0x000000000D000000ULL, NULL);
    Ghast::staticCtor();
    Slime::staticCtor();
    EnderDragon::staticCtor();
    Player::staticCtor();
    Minecart::staticCtor();
    MinecartCommandBlock::staticCtor();
    MinecartFurnace::staticCtor();
    AbstractRotatedItemProjectile::staticCtor();
    TippableArrow::staticCtor();
    SynchedEntityData::defineId<int>(0x000000008A400000ULL, 0x0000000008000000ULL, NULL);
    WitherSkull::staticCtor();
    SynchedEntityData::defineId<int>(0x000000008A200000ULL, 0x0000000008000000ULL, NULL);
    ThrownPotion::staticCtor();
    SynchedEntityData::defineId<int>(0x000000000A080000ULL, 0x0000000008000000ULL, NULL);
    ItemFrame::staticCtor();
    PrimedTnt::staticCtor();
    FireworksRocketEntity::staticCtor();
    EnderCrystal::staticCtor();
    FishingHook::staticCtor();
    FallingBlockEntity::staticCtor();
    ItemEntity::staticCtor();
    Boat::staticCtor();
    AreaEffectCloud::staticCtor();
    ThrownTrident::staticCtor();
    PotionBrewing::StaticCtor();
    Enchantment::staticCtor();
    SharedConstants::staticCtor();
    ServerLevel::staticCtor();
    SparseLightStorage::staticCtor();
    CompressedBlockStorage::staticCtor();
    SparseDataStorage::staticCtor();
    McRegionChunkStorage::staticCtor();
    GameType::staticCtor();
    BeaconBlockEntity::staticCtor();
    DefaultVertexFormat::staticCtor();
    FaceInfo::staticCtor();
    EatBlockGoal::staticCtor();
    BirchFeature::staticCtor();
    MegaPineTreeFeature::staticCtor();
    PineFeature::staticCtor();
    RoofTreeFeature::staticCtor();
    SavannaTreeFeature::staticCtor();
    SpruceFeature::staticCtor();
    SwampTreeFeature::staticCtor();
    TreeFeature::staticCtor();
    BuiltInLootTables::staticCtor();
    Enchantments::staticCtor();
    DispenserBootstrap::StaticCtor();
    ItemIdFix::staticCtor();
    ItemInstanceBlockEntityTagWalker::staticCtor();
}

ClientPacketListener* Minecraft::getConnection(int playerIdx)
{
    return mLocalPlayers[playerIdx]->mPacketListener;
}

void Minecraft::main()
{
    sStaticsInitialised = true;

    MinecraftWorld_RunStaticCtors();
    User::staticCtor();
    Tutorial::staticCtor();
    ColourTable::staticCtor();
    StructureManager::staticCtor();
    Tooltips::staticCtor();
    GlStateManager::staticCtor();
    gApp.loadDefaultGameRules();

    std::wstring userName =
        std::wstring(L"Player") + _toString(System::processTimeInMilliSecs() % 1000);

    UIScene_CreativeMenu::staticCtor();
    UIScene_LeaderboardsMenu::staticCtor();
    BlockEntityRenderDispatcher::staticCtor();

    start(userName, L"-");

    ParticleType::staticCtor();
    MasterGameMode::StaticCtor();
    MiniGameMedals::staticCtor();
    CMinecraftApp::StaticCtor();
    MiniGameDef::StaticCtor();

    GetInstance()->mMiniGame = MiniGameDef::GetCustomGameModeById(eMiniGameId_Any, true);
    ClientPacketListener::staticCtor();
}

bool Minecraft::renderNames()
{
    if (sInstance != NULL && sInstance->mOptions->mHideGui)
        return false;
    return true;
}

bool Minecraft::useFancyGraphics()
{
    return sInstance != NULL && sInstance->mOptions->mFancyGraphics;
}

bool Minecraft::useAmbientOcclusion()
{
    return sInstance != NULL && sInstance->mOptions->mAmbientOcclusion;
}

bool Minecraft::renderDebug()
{
    return sInstance != NULL && sInstance->mOptions->mRenderDebug != 0;
}

int Minecraft::getAverageFps()
{
    return sAverageFps;
}

bool Minecraft::addPendingClientTextureRequest(const std::wstring& textureName)
{
    std::vector<std::wstring>::iterator it = mPendingClientTextureRequests.begin();
    while (it != mPendingClientTextureRequests.end())
    {
        if (it->compare(0, it->size(), textureName.c_str(), textureName.size()) == 0)
            break;
        ++it;
    }

    if (it == mPendingClientTextureRequests.end())
    {
        mPendingClientTextureRequests.push_back(textureName);
        return true;
    }
    return false;
}

void Minecraft::handleClientTextureReceived(const std::wstring& textureName)
{
    std::vector<std::wstring>::iterator it = mPendingClientTextureRequests.begin();
    while (it != mPendingClientTextureRequests.end())
    {
        if (it->compare(0, it->size(), textureName.c_str(), textureName.size()) == 0)
            break;
        ++it;
    }

    if (it != mPendingClientTextureRequests.end())
        mPendingClientTextureRequests.erase(it);
}

void Minecraft::fjReloadTexturePack()
{
    getEntityRenderDispatcher()->setupForTexturePack(mTexturePackRepository->getSelected());
    BlockEntityRenderDispatcher::instance->setupForTexturePack(mTexturePackRepository->getSelected());
    mTextures->reloadAll();
    mTexturePackRepository->updateUI();

    TexturePack* selected = mTexturePackRepository->getSelected();
    unsigned int packId = selected->getId();
    if (selected->hasAudio() || packId == 0x801 || packId == 0x803 || packId == 0x806 || packId == 0x808)
    {
        const MiniGameDef* miniGame = GetMiniGame();
        ClientMasterGameMode* masterGameMode = GetInstance()->GetClientMasterGameMode();
        EMiniGameId miniGameId = static_cast<EMiniGameId>(miniGame->GetId());
        if (miniGameId != eMiniGameId_Lobby && miniGameId < 4)
        {
            mSoundEngine->playStreaming(NULL, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, false,
                                        masterGameMode->IsWaitingForMusicStartEvent(),
                                        ConsoleSoundEngine::eAudioDelayType_None);
        }
        else
        {
            mSoundEngine->playStreaming(NULL, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, false, false,
                                        ConsoleSoundEngine::eAudioDelayType_None);
        }
    }
}

boost::shared_ptr<AbstractClientPlayer> Minecraft::GetLocalPlayerByName(const std::wstring& name)
{
    boost::shared_ptr<Player> found;
    for (int i = 0; i < mLevelCount; ++i)
    {
        if (mLevels[i] == NULL)
            continue;
        found = mLevels[i]->getPlayerByName(name);
        if (found)
            break;
    }
    return boost::dynamic_pointer_cast<AbstractClientPlayer>(found);
}

boost::shared_ptr<Player> Minecraft::GetPlayerByPlayerIndex(int playerIndex)
{
    for (int i = 0; i < 3; ++i)
    {
        MultiPlayerLevel* level = mLevels[i];
        if (level == NULL)
            continue;

        for (std::vector<boost::shared_ptr<Player> >::iterator it = level->mPlayers.begin();
             it != level->mPlayers.end(); ++it)
        {
            if ((*it)->mPlayerIndex == playerIndex)
                return *it;
        }
    }
    return boost::shared_ptr<Player>();
}

int Minecraft::GetSpectateMode()
{
    if (InMiniGame(eMiniGameId_Any, true))
        return GetMiniGame()->GetSpectateMode();
    return 0;
}

unsigned long long Minecraft::GetSpectateEntity()
{
    if (InMiniGame(eMiniGameId_Any, true))
        return GetMiniGame()->GetSpectateEntity();

    MiniGameDef defaultMiniGame(eMiniGameId_Lobby);
    return defaultMiniGame.GetSpectateEntity();
}

GhostController* Minecraft::GetGhostController()
{
    return mGhostController;
}

void Minecraft::SetGhostController(GhostController* ghostController)
{
    if (mGhostController != NULL)
    {
        mGhostController->setDone();
        mFinishedGhostControllers.push_back(mGhostController);
    }
    mGhostController = ghostController;
}

void Minecraft::SetJoiningFromFriendsTabOrInvite(bool joining)
{
    mJoiningFromFriendsTabOrInvite = joining;
}

bool Minecraft::IsJoiningFromFriendsTabOrInvite() const
{
    return mJoiningFromFriendsTabOrInvite;
}
