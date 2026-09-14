#include "game/entity/mob/EnderDragon.hpp"
#include "game/world/generation/Player.hpp"

#include <cmath>

#include "game/data/DataFixerUpper.hpp"
#include "game/core/BinaryHeap.hpp"
#include "game/core/Node.hpp"
#include "game/core/Path.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MultiEntityMobPart.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/entity/mob/EndDragonFight.hpp"
#include "game/util/AABB.hpp"
#include "game/core/Mth.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"

#include <limits>
#include <typeinfo>

static const EntityDataAccessor<int>* DATA_DRAGON_PHASE = NULL;

namespace
{

struct DragonNodeList
{
    Node** nodes;
    int count;
};

} // namespace

EnderDragon::EnderDragon(Level* level)
    : Mob(level), mOpenSet(NULL), mPath(NULL), mDragonFight(NULL),
      mDragonPhase(0), mTicksSinceDeath(0), mTarget(), mNearbyCrystal(0),
      mDragonKilled(false), mPreviouslyKilled(false), mHasTarget(false), mWasHurt(false),
      mHealth(200), mTargetX(0.0), mTargetY(0.0), mTargetZ(0.0),
      mSittingDamageReceived(0), mHurtWhilePerched(false),
      mFlapTime(0.0f), mOldFlapTime(0.0f), mDeathTime(0), mBeamTarget()
{
    _init();
}

EnderDragon::~EnderDragon()
{
    delete mOpenSet;
    delete mPath;
}

std::wstring EnderDragon::getDisplayName()
{
    return L"entity.EnderDragon.name";
}

float EnderDragon::getMaxHealth() { return 200.0f; }
float EnderDragon::getHealth() { return LivingEntity::getHealth(); }
unsigned int EnderDragon::getDimension() { return 1; }
Level* EnderDragon::getLevel() { return Entity::getCommandSenderWorld(); }

unsigned int EnderDragon::getSynchedAction()
{
    return static_cast<unsigned int>(getEntityData()->get(DATA_DRAGON_PHASE));
}

int EnderDragon::setSynchedAction(EEnderdragonAction action, bool force)
{
    if (!force && action == static_cast<EEnderdragonAction>(mDragonPhase))
        return 0;
    mDragonPhase = static_cast<int>(action);
    return 1;
}

unsigned int EnderDragon::findClosestNode(double, double, double) { return 0; }

namespace
{

static Node* makeDragonPathNode(Level* level, float step, unsigned int index, unsigned int indexSubtract,
                                float angleBase, double radius, int yOffset)
{
    const float angle = step * static_cast<float>(static_cast<int>(index - indexSubtract)) + angleBase;
    const float doubled = angle + angle;
    const int x = static_cast<int>(radius * static_cast<double>(Mth::cos(doubled)));
    const int z = static_cast<int>(radius * static_cast<double>(Mth::sin(doubled)));
    BlockPos probe(x, 0, z);
    const BlockPos top = level->getTopSolidBlockPos(probe);
    const int minY = level->getSeaLevel() + 10;
    int y = top.y + yOffset;
    if (y < minY)
        y = minY;
    return new Node(x, y, z);
}

static void storeDragonPathNode(DragonNodeList* nodeList, int slotOffset, Node* node)
{
    nodeList->nodes[slotOffset / 4] = node;
}

} // namespace

unsigned int EnderDragon::findClosestNode()
{
    DragonNodeList* nodeList =
        *reinterpret_cast<DragonNodeList**>(reinterpret_cast<char*>(this) + 0xdb0);
    if (nodeList != NULL && nodeList->nodes[0] == NULL)
    {
        Level* level = mLevel;
        const float angleBase = -3.1415927410125732f;
        const float innerStep = 0.2617993950843811f;
        const float middleStep = 0.39269909262657166f;
        const float outerStep = 0.7853981852531433f;
        const double innerRadius = 60.0;
        const double middleRadius = 40.0;
        const double outerRadius = 20.0;

        unsigned int ringCounter = 1;
        unsigned int angleIndex = 0;
        int slotOffset = 4;
        int iterations = 12;

        do
        {
            while (angleIndex > 11)
            {
                if (angleIndex > 19)
                {
                    storeDragonPathNode(
                        nodeList, slotOffset - 4,
                        makeDragonPathNode(level, outerStep, angleIndex, 20, angleBase, outerRadius, 5));
                    goto storeSecondNode;
                }
                storeDragonPathNode(
                    nodeList, slotOffset - 4,
                    makeDragonPathNode(level, middleStep, angleIndex, 12, angleBase, middleRadius, 15));
                goto storeSecondNode;
            }

            storeDragonPathNode(
                nodeList, slotOffset - 4,
                makeDragonPathNode(level, innerStep, angleIndex, 0, angleBase, innerRadius, 5));

        storeSecondNode:
            if (ringCounter < 12)
            {
                storeDragonPathNode(
                    nodeList, slotOffset,
                    makeDragonPathNode(level, innerStep, angleIndex + 1, 0, angleBase, innerRadius, 5));
                slotOffset += 8;
                --iterations;
                angleIndex += 2;
                ringCounter += 2;
                if (iterations == 0)
                    goto initializeConnections;
                continue;
            }

            if (ringCounter < 20)
            {
                storeDragonPathNode(
                    nodeList, slotOffset,
                    makeDragonPathNode(level, middleStep, angleIndex, 11, angleBase, middleRadius, 15));
            }
            else
            {
                storeDragonPathNode(
                    nodeList, slotOffset,
                    makeDragonPathNode(level, outerStep, angleIndex, 19, angleBase, outerRadius, 5));
            }

            ringCounter += 2;
            angleIndex += 2;
            --iterations;
            slotOffset += 8;
        } while (iterations != 0);

    initializeConnections:
        static const int nodeConnections[] = {
            0x1802, 0x2005, 0x200a, 0x4014, 0x8028, 0x8050, 0x100a0, 0x20140, 0x20280, 0x40500,
            0x80a00, 0x80401, 0x182001, 0x305006, 0x20a008, 0x614030, 0x428040, 0xc50180,
            0x8a0200, 0x941c00, 0xe83000, 0xd0e000, 0xb38000, 0x7e0000};
        for (unsigned int i = 0; i < sizeof(nodeConnections) / sizeof(nodeConnections[0]); ++i)
        {
            *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0xdb4 + static_cast<int>(i * 4)) =
                nodeConnections[i];
        }
    }

    return findClosestNode(mX, mY, mZ);
}
double EnderDragon::getHeadYRotDiff(float) { return 0.0; }

Vec3* EnderDragon::getHeadLookVector(float partialTicks)
{
    const EEnderdragonAction action = static_cast<EEnderdragonAction>(getSynchedAction());
    if (action == ACTION_3 || action == ACTION_4)
    {
        const BlockPos top = mLevel->getTopSolidBlockPos(BlockPos(0, 0, 0));
        double distance = std::sqrt(distanceToSqr(0.0, static_cast<double>(top.y), 0.0));
        float scale = static_cast<float>(distance) * 0.25f;
        if (scale < 1.0f)
            scale = 1.0f;

        const float savedYRot = mYRot;
        const float savedXRot = mXRot;
        mXRot = -((6.0f / scale) * 1.5f * 5.0f);
        mYRot = savedYRot + static_cast<float>(getHeadYRotDiff(partialTicks));
        Vec3* result = getViewVector(partialTicks);
        mXRot = savedXRot;
        mYRot = savedYRot;
        return result;
    }

    if (action == ACTION_5 || action == ACTION_6 || action == ACTION_7)
    {
        const float savedYRot = mYRot;
        const float savedXRot = mXRot;
        mXRot = -45.0f;
        mYRot = savedYRot + static_cast<float>(getHeadYRotDiff(partialTicks));
        Vec3* result = getViewVector(partialTicks);
        mXRot = savedXRot;
        mYRot = savedYRot;
        return result;
    }

    return getViewVector(partialTicks);
}

Path* EnderDragon::reconstruct_path(Node*, Node* end)
{
    unsigned int count = 1;
    for (Node* node = end->cameFrom; node != NULL; node = node->cameFrom)
    {
        ++count;
    }

    Node** nodes = new Node*[count];
    unsigned int index = count - 1;
    nodes[index] = end;
    for (Node* node = end->cameFrom; node != NULL; node = node->cameFrom)
    {
        --index;
        nodes[index] = node;
    }

    arrayWithLength<Node*> nodeArray(count, false);
    nodeArray.data = nodes;
    nodeArray.length = count;

    Path* path = new Path(nodeArray);
    delete[] nodes;
    return path;
}
Path* EnderDragon::findPath(int start, int end, Node* target)
{
    DragonNodeList* nodeList =
        *reinterpret_cast<DragonNodeList**>(reinterpret_cast<char*>(this) + 0xdb0);
    if (nodeList == NULL || mOpenSet == NULL)
        return NULL;

    Node** nodes = nodeList->nodes;
    const float resetCost = 0.0f;
    for (int group = 0; group < 3; ++group)
    {
        for (int slot = 0; slot < 8; ++slot)
        {
            Node* node = nodes[group * 8 + slot];
            if (node == NULL)
                continue;
            node->cameFrom = NULL;
            node->heapIdx = -1;
            node->g = resetCost;
            node->h = resetCost;
            node->f = resetCost;
            node->closed = false;
        }
    }

    Node* startNode = nodes[start];
    Node* endNode = nodes[end];
    if (startNode == NULL || endNode == NULL)
        return NULL;

    startNode->g = resetCost;
    const double startDistance = startNode->distanceTo(endNode);
    startNode->h = static_cast<float>(startDistance);
    startNode->f = static_cast<float>(startDistance);

    mOpenSet->clear();
    mOpenSet->insert(startNode);

    unsigned int connectionStart = 0;
    if (mDragonFight != NULL && mDragonFight->getCrystalsAlive() == 0)
        connectionStart = 12;

    int* nodeConnections =
        reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0xdb4);
    Node* bestNode = startNode;

    while (!mOpenSet->isEmpty())
    {
        Node* current = mOpenSet->pop();
        if (current->equals(endNode))
        {
            if (target != NULL)
                target->cameFrom = endNode;
            return reconstruct_path(startNode, target != NULL ? target : endNode);
        }

        if (current->distanceTo(endNode) < bestNode->distanceTo(endNode))
            bestNode = current;

        current->closed = true;

        int nodeIndex = -1;
        for (int base = 0; base < 24 && nodeIndex < 0; base += 8)
        {
            for (int offset = 0; offset < 8; ++offset)
            {
                if (nodes[base + offset] == current)
                {
                    nodeIndex = base + offset;
                    break;
                }
            }
        }

        if (nodeIndex >= 0 && connectionStart + 1 < 24)
        {
            const unsigned int mask = nodeConnections[nodeIndex];
            unsigned int bitIndex = connectionStart;
            unsigned int pairCount = (24 - connectionStart) >> 1;
            int neighborSlot = static_cast<int>(connectionStart) * 4 + 4;
            while (pairCount > 0)
            {
                if ((mask & (1u << (bitIndex & 0x3f))) != 0)
                {
                    Node* neighbor = nodes[(neighborSlot - 4) >> 2];
                    if (neighbor != NULL && !neighbor->closed)
                    {
                        const float newG =
                            current->g + static_cast<float>(current->distanceTo(neighbor));
                        if (!neighbor->inOpenSet() || newG < neighbor->g)
                        {
                            neighbor->g = newG;
                            neighbor->cameFrom = current;
                            neighbor->h = static_cast<float>(neighbor->distanceTo(endNode));
                            neighbor->f = neighbor->g + neighbor->h;
                            if (neighbor->inOpenSet())
                                mOpenSet->changeCost(neighbor, neighbor->f);
                            else
                                mOpenSet->insert(neighbor);
                        }
                    }
                }

                ++bitIndex;
                if ((mask & (1u << (bitIndex & 0x3f))) != 0)
                {
                    Node* neighbor = nodes[neighborSlot >> 2];
                    if (neighbor != NULL && !neighbor->closed)
                    {
                        const float newG =
                            current->g + static_cast<float>(current->distanceTo(neighbor));
                        if (!neighbor->inOpenSet() || newG < neighbor->g)
                        {
                            neighbor->g = newG;
                            neighbor->cameFrom = current;
                            neighbor->h = static_cast<float>(neighbor->distanceTo(endNode));
                            neighbor->f = neighbor->g + neighbor->h;
                            if (neighbor->inOpenSet())
                                mOpenSet->changeCost(neighbor, neighbor->f);
                            else
                                mOpenSet->insert(neighbor);
                        }
                    }
                }

                neighborSlot += 8;
                --pairCount;
                bitIndex++;
            }
        }
        else if (nodeIndex >= 0)
        {
            const unsigned int mask = nodeConnections[nodeIndex];
            unsigned int remaining = 24 - connectionStart;
            unsigned int neighborIndex = connectionStart;
            while (remaining > 0)
            {
                if ((mask & (1u << (neighborIndex & 0x3f))) != 0)
                {
                    Node* neighbor = nodes[neighborIndex];
                    if (neighbor != NULL && !neighbor->closed)
                    {
                        const float newG =
                            current->g + static_cast<float>(current->distanceTo(neighbor));
                        if (!neighbor->inOpenSet() || newG < neighbor->g)
                        {
                            neighbor->g = newG;
                            neighbor->cameFrom = current;
                            neighbor->h = static_cast<float>(neighbor->distanceTo(endNode));
                            neighbor->f = neighbor->g + neighbor->h;
                            if (neighbor->inOpenSet())
                                mOpenSet->changeCost(neighbor, neighbor->f);
                            else
                                mOpenSet->insert(neighbor);
                        }
                    }
                }
                --remaining;
                ++neighborIndex;
            }
        }
    }

    if (bestNode != startNode)
    {
        if (target != NULL)
            target->cameFrom = bestNode;
        return reconstruct_path(startNode, target != NULL ? target : bestNode);
    }

    return NULL;
}

void EnderDragon::navigateToNextPathNode()
{
    if (mPath != NULL && !mPath->isDone())
        mPath->next();
}

void EnderDragon::strafeAttackTarget()
{
    const unsigned int startNode = findClosestNode();
    if (mTarget == 0)
        return;
    const unsigned int endNode = findClosestNode(mTarget->mX, mTarget->mY, mTarget->mZ);
    const double dx = mTarget->mX - mX;
    const double dz = mTarget->mZ - mZ;
    double distance = std::sqrt(dx * dx + dz * dz);
    distance = distance / 80.0 + 10.0;
    if (distance > 10.0)
        distance = 10.0;
    const double targetY = mTarget->mY + distance;
    Node targetNode(static_cast<int>(mTarget->mX), static_cast<int>(targetY), static_cast<int>(mTarget->mZ));
    if (mPath != NULL)
        delete mPath;
    mPath = findPath(static_cast<int>(startNode), static_cast<int>(endNode), &targetNode);
    if (mPath != NULL)
    {
        mPath->next();
        navigateToNextPathNode();
    }
}
unsigned int EnderDragon::findNewTarget()
{
    if (mLevel == NULL)
        return 0;

    boost::shared_ptr<Player> target = mLevel->getNearestPlayer(mX, mY, mZ, 64.0, true, -1.0);
    mTarget = boost::static_pointer_cast<Entity>(target);
    mHasTarget = mTarget != 0;
    if (mHasTarget)
        setSynchedAction(ACTION_1, false);
    else
        setSynchedAction(ACTION_NONE, false);
    return mHasTarget ? 1U : 0U;
}
bool EnderDragon::hurt(boost::shared_ptr<MultiEntityMobPart> part, const DamageSource* source, float amount)
{
    if (part.get() != mHead.get())
    {
        float minAmount = 1.0f;
        if ((amount - 1.0f) < 0.0f)
            minAmount = amount;
        amount = amount * 0.25f + minAmount;
    }
    if (amount < 0.01f)
        return false;

    bool isRealDamage = true;
    boost::shared_ptr<Entity> attacker = source->getEntity();
    if (attacker != 0)
    {
        if ((attacker->GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER)
            isRealDamage = source->isExplosion();
    }
    else
    {
        isRealDamage = source->isExplosion();
    }

    if (isRealDamage)
    {
        float healthBefore = getHealth();
        LivingEntity::actuallyHurt(source, amount);
        EEnderdragonAction action = static_cast<EEnderdragonAction>(getSynchedAction());
        if (getHealth() <= 0.0f && action != ACTION_5 && action != ACTION_6 && action != ACTION_7)
        {
            setHealth(1.0f);
            if (setSynchedAction(ACTION_2, false) != 0)
            {
                while (mPath != NULL && !mPath->isDone())
                    mPath->next();
                findNewTarget();
            }
        }

        action = static_cast<EEnderdragonAction>(getSynchedAction());
        if (action == ACTION_5 || action == ACTION_6 || action == ACTION_7)
        {
            mSittingDamageReceived += static_cast<int>(healthBefore) - static_cast<int>(getHealth());
            if (static_cast<float>(mSittingDamageReceived) > getMaxHealth() * 0.25f)
            {
                mSittingDamageReceived = 0;
                setSynchedAction(ACTION_4, false);
                mHurtWhilePerched = true;
            }
        }
    }
    return true;
}
void EnderDragon::GetInteractTooltip(boost::shared_ptr<MultiEntityMobPart>, const EntityTooltipDataHolder& holder)
{
    GetInteractTooltip(holder);
}
void EnderDragon::AddParts()
{
    boost::shared_ptr<MultiEntityMob> parent =
        boost::dynamic_pointer_cast<MultiEntityMob>(shared_from_this());

    mHead = boost::shared_ptr<MultiEntityMobPart>(new MultiEntityMobPart(parent, L"head", 6.0f, 6.0f));
    mNeck = boost::shared_ptr<MultiEntityMobPart>(new MultiEntityMobPart(parent, L"neck", 6.0f, 6.0f));
    mBody = boost::shared_ptr<MultiEntityMobPart>(new MultiEntityMobPart(parent, L"body", 8.0f, 8.0f));
    mTail1 = boost::shared_ptr<MultiEntityMobPart>(new MultiEntityMobPart(parent, L"tail", 4.0f, 4.0f));
    mTail2 = boost::shared_ptr<MultiEntityMobPart>(new MultiEntityMobPart(parent, L"tail", 4.0f, 4.0f));
    mTail3 = boost::shared_ptr<MultiEntityMobPart>(new MultiEntityMobPart(parent, L"tail", 4.0f, 4.0f));
    mWing1 = boost::shared_ptr<MultiEntityMobPart>(new MultiEntityMobPart(parent, L"wing", 4.0f, 4.0f));
    mWing2 = boost::shared_ptr<MultiEntityMobPart>(new MultiEntityMobPart(parent, L"wing", 4.0f, 4.0f));

    mParts.push_back(boost::static_pointer_cast<Entity>(mHead));
    mParts.push_back(boost::static_pointer_cast<Entity>(mNeck));
    mParts.push_back(boost::static_pointer_cast<Entity>(mBody));
    mParts.push_back(boost::static_pointer_cast<Entity>(mTail1));
    mParts.push_back(boost::static_pointer_cast<Entity>(mTail2));
    mParts.push_back(boost::static_pointer_cast<Entity>(mTail3));
    mParts.push_back(boost::static_pointer_cast<Entity>(mWing1));
    mParts.push_back(boost::static_pointer_cast<Entity>(mWing2));
}
void EnderDragon::onCrystalDestroyed(boost::shared_ptr<EnderCrystal> crystal, const BlockPos& position, const DamageSource* source)
{
    boost::shared_ptr<Player> player;
    if (source != NULL)
    {
        boost::shared_ptr<Entity> attacker = source->getEntity();
        if (Entity::instanceof(attacker, ENTITY_TYPE_PLAYER))
            player = boost::dynamic_pointer_cast<Player>(attacker);
    }
    if (player == 0 && mLevel != NULL)
        player = mLevel->getNearestAttackablePlayer(position, 64.0, 16.0);

    if (crystal != 0 && crystal == mBeamTarget)
    {
        mBeamTarget.reset();
        if (player != 0)
        {
            boost::shared_ptr<LivingEntity> sourceEntity =
                boost::static_pointer_cast<LivingEntity>(player);
            boost::shared_ptr<EntityDamageSource> explosion =
                DamageSource::CreateExplosion(sourceEntity);
            hurt(explosion.get(), 10.0f);
        }
    }

    if (mDragonFight != NULL)
    {
        if (mDragonFight->getCrystalsAlive() == 0)
        {
            if (setSynchedAction(ACTION_2, false) == 0)
            {
                if (mTarget != 0 && Entity::instanceof(mTarget, ENTITY_TYPE_PLAYER))
                {
                    mTarget = boost::static_pointer_cast<Entity>(
                        boost::dynamic_pointer_cast<Player>(mTarget));
                    setSynchedAction(ACTION_1, false);
                    strafeAttackTarget();
                }
                while (mPath != NULL && !mPath->isDone())
                    mPath->next();
            }
        }
    }
}

void EnderDragon::_init()
{
    for (int i = 0; i < 64; ++i)
        for (int j = 0; j < 3; ++j)
            mPosHistory[i][j] = 0.0;
    mDeathTime = 0;
    mBeamTarget.reset();
    setSynchedAction(ACTION_NONE, false);
    mNearbyCrystal = -1;
    mSittingDamageReceived = 0;
    mHurtWhilePerched = false;

    mOpenSet = new BinaryHeap();
    mPath = NULL;
}

void EnderDragon::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x0d000005ULL);
}

void EnderDragon::staticCtor()
{
    DATA_DRAGON_PHASE = SynchedEntityData::defineId<int>(ENTITY_TYPE_ENDER_DRAGON, ENTITY_TYPE_MOB,
                                                         EntityDataSerializers::INT);
}
void EnderDragon::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(200.0);
}
void EnderDragon::defineSynchedData()
{
    Mob::defineSynchedData();
    getEntityData()->createDateItem(DATA_DRAGON_PHASE, 0);
}
bool EnderDragon::canRide(boost::shared_ptr<Entity>) { return false; }

void EnderDragon::getLatencyPos(arrayWithLength<double> positions, int index, float)
{
    if (index < 0 || index >= 64 || positions.data == NULL)
        return;
    positions.data[0] = mPosHistory[index][0];
    positions.data[1] = mPosHistory[index][1];
    positions.data[2] = mPosHistory[index][2];
}

double EnderDragon::getTilt(float partialTicks)
{
    double pos5[3];
    double pos10[3];
    arrayWithLength<double> array5(3, false);
    arrayWithLength<double> array10(3, false);
    array5.data = pos5;
    array10.data = pos10;
    array5.length = 3;
    array10.length = 3;
    getLatencyPos(array5, 5, partialTicks);
    getLatencyPos(array10, 10, partialTicks);
    return static_cast<double>(static_cast<float>((pos5[1] - pos10[1]) * 10.0));
}

double EnderDragon::getHeadYOffset(float partialTicks)
{
    const EEnderdragonAction action = static_cast<EEnderdragonAction>(getSynchedAction());
    if (action == ACTION_5 || action == ACTION_6 || action == ACTION_7)
        return -1.0;

    double headPos[3];
    double tailPos[3];
    arrayWithLength<double> headArray(3, false);
    arrayWithLength<double> tailArray(3, false);
    headArray.data = headPos;
    tailArray.data = tailPos;
    headArray.length = 3;
    tailArray.length = 3;
    getLatencyPos(headArray, 5, 1.0f);
    getLatencyPos(tailArray, 0, 1.0f);
    return tailPos[1] - headPos[1];
}

double EnderDragon::rotWrap(double angle)
{
    while (angle >= 180.0) angle -= 360.0;
    while (angle < -180.0) angle += 360.0;
    return angle;
}

bool EnderDragon::checkCrystals()
{
    if (mBeamTarget != 0)
    {
        if (!mBeamTarget->showsBottom())
        {
            if (mTicksSinceDeath == (mTicksSinceDeath / 10) * 10)
            {
                if (getHealth() < getMaxHealth())
                    setHealth(getHealth() + 1.0f);
            }
        }
        else
        {
            mBeamTarget.reset();
        }
    }

    if (mRandom->nextInt(10) != 0)
        return mBeamTarget != 0;

    if (mLevel == NULL || mBoundingBox == NULL)
        return mBeamTarget != 0;

    AABB* searchBox = mBoundingBox->grow(32.0, 32.0, 32.0);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mLevel->getEntitiesOfClass(typeid(EnderCrystal), searchBox);
    delete searchBox;

    boost::shared_ptr<EnderCrystal> closest;
    double closestDistance = std::numeric_limits<double>::max();

    if (entities != NULL)
    {
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
             it != entities->end(); ++it)
        {
            boost::shared_ptr<EnderCrystal> crystal =
                boost::dynamic_pointer_cast<EnderCrystal>(*it);
            if (crystal == 0)
                continue;

            const double distance = distanceToSqr(crystal);
            if (distance < closestDistance)
            {
                closestDistance = distance;
                closest = crystal;
            }
        }
        delete entities;
    }

    if (closest != 0)
        mBeamTarget = closest;

    return mBeamTarget != 0;
}
void EnderDragon::knockBack(std::vector<boost::shared_ptr<Entity> >* entities)
{
    if (entities == NULL || mParts.empty() || mParts[0] == 0)
        return;
    boost::shared_ptr<Entity> head = mParts[0];
    const double headX = head->mX;
    const double headZ = head->mZ;
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;
        if (!entity || (entity->GetType() & 0x0C000000ULL) != 0x0C000000ULL)
            continue;
        boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(entity);
        if (!living)
            continue;
        const double dx = living->mX - headX;
        const double dz = living->mZ - headZ;
        const double distSq = dx * dx + dz * dz;
        if (distSq <= 0.0)
            continue;
        living->knockback(shared_from_this(), 0.2f, (dx / distSq) * 4.0, (dz / distSq) * 4.0);
    }
}
void EnderDragon::hurt(std::vector<boost::shared_ptr<Entity> >* entities)
{
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;
        if ((entity->GetType() & 0x0C000000ULL) == 0x0C000000ULL)
        {
            boost::shared_ptr<LivingEntity> target = boost::dynamic_pointer_cast<LivingEntity>(entity);
            boost::shared_ptr<LivingEntity> self = boost::dynamic_pointer_cast<LivingEntity>(shared_from_this());
            EntityDamageSource* source = DamageSource::CreateMobAttack(self);
            target->hurt(source, 10.0f);
            delete source;
        }
    }
}
namespace
{
bool isCheckWallsExcludedBlock(Block* block)
{
    return block == Blocks::bedrock
        || block == Blocks::obsidian
        || block == Blocks::end_stone
        || block == Blocks::end_portal
        || block == Blocks::end_portal_frame
        || block == Blocks::command_block
        || block == Blocks::BARRIER
        || block == Blocks::repeating_command_block
        || block == Blocks::chain_command_block
        || block == Blocks::iron_bars
        || block == Blocks::end_gateway;
}
}

bool EnderDragon::checkWalls(AABB* box)
{
    if (box == NULL || mLevel == NULL)
        return false;

    const int minX = Mth::floor(box->mMinX);
    const int minY = Mth::floor(box->mMinY);
    const int minZ = Mth::floor(box->mMinZ);
    const int maxX = Mth::floor(box->mMaxX);
    const int maxY = Mth::floor(box->mMaxY);
    const int maxZ = Mth::floor(box->mMaxZ);

    bool hitWall = false;
    bool foundDestructible = false;
    const bool mobGriefing = mLevel->getGameRules()->getBoolean(1);

    for (int x = minX; x <= maxX; ++x)
    {
        for (int y = minY; y <= maxY; ++y)
        {
            for (int z = minZ; z <= maxZ; ++z)
            {
                const BlockPos pos(x, y, z);
                const BlockState* state = mLevel->getBlockState(pos);
                Material* material = state->getMaterial();
                if (material == Material::AIR || material == Material::FIRE)
                    continue;

                Block* block = state->getBlock();
                if (mobGriefing && !isCheckWallsExcludedBlock(block))
                {
                    bool destructible = true;
                    if (!mLevel->isEmptyBlock(pos) && !foundDestructible)
                        destructible = false;
                    if (destructible)
                        foundDestructible = true;
                }
                else
                {
                    hitWall = true;
                }
            }
        }
    }

    if (foundDestructible)
    {
        const double px = box->mMinX + mRandom->nextFloat() * (box->mMaxX - box->mMinX);
        const double py = box->mMinY + mRandom->nextFloat() * (box->mMaxY - box->mMinY);
        const double pz = box->mMinZ + mRandom->nextFloat() * (box->mMaxZ - box->mMinZ);
        mLevel->addParticle(PARTICLE_LARGE_EXPLODE, px, py, pz, 128.0, 128.0, 128.0, arrayWithLength<int>());
    }

    return hitWall;
}
void EnderDragon::aiStep() { Mob::aiStep(); }
ResourceLocation* EnderDragon::getDefaultLootTable() { return NULL; }
bool EnderDragon::hurt(const DamageSource* source, float amount) { return LivingEntity::hurt(source, amount); }
void EnderDragon::reallyHurt(const DamageSource* source, float amount) { LivingEntity::actuallyHurt(source, amount); }
void EnderDragon::kill()
{
    remove();
    if (mDragonFight != NULL)
    {
        boost::shared_ptr<EnderDragon> self =
            boost::static_pointer_cast<EnderDragon>(shared_from_this());
        mDragonFight->updateDragon(self);
        mDragonFight->setDragonKilled(self);
    }
}

void EnderDragon::dropExperience(int experience)
{
    while (experience > 0)
    {
        const int orbValue = ExperienceOrb::getExperienceValue(experience);
        experience -= orbValue;
        boost::shared_ptr<ExperienceOrb> orb(
            new ExperienceOrb(mLevel, mX, mY, mZ, orbValue));
        mLevel->addEntity(boost::static_pointer_cast<Entity>(orb));
    }
}

void EnderDragon::tickDeath()
{
    const EEnderdragonAction action = static_cast<EEnderdragonAction>(getSynchedAction());
    if (action != ACTION_5 && action != ACTION_6 && action != ACTION_7)
    {
        if (mLevel != NULL && !mLevel->mIsClientSide)
            LivingEntity::tickDeath();
        return;
    }

    if (mDragonFight != NULL)
    {
        boost::shared_ptr<EnderDragon> self =
            boost::static_pointer_cast<EnderDragon>(shared_from_this());
        mDragonFight->updateDragon(self);
    }

    const int prevDeathTime = mDeathTime;
    ++mDeathTime;

    if (mLevel != NULL && (unsigned)(prevDeathTime - 0xB3) < 0x15U)
    {
        const float spread = 8.0f;
        const double px = mX + (double)((mRandom->nextFloat() - 0.5f) * spread);
        const double py = mY + (double)((mRandom->nextFloat() - 0.5f) * 4.0f) + 2.0;
        const double pz = mZ + (double)((mRandom->nextFloat() - 0.5f) * spread);
        mLevel->addParticle(PARTICLE_DRAGON_BREATH, px, py, pz, 0.0, 128.0, 128.0,
                            arrayWithLength<int>());
    }

    const bool doExpDrop = mLevel != NULL && mLevel->getGameRules()->getBoolean(4);
    unsigned int totalExp = 500U;
    if (mDragonFight != NULL && !mDragonFight->hasPreviouslyKilledDragon())
        totalExp = 12000U;

    if (mLevel != NULL && !mLevel->mIsClientSide)
    {
        if (mDeathTime > 150)
        {
            if (mDeathTime % 5 == 0 && doExpDrop)
                dropExperience(static_cast<int>((float)totalExp * 0.05f));
        }
        if (mDeathTime == 1)
        {
            BlockPos pos(shared_from_this());
            mLevel->globalLevelEvent(0x404, pos, 0);
        }
    }

    makeStepSound();

    mFlapTime += 20.0f;
    mOldFlapTime = mFlapTime;

    if (mDeathTime == 200 && mLevel != NULL && !mLevel->mIsClientSide)
    {
        if (doExpDrop)
            dropExperience(static_cast<int>((float)totalExp * 0.9f));
        if (mDragonFight != NULL)
        {
            boost::shared_ptr<EnderDragon> self =
                boost::static_pointer_cast<EnderDragon>(shared_from_this());
            mDragonFight->setDragonKilled(self);
        }
        kill();
    }
}
void EnderDragon::checkDespawn() {}
std::vector<boost::shared_ptr<Entity> >* EnderDragon::getSubEntities() { return &mParts; }
bool EnderDragon::isPickable() { return false; }
const SoundEvent* EnderDragon::getAmbientSound() { return NULL; }
const SoundEvent* EnderDragon::getHurtSound(const DamageSource*) { return NULL; }
float EnderDragon::getSoundVolume() { return 5.0f; }

void EnderDragon::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    if (tag != NULL)
        tag->putInt(L"DragonState", getSynchedAction());
}

void EnderDragon::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    if (tag != NULL && tag->contains(L"DragonState"))
        setSynchedAction(static_cast<EEnderdragonAction>(tag->getInt(L"DragonState")), true);
}

double EnderDragon::getHeadPartYOffset(int part, arrayWithLength<double> first, arrayWithLength<double> second)
{
    if (part == 6 || first.data == NULL || second.data == NULL)
        return static_cast<double>(part);
    return second.data[1] - first.data[1];
}

double EnderDragon::getHeadPartYRotDiff(int, arrayWithLength<double> first, arrayWithLength<double> second)
{
    return second.data[0] - first.data[0];
}

unsigned int EnderDragon::GetInteractTooltip(const EntityTooltipDataHolder&) { return 0xffffffffU; }
unsigned long long EnderDragon::GetType() { return 0x000000000D000005ULL; }

boost::shared_ptr<EnderDragon> EnderDragon::create(Level* level)
{
    return boost::shared_ptr<EnderDragon>(new EnderDragon(level));
}
