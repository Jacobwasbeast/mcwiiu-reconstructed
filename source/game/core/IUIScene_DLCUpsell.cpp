#include "game/core/IUIScene_DLCUpsell.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/network/CGameNetworkManager.hpp"

extern ConsoleUIController gConsoleUIController;

uint32_t IUIScene_DLCUpsell::NotOnlineDialogCallback(
    void*, int userIndex, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_4)
        CGameNetworkManager::sInstance.ShowConnectOnlineNintendoProgress(userIndex, NULL, NULL);
    return 0;
}

void IUIScene_DLCUpsell::ShowNotOnlineDialog(int userIndex)
{
    int messageParameters[2] = { 0x051d141b, 0x3f0766c7 };
    gConsoleUIController.RequestErrorMessage(0x8a21894b, 0xb837024e, messageParameters, 2,
                                             userIndex, NotOnlineDialogCallback, this, NULL, true);
}
