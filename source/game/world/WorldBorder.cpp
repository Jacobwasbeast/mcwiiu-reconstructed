#include "game/world/WorldBorder.hpp"

#include <algorithm>
#include "game/core/System.hpp"
#include "game/network/BorderChangeListener.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/entity/mob/Entity.hpp"

WorldBorder::WorldBorder(Level* level)
    : m_level(level), m_size(5.9999968E7), m_lerpTarget(m_size), m_centerX(0.0), m_centerZ(0.0),
      m_lerpEnd(0), m_lerpStart(0), m_absoluteMaxSize(30000000), m_damageSafeZone(5.0),
      m_damagePerBlock(0.2), m_warningTime(15), m_warningBlocks(5)
{
}

WorldBorder::~WorldBorder() {}

int WorldBorder::getStatus()
{
    if (m_lerpTarget < m_size) return 1;
    if (m_lerpTarget > m_size) return 2;
    return 0;
}

std::vector<BorderChangeListener*>& WorldBorder::getListeners() { return m_listeners; }

void WorldBorder::setSize(double size)
{
    m_size = size;
    m_lerpTarget = size;
    m_lerpStart = 0;
    m_lerpEnd = 0;
    for (std::vector<BorderChangeListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
        if (*it != NULL) (*it)->onBorderSizeSet(this, size);
}

double WorldBorder::getSize()
{
    if (getStatus() == 0) return m_size;
    long long now = System::processTimeInMilliSecs();
    if (now >= m_lerpEnd) { setSize(m_lerpTarget); return m_size; }
    double t = static_cast<double>(now - m_lerpStart) / static_cast<double>(m_lerpEnd - m_lerpStart);
    return m_size + (m_lerpTarget - m_size) * t;
}

double WorldBorder::getMinX() { return std::max(-static_cast<double>(m_absoluteMaxSize), m_centerX - getSize() * 0.5); }
double WorldBorder::getMaxX() { return std::min(static_cast<double>(m_absoluteMaxSize), m_centerX + getSize() * 0.5); }
double WorldBorder::getMinZ() { return std::max(-static_cast<double>(m_absoluteMaxSize), m_centerZ - getSize() * 0.5); }
double WorldBorder::getMaxZ() { return std::min(static_cast<double>(m_absoluteMaxSize), m_centerZ + getSize() * 0.5); }

bool WorldBorder::isWithinBounds(const BlockPos& pos)
{
    return isWithinBounds(static_cast<double>(pos.x) + 0.5, static_cast<double>(pos.z) + 0.5);
}

bool WorldBorder::isWithinBounds(AABB* box)
{
    return box != NULL && box->mMaxX > getMinX() && box->mMinX < getMaxX() && box->mMaxZ > getMinZ() && box->mMinZ < getMaxZ();
}

double WorldBorder::getDistanceToBorder(double x, double z)
{
    return std::min(std::min(x - getMinX(), getMaxX() - x), std::min(z - getMinZ(), getMaxZ() - z));
}

double WorldBorder::getDistanceToBorder(boost::shared_ptr<Entity> entity)
{
    return entity ? getDistanceToBorder(entity->mX, entity->mZ) : 0.0;
}

double WorldBorder::getCenterX() { return m_centerX; }
double WorldBorder::getCenterZ() { return m_centerZ; }

void WorldBorder::setCenter(double x, double z)
{
    m_centerX = x; m_centerZ = z;
    for (std::vector<BorderChangeListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
        if (*it != NULL) (*it)->onBorderCenterSet(this, x, z);
}

long long WorldBorder::getLerpRemainingTime() { return getStatus() == 0 ? 0 : m_lerpEnd - static_cast<long long>(System::processTimeInMilliSecs()); }
double WorldBorder::getLerpTarget() { return m_lerpTarget; }

void WorldBorder::lerpSizeBetween(double oldSize, double newSize, long long timeMs)
{
    m_size = oldSize; m_lerpTarget = newSize; m_lerpStart = System::processTimeInMilliSecs(); m_lerpEnd = m_lerpStart + timeMs;
    for (std::vector<BorderChangeListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
        if (*it != NULL) (*it)->onBorderSizeLerping(this, oldSize, newSize, timeMs);
}

void WorldBorder::addListener(BorderChangeListener* listener) { if (listener != NULL) m_listeners.push_back(listener); }
void WorldBorder::setAbsoluteMaxSize(int size) { m_absoluteMaxSize = size; }
int WorldBorder::getAbsoluteMaxSize() { return m_absoluteMaxSize; }
double WorldBorder::getDamageSafeZone() { return m_damageSafeZone; }
void WorldBorder::setDamageSafeZone(double distance)
{
    m_damageSafeZone = distance;
    for (std::vector<BorderChangeListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
        (*it)->onBorderSetDamageSafeZOne(this, distance);
}
double WorldBorder::getDamagePerBlock() { return m_damagePerBlock; }
void WorldBorder::setDamagePerBlock(double damage)
{
    m_damagePerBlock = damage;
    for (std::vector<BorderChangeListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
        (*it)->onBorderSetDamagePerBlock(this, damage);
}
int WorldBorder::getWarningTime() { return m_warningTime; }
void WorldBorder::setWarningTime(int time)
{
    m_warningTime = time;
    for (std::vector<BorderChangeListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
        (*it)->onBorderSetWarningTime(this, time);
}
int WorldBorder::getWarningBlocks() { return m_warningBlocks; }
void WorldBorder::setWarningBlocks(int blocks)
{
    m_warningBlocks = blocks;
    for (std::vector<BorderChangeListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
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
