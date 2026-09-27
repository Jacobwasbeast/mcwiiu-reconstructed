#include "game/core/MultiListSliderLogic.hpp"

#include <vector>
#include "game/core/L10N.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/ui/UIControl_MultiList.hpp"
#include "game/ui/UIString.hpp"

MultiListSliderLogic::MultiListSliderLogic()
    : mLabelId(0), mDescriptionId(0), mOption(eCustomGameModeHostOption_NoSmallSkins), mMiniGameId(eMiniGameId_Lobby)
{
}

MultiListSliderLogic::MultiListSliderLogic(int labelId, int descriptionId, EMiniGameId miniGameId, eCustomGameModeHostOption option)
    : mLabelId(labelId), mDescriptionId(descriptionId), mOption(option), mMiniGameId(miniGameId)
{
}

void MultiListSliderLogic::AddTo(UIControl_MultiList& list, int itemIndex) const
{
    std::wstring label = GetLabel(itemIndex);
    const std::vector<int>& options = MasterGameMode::GetOptionsList(mMiniGameId, mOption);
    int maximum = options.empty() ? 0 : static_cast<int>(options.size());
    list.AddNewSlider(label, mLabelId, 0, maximum - 1, itemIndex, false, 7, 3);
}

void MultiListSliderLogic::Update(UIControl_MultiList& list, int itemIndex) const
{
    UIString label(GetLabel(itemIndex));
    list.SetItemLabel(mLabelId, label, false);
}

std::wstring MultiListSliderLogic::GetLabel(int value) const
{
    std::wstring label(L"Description: ");
    std::wstring valueText = MasterGameMode::GetValueText(mMiniGameId, mOption, value);

    if (mDescriptionId != -1 && mDescriptionId != -2)
        label = std::wstring(L10N::GetString(mDescriptionId)) + L": ";

    return label + valueText;
}
