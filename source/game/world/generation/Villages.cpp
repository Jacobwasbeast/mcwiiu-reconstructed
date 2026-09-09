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

Villages::Villages(const std::wstring& id)
    : SavedData(id), mLevel(NULL), mTick(0)
{
}

Villages::Villages(Level* level)
    : SavedData(L"villages"), mLevel(level), mTick(0)
{
}

Villages::~Villages()
{
}

void Villages::setLevel(Level* level)
{
    mLevel = level;
    std::vector<boost::shared_ptr<Village> >::iterator it;
    for (it = mVillages.begin(); it != mVillages.end(); ++it)
        (*it)->setLevel(level);
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
    if (mToUpdate.size() < 65 && !hasQuery(pos))
        mToUpdate.push_back(pos);
}

void Villages::removeVillages()
{
    std::vector<boost::shared_ptr<Village> >::iterator it = mVillages.begin();
    while (it != mVillages.end())
    {
        if ((*it)->canRemove())
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
    return DoorBlock::isDoor(state->getBlock()) && state->getMaterial() == Material::WOOD;
}

boost::shared_ptr<DoorInfo> Villages::getDoorInfo(const BlockPos& pos)
{
    std::vector<boost::shared_ptr<DoorInfo> >::iterator it;
    for (it = mDoorInfos.begin(); it != mDoorInfos.end(); ++it)
    {
        BlockPos doorPos = (*it)->getPos();
        if (doorPos.x == pos.x && doorPos.z == pos.z && std::abs(doorPos.y - pos.y) < 2)
            return *it;
    }

    std::vector<boost::shared_ptr<Village> >::iterator village;
    for (village = mVillages.begin(); village != mVillages.end(); ++village)
    {
        boost::shared_ptr<DoorInfo> info = (*village)->getDoorInfo(pos);
        if (info)
            return info;
    }

    return boost::shared_ptr<DoorInfo>();
}

int Villages::skyVisibleBlockCount(const BlockPos& pos, const Direction* direction, int count)
{
    int visible = 0;
    int i;
    for (i = 1; i <= 4 && visible < count; ++i)
        if (mLevel->canSeeSky(pos.relative(direction, i)))
            ++visible;
    return visible;
}

void Villages::createDoorInfo(const BlockPos& pos)
{
    const Direction* facing = DoorBlock::getFacing(mLevel, pos);
    const Direction* opposite = facing->getOpposite();
    int front = skyVisibleBlockCount(pos, facing, 5);
    int back = skyVisibleBlockCount(pos, opposite, front + 1);
    boost::shared_ptr<DoorInfo> info;
    if (back < front)
        info.reset(new DoorInfo(pos, opposite, mTick));
    else if (back > front)
        info.reset(new DoorInfo(pos, facing, mTick));
    if (info)
        mDoorInfos.push_back(info);
}

void Villages::addDoorInfos(const BlockPos& pos)
{
    int x;
    int y;
    int z;
    for (x = -16; x < 16; ++x)
        for (y = -4; y < 4; ++y)
            for (z = -16; z < 16; z += 2)
            {
                BlockPos first = pos.offset(x, y, z);
                BlockPos second = first.offset(0, 0, 1);
                BlockPos candidates[2] = { first, second };
                int i;
                for (i = 0; i < 2; ++i)
                    if (isDoor(candidates[i]))
                    {
                        boost::shared_ptr<DoorInfo> info = getDoorInfo(candidates[i]);
                        if (info)
                            info->setTimeStamp(mTick);
                        else
                            createDoorInfo(candidates[i]);
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
        const BlockPos* center = (*it)->getCenter();
        float d = (float)center->distSqr(pos);
        if (d < best)
        {
            int radius = (*it)->getRadius() + distance;
            if (d <= (float)(radius * radius))
            {
                best = d;
                closest = *it;
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
        BlockPos pos = (*it)->getPos();
        boost::shared_ptr<Village> village = getClosestVillage(pos, 32);
        if (!village)
        {
            village.reset(new Village(mLevel));
            mVillages.push_back(village);
            setDirty();
        }
        village->addDoorInfo(*it);
    }
    mDoorInfos.clear();
}

void Villages::tick()
{
    ++mTick;
    std::vector<boost::shared_ptr<Village> >::iterator it;
    for (it = mVillages.begin(); it != mVillages.end(); ++it)
        (*it)->tick(mTick);
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
    ListTag* list = tag->getList(L"Villages", 10);
    int i;
    for (i = 0; i < list->size(); ++i)
    {
        boost::shared_ptr<Village> village(new Village());
        village->readAdditionalSaveData(list->getCompound(i));
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
        CompoundTag* village = new CompoundTag();
        (*it)->addAdditonalSaveData(village);
        list->add(village);
    }
    tag->put(L"Villages", list);
    return tag;
}

std::wstring Villages::getFileId(Dimension* dimension)
{
    return std::wstring(L"villages") + dimension->getType()->getFileSuffix();
}
