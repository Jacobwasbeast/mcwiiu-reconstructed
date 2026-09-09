#ifndef PLATFORM_WIIU_CCOMMERCENINTENDO_HPP
#define PLATFORM_WIIU_CCOMMERCENINTENDO_HPP

#include "game/core/cCommerce.hpp"

class cCommerceNintendo : public cCommerce
{
public:
    struct CatalogRecord
    {
        unsigned int catalogIndex;
        char name[128];
        char price[32];
        char description[2000];
        unsigned int contentCount;
        int contentIndexes[8];
        bool canPurchase;
        bool installed;
    };

    enum eCommerce_Process_State
    {
        eCommerce_Process_State_Unknown = 0
    };

    enum eCommerce_ShoppingCatalogState
    {
        eCommerce_ShoppingCatalogState_Unknown = 0
    };

    cCommerceNintendo();
    virtual ~cCommerceNintendo();

    void SetLoggedIn(bool loggedIn);
    void SetCommerceInitialised(bool initialised);
    void SetFirstLogIn(bool firstLogIn);
    void SetProcessState(eCommerce_Process_State state);
    void SetShoppingCatalogState(eCommerce_ShoppingCatalogState state);
    void SetItemCount(unsigned int itemCount);
    unsigned char GetLoggedIn();
    unsigned char GetFirstLogIn();
    unsigned char GetCommerceInitialised();
    void SetNetworkConnected(bool connected);
    unsigned char GetNetworkConnected();
    eCommerce_Process_State GetProcessState();
    unsigned int GetItemCount();
    virtual unsigned int LogIn() = 0;
    virtual void GetShoppingCatalog() = 0;
    virtual CatalogRecord* GetCommerceInfo() = 0;
    virtual void tick() = 0;
    virtual unsigned int PurchaseBedrock();
    eCommerce_ShoppingCatalogState GetShoppingCatalogState()
    {
        return m_shoppingCatalogState;
    }

private:
    eCommerce_Process_State m_processState;
    unsigned int m_itemCount;
    eCommerce_ShoppingCatalogState m_shoppingCatalogState;
    bool m_loggedIn;
    bool m_networkConnected;
    bool m_commerceInitialised;
    bool m_firstLogIn;
};

#endif
