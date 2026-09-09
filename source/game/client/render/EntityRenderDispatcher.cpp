#include "game/client/render/EntityRenderDispatcher.hpp"

#include "game/client/render/AreaEffectCloudRenderer.hpp"
#include "game/client/render/ArmorStandRenderer.hpp"
#include "game/client/render/ArrowRenderer.hpp"
#include "game/client/render/BatRenderer.hpp"
#include "game/client/render/BlazeRenderer.hpp"
#include "game/client/render/BoatRenderer.hpp"
#include "game/client/render/CaveSpiderRenderer.hpp"
#include "game/client/render/ChestedHorseRenderer.hpp"
#include "game/client/render/ChickenModel.hpp"
#include "game/client/render/ChickenRenderer.hpp"
#include "game/client/render/CowModel.hpp"
#include "game/client/render/CowRenderer.hpp"
#include "game/client/render/CreeperRenderer.hpp"
#include "game/client/render/DefaultRenderer.hpp"
#include "game/client/render/DolphinRenderer.hpp"
#include "game/client/render/DragonFireballRenderer.hpp"
#include "game/client/render/DrownedRenderer.hpp"
#include "game/client/render/ElderGuardianRenderer.hpp"
#include "game/client/render/EnderCrystalRenderer.hpp"
#include "game/client/render/EnderDragonRenderer.hpp"
#include "game/client/render/EndermanRenderer.hpp"
#include "game/client/render/EndermiteRenderer.hpp"
#include "game/client/render/EvocationFangsRenderer.hpp"
#include "game/client/render/EvocationIllagerRenderer.hpp"
#include "game/client/render/ExperienceOrbRenderer.hpp"
#include "game/client/render/FallingBlockRenderer.hpp"
#include "game/client/render/FireballRenderer.hpp"
#include "game/client/render/FishRenderer.hpp"
#include "game/client/render/FishingHookRenderer.hpp"
#include "game/client/render/GhastRenderer.hpp"
#include "game/client/render/GiantMobRenderer.hpp"
#include "game/client/render/GuardianRenderer.hpp"
#include "game/client/render/HorseRenderer.hpp"
#include "game/client/render/HuskRenderer.hpp"
#include "game/client/render/IllusionIllagerRenderer.hpp"
#include "game/client/render/ItemEntityRenderer.hpp"
#include "game/client/render/ItemFrameRenderer.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/LavaSlimeRenderer.hpp"
#include "game/client/render/LeashKnotRenderer.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/LightningBoltRenderer.hpp"
#include "game/client/render/LlamaRenderer.hpp"
#include "game/client/render/LlamaSpitRenderer.hpp"
#include "game/client/render/MinecartRenderer.hpp"
#include "game/client/render/MinecartSpawnerRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/MushroomCowRenderer.hpp"
#include "game/client/render/OcelotRenderer.hpp"
#include "game/client/render/PaintingRenderer.hpp"
#include "game/client/render/ParrotRenderer.hpp"
#include "game/client/render/PhantomRenderer.hpp"
#include "game/client/render/PigRenderer.hpp"
#include "game/client/render/PigZombieRenderer.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/client/render/PolarBearRenderer.hpp"
#include "game/client/render/PufferfishRenderer.hpp"
#include "game/client/render/RabbitRenderer.hpp"
#include "game/client/render/SheepFurModel.hpp"
#include "game/client/render/SheepModel.hpp"
#include "game/client/render/SheepRenderer.hpp"
#include "game/client/render/ShulkerBulletRenderer.hpp"
#include "game/client/render/ShulkerRenderer.hpp"
#include "game/client/render/SilverfishRenderer.hpp"
#include "game/client/render/SkeletonRenderer.hpp"
#include "game/client/render/SlimeRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/SnowManRenderer.hpp"
#include "game/client/render/SpectralArrowRenderer.hpp"
#include "game/client/render/SpiderRenderer.hpp"
#include "game/client/render/SquidRenderer.hpp"
#include "game/client/render/StrayRenderer.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/Textures.hpp"
#include "game/client/render/ThrownItemRenderer.hpp"
#include "game/client/render/ThrownPotionRenderer.hpp"
#include "game/client/render/ThrownTridentRenderer.hpp"
#include "game/client/render/TippableArrowRenderer.hpp"
#include "game/client/render/TntMinecartRenderer.hpp"
#include "game/client/render/TntRenderer.hpp"
#include "game/client/render/TropicalFishRenderer.hpp"
#include "game/client/render/TurtleRenderer.hpp"
#include "game/client/render/UndeadHorseRenderer.hpp"
#include "game/client/render/VexRenderer.hpp"
#include "game/client/render/VillagerGolemRenderer.hpp"
#include "game/client/render/VillagerRenderer.hpp"
#include "game/client/render/VindicationIllagerRenderer.hpp"
#include "game/client/render/WitchRenderer.hpp"
#include "game/client/render/WitherBossRenderer.hpp"
#include "game/client/render/WitherSkeletonRenderer.hpp"
#include "game/client/render/WitherSkullRenderer.hpp"
#include "game/client/render/WolfModel.hpp"
#include "game/client/render/WolfRenderer.hpp"
#include "game/client/render/ZombieRenderer.hpp"
#include "game/client/render/ZombieVillagerRenderer.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Culler.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

EntityRenderDispatcher::EntityRenderDispatcher(Textures* textures, ItemRenderer* itemRenderer)
    : boost::unordered_map<unsigned long long, EntityRenderer*>(), mPlayerRenderers(),
      mX(0.0), mY(0.0), mZ(0.0), mTextures(textures), mCamera(), mPlayer(),
      mRenderShadow(true), mRenderHitBoxes(false)
{
    GlStateManager::enableLighting();
    (*this)[0x000000005D560040ULL] = new SpiderRenderer(this);
    (*this)[0x000000005D560041ULL] = new CaveSpiderRenderer(this);
    (*this)[0x000000002D680003ULL] = new PigRenderer(this);
    (*this)[0x000000002D680002ULL] = new SheepRenderer(this, new SheepModel(ModelDefinition()), new SheepFurModel(ModelDefinition()), 0.7F);
    (*this)[0x000000002D6A0000ULL] = new CowRenderer(this, new CowModel(ModelDefinition()), 0.7F);
    (*this)[0x000000002D6A0001ULL] = new MushroomCowRenderer(this);
    (*this)[0x000000000D6C0002ULL] = new WolfRenderer(this, new WolfModel(ModelDefinition()), 0.5F);
    (*this)[0x000000000D680004ULL] = new ChickenRenderer(this, new ChickenModel(ModelDefinition()), 0.3F);
    (*this)[0x000000002D6C0001ULL] = new OcelotRenderer(this);
    (*this)[0x000000002D680005ULL] = new RabbitRenderer(this);
    (*this)[0x000000002D6C0003ULL] = new ParrotRenderer(this);
    (*this)[0x000000005D540044ULL] = new SilverfishRenderer(this);
    (*this)[0x000000004D54004BULL] = new EndermiteRenderer(this);
    (*this)[0x000000005D540041ULL] = new CreeperRenderer(this);
    (*this)[0x000000005D540043ULL] = new EndermanRenderer(this);
    (*this)[0x000000002D580001ULL] = new SnowManRenderer(this);
    (*this)[0x000000005D540441ULL] = new SkeletonRenderer(this);
    (*this)[0x000000005D540442ULL] = new WitherSkeletonRenderer(this);
    (*this)[0x000000005D540443ULL] = new StrayRenderer(this);
    (*this)[0x000000005D540046ULL] = new WitchRenderer(this);
    (*this)[0x000000005D540045ULL] = new BlazeRenderer(this);
    (*this)[0x000000005D550041ULL] = new PigZombieRenderer(this);
    (*this)[0x000000005D550040ULL] = new ZombieRenderer(this);
    (*this)[0x000000005D550042ULL] = new ZombieVillagerRenderer(this);
    (*this)[0x000000005D550043ULL] = new HuskRenderer(this);
    (*this)[0x000000005D550044ULL] = new DrownedRenderer(this);
    (*this)[0x000000005D108000ULL] = new SlimeRenderer(this);
    (*this)[0x000000005D108001ULL] = new LavaSlimeRenderer(this);
    (*this)[0x000000005D540042ULL] = new GiantMobRenderer(this, 6.0F);
    (*this)[0x000000005D0000C1ULL] = new GhastRenderer(this);
    (*this)[0x000000000D502001ULL] = new SquidRenderer(this);
    (*this)[0x000000000D502005ULL] = new TropicalFishRenderer(this);
    (*this)[0x000000000D502002ULL] = new FishRenderer(this, FishRenderer::FISH_TYPE_COD, 0.2F);
    (*this)[0x000000000D502003ULL] = new PufferfishRenderer(this);
    (*this)[0x000000000D502004ULL] = new FishRenderer(this, FishRenderer::FISH_TYPE_SALMON, 0.25F);
    (*this)[0x000000002D680007ULL] = new TurtleRenderer(this);
    (*this)[0x000000000D602001ULL] = new DolphinRenderer(this);
    (*this)[0x000000000D600001ULL] = new VillagerRenderer(this);
    (*this)[0x000000000D580002ULL] = new VillagerGolemRenderer(this);
    (*this)[0x000000001D000101ULL] = new BatRenderer(this);
    (*this)[0x000000005D544000ULL] = new GuardianRenderer(this);
    (*this)[0x000000005D544001ULL] = new ElderGuardianRenderer(this);
    (*this)[0x000000004D580043ULL] = new ShulkerRenderer(this);
    (*this)[0x000000002D680006ULL] = new PolarBearRenderer(this);
    (*this)[0x000000005D541841ULL] = new EvocationIllagerRenderer(this);
    (*this)[0x000000005D541041ULL] = new VindicationIllagerRenderer(this);
    (*this)[0x000000005D540048ULL] = new VexRenderer(this);
    (*this)[0x000000005D540049ULL] = new PhantomRenderer(this);
    (*this)[0x000000005D541842ULL] = new IllusionIllagerRenderer(this);
    (*this)[0x000000000D000005ULL] = new EnderDragonRenderer(this);
    (*this)[0x000000000800000EULL] = new EnderCrystalRenderer(this);
    (*this)[0x000000005D540047ULL] = new WitherBossRenderer(this);
    (*this)[0x0000000008000000ULL] = new DefaultRenderer(this);
    (*this)[0x000000000A080001ULL] = new PaintingRenderer(this);
    (*this)[0x000000000A080002ULL] = new ItemFrameRenderer(this, itemRenderer);
    (*this)[0x000000000A080003ULL] = new LeashKnotRenderer(this);
    (*this)[0x000000008A020000ULL] = new ArrowRenderer(this);
    (*this)[0x000000008A020002ULL] = new TippableArrowRenderer(this);
    (*this)[0x000000008A020001ULL] = new SpectralArrowRenderer(this);
    (*this)[0x000000008A200001ULL] = new ThrownItemRenderer(this, Items::snowball);
    (*this)[0x000000008A200003ULL] = new ThrownItemRenderer(this, Items::enderPearl);
    (*this)[0x0000000088000004ULL] = new ThrownItemRenderer(this, Items::enderEye);
    (*this)[0x000000008A200002ULL] = new ThrownItemRenderer(this, Items::egg);
    (*this)[0x000000008A200004ULL] = new ThrownPotionRenderer(this);
    (*this)[0x000000008A200005ULL] = new ThrownItemRenderer(this, Items::experienceBottle);
    (*this)[0x0000000088000005ULL] = new ThrownItemRenderer(this, Items::fireworks);
    (*this)[0x000000008A400003ULL] = new FireballRenderer(this, 2.0F);
    (*this)[0x000000008A400004ULL] = new FireballRenderer(this, 0.5F);
    (*this)[0x000000008A400001ULL] = new DragonFireballRenderer(this);
    (*this)[0x000000008A400002ULL] = new WitherSkullRenderer(this);
    (*this)[0x0000000008000010ULL] = new ShulkerBulletRenderer(this);
    (*this)[0x000000000800000AULL] = new ItemEntityRenderer(this, itemRenderer);
    (*this)[0x0000000008000003ULL] = new ExperienceOrbRenderer(this);
    (*this)[0x000000000800000BULL] = new TntRenderer(this);
    (*this)[0x0000000008000009ULL] = new FallingBlockRenderer(this);
    (*this)[0x000000000C000001ULL] = new ArmorStandRenderer(this);
    (*this)[0x0000000008000011ULL] = new EvocationFangsRenderer(this);
    (*this)[0x000000000A800004ULL] = new TntMinecartRenderer(this);
    (*this)[0x000000000A800006ULL] = new MinecartSpawnerRenderer(this);
    (*this)[0x000000000A800001ULL] = new MinecartRenderer(this);
    (*this)[0x000000000A800003ULL] = new MinecartRenderer(this);
    (*this)[0x000000000A840002ULL] = new MinecartRenderer(this);
    (*this)[0x000000000A840005ULL] = new MinecartRenderer(this);
    (*this)[0x0000000008000008ULL] = new BoatRenderer(this);
    (*this)[0x0000000088000006ULL] = new FishingHookRenderer(this);
    (*this)[0x000000000800000FULL] = new AreaEffectCloudRenderer(this);
    (*this)[0x000000002D690001ULL] = new HorseRenderer(this);
    (*this)[0x000000002D690002ULL] = new UndeadHorseRenderer(this);
    (*this)[0x000000002D690003ULL] = new UndeadHorseRenderer(this);
    (*this)[0x000000002D698002ULL] = new ChestedHorseRenderer(this, 0.92F);
    (*this)[0x000000002D698001ULL] = new ChestedHorseRenderer(this, 0.87F);
    (*this)[0x000000002D698003ULL] = new LlamaRenderer(this);
    (*this)[0x0000000088000012ULL] = new LlamaSpitRenderer(this);
    (*this)[0x000000000B000001ULL] = new LightningBoltRenderer(this);
    (*this)[0x000000008A200006ULL] = new ThrownTridentRenderer(this);
    mPlayerRenderer = new PlayerRenderer(this, false, false);
    mPlayerRenderers[EPLAYER_MODEL_DEFAULT] = mPlayerRenderer;
    mPlayerRenderers[EPLAYER_MODEL_SLIM] = new PlayerRenderer(this, false, true);
    mPlayerRenderers[EPLAYER_MODEL_SLIM_64] = new PlayerRenderer(this, true, true);
    GlStateManager::disableLighting();
    mRenderName = false;
    mRenderLeash = false;
    mSolidRendering = false;
}

double EntityRenderDispatcher::distanceToSqr(double x, double y, double z)
{
    x -= mCameraX;
    y -= mCameraY;
    z -= mCameraZ;
    return x * x + y * y + z * z;
}

unsigned char EntityRenderDispatcher::shouldRenderShadow()
{
    return mRenderShadow;
}

Font* EntityRenderDispatcher::getFont()
{
    return mFont;
}

void EntityRenderDispatcher::setPosition(double x, double y, double z)
{
    mX = x;
    mY = y;
    mZ = z;
}

EntityRenderer* EntityRenderDispatcher::getRenderer(unsigned long long type)
{
    if ((type & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
        return mPlayerRenderer;
    iterator it = find(type);
    return it->second;
}

PlayerRenderer* EntityRenderDispatcher::getPlayerRenderer(EPlayerModelType modelType)
{
    boost::unordered_map<EPlayerModelType, PlayerRenderer*>::iterator it = mPlayerRenderers.find(modelType);
    if (it != mPlayerRenderers.end())
        return it->second;
    return mPlayerRenderer;
}

EntityRenderer* EntityRenderDispatcher::getRenderer(boost::shared_ptr<Entity> entity)
{
    if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        if (player->isSpectator() && player->SpectateAsOtherEntity())
            return getRenderer(player->GetSpectateEntity());
        return getPlayerRenderer(static_cast<EPlayerModelType>(player->getModelName()));
    }

    if (entity->GetType() == 0x0000000088000005ULL)
        return getRenderer(0x000000008A020000ULL);

    return getRenderer(entity->GetType());
}

void EntityRenderDispatcher::prepare(Level* level, Font* font, boost::shared_ptr<Entity> camera,
                                     boost::shared_ptr<Entity> player, Options* options, float partialTicks)
{
    mLevel = level;
    mOptions = options;
    mCamera = camera;
    mFont = font;
    mPlayer = player;

    bool sleeping = (camera->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY
        && boost::static_pointer_cast<LivingEntity>(camera)->isSleeping();

    if (sleeping)
    {
        const BlockState* state = level->getBlockState(BlockPos(camera));
        if (state->getBlock() == Blocks::bed)
        {
            mCameraYRot = (float)(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)
                                      ->get2DDataValue() * 90 + 180);
            mCameraXRot = 0.0F;
        }
    }
    else
    {
        mCameraYRot = (camera->mYRot - camera->mYRotO) * partialTicks
            + Minecraft::GetInstance()->mPlayer->mYRotO;
        mCameraXRot = (camera->mXRot - camera->mXRotO) * partialTicks
            + Minecraft::GetInstance()->mPlayer->mXRotO;
    }

    if ((Minecraft::GetInstance()->mPlayer->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER
        && boost::static_pointer_cast<Player>(camera)->GetThirdPersonView(false) == 2)
    {
        mCameraYRot += 180.0F;
        mCameraXRot = -((camera->mXRot - camera->mXRotO) * partialTicks + camera->mXRotO);
    }

    mCameraX = (camera->mX - camera->mXOld) * partialTicks + camera->mXOld;
    mCameraY = (camera->mY - camera->mYOld) * partialTicks + camera->mYOld;
    mCameraZ = (camera->mZ - camera->mZOld) * partialTicks + camera->mZOld;
}

bool EntityRenderDispatcher::hasSecondPass(boost::shared_ptr<Entity>& entity)
{
    return getRenderer(entity)->hasSecondPass();
}

bool EntityRenderDispatcher::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler,
                                          double x, double y, double z)
{
    EntityRenderer* renderer = getRenderer(entity);
    return renderer != NULL && renderer->shouldRender(entity, culler, x, y, z);
}

bool EntityRenderDispatcher::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double distance)
{
    EntityRenderer* renderer = getRenderer(entity);
    return renderer != NULL && renderer->shouldRender(entity, culler, distance);
}

void EntityRenderDispatcher::renderHitbox(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                          float, float partialTicks, int color)
{
    GlStateManager::depthMask(false);
    GlStateManager::disableTexture();
    GlStateManager::disableLighting();
    GlStateManager::disableCull();
    GlStateManager::disableBlend();
    GlStateManager::pushMatrix();

    float halfWidth = entity->mBbWidth * 0.5F;
    AABB* box = entity->mBoundingBox;
    LevelRenderer::renderLineBox(AABB::newTemp(box->mMinX - entity->mX + x,
                                               box->mMinY - entity->mY + y,
                                               box->mMinZ - entity->mZ + z,
                                               box->mMaxX - entity->mX + x,
                                               box->mMaxY - entity->mY + y,
                                               box->mMaxZ - entity->mZ + z), color);

    if ((entity->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
    {
        LevelRenderer::renderLineBox(AABB::newTemp(x - halfWidth,
                                                   y + entity->getEyeHeight() - 0.01F,
                                                   z - halfWidth,
                                                   x + halfWidth,
                                                   y + entity->getEyeHeight() + 0.01F,
                                                   z + halfWidth), 0xFFFFFF);
    }

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Vec3* look = entity->getViewVector(partialTicks);
    builder->begin(5);
    builder->color(0xFF);
    builder->vertex((float)x, (float)(y + entity->getEyeHeight()), (float)z);
    builder->vertex((float)(x + look->x * 2.0),
                    (float)(y + entity->getEyeHeight() + look->y * 2.0),
                    (float)(z + look->z * 2.0));
    builder->end();

    GlStateManager::popMatrix();
    GlStateManager::enableTexture();
    GlStateManager::enableLighting();
    GlStateManager::enableCull();
    GlStateManager::disableBlend();
    GlStateManager::depthMask(true);
}

void EntityRenderDispatcher::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                    float yaw, float partialTicks, bool renderOutlines, bool itemFrame,
                                    bool renderShadow, bool renderLeash)
{
    EntityRenderer* renderer = getRenderer(entity);
    if (renderer == NULL || mTextures == NULL)
        return;

    renderer->SetItemFrame(itemFrame);
    renderer->setSolidRender(mSolidRendering);
    mRenderLeash = renderLeash;
    renderer->render(entity, x, y, z, yaw, partialTicks);
    mRenderLeash = false;

    if (!mSolidRendering)
        renderer->postRender(entity, x, y, z, yaw, partialTicks, renderShadow);

    if (mRenderHitBoxes && !entity->isInvisible() && !renderOutlines
        && !Minecraft::GetInstance()->showOnlyReducedInfo())
    {
        renderHitbox(entity, x, y, z, yaw, partialTicks, 0xFFFFFF);
    }
}

void EntityRenderDispatcher::render(boost::shared_ptr<Entity>& entity, float partialTicks, bool renderOutlines)
{
    double x = (entity->mX - entity->mXOld) * partialTicks + entity->mXOld - mX;
    double y = (entity->mY - entity->mYOld) * partialTicks + entity->mYOld - mY;
    double z = (entity->mZ - entity->mZOld) * partialTicks + entity->mZOld - mZ;

    float rot = entity->mYRot - entity->mYRotO;
    if (rot > 180.0F || rot < -180.0F)
    {
        if (entity->mYRotO < entity->mYRot)
            rot = entity->mYRot - 360.0F - entity->mYRotO;
        else
            rot = entity->mYRot - (entity->mYRotO - 360.0F);
    }
    float yaw = rot * partialTicks + entity->mYRotO;

    int light = entity->getLightColor();
    if (entity->isOnFire())
        light = 0x00F000F0;
    GlStateManager::multiTexCoord2f((float)(light % 65536), (float)(light / 65536));
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);

    render(entity, x, y, z, yaw, partialTicks, renderOutlines,
           entity->GetType() == 0x000000000A080002ULL, entity->ShouldRenderShadow(), false);
}

void EntityRenderDispatcher::renderSecondPass(boost::shared_ptr<Entity>& entity, float partialTicks)
{
    if (entity->mTickCount == 0)
    {
        entity->mXOld = entity->mX;
        entity->mYOld = entity->mY;
        entity->mZOld = entity->mZ;
    }

    double x = (entity->mX - entity->mXOld) * partialTicks + entity->mXOld - mX;
    double y = (entity->mY - entity->mYOld) * partialTicks + entity->mYOld - mY;
    double z = (entity->mZ - entity->mZOld) * partialTicks + entity->mZOld - mZ;
    float yaw = (entity->mYRot - entity->mYRotO) * partialTicks + entity->mYRotO;

    int light = entity->getLightColor();
    if (entity->isOnFire())
        light = 0x00F000F0;
    GlStateManager::multiTexCoord2f((float)(light % 65536), (float)(light / 65536));
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);

    EntityRenderer* renderer = getRenderer(entity);
    if (renderer != NULL && mTextures != NULL)
        renderer->renderSecondPass(entity, x, y, z, yaw, partialTicks);
}

void EntityRenderDispatcher::setupForTexturePack(TexturePack* texturePack)
{
    for (iterator it = begin(); it != end(); ++it)
        it->second->setupForTexturePack(texturePack);

    boost::unordered_map<EPlayerModelType, PlayerRenderer*>::iterator player = mPlayerRenderers.begin();
    for (; player != mPlayerRenderers.end(); ++player)
        player->second->setupForTexturePack(texturePack);
}
