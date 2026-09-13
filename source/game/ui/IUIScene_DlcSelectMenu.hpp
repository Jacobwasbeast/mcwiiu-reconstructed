#ifndef GAME_UI_IUISCENE_DLCSELECTMENU_HPP
#define GAME_UI_IUISCENE_DLCSELECTMENU_HPP

#include <stdint.h>

#include "platform/wiiu/C4JStorage.hpp"

class DLCPack;
class UIScene_SkinSelectMenu;

enum eDLCContentType
{
    eDLCContentType_Unknown = 0
};

class IUIScene_DlcSelectMenu
{
public:
    friend class UIScene_SkinSelectMenu;

    IUIScene_DlcSelectMenu(eDLCContentType contentType);

    void tick();
    static int MustSignInReturned(void* context, int userIndex,
                                  C4JStorage::EMessageResult result);
    void showNotOnlineDialog(int userIndex);
    static int UnlockPackReturned(void* context, int userIndex,
                                  C4JStorage::EMessageResult result);
    void tryPurchaseDlc(int userIndex, bool& handled, eDLCContentType contentType);

private:
    bool m_purchasePending;
    DLCPack* m_pack;
    eDLCContentType m_contentType;
};

#endif
