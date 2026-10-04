#include "game/world/WorldBorder.hpp"

#include <algorithm>
#include "game/core/BorderStatus.hpp"
#include "game/core/System.hpp"
#include "game/network/BorderChangeListener.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/entity/mob/Entity.hpp"

WorldBorder::WorldBorder(Level*)
    : m_centerX(0.0), m_centerZ(0.0), m_size(60000000.0), m_lerpTarget(60000000.0), m_lerpEnd(0), m_lerpStart(0),
      m_absoluteMaxSize(30000000), m_damagePerBlock(0.2), m_damageSafeZone(5.0), m_warningTime(15),
      m_warningBlocks(5)
{
}

WorldBorder::~WorldBorder()
{
}

BorderStatus* WorldBorder::GROWING = new BorderStatus(0x40ff80);
BorderStatus* WorldBorder::SHRINKING = new BorderStatus(0xff3030);
BorderStatus* WorldBorder::STATIONARY = new BorderStatus(0x20a0ff);

BorderStatus* WorldBorder::getStatus()
{
    if (m_lerpTarget < m_size) return SHRINKING;
    if (m_lerpTarget > m_size) return GROWING;
    return STATIONARY;
}

std::vector<BorderChangeListener*>& WorldBorder::getListeners() { return m_listeners; }

void WorldBorder::setSize(double size)
{
    m_size = size;
    m_lerpTarget = size;
    m_lerpEnd = System::processTimeInMilliSecs();
    m_lerpStart = m_lerpEnd;
    std::vector<BorderChangeListener*>& listeners = getListeners();
    for (std::vector<BorderChangeListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it)
    {
        (*it)->onBorderSizeSet(this, size);
    }
}

double WorldBorder::getSize()
{
    double size;
    if (getStatus() != STATIONARY)
    {
        long long now = System::processTimeInMilliSecs();
        float progress = static_cast<float>(now - m_lerpStart) / static_cast<float>(m_lerpEnd - m_lerpStart);
        if (progress < 1.0) return m_size + (m_lerpTarget - m_size) * progress;
        setSize(m_lerpTarget);
        size = m_size;
    }
    else
    {
        size = m_size;
    }
    return size;
}

double WorldBorder::getMinX()
{
    double lowest = getCenterX() - getSize() * 0.5;
    double limit = static_cast<double>(-m_absoluteMaxSize);
    if (lowest < limit) lowest = limit;
    return lowest;
}
double WorldBorder::getMaxX()
{
    double highest = getCenterX() + getSize() * 0.5;
    double limit = static_cast<double>(m_absoluteMaxSize);
    if (limit < highest) highest = limit;
    return highest;
}
double WorldBorder::getMinZ()
{
    double lowest = getCenterZ() - getSize() * 0.5;
    double limit = static_cast<double>(-m_absoluteMaxSize);
    if (lowest < limit) lowest = limit;
    return lowest;
}
double WorldBorder::getMaxZ()
{
    double highest = getCenterZ() + getSize() * 0.5;
    double limit = static_cast<double>(m_absoluteMaxSize);
    if (limit < highest) highest = limit;
    return highest;
}

bool WorldBorder::isWithinBounds(const BlockPos& pos)
{
    return static_cast<double>(pos.x + 1) > getMinX() && static_cast<double>(pos.x) < getMaxX()
        && static_cast<double>(pos.z + 1) > getMinZ() && static_cast<double>(pos.z) < getMaxZ();
}

bool WorldBorder::isWithinBounds(AABB* box)
{
    return box->mMaxX > getMinX() && box->mMinX < getMaxX() && box->mMaxZ > getMinZ() && box->mMinZ < getMaxZ();
}

double WorldBorder::getDistanceToBorder(double x, double z)
{
    double fromMinZ = z - getMinZ();
    double toMaxZ = getMaxZ() - z;
    double fromMinX = x - getMinX();
    double toMaxX = getMaxX() - x;
    double distanceX = fromMinX < toMaxX ? fromMinX : toMaxX;
    double distanceXZ = distanceX < fromMinZ ? distanceX : fromMinZ;
    return distanceXZ < toMaxZ ? distanceXZ : toMaxZ;
}

double WorldBorder::getDistanceToBorder(boost::shared_ptr<Entity> entity)
{
    return getDistanceToBorder(entity->mX, entity->mZ);
}

double WorldBorder::getCenterX() { return m_centerX; }
double WorldBorder::getCenterZ() { return m_centerZ; }

void WorldBorder::setCenter(double x, double z)
{
    m_centerX = x; m_centerZ = z;
    std::vector<BorderChangeListener*>& listeners = getListeners();
    for (std::vector<BorderChangeListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it)
        (*it)->onBorderCenterSet(this, x, z);
}

long long WorldBorder::getLerpRemainingTime() { return getStatus() == STATIONARY ? 0 : m_lerpEnd - static_cast<long long>(System::processTimeInMilliSecs()); }
double WorldBorder::getLerpTarget() { return m_lerpTarget; }

void WorldBorder::lerpSizeBetween(double oldSize, double newSize, long long timeMs)
{
    m_size = oldSize; m_lerpTarget = newSize; m_lerpStart = System::processTimeInMilliSecs(); m_lerpEnd = m_lerpStart + timeMs;
    std::vector<BorderChangeListener*>& listeners = getListeners();
    for (std::vector<BorderChangeListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it)
        (*it)->onBorderSizeLerping(this, oldSize, newSize, timeMs);
}

void WorldBorder::addListener(BorderChangeListener* listener) { m_listeners.push_back(listener); }
void WorldBorder::setAbsoluteMaxSize(int size) { m_absoluteMaxSize = size; }
int WorldBorder::getAbsoluteMaxSize() { return m_absoluteMaxSize; }
double WorldBorder::getDamageSafeZone() { return m_damageSafeZone; }
void WorldBorder::setDamageSafeZone(double distance)
{
    m_damageSafeZone = distance;
    std::vector<BorderChangeListener*>& listeners = getListeners();
    for (std::vector<BorderChangeListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it)
        (*it)->onBorderSetDamageSafeZOne(this, distance);
}
double WorldBorder::getDamagePerBlock() { return m_damagePerBlock; }
void WorldBorder::setDamagePerBlock(double damage)
{
    m_damagePerBlock = damage;
    std::vector<BorderChangeListener*>& listeners = getListeners();
    for (std::vector<BorderChangeListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it)
        (*it)->onBorderSetDamagePerBlock(this, damage);
}
int WorldBorder::getWarningTime() { return m_warningTime; }
void WorldBorder::setWarningTime(int time)
{
    m_warningTime = time;
    std::vector<BorderChangeListener*>& listeners = getListeners();
    for (std::vector<BorderChangeListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it)
        (*it)->onBorderSetWarningTime(this, time);
}
int WorldBorder::getWarningBlocks() { return m_warningBlocks; }
void WorldBorder::setWarningBlocks(int blocks)
{
    m_warningBlocks = blocks;
    std::vector<BorderChangeListener*>& listeners = getListeners();
    for (std::vector<BorderChangeListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it)
        (*it)->onBorderSetWarningBlocks(this, blocks);
}
bool WorldBorder::isWithinBounds(ChunkPos* pos)
{
    return pos != NULL && isWithinBounds(static_cast<double>(pos->getMinBlockX()), static_cast<double>(pos->getMinBlockZ())) &&
           isWithinBounds(static_cast<double>(pos->getMaxBlockX()), static_cast<double>(pos->getMaxBlockZ()));
}
bool WorldBorder::isWithinBounds(double x, double z) { return x > getMinX() && x < getMaxX() && z > getMinZ() && z < getMaxZ(); }
void WorldBorder::removeListener(BorderChangeListener* listener) { m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end()); }
long long WorldBorder::getLerpSpeed()
{
    long long remaining = getLerpRemainingTime();
    if (remaining == 0) return 0;
    double delta = m_lerpTarget - m_size;
    return static_cast<long long>(delta / static_cast<double>(remaining));
}
