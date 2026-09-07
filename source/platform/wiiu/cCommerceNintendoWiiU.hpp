#ifndef PLATFORM_WIIU_CCOMMERCENINTENDOWIIU_HPP
#define PLATFORM_WIIU_CCOMMERCENINTENDOWIIU_HPP

#include <stdint.h>

#include "platform/wiiu/cCommerceNintendo.hpp"

#include <nn/result.h>

namespace nn
{
    namespace ec
    {
        class Money
        {
        public:
            Money(const char*, const char*, const char*);
            const char* GetAmount() const;
        private:
            unsigned char m_data[64];
        };

        class ShoppingCatalog
        {
        public:
            ShoppingCatalog();
            uint32_t GetTotal() const;
            void* GetItem(uint32_t);
        private:
            unsigned char m_data[8];
        };

        class Query
        {
        public:
            Query();
            ~Query();
        };

        class DateTime
        {
        public:
            DateTime();
            long long operator-(const DateTime&) const;
        };

        uint32_t Connect();
        uint32_t SynchronizeAocRights();
        uint32_t Initialize(uint32_t);
        void GetBalance(Money*);
        void GetServerTime(DateTime*);
        uint32_t GetErrorCode(const nn::Result&);
        void GetLastServerErrorMessage(wchar_t*, wchar_t*);
        namespace applet
        {
            uint32_t Purchase(const void*, Money*);
            uint32_t Download(const void*);
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
    virtual DLC_INFO* GetCommerceInfo();
    void SetErrorOccurred(bool errorOccurred);
    bool GetErrorOccurred();
    void AsserteCommerceError(nn::Result& result);
    void Finalize();
    const wchar_t* GeteCommerceErrorString(nn::Result& result);

private:
    unsigned char m_prefixData[32];
    void* m_catalogData;
    unsigned char m_catalogStateData[0x684];
    nn::ec::Money m_balance;
    nn::ec::ShoppingCatalog m_shoppingCatalog;
    uint32_t m_errorCode;
    bool m_errorOccurred;
    unsigned char m_tailData[7];
};

#endif
