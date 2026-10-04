#include "game/core/BossHealthOverlay.hpp"


#include "game/core/BossEvent.hpp"
#include "game/core/LerpingBossEvent.hpp"
#include "game/network/ClientboundBossEventPacket.hpp"

BossHealthOverlay::BossHealthOverlay(Minecraft* minecraft)
    : GuiComponent()
{
    mMinecraft = minecraft;
}

void BossHealthOverlay::update(boost::shared_ptr<ClientboundBossEventPacket> packet)
{
    if (packet->getOperation() == ClientboundBossEventPacket::ADD)
    {
        mBossEvents[packet->getId()] = new LerpingBossEvent(packet);
    }
    else if (packet->getOperation() == ClientboundBossEventPacket::REMOVE)
    {
        boost::unordered_map<std::wstring, LerpingBossEvent*>::const_iterator iterator =
            mBossEvents.find(packet->getId());
        if (iterator != mBossEvents.end())
        {
            mBossEvents.erase(iterator);
        }
    }
    else
    {
        boost::unordered_map<std::wstring, LerpingBossEvent*>::const_iterator iterator =
            mBossEvents.find(packet->getId());
        if (iterator != mBossEvents.end())
        {
            mBossEvents[packet->getId()]->update(packet);
        }
    }
}

void BossHealthOverlay::reset()
{
    mBossEvents.clear();
}

bool BossHealthOverlay::shouldPlayMusic()
{
    boost::unordered_map<std::wstring, LerpingBossEvent*>::iterator iterator;
    bool result = false;
    for (iterator = mBossEvents.begin(); iterator != mBossEvents.end(); ++iterator)
    {
        if (iterator->second->shouldPlayBossMusic())
        {
            result = true;
            break;
        }
    }
    return result;
}

bool BossHealthOverlay::shouldDarkenScreen()
{
    boost::unordered_map<std::wstring, LerpingBossEvent*>::iterator iterator;
    bool result = false;
    for (iterator = mBossEvents.begin(); iterator != mBossEvents.end(); ++iterator)
    {
        if (iterator->second->shouldDarkenScreen())
        {
            result = true;
            break;
        }
    }
    return result;
}

bool BossHealthOverlay::shouldCreateWorldFog()
{
    boost::unordered_map<std::wstring, LerpingBossEvent*>::iterator iterator;
    bool result = false;
    for (iterator = mBossEvents.begin(); iterator != mBossEvents.end(); ++iterator)
    {
        if (iterator->second->shouldCreateWorldFog())
        {
            result = true;
            break;
        }
    }
    return result;
}
