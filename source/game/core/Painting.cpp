#include "game/core/Painting.hpp"

#include <vector>

#include "game/client/audio/SoundEvents.hpp"
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

static Painting::Motive* paintingMotives[36] = { NULL };

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

    for (int i = 0; i < 6; ++i)
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

    for (int i = 0; i < 36; ++i)
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
    for (int i = 0; i < 36; ++i)
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
    if (!mLevel->getGameRules()->getBoolean(6))
        return;
    if (breaker && breaker->isCreative())
        return;
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
    return 0x000000000A080001ULL;
}
