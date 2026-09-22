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

namespace
{
const unsigned long long ENTITY_RENDERER_TYPE_SPIDER = 0x000000005D560040ULL;
const unsigned long long ENTITY_RENDERER_TYPE_CAVESPIDER = 0x000000005D560041ULL;
const unsigned long long ENTITY_RENDERER_TYPE_PIG = 0x000000002D680003ULL;
const unsigned long long ENTITY_RENDERER_TYPE_SHEEP = 0x000000002D680002ULL;
const unsigned long long ENTITY_RENDERER_TYPE_COW = 0x000000002D6A0000ULL;
const unsigned long long ENTITY_RENDERER_TYPE_MUSHROOMCOW = 0x000000002D6A0001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_WOLF = 0x000000000D6C0002ULL;
const unsigned long long ENTITY_RENDERER_TYPE_CHICKEN = 0x000000000D680004ULL;
const unsigned long long ENTITY_RENDERER_TYPE_OCELOT = 0x000000002D6C0001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_RABBIT = 0x000000002D680005ULL;
const unsigned long long ENTITY_RENDERER_TYPE_PARROT = 0x000000002D6C0003ULL;
const unsigned long long ENTITY_RENDERER_TYPE_SILVERFISH = 0x000000005D540044ULL;
const unsigned long long ENTITY_RENDERER_TYPE_ENDERMITE = 0x000000004D54004BULL;
const unsigned long long ENTITY_RENDERER_TYPE_CREEPER = 0x000000005D540041ULL;
const unsigned long long ENTITY_RENDERER_TYPE_ENDERMAN = 0x000000005D540043ULL;
const unsigned long long ENTITY_RENDERER_TYPE_SNOWMAN = 0x000000002D580001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_SKELETON = 0x000000005D540441ULL;
const unsigned long long ENTITY_RENDERER_TYPE_WITHERSKELETON = 0x000000005D540442ULL;
const unsigned long long ENTITY_RENDERER_TYPE_STRAY = 0x000000005D540443ULL;
const unsigned long long ENTITY_RENDERER_TYPE_WITCH = 0x000000005D540046ULL;
const unsigned long long ENTITY_RENDERER_TYPE_BLAZE = 0x000000005D540045ULL;
const unsigned long long ENTITY_RENDERER_TYPE_PIGZOMBIE = 0x000000005D550041ULL;
const unsigned long long ENTITY_RENDERER_TYPE_ZOMBIE = 0x000000005D550040ULL;
const unsigned long long ENTITY_RENDERER_TYPE_ZOMBIEVILLAGER = 0x000000005D550042ULL;
const unsigned long long ENTITY_RENDERER_TYPE_HUSK = 0x000000005D550043ULL;
const unsigned long long ENTITY_RENDERER_TYPE_DROWNED = 0x000000005D550044ULL;
const unsigned long long ENTITY_RENDERER_TYPE_SLIME = 0x000000005D108000ULL;
const unsigned long long ENTITY_RENDERER_TYPE_LAVASLIME = 0x000000005D108001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_GIANTMOB = 0x000000005D540042ULL;
const unsigned long long ENTITY_RENDERER_TYPE_GHAST = 0x000000005D0000C1ULL;
const unsigned long long ENTITY_RENDERER_TYPE_SQUID = 0x000000000D502001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_TROPICALFISH = 0x000000000D502005ULL;
const unsigned long long ENTITY_RENDERER_TYPE_FISH = 0x000000000D502002ULL;
const unsigned long long ENTITY_RENDERER_TYPE_PUFFERFISH = 0x000000000D502003ULL;
const unsigned long long ENTITY_RENDERER_TYPE_FISH_2 = 0x000000000D502004ULL;
const unsigned long long ENTITY_RENDERER_TYPE_TURTLE = 0x000000002D680007ULL;
const unsigned long long ENTITY_RENDERER_TYPE_DOLPHIN = 0x000000000D602001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_VILLAGER = 0x000000000D600001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_VILLAGERGOLEM = 0x000000000D580002ULL;
const unsigned long long ENTITY_RENDERER_TYPE_BAT = 0x000000001D000101ULL;
const unsigned long long ENTITY_RENDERER_TYPE_GUARDIAN = 0x000000005D544000ULL;
const unsigned long long ENTITY_RENDERER_TYPE_ELDERGUARDIAN = 0x000000005D544001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_SHULKER = 0x000000004D580043ULL;
const unsigned long long ENTITY_RENDERER_TYPE_POLARBEAR = 0x000000002D680006ULL;
const unsigned long long ENTITY_RENDERER_TYPE_EVOCATIONILLAGER = 0x000000005D541841ULL;
const unsigned long long ENTITY_RENDERER_TYPE_VINDICATIONILLAGER = 0x000000005D541041ULL;
const unsigned long long ENTITY_RENDERER_TYPE_VEX = 0x000000005D540048ULL;
const unsigned long long ENTITY_RENDERER_TYPE_PHANTOM = 0x000000005D540049ULL;
const unsigned long long ENTITY_RENDERER_TYPE_ILLUSIONILLAGER = 0x000000005D541842ULL;
const unsigned long long ENTITY_RENDERER_TYPE_ENDERDRAGON = 0x000000000D000005ULL;
const unsigned long long ENTITY_RENDERER_TYPE_ENDERCRYSTAL = 0x000000000800000EULL;
const unsigned long long ENTITY_RENDERER_TYPE_WITHERBOSS = 0x000000005D540047ULL;
const unsigned long long ENTITY_RENDERER_TYPE_DEFAULT = 0x0000000008000000ULL;
const unsigned long long ENTITY_RENDERER_TYPE_PAINTING = 0x000000000A080001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_LEASHKNOT = 0x000000000A080003ULL;
const unsigned long long ENTITY_RENDERER_TYPE_TIPPABLEARROW = 0x000000008A020002ULL;
const unsigned long long ENTITY_RENDERER_TYPE_SPECTRALARROW = 0x000000008A020001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_THROWNITEM = 0x000000008A200001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_THROWNITEM_2 = 0x000000008A200003ULL;
const unsigned long long ENTITY_RENDERER_TYPE_THROWNITEM_3 = 0x0000000088000004ULL;
const unsigned long long ENTITY_RENDERER_TYPE_THROWNITEM_4 = 0x000000008A200002ULL;
const unsigned long long ENTITY_RENDERER_TYPE_THROWNPOTION = 0x000000008A200004ULL;
const unsigned long long ENTITY_RENDERER_TYPE_THROWNITEM_5 = 0x000000008A200005ULL;
const unsigned long long ENTITY_RENDERER_TYPE_FIREBALL = 0x000000008A400003ULL;
const unsigned long long ENTITY_RENDERER_TYPE_FIREBALL_2 = 0x000000008A400004ULL;
const unsigned long long ENTITY_RENDERER_TYPE_DRAGONFIREBALL = 0x000000008A400001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_WITHERSKULL = 0x000000008A400002ULL;
const unsigned long long ENTITY_RENDERER_TYPE_SHULKERBULLET = 0x0000000008000010ULL;
const unsigned long long ENTITY_RENDERER_TYPE_ITEMENTITY = 0x000000000800000AULL;
const unsigned long long ENTITY_RENDERER_TYPE_EXPERIENCEORB = 0x0000000008000003ULL;
const unsigned long long ENTITY_RENDERER_TYPE_TNT = 0x000000000800000BULL;
const unsigned long long ENTITY_RENDERER_TYPE_FALLINGBLOCK = 0x0000000008000009ULL;
const unsigned long long ENTITY_RENDERER_TYPE_ARMORSTAND = 0x000000000C000001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_EVOCATIONFANGS = 0x0000000008000011ULL;
const unsigned long long ENTITY_RENDERER_TYPE_TNTMINECART = 0x000000000A800004ULL;
const unsigned long long ENTITY_RENDERER_TYPE_MINECARTSPAWNER = 0x000000000A800006ULL;
const unsigned long long ENTITY_RENDERER_TYPE_MINECART = 0x000000000A800001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_MINECART_2 = 0x000000000A800003ULL;
const unsigned long long ENTITY_RENDERER_TYPE_MINECART_3 = 0x000000000A840002ULL;
const unsigned long long ENTITY_RENDERER_TYPE_MINECART_4 = 0x000000000A840005ULL;
const unsigned long long ENTITY_RENDERER_TYPE_BOAT = 0x0000000008000008ULL;
const unsigned long long ENTITY_RENDERER_TYPE_FISHINGHOOK = 0x0000000088000006ULL;
const unsigned long long ENTITY_RENDERER_TYPE_AREAEFFECTCLOUD = 0x000000000800000FULL;
const unsigned long long ENTITY_RENDERER_TYPE_HORSE = 0x000000002D690001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_UNDEADHORSE = 0x000000002D690002ULL;
const unsigned long long ENTITY_RENDERER_TYPE_UNDEADHORSE_2 = 0x000000002D690003ULL;
const unsigned long long ENTITY_RENDERER_TYPE_CHESTEDHORSE = 0x000000002D698002ULL;
const unsigned long long ENTITY_RENDERER_TYPE_CHESTEDHORSE_2 = 0x000000002D698001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_LLAMA = 0x000000002D698003ULL;
const unsigned long long ENTITY_RENDERER_TYPE_LLAMASPIT = 0x0000000088000012ULL;
const unsigned long long ENTITY_RENDERER_TYPE_LIGHTNINGBOLT = 0x000000000B000001ULL;
const unsigned long long ENTITY_RENDERER_TYPE_THROWNTRIDENT = 0x000000008A200006ULL;
const unsigned long long FIREWORK_ROCKET_ENTITY_TYPE = 0x0000000088000005ULL;
const unsigned long long ITEM_FRAME_ENTITY_TYPE = 0x000000000A080002ULL;
const unsigned long long ARROW_ENTITY_TYPE = 0x000000008A020000ULL;
}
EntityRenderDispatcher::EntityRenderDispatcher(Textures* textures, ItemRenderer* itemRenderer)
    : boost::unordered_map<unsigned long long, EntityRenderer*>(), mPlayerRenderers(),
      mX(0.0), mY(0.0), mZ(0.0), mTextures(textures), mCamera(), mPlayer(),
      mRenderShadow(true), mRenderHitBoxes(false)
{
    GlStateManager::enableLighting();
    (*this)[ENTITY_RENDERER_TYPE_SPIDER] = new SpiderRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_CAVESPIDER] = new CaveSpiderRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_PIG] = new PigRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_SHEEP] = new SheepRenderer(this, new SheepModel(ModelDefinition()), new SheepFurModel(ModelDefinition()), 0.7F);
    (*this)[ENTITY_RENDERER_TYPE_COW] = new CowRenderer(this, new CowModel(ModelDefinition()), 0.7F);
    (*this)[ENTITY_RENDERER_TYPE_MUSHROOMCOW] = new MushroomCowRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_WOLF] = new WolfRenderer(this, new WolfModel(ModelDefinition()), 0.5F);
    (*this)[ENTITY_RENDERER_TYPE_CHICKEN] = new ChickenRenderer(this, new ChickenModel(ModelDefinition()), 0.3F);
    (*this)[ENTITY_RENDERER_TYPE_OCELOT] = new OcelotRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_RABBIT] = new RabbitRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_PARROT] = new ParrotRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_SILVERFISH] = new SilverfishRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_ENDERMITE] = new EndermiteRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_CREEPER] = new CreeperRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_ENDERMAN] = new EndermanRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_SNOWMAN] = new SnowManRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_SKELETON] = new SkeletonRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_WITHERSKELETON] = new WitherSkeletonRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_STRAY] = new StrayRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_WITCH] = new WitchRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_BLAZE] = new BlazeRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_PIGZOMBIE] = new PigZombieRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_ZOMBIE] = new ZombieRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_ZOMBIEVILLAGER] = new ZombieVillagerRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_HUSK] = new HuskRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_DROWNED] = new DrownedRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_SLIME] = new SlimeRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_LAVASLIME] = new LavaSlimeRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_GIANTMOB] = new GiantMobRenderer(this, 6.0F);
    (*this)[ENTITY_RENDERER_TYPE_GHAST] = new GhastRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_SQUID] = new SquidRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_TROPICALFISH] = new TropicalFishRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_FISH] = new FishRenderer(this, FishRenderer::FISH_TYPE_COD, 0.2F);
    (*this)[ENTITY_RENDERER_TYPE_PUFFERFISH] = new PufferfishRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_FISH_2] = new FishRenderer(this, FishRenderer::FISH_TYPE_SALMON, 0.25F);
    (*this)[ENTITY_RENDERER_TYPE_TURTLE] = new TurtleRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_DOLPHIN] = new DolphinRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_VILLAGER] = new VillagerRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_VILLAGERGOLEM] = new VillagerGolemRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_BAT] = new BatRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_GUARDIAN] = new GuardianRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_ELDERGUARDIAN] = new ElderGuardianRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_SHULKER] = new ShulkerRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_POLARBEAR] = new PolarBearRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_EVOCATIONILLAGER] = new EvocationIllagerRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_VINDICATIONILLAGER] = new VindicationIllagerRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_VEX] = new VexRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_PHANTOM] = new PhantomRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_ILLUSIONILLAGER] = new IllusionIllagerRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_ENDERDRAGON] = new EnderDragonRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_ENDERCRYSTAL] = new EnderCrystalRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_WITHERBOSS] = new WitherBossRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_DEFAULT] = new DefaultRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_PAINTING] = new PaintingRenderer(this);
    (*this)[ITEM_FRAME_ENTITY_TYPE] = new ItemFrameRenderer(this, itemRenderer);
    (*this)[ENTITY_RENDERER_TYPE_LEASHKNOT] = new LeashKnotRenderer(this);
    (*this)[ARROW_ENTITY_TYPE] = new ArrowRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_TIPPABLEARROW] = new TippableArrowRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_SPECTRALARROW] = new SpectralArrowRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_THROWNITEM] = new ThrownItemRenderer(this, Items::snowball);
    (*this)[ENTITY_RENDERER_TYPE_THROWNITEM_2] = new ThrownItemRenderer(this, Items::enderPearl);
    (*this)[ENTITY_RENDERER_TYPE_THROWNITEM_3] = new ThrownItemRenderer(this, Items::enderEye);
    (*this)[ENTITY_RENDERER_TYPE_THROWNITEM_4] = new ThrownItemRenderer(this, Items::egg);
    (*this)[ENTITY_RENDERER_TYPE_THROWNPOTION] = new ThrownPotionRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_THROWNITEM_5] = new ThrownItemRenderer(this, Items::experienceBottle);
    (*this)[FIREWORK_ROCKET_ENTITY_TYPE] = new ThrownItemRenderer(this, Items::fireworks);
    (*this)[ENTITY_RENDERER_TYPE_FIREBALL] = new FireballRenderer(this, 2.0F);
    (*this)[ENTITY_RENDERER_TYPE_FIREBALL_2] = new FireballRenderer(this, 0.5F);
    (*this)[ENTITY_RENDERER_TYPE_DRAGONFIREBALL] = new DragonFireballRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_WITHERSKULL] = new WitherSkullRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_SHULKERBULLET] = new ShulkerBulletRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_ITEMENTITY] = new ItemEntityRenderer(this, itemRenderer);
    (*this)[ENTITY_RENDERER_TYPE_EXPERIENCEORB] = new ExperienceOrbRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_TNT] = new TntRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_FALLINGBLOCK] = new FallingBlockRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_ARMORSTAND] = new ArmorStandRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_EVOCATIONFANGS] = new EvocationFangsRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_TNTMINECART] = new TntMinecartRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_MINECARTSPAWNER] = new MinecartSpawnerRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_MINECART] = new MinecartRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_MINECART_2] = new MinecartRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_MINECART_3] = new MinecartRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_MINECART_4] = new MinecartRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_BOAT] = new BoatRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_FISHINGHOOK] = new FishingHookRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_AREAEFFECTCLOUD] = new AreaEffectCloudRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_HORSE] = new HorseRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_UNDEADHORSE] = new UndeadHorseRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_UNDEADHORSE_2] = new UndeadHorseRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_CHESTEDHORSE] = new ChestedHorseRenderer(this, 0.92F);
    (*this)[ENTITY_RENDERER_TYPE_CHESTEDHORSE_2] = new ChestedHorseRenderer(this, 0.87F);
    (*this)[ENTITY_RENDERER_TYPE_LLAMA] = new LlamaRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_LLAMASPIT] = new LlamaSpitRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_LIGHTNINGBOLT] = new LightningBoltRenderer(this);
    (*this)[ENTITY_RENDERER_TYPE_THROWNTRIDENT] = new ThrownTridentRenderer(this);
    mPlayerRenderer = new PlayerRenderer(this, false, false);
    const EPlayerModelType defaultModel = EPLAYER_MODEL_DEFAULT;
    mPlayerRenderers[defaultModel] = mPlayerRenderer;
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

    if (entity->GetType() == FIREWORK_ROCKET_ENTITY_TYPE)
        return getRenderer(ARROW_ENTITY_TYPE);

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
    EntityRenderer* renderer = getRenderer(entity);
    return renderer != NULL && renderer->hasSecondPass();
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
           entity->GetType() == ITEM_FRAME_ENTITY_TYPE, entity->ShouldRenderShadow(), false);
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
