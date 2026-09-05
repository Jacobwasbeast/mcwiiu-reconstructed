#ifndef GAME_ENTITY_MOB_HORSE_HPP
#define GAME_ENTITY_MOB_HORSE_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/core/InteractionHand.hpp"

class CompoundTag;
class Container;
class DamageSource;
class DataFixerUpper;
class EntityDataAccessor_Base;
class EntityTooltipDataHolder;
class HorseArmorItem;
class SoundEvent;
class SoundType;

class Horse : public AbstractHorse
{
public:
    class HorseGroupData : public MobGroupData
    {
    public:
        HorseGroupData(int variant);
        virtual ~HorseGroupData();
        int mVariant;
    };

    Horse(Level* level);
    virtual ~Horse();
    static void create(Level* level);
    HorseArmorItem* getArmorType();
    virtual void containerChanged(Container* container);
    static void addDataWalkers(DataFixerUpper* fixer);
    static void staticCtor();
    virtual void defineSynchedData();
    int getVariant();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void clearLayeredTextureInfo();
    void setVariant(int variant);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    void rebuildLayeredTextureInfo();
    std::wstring getLayeredTextureHashName();
    int getLayeredTextureLayers();
    bool isWearingDyedArmor();
    int getDyedArmorColor();
    int getDyedArmorLayer();
    void setArmorType(const not_null_ptr<ItemInstance>& item);
    virtual void updateEquipment();
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual void playGallopSound(const SoundType* sound);
    virtual void registerAttributes();
    virtual void tick();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getAngrySound();
    virtual const void* getDefaultLootTable();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual bool HideUseTooltips();
    virtual bool canMate(const boost::shared_ptr<Animal>& other);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    virtual bool wearsArmor();
    virtual bool isArmor(const not_null_ptr<ItemInstance>& item);
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& name);
    virtual bool removeWhenFarAway();
    virtual unsigned long long GetType();
    bool hasTexture();

private:
    int mVariant;
    HorseArmorItem* mArmorType;
    std::wstring mLayeredTextureHashName;
    std::vector<int> mLayeredTextureLayers;
    bool mWearingDyedArmor;
    int mDyedArmorColor;
    int mDyedArmorLayer;
    not_null_ptr<ItemInstance> mArmorItem;
};

#endif
