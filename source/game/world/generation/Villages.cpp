#include "game/world/generation/Villages.hpp"

#include <cmath>

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Village.hpp"

static const std::wstring VILLAGES_FILE_ID = L"villages";

Villages::Villages(const std::wstring& id)
    : SavedData(id)
{
    mLevel = NULL;
    mTick = 0;
}

Villages::Villages(Level* level)
    : SavedData(VILLAGES_FILE_ID)
{
    mLevel = level;
    mTick = 0;
}

Villages::~Villages()
{
}

void Villages::setLevel(Level* level)
{
    mLevel = level;
    for (std::vector<boost::shared_ptr<Village> >::iterator it = mVillages.begin(); it != mVillages.end(); ++it)
    {
        boost::shared_ptr<Village> village = *it;
        village->setLevel(level);
    }
}

bool Villages::hasQuery(const BlockPos& pos)
{
    std::deque<BlockPos>::const_iterator it;
    for (it = mToUpdate.begin(); it != mToUpdate.end(); ++it)
        if (it->equals(pos))
            return true;
    return false;
}

void Villages::queryUpdateAround(const BlockPos& pos)
{
    if (mToUpdate.size() <= 64 && !hasQuery(pos))
        mToUpdate.push_back(pos);
}

void Villages::removeVillages()
{
    std::vector<boost::shared_ptr<Village> >::iterator it = mVillages.begin();
    while (it != mVillages.end())
    {
        boost::shared_ptr<Village> village = *it;
        if (village->canRemove())
        {
            it = mVillages.erase(it);
            setDirty();
        }
        else
            ++it;
    }
}

bool Villages::isDoor(const BlockPos& pos)
{
    const BlockState* state = mLevel->getBlockState(pos);
    if (DoorBlock::isDoor(state->getBlock()))
        return state->getMaterial() == Material::WOOD;
    return false;
}

boost::shared_ptr<DoorInfo> Villages::getDoorInfo(const BlockPos& pos)
{
    std::vector<boost::shared_ptr<DoorInfo> >::iterator it;
    for (it = mDoorInfos.begin(); it != mDoorInfos.end(); ++it)
    {
        boost::shared_ptr<DoorInfo> doorInfo = *it;
        if (doorInfo->getPos().x == pos.x && doorInfo->getPos().z == pos.z && std::abs(doorInfo->getPos().y - pos.y) < 2)
            return doorInfo;
    }

    std::vector<boost::shared_ptr<Village> >::iterator village;
    for (village = mVillages.begin(); village != mVillages.end(); ++village)
    {
        boost::shared_ptr<Village> current = *village;
        boost::shared_ptr<DoorInfo> info = current->getDoorInfo(pos);
        if (info)
            return info;
    }

    return boost::shared_ptr<DoorInfo>();
}

int Villages::skyVisibleBlockCount(const BlockPos& pos, const Direction* direction, int count)
{
    int visible = 0;
    int i;
    for (i = 1; i <= 4; ++i)
    {
        if (mLevel->canSeeSky(pos.relative(direction, i)))
        {
            ++visible;
            if (visible >= count)
                break;
        }
    }
    return visible;
}

void Villages::createDoorInfo(const BlockPos& pos)
{
    const Direction* facing = DoorBlock::getFacing(mLevel, pos);
    const Direction* opposite = facing->getOpposite();
    int front = skyVisibleBlockCount(pos, facing, 5);
    int back = skyVisibleBlockCount(pos, opposite, front + 1);
    if (back > front)
        mDoorInfos.push_back(boost::shared_ptr<DoorInfo>(new DoorInfo(pos, facing, mTick)));
    else if (back < front)
        mDoorInfos.push_back(boost::shared_ptr<DoorInfo>(new DoorInfo(pos, opposite, mTick)));
}

void Villages::addDoorInfos(const BlockPos& pos)
{
    for (int x = -16; x < 16; ++x)
        for (int y = -4; y < 4; ++y)
            for (int z = -16; z < 16; ++z)
            {
                BlockPos doorPos = pos.offset(x, y, z);
                if (isDoor(doorPos))
                {
                    boost::shared_ptr<DoorInfo> info = getDoorInfo(doorPos);
                    if (info)
                        info->setTimeStamp(mTick);
                    else
                        createDoorInfo(doorPos);
                }
            }
}

void Villages::processNextQuery()
{
    if (!mToUpdate.empty())
    {
        BlockPos pos = mToUpdate.front();
        mToUpdate.pop_front();
        addDoorInfos(pos);
    }
}

boost::shared_ptr<Village> Villages::getClosestVillage(const BlockPos& pos, int distance)
{
    boost::shared_ptr<Village> closest;
    float best = 3.402823466e+38F;
    std::vector<boost::shared_ptr<Village> >::iterator it;
    for (it = mVillages.begin(); it != mVillages.end(); ++it)
    {
        boost::shared_ptr<Village> village = *it;
        const BlockPos* center = village->getCenter();
        float d = (float)center->distSqr(pos);
        if (d < best)
        {
            float radius = (float)(village->getRadius() + distance);
            if (d <= radius * radius)
            {
                best = d;
                closest = village;
            }
        }
    }
    return closest;
}

void Villages::cluster()
{
    std::vector<boost::shared_ptr<DoorInfo> >::iterator it;
    for (it = mDoorInfos.begin(); it != mDoorInfos.end(); ++it)
    {
        boost::shared_ptr<DoorInfo> doorInfo = *it;
        BlockPos pos = doorInfo->getPos();
        boost::shared_ptr<Village> village = getClosestVillage(pos, 32);
        if (!village)
        {
            village = boost::shared_ptr<Village>(new Village(mLevel));
            mVillages.push_back(village);
            setDirty();
        }
        village->addDoorInfo(doorInfo);
    }
    mDoorInfos.clear();
}

void Villages::tick()
{
    ++mTick;
    for (std::vector<boost::shared_ptr<Village> >::iterator it = mVillages.begin(); it != mVillages.end(); ++it)
    {
        boost::shared_ptr<Village> village = *it;
        village->tick(mTick);
    }
    removeVillages();
    processNextQuery();
    cluster();
    if (mTick % 400 == 0)
        setDirty();
}

std::vector<boost::shared_ptr<Village> >* Villages::getVillages()
{
    return &mVillages;
}

void Villages::load(CompoundTag* tag)
{
    mTick = tag->getInt(L"Tick");
    ListTag* list = tag->getList(L"Villages", Tag::TAG_COMPOUND);
    int i;
    for (i = 0; i < list->size(); ++i)
    {
        CompoundTag* villageTag = list->getCompound(i);
        boost::shared_ptr<Village> village(new Village());
        village->readAdditionalSaveData(villageTag);
        mVillages.push_back(village);
    }
}

CompoundTag* Villages::save(CompoundTag* tag)
{
    tag->putInt(L"Tick", mTick);
    ListTag* list = new ListTag();
    std::vector<boost::shared_ptr<Village> >::iterator it;
    for (it = mVillages.begin(); it != mVillages.end(); ++it)
    {
        boost::shared_ptr<Village> village = *it;
        CompoundTag* villageTag = new CompoundTag();
        village->addAdditonalSaveData(villageTag);
        list->add(villageTag);
    }
    tag->put(L"Villages", list);
    return tag;
}

std::wstring Villages::getFileId(Dimension* dimension)
{
    return VILLAGES_FILE_ID + dimension->getType()->getFileSuffix();
}
