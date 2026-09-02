#include "game/core/StatsUID.hpp"

#include "game/network/INetworkPlayer.hpp"

StatsUID::StatsUID(const PlayerUID& uid)
    : PlayerUID(0)
{
    PlayerUID::operator=(uid);
}

StatsUID StatsUID::EMPTY_SUID()
{
    return StatsUID(PlayerUID(0));
}

StatsUID StatsUID::NETWORKPLAYER_SUID(INetworkPlayer* networkPlayer)
{
    if (networkPlayer == NULL)
    {
        return EMPTY_SUID();
    }

    return StatsUID(networkPlayer->GetUID());
}

PlayerUID StatsUID::GetID() const
{
    return *this;
}

bool StatsUID::operator==(const StatsUID& other) const
{
    return PlayerUID::operator==(other);
}

bool StatsUID::operator!=(const StatsUID& other) const
{
    return PlayerUID::operator!=(other);
}
