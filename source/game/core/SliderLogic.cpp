#include "game/core/SliderLogic.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/UIControl_MultiList.hpp"
#include "game/ui/UIString.hpp"

SliderLogic::SliderLogic()
    : mId(0), mLabel(0), mIncludeRandom(false), mPadding(), mOption(eCustomGameModeHostOption_NoSmallSkins), mMiniGameId(eMiniGameId_Lobby)
{
}

SliderLogic::SliderLogic(int id, int label, eCustomGameModeHostOption option, EMiniGameId miniGameId, bool includeRandom)
    : mId(id), mLabel(label), mIncludeRandom(includeRandom), mPadding(), mOption(option), mMiniGameId(miniGameId)
{
}

void SliderLogic::AddTo(UIControl_MultiList& list, int value) const
{
    list.AddNewSlider(GetLabel(value), mId, MasterGameMode::GetOptionMin(mMiniGameId, mOption, mIncludeRandom), MasterGameMode::GetOptionMax(mMiniGameId, mOption), value, false, 7, 3);
}

void SliderLogic::Update(UIControl_MultiList& list, int value) const
{
    list.SetItemLabel(mId, UIString(GetLabel(value)), false);
}

std::wstring SliderLogic::GetLabel(int value) const
{
    std::wstring label(L"Game");
    std::wstring valueText = MasterGameMode::GetValueText(mMiniGameId, mOption, value);
    if (mLabel != -1 && mLabel != -2)
        label = std::wstring(L10N::GetString(mLabel)) + L": ";
    label += valueText;
    return label;
}
