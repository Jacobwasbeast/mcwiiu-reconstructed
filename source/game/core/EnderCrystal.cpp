#include "game/core/EnderCrystal.hpp"

#include <string>

#include "game/core/DamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EndDragonFight.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/util/NbtUtils.hpp"
#include "game/util/Random.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/TheEndDimension.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

class CConsoleMinecraftApp
{
public:
    uint32_t GetGameHostOption(uint32_t option);
};

extern CConsoleMinecraftApp gApp;

static const unsigned long long ENTITY_TYPE_ENDER_CRYSTAL = 0x000000000800000EULL;

const EntityDataAccessor<BlockPos*>* EnderCrystal::DATA_BEAM_TARGET = NULL;
const EntityDataAccessor<bool>* EnderCrystal::DATA_SHOW_BOTTOM = NULL;

EnderCrystal::EnderCrystal(Level* level) : Entity(level, true)
{
    _init(level);
}

EnderCrystal::EnderCrystal(Level* level, double x, double y, double z) : Entity(level, true)
{
    _init(level);
    setPos(x, y, z);
}

EnderCrystal::~EnderCrystal()
{
}

void EnderCrystal::staticCtor()
{
    DATA_BEAM_TARGET = SynchedEntityData::defineId(ENTITY_TYPE_ENDER_CRYSTAL,
                                                    ENTITY_TYPE_ENTITY,
                                                    EntityDataSerializers::OPTIONAL_BLOCK_POS);
    DATA_SHOW_BOTTOM = SynchedEntityData::defineId(ENTITY_TYPE_ENDER_CRYSTAL,
                                                   ENTITY_TYPE_ENTITY,
                                                   EntityDataSerializers::BOOLEAN);
}

void EnderCrystal::_init(Level* level)
{
    fjDerivedCtorCalls();
    mBlocksBuilding = true;
    setSize(2.0f, 2.0f);
    mTime = mRandom->nextInt(100000);
}

bool EnderCrystal::makeStepSound()
{
    return false;
}

void EnderCrystal::defineSynchedData()
{
    getEntityData()->createDateItem(DATA_BEAM_TARGET, static_cast<BlockPos*>(NULL));
    getEntityData()->createDateItem(DATA_SHOW_BOTTOM, true);
}

void EnderCrystal::tick()
{
    ++mTime;
    mXo = mX;
    mYo = mY;
    mZo = mZ;

    if (!mLevel->mIsClientSide)
    {
        BlockPos pos(this);
        if (mLevel->mDimension->getType() == DimensionType::THE_END &&
            mLevel->getBlockState(pos)->getBlock() != Blocks::FIRE)
        {
            mLevel->setBlockAndUpdate(pos, Blocks::FIRE->defaultBlockState());
        }
    }
}

BlockPos* EnderCrystal::getBeamTarget()
{
    return getEntityData()->get(DATA_BEAM_TARGET);
}

bool EnderCrystal::showsBottom()
{
    return getEntityData()->get(DATA_SHOW_BOTTOM);
}

void EnderCrystal::addAdditonalSaveData(CompoundTag* tag)
{
    if (getBeamTarget() != 0)
        tag->put(L"BeamTarget", NbtUtils::writeBlockPos(*getBeamTarget()));
    tag->putBoolean(L"ShowBottom", showsBottom());
}

void EnderCrystal::setBeamTarget(BlockPos* pos)
{
    getEntityData()->set(DATA_BEAM_TARGET, pos, true);
}

void EnderCrystal::setShowBottom(bool showBottom)
{
    getEntityData()->set(DATA_SHOW_BOTTOM, showBottom, true);
}

void EnderCrystal::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag->contains(L"BeamTarget", 10))
        setBeamTarget(new BlockPos(NbtUtils::readBlockPos(tag->getCompound(L"BeamTarget"))));
    if (tag->contains(L"ShowBottom", 1))
        setShowBottom(tag->getBoolean(L"ShowBottom"));
}

bool EnderCrystal::isPickable()
{
    return true;
}

void EnderCrystal::onDestroyedBy(const DamageSource* source)
{
    TheEndDimension* dimension = static_cast<TheEndDimension*>(mLevel->mDimension);
    if (dimension != NULL && dimension->getDragonFight() != NULL)
        dimension->getDragonFight()->onCrystalDestroyed(boost::static_pointer_cast<EnderCrystal>(shared_from_this()), source);
}

bool EnderCrystal::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;

    boost::shared_ptr<Entity> entity = source->getEntity();
    if (entity && entity->GetType() == ENTITY_TYPE_ENDER_DRAGON)
        return false;

    if (!mRemoved && !mLevel->mIsClientSide)
    {
        remove();
        if (!mLevel->mIsClientSide && gApp.GetGameHostOption(0xd) && !source->isExplosion())
            mLevel->explode(shared_from_this(), mX, mY, mZ, 6.0f, false, false, true, false, false, -1.0f);
        onDestroyedBy(source);
    }
    return true;
}

void EnderCrystal::kill()
{
    onDestroyedBy(NULL);
    Entity::kill();
}

bool EnderCrystal::shouldRenderAtSqrDistance(double distanceSqr)
{
    return Entity::shouldRenderAtSqrDistance(distanceSqr) || getBeamTarget() != NULL;
}

unsigned int EnderCrystal::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return static_cast<unsigned int>(-1);
}

unsigned long long EnderCrystal::GetType()
{
    return 0x000000000800000EULL;
}

void EnderCrystal::create(Level* level)
{
    new EnderCrystal(level);
}
