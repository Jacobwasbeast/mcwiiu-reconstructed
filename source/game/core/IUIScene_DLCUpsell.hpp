#ifndef GAME_CORE_IUISCENE_DLCUPSELL_HPP
#define GAME_CORE_IUISCENE_DLCUPSELL_HPP

#include "platform/wiiu/C4JStorage.hpp"

class IUIScene_DLCUpsell
{
public:
    static uint32_t NotOnlineDialogCallback(void* context, int userIndex,
                                            C4JStorage::EMessageResult result);
    void ShowNotOnlineDialog(int userIndex);
};

#endif
