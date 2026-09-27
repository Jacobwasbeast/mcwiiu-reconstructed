#include "game/core/StatsUID.hpp"

#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/PlayerUID.hpp"

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

std::wstring StatsUID::toString() const
{
    return PlayerUID::toString();
}

bool StatsUID::operator==(const StatsUID& other) const
{
    return PlayerUID::operator==(other);
}

bool StatsUID::operator!=(const StatsUID& other) const
{
    return PlayerUID::operator!=(other);
}

unsigned int hash_value(const StatsUID& uid)
{
    return hash_value(uid.GetID());
}
