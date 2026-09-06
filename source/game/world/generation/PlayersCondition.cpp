#include "game/world/generation/PlayersCondition.hpp"

#include "game/core/GameModeOptions.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/XmlObject.hpp"

PlayersCondition::PlayersCondition(const XmlObject& object)
    : IntegerCondition(object), m_activeOnly(object[L"activeOnly"].asBool()), m_readyOnly(object[L"readyOnly"].asBool())
{
}

PlayersCondition::~PlayersCondition()
{
}

int PlayersCondition::GetValue(MasterGameMode* gameMode) const
{
    return gameMode->GetPlayerCount(m_activeOnly, m_readyOnly);
}

void PlayersCondition::AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const
{
    params->push_back(GetValue(gameMode));
    params->push_back(0x57a4fda9);
}

bool PlayersCondition::IsSatisfied(MasterGameMode* gameMode) const
{
    if (Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0)
        return IntegerCondition::IsSatisfied(gameMode);
    return true;
}
