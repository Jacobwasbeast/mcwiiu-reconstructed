#include "game/core/Painting.hpp"
#include "game/core/EntityType.hpp"

#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/GameRules.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

static Painting::Motive* paintingMotives[30] = {
    new Painting::Motive(L"Kebab", 16, 16, 0, 0),
    new Painting::Motive(L"Aztec", 16, 16, 16, 0),
    new Painting::Motive(L"Alban", 16, 16, 32, 0),
    new Painting::Motive(L"Aztec2", 16, 16, 48, 0),
    new Painting::Motive(L"Bomb", 16, 16, 64, 0),
    new Painting::Motive(L"Plant", 16, 16, 80, 0),
    new Painting::Motive(L"Wasteland", 16, 16, 96, 0),
    new Painting::Motive(L"Pool", 32, 16, 0, 32),
    new Painting::Motive(L"Courbet", 32, 16, 32, 32),
    new Painting::Motive(L"Sea", 32, 16, 64, 32),
    new Painting::Motive(L"Sunset", 32, 16, 96, 32),
    new Painting::Motive(L"Creebet", 32, 16, 128, 32),
    new Painting::Motive(L"Wanderer", 16, 32, 0, 64),
    new Painting::Motive(L"Graham", 16, 32, 16, 64),
    new Painting::Motive(L"Match", 32, 32, 0, 128),
    new Painting::Motive(L"Bust", 32, 32, 32, 128),
    new Painting::Motive(L"Stage", 32, 32, 64, 128),
    new Painting::Motive(L"Void", 32, 32, 96, 128),
    new Painting::Motive(L"SkullAndRoses", 32, 32, 128, 128),
    new Painting::Motive(L"Wither", 32, 32, 160, 128),
    new Painting::Motive(L"Fighters", 64, 32, 0, 96),
    new Painting::Motive(L"Pointer", 64, 64, 0, 192),
    new Painting::Motive(L"Pigscene", 64, 64, 64, 192),
    new Painting::Motive(L"BurningSkull", 64, 64, 128, 192),
    new Painting::Motive(L"Skeleton", 64, 48, 192, 64),
    new Painting::Motive(L"DonkeyKong", 64, 48, 192, 112),
    new Painting::Motive(L"1x1E1", 16, 16, 0, 16),
    new Painting::Motive(L"1x1E2", 16, 16, 16, 16),
    new Painting::Motive(L"1x1E3", 16, 16, 32, 16),
    new Painting::Motive(L"1x1E4", 16, 16, 48, 16)
};

Painting::Motive::Motive(std::wstring motiveName, int motiveWidth, int motiveHeight,
                         int motiveTextureX, int motiveTextureY)
    : name(motiveName), width(motiveWidth), height(motiveHeight),
      textureX(motiveTextureX), textureY(motiveTextureY)
{
}

Painting::Painting(Level* level)
    : HangingEntity(level)
{
    Entity::fjDerivedCtorCalls();
    _init(level);
}

Painting::Painting(Level* level, const BlockPos& pos, const Direction* direction)
    : HangingEntity(level, pos)
{
    Entity::fjDerivedCtorCalls();
    _init(level);
}

Painting::Painting(Level* level, const BlockPos& pos, const Direction* direction,
                   const std::wstring& motiveName)
    : HangingEntity(level, pos)
{
    Entity::fjDerivedCtorCalls();
    _init(level);

    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 6; ++j)
        {
            Motive* candidate = paintingMotives[i * 6 + j];
            if (candidate != NULL && candidate->name == motiveName)
            {
                motive = candidate;
                setDirection(direction);
                return;
            }
        }
    }
    setDirection(direction);
}

Painting::~Painting()
{
}

void Painting::create(Level* level)
{
    new Painting(level);
}

void Painting::_init(Level* level)
{
    motive = NULL;
}

void Painting::PaintingPostConstructor(const Direction* direction, int unused)
{
    std::vector<Motive*> validMotives;
    int largestArea = 0;

    for (int i = 0; i < 30; ++i)
    {
        motive = paintingMotives[i];
        if (motive != NULL && survives())
        {
            validMotives.push_back(motive);
            int area = motive->width * motive->height;
            if (area > largestArea)
                largestArea = area;
        }
    }

    for (std::vector<Motive*>::iterator it = validMotives.begin(); it != validMotives.end();)
    {
        if ((*it)->width * (*it)->height < largestArea)
            it = validMotives.erase(it);
        else
            ++it;
    }

    if (!validMotives.empty())
        motive = validMotives.at((unsigned int)mRandom->nextInt((int)validMotives.size()));

    setDirection(direction);
}

void Painting::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putString(L"Motive", motive != NULL ? motive->name : L"");
    HangingEntity::addAdditonalSaveData(tag);
}

void Painting::readAdditionalSaveData(CompoundTag* tag)
{
    std::wstring motiveName = tag->getString(L"Motive");
    motive = NULL;
    for (int i = 0; i < 30; ++i)
    {
        if (paintingMotives[i] != NULL && paintingMotives[i]->name == motiveName)
            motive = paintingMotives[i];
    }
    if (motive == NULL)
        motive = paintingMotives[0];
    HangingEntity::readAdditionalSaveData(tag);
}

int Painting::getWidth()
{
    return motive->width;
}

int Painting::getHeight()
{
    return motive->height;
}

void Painting::dropItem(boost::shared_ptr<Entity> breaker)
{
    if (!mLevel->getGameRules()->getBoolean(GameRules::RULE_DOENTITYDROPS))
        return;
    playSound(SoundEvents::PAINTING_BREAK, 1.0f, 1.0f);
    if (breaker != NULL && (breaker->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
    {
        boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(breaker);
        if (player->mAbilities.mInstabuild)
            return;
    }
    spawnAtLocation(not_null_ptr<ItemInstance>(new ItemInstance(Items::painting)), 0.0f);
}

void Painting::playPlacementSound()
{
    playSound(SoundEvents::PAINTING_PLACE, 1.0f, 1.0f);
}

void Painting::moveTo(double x, double y, double z, float yRot, float xRot)
{
    setPos(x, y, z);
}

void Painting::lerpTo(double x, double y, double z, float yRot, float xRot,
                      int steps, bool teleport)
{
    BlockPos pos = mPos.offset(x - mX, y - mY, z - mZ);
    setPos((double)pos.x, (double)pos.y, (double)pos.z);
}

unsigned long long Painting::GetType()
{
    return EntityType::PAINTING;
}
