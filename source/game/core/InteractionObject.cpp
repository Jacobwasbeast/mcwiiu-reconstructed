#include "game/core/InteractionObject.hpp"

InteractionObject::NameData::NameData(std::wstring name, std::wstring ownerUuid, bool createdOnHost, bool createdByRestrictedPlayer)
    : name(name)
    , ownerUuid(ownerUuid)
    , createdOnHost(createdOnHost)
    , createdByRestrictedPlayer(createdByRestrictedPlayer)
{
}

InteractionObject::NameData::NameData(const NameData& other)
    : name(other.name)
    , ownerUuid(other.ownerUuid)
    , createdOnHost(other.createdOnHost)
    , createdByRestrictedPlayer(other.createdByRestrictedPlayer)
{
}

InteractionObject::InteractionObject()
{
}

InteractionObject::NameData InteractionObject::GetNetworkNameData()
{
    return NameData(getDisplayName(), L"", true, false);
}
