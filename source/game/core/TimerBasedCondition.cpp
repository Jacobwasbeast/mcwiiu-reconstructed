#include "game/core/TimerBasedCondition.hpp"

#include "game/core/MasterGameMode.hpp"
#include "game/core/XmlObject.hpp"

TimerBasedCondition::TimerBasedCondition(const XmlObject& object)
    : TimeCondition(object)
{
}

TimerBasedCondition::~TimerBasedCondition()
{
}

bool TimerBasedCondition::UpdateMessage(MasterGameMode* gameMode, int message) const
{
    if (!gameMode->HasTimer(static_cast<MasterGameMode::ETimerID>(GetTimerID())))
        GameCondition::UpdateMessage(gameMode, message);
    return true;
}

void TimerBasedCondition::SetUpTimer(MasterGameMode* gameMode, int message) const
{
    if (!CanStartTimer(gameMode))
        return;

    MasterGameMode::CountdownInfo* countdownInfo = new MasterGameMode::CountdownInfo();
    countdownInfo->message = GetMessage();
    AddIntParams(gameMode, &countdownInfo->arguments);
    countdownInfo->arguments.push_back(message);

    if (!gameMode->StartTimer(static_cast<MasterGameMode::ETimerID>(GetTimerID()), countdownInfo,
                              static_cast<int>(GetThreshold(gameMode)) - GetValue(gameMode)))
        delete countdownInfo;
}

unsigned char TimerBasedCondition::GetMessage() const
{
    return 0x86;
}

bool TimerBasedCondition::CanStartTimer(MasterGameMode* gameMode) const
{
    return gameMode->GetState() != 0;
}
