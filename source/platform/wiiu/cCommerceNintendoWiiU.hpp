#ifndef PLATFORM_WIIU_CCOMMERCENINTENDOWIIU_HPP
#define PLATFORM_WIIU_CCOMMERCENINTENDOWIIU_HPP

#include <stdint.h>

#include "platform/wiiu/cCommerceNintendo.hpp"

#include <nn/result.h>
#include <nn/ac.h>

namespace nn
{
    namespace ec
    {
        class Money
        {
        public:
            Money(const char*, const char*, const char*);
            const char* GetAmount() const
                asm("GetAmount__Q3_2nn2ec5MoneyCFv");
        private:
            unsigned char m_data[64];
        };

        class Item;
        class Query;
        enum SalesStatusFilter
        {
            SalesStatusFilter_0 = 0
        };

        class ItemList
        {
        public:
            Item* GetItem(uint32_t)
                asm("GetItem__Q3_2nn2ec8ItemListFUi");
        };

        class ShoppingCatalog : public ItemList
        {
        public:
            ShoppingCatalog();
            uint32_t GetTotal() const
                asm("GetTotal__Q3_2nn2ec15ShoppingCatalogCFv");
            uint32_t RetrieveAocs(const Query*, uint32_t, uint32_t)
                asm("RetrieveAocs__Q3_2nn2ec15ShoppingCatalogFPCQ3_2nn2ec5QueryUiT2");
        private:
            unsigned char m_data[8];
        };

        class Query
        {
        public:
            Query();
            ~Query();
            uint32_t SetSalesStatusFilter(nn::ec::SalesStatusFilter)
                asm("SetSalesStatusFilter__Q3_2nn2ec5QueryFQ3_2nn2ec17SalesStatusFilter");
        };

        class DateTime
        {
        public:
            DateTime()
                asm("__ct__Q3_2nn2ec8DateTimeFv");
            long long operator-(const DateTime&) const
                asm("__mi__Q3_2nn2ec8DateTimeCFRCQ3_2nn2ec8DateTime");
        };

        uint32_t SynchronizeAocRights()
            asm("SynchronizeAocRights__Q2_2nn2ecFv");
        uint32_t Initialize(uint32_t)
            asm("Initialize__Q2_2nn2ecFUi");
        uint32_t Login()
            asm("Login__Q2_2nn2ecFv");
        void GetBalance(Money*)
            asm("GetBalance__Q2_2nn2ecFPQ3_2nn2ec5Money");
        void GetServerTime(DateTime*)
            asm("GetServerTime__Q2_2nn2ecFPQ3_2nn2ec8DateTime");
        uint32_t GetErrorCode(const nn::Result&)
            asm("GetErrorCode__Q2_2nn2ecFRCQ2_2nn6Result");
        void GetLastServerErrorMessage(wchar_t*, wchar_t*)
            asm("GetLastServerErrorMessage__Q2_2nn2ecFPwT1");
        namespace applet
        {
            uint32_t Purchase(const Item*, Money*)
                asm("Purchase__Q3_2nn2ec6appletFPCQ3_2nn2ec4ItemPQ3_2nn2ec5Money");
            uint32_t Download(const Item*)
                asm("Download__Q3_2nn2ec6appletFPCQ3_2nn2ec4Item");
        }
    }

}

struct DLC_INFO;

class cCommerceNintendoWiiU : public cCommerceNintendo
{
public:
    cCommerceNintendoWiiU();
    virtual ~cCommerceNintendoWiiU();
    void GetBalance();
    virtual unsigned int LogIn();
    void Init();
    void LoggedIn();
    unsigned int Purchase(unsigned int index);
    unsigned int ReDownload(unsigned int index);
    static int GetShoppingCatalog_threadEntry(int argumentCount, void* argument);
    void LaunchThread_GetShoppingCatalog();
    void ClearShoppingCatalogRetrieved();
    virtual void tick();
    unsigned int PurchaseDLC(DLC_INFO* info);
    virtual unsigned int PurchaseDLCByIndex(unsigned int index, int controllerId);
    virtual unsigned int PurchaseDLCByPackID(unsigned int packId);
    static int OrderSortFunction(const void* first, const void* second);
    virtual unsigned int GetDLCProductInfo(cProductInfo** productInfo, unsigned int* count,
                                           unsigned int category, bool includeUnavailable);
    virtual void SetDLCCatalogState(cCommerce::eDLCCatalogState state);
    virtual unsigned int AttemptLogIn();
    virtual void GetShoppingCatalog();
    virtual CatalogRecord* GetCommerceInfo();
    void SetErrorOccurred(bool errorOccurred);
    bool GetErrorOccurred();
    void AsserteCommerceError(nn::Result& result);
    void Finalize();
    const wchar_t* GeteCommerceErrorString(nn::Result& result);

private:
    unsigned char m_prefixData[32];
    CatalogRecord* m_catalogData;
    unsigned char m_catalogStateData[0x684];
    nn::ec::Money m_balance;
    nn::ec::ShoppingCatalog m_shoppingCatalog;
    uint32_t m_errorCode;
    bool m_errorOccurred;
    unsigned char m_tailData[7];
};

#endif
