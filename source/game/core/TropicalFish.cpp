#include "game/core/TropicalFish.hpp"

#include <typeinfo>

#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Dolphin.hpp"
#include "game/util/Random.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/entity/ai/RandomSwimmingGoal.hpp"
#include "game/entity/ai/SwimIdleGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/world/Level.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

static const unsigned long long ENTITY_TYPE_TROPICAL_FISH = 0x000000000D502005ULL;
static const unsigned long long ENTITY_TYPE_FISH = 0x0D50200200000000ULL;

const EntityDataAccessor<int>* TropicalFish::DATA_VARIANT = NULL;
const EntityDataAccessor<int>* TropicalFish::DATA_COLOR = NULL;

static const int TROPICAL_FISH_BASE_VARIANTS[22] = { 1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,7,1,1,1,1 };
static const int TROPICAL_FISH_MARK_VARIANTS[22] = { 1,1,1,1,1,1,7,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1 };
static const int TROPICAL_FISH_COLOR1_VARIANTS[22] = { 1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
static const int TROPICAL_FISH_COLOR2_VARIANTS[22] = { 7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7 };
static const int TROPICAL_FISH_BASE_TEXTURES[2][2] = { { 0xD6, 0xDD }, { 0xDE, 0xDF } };
static const int TROPICAL_FISH_MARK_TEXTURES[2][6] = { { 0xD7,0xD8,0xD9,0xDA,0xDB,0xDC }, { 0xDE,0xDF,0xE0,0xE1,0xE2,0xE3 } };

TropicalFish::TropicalFish(Level* level)
    : Fish(level, true), mLayeredTextureHashName(L"fish_"),
      mLayeredTextureLayers(2, true)
{
    mLayeredTextureLayers.data[0] = -1;
    mLayeredTextureLayers.data[1] = -1;
    fjDerivedCtorCalls();
    mWaterValueA = 0.02f;
    mWaterValueB = 0.01f;
    setSize(0.4f, 0.4f);
}

TropicalFish::~TropicalFish() {}

void TropicalFish::create(Level* level) { new TropicalFish(level); }

void TropicalFish::staticCtor()
{
    DATA_VARIANT = SynchedEntityData::defineId(ENTITY_TYPE_TROPICAL_FISH, ENTITY_TYPE_FISH, EntityDataSerializers::INT);
    DATA_COLOR = SynchedEntityData::defineId(ENTITY_TYPE_TROPICAL_FISH, ENTITY_TYPE_FISH, EntityDataSerializers::INT);
}

void TropicalFish::clearLayeredTextureInfo()
{
    mLayeredTextureHashName.erase(mLayeredTextureHashName.begin() + 5, mLayeredTextureHashName.end());
}

void TropicalFish::tick()
{
    PIXBeginNamedEvent(0.0f, "TropicalFish tick");
    Mob::tick();
    if (mEntityData != NULL && mEntityData->isDirty())
    {
        mEntityData->clearDirty();
        clearLayeredTextureInfo();
    }
    PIXEndNamedEvent();
}

int TropicalFish::getVariant() { return getEntityData()->get(DATA_VARIANT); }
int TropicalFish::getBaseVariant() { return getVariant() & 1; }
int TropicalFish::getMarkVariant() { return (getVariant() >> 8) % 6; }

void TropicalFish::setVariant(int variant)
{
    getEntityData()->set(DATA_VARIANT, variant, true);
    clearLayeredTextureInfo();
}

int TropicalFish::getColor() { return getEntityData()->get(DATA_COLOR); }
int TropicalFish::getColor1() { return getColor() % 15; }
int TropicalFish::getColor2() { return (getColor() >> 8) % 15; }
void TropicalFish::setColor(int color) { getEntityData()->set(DATA_COLOR, color, true); }

void TropicalFish::rebuildLayeredTextureInfo()
{
    int baseVariant = getBaseVariant();
    int markVariant = getMarkVariant();
    mLayeredTextureHashName = L"fish_";
    mLayeredTextureLayers.data[0] = TROPICAL_FISH_BASE_TEXTURES[baseVariant][0];
    mLayeredTextureLayers.data[1] = baseVariant == 0 ? TROPICAL_FISH_MARK_TEXTURES[0][markVariant] : TROPICAL_FISH_MARK_TEXTURES[1][markVariant];
    mLayeredTextureHashName += baseVariant == 0 ? L"0" : L"1";
    mLayeredTextureHashName += L"_";
    static const wchar_t* markNames[6] = { L"0", L"1", L"2", L"3", L"4", L"5" };
    mLayeredTextureHashName += markNames[markVariant];
}

std::wstring TropicalFish::getLayeredTextureHashName()
{
    if (mLayeredTextureHashName.empty()) rebuildLayeredTextureInfo();
    return mLayeredTextureHashName;
}

arrayWithLength<int> TropicalFish::getLayeredTextureLayers()
{
    if (mLayeredTextureHashName.empty()) rebuildLayeredTextureInfo();
    return mLayeredTextureLayers;
}

void TropicalFish::registerGoals()
{
    mGoalSelector.addGoal(3, new SwimIdleGoal(this), true);
    mGoalSelector.addGoal(4, new RandomSwimmingGoal(this, 1.0, 10, 16, 4), true);
    mGoalSelector.addGoal(2, new AvoidEntityGoal(this, typeid(Dolphin), 6.0f, 1.2, 1.4), true);
}

void TropicalFish::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(6.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(1.2);
}

void TropicalFish::defineSynchedData()
{
    Mob::defineSynchedData();
    getEntityData()->createDateItem(DATA_VARIANT, 0);
    getEntityData()->createDateItem(DATA_COLOR, 0);
}

void TropicalFish::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(L"Variant", getVariant());
    tag->putInt(L"Color", getColor());
}

void TropicalFish::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    setVariant(tag->getInt(L"Variant"));
    setColor(tag->getInt(L"Color"));
}

MobGroupData* TropicalFish::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& name)
{
    mRandom = new Random();
    if (data == NULL)
    {
        if (mRandom->nextFloat() < 0.9)
        {
            int index = mRandom->nextInt(22);
            TropicalFishGroupData* group = new TropicalFishGroupData(TROPICAL_FISH_BASE_VARIANTS[index], TROPICAL_FISH_MARK_VARIANTS[index], TROPICAL_FISH_COLOR1_VARIANTS[index], TROPICAL_FISH_COLOR2_VARIANTS[index]);
            setVariant(group->mBaseVariant | (group->mMarkVariant << 8));
            setColor(group->mColor1 | (group->mColor2 << 8));
            data = group;
        }
    }
    else
    {
        TropicalFishGroupData* group = dynamic_cast<TropicalFishGroupData*>(data);
        if (group != NULL)
        {
            setVariant(group->mBaseVariant | (group->mMarkVariant << 8));
            setColor(group->mColor1 | (group->mColor2 << 8));
        }
        else if (mRandom->nextFloat() < 0.9)
        {
            int index = mRandom->nextInt(22);
            setVariant(TROPICAL_FISH_BASE_VARIANTS[index] | (TROPICAL_FISH_MARK_VARIANTS[index] << 8));
            setColor(TROPICAL_FISH_COLOR1_VARIANTS[index] | (TROPICAL_FISH_COLOR2_VARIANTS[index] << 8));
        }
    }
    if (data == NULL)
    {
        setVariant(mRandom->nextInt(2) | (mRandom->nextInt(6) << 8));
        setColor(mRandom->nextInt(15) | (mRandom->nextInt(15) << 8));
    }
    return Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));
}

const void* TropicalFish::getDefaultLootTable() { return &BuiltInLootTables::ENTITY_FISH; }
unsigned long long TropicalFish::GetType() { return ENTITY_TYPE_TROPICAL_FISH; }

TropicalFish::TropicalFishGroupData::TropicalFishGroupData(int baseVariant, int markVariant, int color1, int color2)
    : mBaseVariant(baseVariant), mMarkVariant(markVariant), mColor1(color1), mColor2(color2) {}

TropicalFish::TropicalFishGroupData::~TropicalFishGroupData() {}
