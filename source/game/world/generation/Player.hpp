#ifndef GAME_WORLD_GENERATION_PLAYER_HPP
#define GAME_WORLD_GENERATION_PLAYER_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/core/StatsUID.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/item/ItemCooldowns.hpp"
#include "game/item/PowerupItems.hpp"
#include "game/world/block/Predicate.hpp"
#include "vendor/_SkinAdjustments.hpp"
#include "game/core/Abilities.hpp"
#include "game/core/FoodData.hpp"

class ItemInstance;
class AbstractContainerMenu;
class Container;
class FishingHook;
class ModelPart;
class SoundEvent;
class FoodData;
class GameMode;
class Inventory;
class PlayerEnderChestContainer;
class SignBlockEntity;
class ItemEntity;
class MultiPlayerGameMode;




struct Achievement;
struct Block;
struct BlockPos;
struct BlockState;
struct CameraController;
struct CompoundTag;
struct DamageSource;
struct DataFixerUpper;
struct Direction;
struct EquipmentSlot;
struct GameType;
struct HumanoidArm;
struct InteractionObject;
struct Level;
struct LockCode;
struct PlayerModelPart;
struct PlayerUID;
struct Stat;
class Scoreboard;
struct arr_unsigned_char;
struct not_null_ptr_ItemInstance;
struct sp_Container;
struct sp_Entity;
struct sp_LivingEntity;
struct sp_Player;


struct Player : LivingEntity {

    friend class MultiPlayerGameMode;
    friend class ServerGamePacketListenerImpl;

    enum EPlayerGamePrivileges {
        ePlayerGamePrivilege_0 = 0,
        ePlayerGamePrivilege_1 = 1,
        ePlayerGamePrivilege_3 = 3,
        ePlayerGamePrivilege_4 = 4,
        ePlayerGamePrivilege_5 = 5,
        ePlayerGamePrivilege_6 = 6,
        ePlayerGamePrivilege_7 = 7,
        ePlayerGamePrivilege_8 = 8,
        ePlayerGamePrivilege_9 = 9,
        ePlayerGamePrivilege_10 = 10,
        ePlayerGamePrivilege_11 = 11,
        ePlayerGamePrivilege_12 = 12,
        ePlayerGamePrivilege_13 = 13,
        ePlayerGamePrivilege_14 = 14,
        ePlayerGamePrivilege_15 = 15,
        ePlayerGamePrivilege_16 = 16,
        ePlayerGamePrivilege_All = 0x21,
        ePlayerGamePrivilege_Moderator = 0x22
    };

    static const EntityDataAccessor<float>* DATA_PLAYER_ABSORPTION_ID;
    static const EntityDataAccessor<int>* DATA_SCORE_ID;
    static const EntityDataAccessor<unsigned char>* DATA_PLAYER_MODE_CUSTOMISATION;
    static const EntityDataAccessor<unsigned char>* DATA_MAIN_HAND;
    static const EntityDataAccessor<CompoundTag*>* DATA_SHOULDER_LEFT;
    static const EntityDataAccessor<CompoundTag*>* DATA_SHOULDER_RIGHT;


    class ChatVisiblity {
    public:
        ChatVisiblity(int id);
        int getId();
        static ChatVisiblity byId(int id);
        std::wstring getKey();
    private:
        int mId;
    };


    class PreventsMeFromSleeping : public Predicate<boost::shared_ptr<Entity> > {
    public:
        PreventsMeFromSleeping(boost::shared_ptr<Player> player);
        bool apply(boost::shared_ptr<Entity> entity) const;
        virtual ~PreventsMeFromSleeping();
    private:
        boost::shared_ptr<Player> mPlayer;
    };


    boost::shared_ptr<Inventory> mInventory;
    boost::shared_ptr<PlayerEnderChestContainer> mEnderChestInventory;
    AbstractContainerMenu* mInventoryMenu;
    AbstractContainerMenu* mContainerMenu;
    FoodData mFoodData;
    uint8_t mPadding0x61c[4];
    float mPrevCameraYaw;
    float mCameraYaw;
    int32_t mTakeXpDelay;
    uint8_t mPadding0x62c[28];
    int32_t mField0x648;
    std::wstring mCapeLocation;
    int32_t mField0x668;
    int32_t mField0x66c;
    double mPrevChasingPosX;
    double mPrevChasingPosY;
    double mPrevChasingPosZ;
    double mChasingPosX;
    double mChasingPosY;
    double mChasingPosZ;
    std::wstring mDisplayName;
    uint8_t mPadding0x6bc[4];
    uint8_t mField0x6c0;
    uint8_t mPadding0x6c1[3];
    BlockPos* mField0x6c4;
    int32_t mField0x6c8;
    int32_t mField0x6cc;
    float mField0x6d0;
    int32_t mField0x6d4;
    float mField0x6d8;
    int32_t mField0x6dc;
    BlockPos* mField0x6e0;
    uint8_t mField0x6e4;
    uint8_t mPadding0x6e5[3];
    int32_t mField0x6e8;
    int32_t mField0x6ec;
    int32_t mField0x6f0;
    int32_t mField0x6f4;
    int32_t mField0x6f8;
    int32_t mField0x6fc;
    int32_t mField0x700;
    int32_t mField0x704;
    int32_t mField0x708;
    Abilities mAbilities;
    int32_t mExperienceLevel;
    int32_t mTotalExperience;
    float mExperienceProgress;
    boost::shared_ptr<FishingHook> mFishing;
    int32_t mEnchantmentSeed;
    uint8_t mPadding0x734[4];
    ItemCooldowns* mField0x738;
    std::wstring mName;
    int32_t mField0x75c;
    uint8_t mField0x760;
    uint8_t mPadding0x761[3];
    int32_t mField0x764;
    uint32_t mField0x768;
    uint32_t mField0x76c;
    int32_t mField0x770;
    uint32_t mField0x774;
    uint32_t mField0x778;
    CameraController* mField0x77c;
    bool mField0x780;
    uint8_t mField0x781;
    uint8_t mField0x782;
    uint8_t mPadding0x783[1];
    PlayerUID m_0x784;
    PlayerUID m_0x798;
    StatsUID mSuid;
    uint8_t mPadding0x7c0[4];
    bool mShowOnMaps;
    bool mIsGuest;
    uint8_t mPadding0x7c6[18];
    uint32_t mField0x7d8;
    int32_t mField0x7dc;
    int32_t mField0x7e0;
    int32_t mPlayerIndex;
    unsigned int mDebugOptions;
    uint8_t mField0x7ec;
    uint8_t mPadding0x7ed[3];
    int32_t mField0x7f0;
    int32_t mField0x7f4;
    uint8_t mField0x7f8;
    uint8_t mPadding0x7f9[3];
    uint32_t mField0x7fc;
    std::vector<ModelPart*>* mAdditionalModelParts;
    uint8_t mField0x804;
    uint8_t mField0x805;
    uint8_t mPadding0x806[2];
    int32_t mField0x808;
    boost::shared_ptr<Entity> mCamera;
    bool mField0x814;
    bool mField0x815;
    uint8_t mPadding0x816[2];
    double mField0x818;
    double mField0x820;
    int32_t mField0x828;
    uint8_t mField0x82c;
    uint8_t mPadding0x82d[3];
    int32_t mField0x830;
    int32_t mField0x834;
    int32_t mField0x838;
    uint8_t mPadding0x83b[1];
    uint8_t mField0x83c;
    uint8_t mPadding0x83d[3];
    float mFlap;
    float mFlapSpeed;
    float mOFlapSpeed;
    float mOFlap;
    float mField0x850;
    float mField0x854;
    float mField0x858;
    uint8_t mField0x85c;
    uint8_t mField0x85d;
    uint8_t mField0x85e;
    uint8_t mPadding0x85f[1];
    int32_t mField0x860;
    int32_t mField0x864;


    Player(Level* , const std::wstring& );
    ~Player();


    std::wstring getDisplayName();
    bool isSendMessageEnabled();
    void SetPowerupTicks(int32_t , int32_t );
    void isStarving();
    void tickArmor();
    static void staticCtor();
    void _init();
    void registerAttributes();
    void defineSynchedData();
    bool shouldRenderAtSqrDistance(double );
    bool shouldTick();
    virtual bool isSpectator() = 0;
    bool checkBedExists();
    void updateFrameTick();
    StatsUID getSUID();
    void AwardTimerAchievement(int32_t );
    uint64_t CheckTimerAchievements();
    static EPlayerGamePrivileges getPlayerGamePrivilege(uint32_t , EPlayerGamePrivileges );
    EPlayerGamePrivileges getPlayerGamePrivilege(EPlayerGamePrivileges );
    uint32_t hasInvulnerablePrivilege();
    void tickCamera();
    void moveCloak();
    void resetAttackStrengthTicker();
    void updatePlayerSize();
    void PlayerRingSoundEffect(int32_t );
    void tick();
    int getPortalWaitTime();
    int GetSuffocateTime();
    float GetInhaleTime();
    bool generatesBubbles();
    const SoundEvent* getSwimSound();
    const SoundEvent* getSwimSplashSound();
    const SoundEvent* getHeavySplashSound();
    int getDimensionChangingDelay();
    void playSound(const SoundEvent* , float , float );
    SoundSource::ESoundSource getSoundSource();
    void handleEntityEvent(uint8_t , int32_t );
    bool isImmobile();
    void onChangedBlock(BlockPos* );
    void closeContainer();
    void prepareCustomTextures();
    void checkRidingStatistiscs(double , double , double );
    void rideTick();
    void SetNormalSize();
    void resetPos();
    void serverAiStep();
    void playShoulderEntityAmbientSound(CompoundTag* );
    CompoundTag* getShoulderEntityLeft();
    CompoundTag* getShoulderEntityRight();
    void respawnEntityOnShoulder(CompoundTag* );
    void setShoulderEntityLeft(CompoundTag* );
    void setShoulderEntityRight(CompoundTag* );
    void removeEntitiesOnShoulder();
    void calculateFlapping();
    void aiStep();
    void touch(boost::shared_ptr<Entity> );
    uint32_t getScore();
    void setScore(int32_t );
    void increaseScore(int32_t );
    boost::shared_ptr<ItemEntity> drop(not_null_ptr<ItemInstance> , bool , bool );
    void destroyVanishingCursedItems();
    void die(const DamageSource* );
    const SoundEvent* getHurtSound(const DamageSource* );
    const SoundEvent* getDeathSound();
    boost::shared_ptr<ItemEntity> drop(not_null_ptr<ItemInstance> , bool );
    boost::shared_ptr<ItemEntity> drop(bool );
    boost::shared_ptr<ItemEntity> reallyDrop(boost::shared_ptr<ItemEntity> );
    float getDestroySpeed(const BlockState* );
    bool canDestroy(const BlockState* );
    static void addWalkers(DataFixerUpper* );
    void readAdditionalSaveData(CompoundTag* );
    void addAdditonalSaveData(CompoundTag* );
    CameraController* GetCameraController();
    void SetCameraController(CameraController* , bool );
    uint32_t isAllowedToFly();
    bool hurt(const DamageSource* , float );
    void blockUsingShield(const boost::shared_ptr<LivingEntity>& );
    uint32_t canHarmPlayer(boost::shared_ptr<Player> );
    uint32_t hasInvisiblePrivilege();
    uint32_t isAllowedToAttackPlayers();
    bool canHarmPlayer(std::wstring );
    void hurtArmor(float );
    double getArmorCoverPercentage();
    void hurtCurrentlyUsedShield(float );
    uint32_t isAllowedToIgnoreExhaustion();
    void causeFoodExhaustion(float );
    void actuallyHurt(const DamageSource* , float );
    void openTextEdit(boost::shared_ptr<SignBlockEntity> );
    void openMinecartCommandBlock(uint32_t );
    void openCommandBlock(uint32_t );
    uint32_t openTrading(uint32_t , std::wstring* , int32_t );
    uint32_t openContainer(boost::shared_ptr<Container> );
    uint32_t openHorseInventory(uint32_t , boost::shared_ptr<Container> );
    uint32_t startBlockInteraction(InteractionObject* );
    void openItemInstanceGui(not_null_ptr<ItemInstance> , uint32_t );
    uint32_t interactOn(boost::shared_ptr<Entity> , InteractionHand::EInteractionHand );
    double getRidingHeight();
    void stopRiding();
    uint64_t attack(boost::shared_ptr<Entity> );
    void disableShield(bool );
    void crit(boost::shared_ptr<Entity> );
    void magicCrit(boost::shared_ptr<Entity> );
    void respawn();
    void onSyncedDataUpdated(const EntityDataAccessor_Base* );
    void remove();
    bool isInWall();
    uint32_t isLocalPlayer();
    std::wstring getGameProfile();
    uint32_t bedInRange(BlockPos* , Direction* );
    void setBedOffset(Direction* );
    uint32_t startSleepInBed(const BlockPos& , bool );
    void stopSleepInBed(bool , bool , bool );
    static BlockPos* checkBedValidRespawnPosition(Level* , const BlockPos& , bool );
    double getSleepRotation();
    bool isSleeping();
    uint32_t isSleepingLongEnough();
    uint32_t getSleepTimer();
    uint32_t getDeathFadeTimer();
    void displayClientMessage(int32_t , bool );
    BlockPos* getRespawnPosition();
    uint8_t isRespawnForced();
    uint64_t setRespawnPosition(BlockPos* , bool );
    uint32_t hasAchievement(Achievement* );
    void awardStat(Stat* , arrayWithLength<unsigned char> );
    void resetStat(Stat* );
    void jumpFromGround();
    void fjCheckSuperSonic(double , double , double , double , double , double );
    void travel(float , float , float );
    float getSpeed();
    void ResetTimerAchievement(int32_t );
    void checkMovementStatistiscs(double , double , double );
    void causeFallDamage(float , float );
    void doWaterSplashEffect();
    const SoundEvent* getFallDamageSound(int32_t );
    void killed(boost::shared_ptr<LivingEntity> );
    void makeStuckInWeb();
    int getItemInHandIcon(not_null_ptr<ItemInstance> , int32_t );
    int32_t getXpNeededForNextLevel();
    void increaseXp(int32_t );
    uint32_t getEnchantmentSeed();
    void onEnchantmentPerformed(int32_t );
    void giveExperienceLevels(int32_t );
    FoodData* getFoodData();
    uint32_t canEat(bool );
    uint32_t isHurt();
    uint8_t mayBuild();
    uint32_t mayUseItemAt(const BlockPos& , const Direction* , not_null_ptr<ItemInstance> );
    int32_t getExperienceReward(boost::shared_ptr<Player> );
    bool isAlwaysExperienceDropper();
    bool shouldShowName();
    bool makeStepSound();
    void onUpdateAbilities();
    void setGameMode(GameType* );
    virtual const GameType* GetGameType() = 0;
    std::wstring getName();
    std::wstring getNetworkName();
    boost::shared_ptr<PlayerEnderChestContainer> getEnderChestInventory();
    not_null_ptr<ItemInstance> getItemSlot(const EquipmentSlot* );
    void setItemSlot(const EquipmentSlot* , not_null_ptr<ItemInstance> );
    uint32_t addItem(not_null_ptr<ItemInstance> );
    std::vector<boost::shared_ptr<ItemInstance> >* getHandSlots();
    std::vector<boost::shared_ptr<ItemInstance> >* getArmorSlots();
    uint32_t setEntityOnShoulder(CompoundTag* );
    bool isInvisibleTo(boost::shared_ptr<Player> );
    bool isPushedByWater();
    Scoreboard* GetScoreboard();
    Team* getTeam();
    uint32_t SpectateAsOtherEntity();
    uint32_t SpectateAsFloatingHead();
    double getEyeHeight();
    void setAbsorptionAmount(float );
    float getAbsorptionAmount();
    bool canUnlock(LockCode* );
    uint32_t isModelPartShown(const PlayerModelPart* );
    bool setSlot(int32_t , const not_null_ptr<ItemInstance>& );
    uint8_t isReducedDebugInfo();
    const HumanoidArm* getMainArm();
    void setMainArm(const HumanoidArm* );
    double getCurrentItemAttackStrengthDelay();
    double getAttackStrengthScale(float );
    ItemCooldowns* getCooldowns();
    void push(boost::shared_ptr<Entity> );
    void pushEntities();
    double getLuck();
    uint8_t canUseCommandBlocks();
    static uint32_t GetModelTypeFromAnimBitmask(uint32_t );
    static uint32_t GetModelTypeFromTextureId(int32_t );
    uint64_t getModelName();
    static uint32_t hash_fnct(boost::shared_ptr<Player> );
    static bool eq_test(boost::shared_ptr<Player> , boost::shared_ptr<Player> );
    uint8_t getTexture();
    void setPlayerDefaultSkin(uint32_t );
    static _SkinAdjustments getSkinAdjustmentsById(uint32_t );
    void SetAdditionalModelParts(std::vector<ModelPart*>* );
    void setCustomSkin(uint32_t );
    static std::wstring getCapePathFromId(uint32_t );
    uint64_t setCustomCape(uint32_t );
    static uint32_t getCapeIdFromPath(const std::wstring& );
    void setXuid(PlayerUID );
    static void setPlayerGamePrivilege(uint32_t& , EPlayerGamePrivileges , uint32_t );
    void setPlayerGamePrivilege(EPlayerGamePrivileges , uint32_t );
    uint32_t isAllowedToUse(Block* );
    uint32_t isAllowedToUse(not_null_ptr<ItemInstance> );
    bool isAllowedToInteract(boost::shared_ptr<Entity> );
    uint32_t isAllowedToMine();
    uint32_t isAllowedToAttackAnimals();
    uint32_t isAllowedToHurtEntity(boost::shared_ptr<Entity> );
    bool isModerator();
    bool IsCreativeFlying();
    static void enableAllPlayerPrivileges(uint32_t& , bool );
    void enableAllPlayerPrivileges(bool );
    bool canCreateParticles();
    std::vector<ModelPart*>* GetAdditionalModelParts();
    void UpdateSize();
    void SetSmallSize();
    void SetNoCollisionSize();
    void setName(const std::wstring& );
    void SetCamera(boost::shared_ptr<Entity> );
    boost::shared_ptr<Entity> GetCamera();
    void Follow(bool );
    uint32_t SpectateAsSkull();
    uint32_t SpectateAsInvisible();
    uint64_t GetSpectateEntity();
    Vec3* getAttachPos(Entity::EEntityLocation , float );
    uint32_t IsSpectatingOtherPlayer();
    int32_t AutoEquip(not_null_ptr<ItemInstance>& , not_null_ptr<ItemInstance>& , bool& );
    uint32_t CheckPowerup(PowerupItems::eGlide_Timed_Powerup_ID );
    uint32_t CanRecordStats(Stat* , arrayWithLength<unsigned char> );
    double getUnderwaterLightLevel();
    void setUnderwaterLightLevel(float );
    double getUnderwaterVisionScale();
    void setUnderwaterVisionScale(float );
    void startSpinAttack();
    void stopSpinAttack();
    void AdvanceTimerAchievement(int32_t , int32_t );
    void SetLastAteKelp(bool );
    void sendMessage(uint32_t , int32_t* , uint32_t , std::wstring* , uint32_t );
    uint64_t GetType();
    uint32_t GetThirdPersonView(bool );
    void SetThirdPersonView(int32_t );
    uint32_t CanChangeThirdPersonView();
    void onCrafted(not_null_ptr<ItemInstance> );
    void setInvisible(bool );
    void handleCollectItem(not_null_ptr<ItemInstance> );
    void SpectatePlayerNext();
    void SpectatePlayerPrev();
    void StopSpectatingPlayer();
    bool ShouldRenderShadow();
    bool PositionLocked();
    void SetPositionLocked(bool );
    bool TakeGlideCollisionDamage();
    void SetGlideCollisionDamage(bool );
    double GetLiftForceModifier();
    void SetLiftForceModifier(double );
    GameMode* GetGameMode();
    bool isPickable();
    void OnEquipArmor(not_null_ptr<ItemInstance> );
    void OnTakeFromAnvil(not_null_ptr<ItemInstance> );
    void ChangePlayerSkin();
    void GetQuickCraftItem(int , int );
    void sweepAttack();
    static PlayerUID createPlayerUUID(const std::wstring& );
    void setReducedDebugInfo(bool );
    bool isAllowedToTeleport();
    double GetCollisionSize();
    void AdvanceAllTimerAchievements(int );


};


#endif
