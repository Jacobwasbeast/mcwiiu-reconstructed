#include "game/world/generation/Village.hpp"

#include <cmath>
#include <typeinfo>

#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"
#include "game/world/generation/Player.hpp"

Village::Village()
{
    mLevel = NULL; mRadius = 0; mStable = 0; mTick = 0; mPopulation = 0; mNoBreedTimer = 0; mGolems = 0;
}

Village::Village(Level* level)
{
    mLevel = level; mRadius = 0; mStable = 0; mTick = 0; mPopulation = 0; mNoBreedTimer = 0; mGolems = 0;
}

Village::~Village()
{
    std::vector<Aggressor*>::iterator it;
    for (it = mAggressors.begin(); it != mAggressors.end(); ++it)
        delete *it;
}

Village::Aggressor::Aggressor(boost::shared_ptr<LivingEntity> entity, int timeStamp)
    : mEntity(entity), mTimeStamp(timeStamp)
{
}

void Village::setLevel(Level* level) { mLevel = level; }

bool Village::isInside(const BlockPos& pos)
{
    return (double)mCenter.distSqr(pos) < (double)(mRadius * mRadius);
}

bool Village::canSpawnAt(const BlockPos& size, const BlockPos& pos)
{
    BlockPos below = pos.below();
    const BlockState* state = mLevel->getBlockState(below);
    if (!state->isTopSolidBlocking() && state->getBlock() != Blocks::WATER)
        return false;
    MutableBlockPos current;
    int x = pos.x - (size.x / 2);
    int z = pos.z - (size.z / 2);
    int ix;
    int iy;
    int iz;
    for (ix = x; ix < x + size.x; ++ix)
        for (iy = pos.y; iy < pos.y + size.y; ++iy)
            for (iz = z; iz < z + size.z; ++iz)
            {
                current.set(ix, iy, iz);
                state = mLevel->getBlockState(current);
                if (state->isSolidBlockingCubeAndNotSignalSource() || state->getBlock() == Blocks::WATER)
                    return false;
            }
    return true;
}

Vec3* Village::findRandomSpawnPos(const BlockPos& pos, int xSize, int ySize, int zSize)
{
    BlockPos size(xSize, ySize, zSize);
    int tries = 5;
    BlockPos candidate;
    while (tries-- > 0)
    {
        candidate = pos.offset(mLevel->mRandom->nextInt(16) - 8, mLevel->mRandom->nextInt(6) - 3, mLevel->mRandom->nextInt(16) - 8);
        if (isInside(candidate) && canSpawnAt(size, candidate))
            return Vec3::newPermanent(candidate.x, candidate.y, candidate.z);
        candidate = pos.offset(mLevel->mRandom->nextInt(16) - 8, mLevel->mRandom->nextInt(6) - 3, mLevel->mRandom->nextInt(16) - 8);
        if (isInside(candidate) && canSpawnAt(size, candidate))
            return Vec3::newPermanent(candidate.x, candidate.y, candidate.z);
    }
    return NULL;
}

bool Village::isDoor(const BlockPos& pos)
{
    const BlockState* state = mLevel->getBlockState(pos);
    return DoorBlock::isDoor(state->getBlock()) && state->getMaterial() == Material::WOOD;
}

void Village::calcInfo()
{
    int count = getDoorCount();
    if (count == 0)
    {
        mCenter = BlockPos();
        mRadius = 0;
        return;
    }
    mCenter = BlockPos(mBottom.x / count, mBottom.y / count, mBottom.z / count);
    int maxDistance = 0;
    std::vector<boost::shared_ptr<DoorInfo> >::iterator it;
    for (it = mDoors.begin(); it != mDoors.end(); ++it)
    {
        int distance = (*it)->distanceToSqr(mCenter);
        if (distance > maxDistance) maxDistance = distance;
    }
    mRadius = Mth::floor(Mth::sqrt((float)maxDistance)) + 1;
    if (mRadius < 32) mRadius = 32;
}

void Village::updateDoors()
{
    bool changed = false;
    int resetBookings = mLevel->mRandom->nextInt(50);
    std::vector<boost::shared_ptr<DoorInfo> >::iterator it = mDoors.begin();
    while (it != mDoors.end())
    {
        if (resetBookings == 0) (*it)->resetBookingCount();
        BlockPos pos = (*it)->getPos();
        if (!isDoor(pos) || std::abs(mTick - (*it)->getTimeStamp()) > 1200)
        {
            (*it)->setRemoved(true);
            mBottom.x -= pos.x; mBottom.y -= pos.y; mBottom.z -= pos.z;
            it = mDoors.erase(it);
            changed = true;
        }
        else ++it;
    }
    if (changed) calcInfo();
}

void Village::updateAggressors()
{
    std::vector<Aggressor*>::iterator it = mAggressors.begin();
    while (it != mAggressors.end())
    {
        if (!(*it)->mEntity || !(*it)->mEntity->isAlive() || std::abs(mTick - (*it)->mTimeStamp) >= 301)
        {
            delete *it;
            it = mAggressors.erase(it);
        }
        else ++it;
    }
}

void Village::countPopulation()
{
    AABB* box = AABB::newTemp(mCenter.x - mRadius, mCenter.y - 4, mCenter.z - mRadius,
                               mCenter.x + mRadius, mCenter.y + 4, mCenter.z + mRadius);
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(Villager), box);
    mPopulation = entities ? (int)entities->size() : 0;
    delete entities;
    if (mPopulation == 0) mPlayerStandings.clear();
}

void Village::countGolem()
{
    AABB* box = AABB::newTemp(mCenter.x - mRadius, mCenter.y - 4, mCenter.z - mRadius,
                               mCenter.x + mRadius, mCenter.y + 4, mCenter.z + mRadius);
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(VillagerGolem), box);
    mGolems = entities ? (int)entities->size() : 0;
    delete entities;
}

void Village::tick(int tick)
{
    mTick = tick;
    updateDoors();
    updateAggressors();
    if (tick % 20 == 0) countPopulation();
    if (tick % 30 == 0) countGolem();
    if (mGolems < mPopulation / 10 && mDoors.size() > 20 && mLevel->mRandom->nextInt(7000) == 0)
    {
        Vec3* pos = findRandomSpawnPos(mCenter, 2, 4, 2);
        if (pos != NULL)
        {
            boost::shared_ptr<VillagerGolem> golem(new VillagerGolem(mLevel));
            golem->setPos(pos->x, pos->y, pos->z);
            mLevel->addEntity(golem);
            ++mGolems;
        }
    }
}

const BlockPos* Village::getCenter() { return &mCenter; }
int Village::getRadius() { return mRadius; }
int Village::getDoorCount() { return (int)mDoors.size(); }
int Village::getStableAge() { return mTick - mStable; }
int Village::getPopulationSize() { return mPopulation; }
std::vector<boost::shared_ptr<DoorInfo> >* Village::getDoorInfos() { return &mDoors; }

boost::shared_ptr<DoorInfo> Village::getClosestDoorInfo(const BlockPos& pos)
{
    boost::shared_ptr<DoorInfo> result;
    int distance = 0x7fffffff;
    std::vector<boost::shared_ptr<DoorInfo> >::iterator it;
    for (it = mDoors.begin(); it != mDoors.end(); ++it)
    {
        int current = (*it)->distanceToSqr(pos);
        if (current < distance) { distance = current; result = *it; }
    }
    return result;
}

boost::shared_ptr<DoorInfo> Village::getBestDoorInfo(const BlockPos& pos)
{
    boost::shared_ptr<DoorInfo> result;
    int score = 0x7fffffff;
    std::vector<boost::shared_ptr<DoorInfo> >::iterator it;
    for (it = mDoors.begin(); it != mDoors.end(); ++it)
    {
        int distance = (*it)->distanceToSqr(pos);
        int current = distance < 257 ? (*it)->getBookingsCount() : distance * 1000;
        if (current < score)
        {
            BlockPos doorPos = (*it)->getPos();
            const Direction* direction = (*it)->getDirection();
            BlockPos outside = doorPos.relative(direction, 1);
            BlockPos inside = doorPos.relative(direction, -1);
            BlockPos outsideAbove = doorPos.above().relative(direction, 1);
            BlockPos insideAbove = doorPos.above().relative(direction, -1);
            if (mLevel->getBlockState(outside)->getBlock()->isPathfindable(mLevel, outside)
                && mLevel->getBlockState(inside)->getBlock()->isPathfindable(mLevel, inside)
                && mLevel->getBlockState(outsideAbove)->getBlock()->isPathfindable(mLevel, outsideAbove)
                && mLevel->getBlockState(insideAbove)->getBlock()->isPathfindable(mLevel, insideAbove))
            {
                score = current;
                result = *it;
            }
        }
    }
    return result;
}

boost::shared_ptr<DoorInfo> Village::getDoorInfo(const BlockPos& pos)
{
    if ((double)mCenter.distSqr(pos) > (double)(mRadius * mRadius)) return boost::shared_ptr<DoorInfo>();
    std::vector<boost::shared_ptr<DoorInfo> >::iterator it;
    for (it = mDoors.begin(); it != mDoors.end(); ++it)
    {
        BlockPos door = (*it)->getPos();
        if (door.x == pos.x && door.z == pos.z && std::abs(door.y - pos.y) < 2) return *it;
    }
    return boost::shared_ptr<DoorInfo>();
}

bool Village::hasDoorInfo(const BlockPos& pos) { return getDoorInfo(pos).get() != NULL; }

void Village::addDoorInfo(boost::shared_ptr<DoorInfo> door)
{
    mDoors.push_back(door);
    BlockPos pos = door->getPos();
    mBottom.x += pos.x; mBottom.y += pos.y; mBottom.z += pos.z;
    calcInfo();
    mStable = door->getTimeStamp();
}

bool Village::canRemove() { return mDoors.empty(); }

void Village::addAggressor(boost::shared_ptr<LivingEntity> entity)
{
    std::vector<Aggressor*>::iterator it;
    for (it = mAggressors.begin(); it != mAggressors.end(); ++it)
    {
        if ((*it)->mEntity == entity)
        {
            (*it)->mTimeStamp = mTick;
            return;
        }
    }
    mAggressors.push_back(new Aggressor(entity, mTick));
}

boost::shared_ptr<LivingEntity> Village::getClosestAggressor(boost::shared_ptr<LivingEntity> entity)
{
    boost::shared_ptr<LivingEntity> result;
    double distance = 1.7976931348623157e+308;
    std::vector<Aggressor*>::iterator it;
    for (it = mAggressors.begin(); it != mAggressors.end(); ++it)
    {
        double current = (*it)->mEntity->distanceToSqr(entity);
        if (current <= distance) { distance = current; result = (*it)->mEntity; }
    }
    return result;
}

int Village::getStanding(const std::wstring& name)
{
    boost::unordered_map<std::wstring, int>::iterator it = mPlayerStandings.find(name);
    return it == mPlayerStandings.end() ? 0 : it->second;
}

bool Village::isVeryBadStanding(std::wstring name) { return getStanding(name) < -15; }

boost::shared_ptr<LivingEntity> Village::getClosestBadStandingPlayer(boost::shared_ptr<LivingEntity> entity)
{
    boost::shared_ptr<LivingEntity> result;
    double distance = 1.7976931348623157e+308;
    boost::unordered_map<std::wstring, int>::iterator it;
    for (it = mPlayerStandings.begin(); it != mPlayerStandings.end(); ++it)
    {
        if (isVeryBadStanding(it->first))
        {
            boost::shared_ptr<Player> player = mLevel->getPlayerByName(it->first);
            if (player)
            {
                double current = player->distanceToSqr(entity);
                if (current <= distance) { distance = current; result = player; }
            }
        }
    }
    return result;
}

int Village::modifyStanding(const std::wstring& name, int amount)
{
    int value = Mth::clamp(getStanding(name) + amount, -30, 10);
    mPlayerStandings[name] = value;
    return value;
}

void Village::readAdditionalSaveData(CompoundTag* tag)
{
    mPopulation = tag->getInt(L"PopSize"); mRadius = tag->getInt(L"Radius"); mGolems = tag->getInt(L"Golems");
    mStable = tag->getInt(L"Stable"); mTick = tag->getInt(L"Tick"); mNoBreedTimer = tag->getInt(L"MTick");
    mCenter = BlockPos(tag->getInt(L"CX"), tag->getInt(L"CY"), tag->getInt(L"CZ"));
    mBottom = BlockPos(tag->getInt(L"ACX"), tag->getInt(L"ACY"), tag->getInt(L"ACZ"));
    ListTag* doors = tag->getList(L"Doors", 10);
    int i;
    for (i = 0; i < doors->size(); ++i)
    {
        CompoundTag* door = doors->getCompound(i);
        BlockPos pos(door->getInt(L"X"), door->getInt(L"Y"), door->getInt(L"Z"));
        mDoors.push_back(boost::shared_ptr<DoorInfo>(new DoorInfo(pos, door->getInt(L"IDX"), door->getInt(L"IDZ"), door->getInt(L"TS"))));
    }
    ListTag* players = tag->getList(L"Players", 10);
    for (i = 0; i < players->size(); ++i)
    {
        CompoundTag* player = players->getCompound(i);
        mPlayerStandings[player->getString(L"Name")] = player->getInt(L"S");
    }
}

void Village::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(L"PopSize", mPopulation); tag->putInt(L"Radius", mRadius); tag->putInt(L"Golems", mGolems);
    tag->putInt(L"Stable", mStable); tag->putInt(L"Tick", mTick); tag->putInt(L"MTick", mNoBreedTimer);
    tag->putInt(L"CX", mCenter.x); tag->putInt(L"CY", mCenter.y); tag->putInt(L"CZ", mCenter.z);
    tag->putInt(L"ACX", mBottom.x); tag->putInt(L"ACY", mBottom.y); tag->putInt(L"ACZ", mBottom.z);
    ListTag* doors = new ListTag(10);
    std::vector<boost::shared_ptr<DoorInfo> >::iterator it;
    for (it = mDoors.begin(); it != mDoors.end(); ++it)
    {
        CompoundTag* door = new CompoundTag(); BlockPos pos = (*it)->getPos();
        door->putInt(L"X", pos.x); door->putInt(L"Y", pos.y); door->putInt(L"Z", pos.z);
        door->putInt(L"IDX", (*it)->getInsideDx()); door->putInt(L"IDZ", (*it)->getInsideDz()); door->putInt(L"TS", (*it)->getTimeStamp());
        doors->add(door);
    }
    tag->put(L"Doors", doors);
    ListTag* players = new ListTag(10);
    boost::unordered_map<std::wstring, int>::iterator player;
    for (player = mPlayerStandings.begin(); player != mPlayerStandings.end(); ++player)
    {
        CompoundTag* entry = new CompoundTag(); entry->putString(L"Name", player->first); entry->putInt(L"S", player->second); players->add(entry);
    }
    tag->put(L"Players", players);
}

void Village::resetNoBreedTimer() { mNoBreedTimer = mTick; }
bool Village::isBreedTimerOk() { return mNoBreedTimer == 0 || mTick - mNoBreedTimer >= 3600; }
void Village::rewardAllPlayers(int amount)
{
    boost::unordered_map<std::wstring, int>::iterator it;
    for (it = mPlayerStandings.begin(); it != mPlayerStandings.end(); ++it) modifyStanding(it->first, amount);
}
