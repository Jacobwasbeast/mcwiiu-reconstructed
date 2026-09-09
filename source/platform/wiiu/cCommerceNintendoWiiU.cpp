#include "platform/wiiu/cCommerceNintendoWiiU.hpp"

#include <cstdlib>
#include <cstring>

#include <nn/act.h>
#include <nsysnet/nssl.h>
#include <curl/curl.h>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/C_AOC.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/cDLCInfo.hpp"
#include "game/core/cProductInfo.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

extern cProductInfo* g_pDLCProductInfo;

namespace nn
{
    namespace ec
    {
        extern "C" void Money_import_ctor(Money*, const char*, const char*, const char*)
            asm("__ct__Q3_2nn2ec5MoneyFPCcN21");
        extern "C" void ShoppingCatalog_import_ctor(ShoppingCatalog*)
            asm("__ct__Q3_2nn2ec15ShoppingCatalogFv");
        extern "C" void Query_import_ctor(Query*)
            asm("__ct__Q3_2nn2ec5QueryFv");
        extern "C" void Query_import_dtor(Query*, unsigned int)
            asm("__dt__Q3_2nn2ec5QueryFv");

        Money::Money(const char* currency, const char* amount, const char* unit)
        {
            Money_import_ctor(this, currency, amount, unit);
        }

        ShoppingCatalog::ShoppingCatalog()
        {
            ShoppingCatalog_import_ctor(this);
        }

        Query::Query()
        {
            Query_import_ctor(this);
        }

        Query::~Query()
        {
            Query_import_dtor(this, 2);
        }
    }
}

cCommerceNintendoWiiU::cCommerceNintendoWiiU()
    : m_catalogData(NULL), m_balance(NULL, NULL, NULL), m_errorCode(0), m_errorOccurred(false)
{
}

cCommerceNintendoWiiU::~cCommerceNintendoWiiU()
{
}

void cCommerceNintendoWiiU::GetBalance()
{
    nn::ec::GetBalance(&m_balance);
}

unsigned int cCommerceNintendoWiiU::LogIn()
{
    if (nn::act::IsNetworkAccount())
    {
        uint32_t result = nn::ac::Connect();
        if (result & 0x80000000U)
        {
            m_errorCode = result;
            m_errorOccurred = true;
            return 0;
        }
        result = nn::ec::Login();
        if ((result & 0x80000000U) == 0)
        {
            result = nn::ec::SynchronizeAocRights();
            GetBalance();
            if ((result & 0x80000000U) == 0)
            {
                SetLoggedIn(true);
                SetFirstLogIn(false);
                return 1;
            }
        }
        else if (GetFirstLogIn() == 0)
        {
            m_errorCode = result;
            m_errorOccurred = true;
        }
        else
        {
            SetFirstLogIn(false);
        }
    }
    return 0;
}

void cCommerceNintendoWiiU::Init()
{
    if (GetCommerceInitialised() == 0 && (nn::ac::Connect() & 0x80000000U) == 0 && NSSLInit())
    {
        curl_global_init(3);
        nn::act::Initialize();
        nn::ec::Initialize(0x104);
        SetCommerceInitialised(true);
    }
}

void cCommerceNintendoWiiU::LoggedIn()
{
    GetLoggedIn();
}

unsigned int cCommerceNintendoWiiU::Purchase(unsigned int index)
{
    if (!GetLoggedIn())
    {
        AttemptLogIn();
        if (!GetLoggedIn())
        {
            SetProcessState((cCommerceNintendo::eCommerce_Process_State)4);
            SetShoppingCatalogState((cCommerceNintendo::eCommerce_ShoppingCatalogState)2);
            return 1;
        }
        SetShoppingCatalogState((cCommerceNintendo::eCommerce_ShoppingCatalogState)1);
        GetShoppingCatalog();
    }
    nn::ec::SynchronizeAocRights();
    if (index < m_shoppingCatalog.GetTotal())
    {
        nn::ec::Item* item = m_shoppingCatalog.GetItem(index);
        if ((nn::ec::applet::Purchase(item, NULL) & 0x80000000U) != 0)
        {
            uint32_t result = nn::ec::applet::Download(item);
            if ((result & 0x80000000U) == 0)
                return 1;
            m_errorCode = result;
            m_errorOccurred = true;
        }
    }
    return 1;
}

unsigned int cCommerceNintendoWiiU::ReDownload(unsigned int index)
{
    nn::ec::SynchronizeAocRights();
    nn::ec::Query query;
    query.SetSalesStatusFilter(nn::ec::SalesStatusFilter_0);
    m_shoppingCatalog.RetrieveAocs(&query, 0, 100);
    if (m_shoppingCatalog.GetTotal() != 0)
    {
        nn::ec::Item* item = m_shoppingCatalog.GetItem(index);
        uint32_t result = nn::ec::applet::Download(item);
        if ((result & 0x80000000U) == 0)
            return 1;
        if (result == 0xa1608200U)
            return 0;
        m_errorCode = result;
        m_errorOccurred = true;
    }
    return 1;
}

int cCommerceNintendoWiiU::GetShoppingCatalog_threadEntry(int, void* argument)
{
    cCommerceNintendoWiiU* commerce = (cCommerceNintendoWiiU*)argument;
    if (!commerce->GetLoggedIn())
    {
        commerce->AttemptLogIn();
        if (!commerce->GetLoggedIn())
        {
            commerce->SetProcessState((cCommerceNintendo::eCommerce_Process_State)4);
            commerce->SetShoppingCatalogState((cCommerceNintendo::eCommerce_ShoppingCatalogState)2);
            return 0;
        }
    }
    commerce->SetShoppingCatalogState((cCommerceNintendo::eCommerce_ShoppingCatalogState)1);
    commerce->GetShoppingCatalog();
    return 0;
}

void cCommerceNintendoWiiU::LaunchThread_GetShoppingCatalog()
{
    static OSThread thread;
    if (C4JThreadImpl::OSCreateThread4J(&thread, GetShoppingCatalog_threadEntry, 0, this,
                                        NULL, 0x10000, 0x10, 0xc) != 0)
        OSResumeThread(&thread);
}

void cCommerceNintendoWiiU::ClearShoppingCatalogRetrieved()
{
    SetShoppingCatalogState((cCommerceNintendo::eCommerce_ShoppingCatalogState)0);
    if (m_catalogData != NULL)
    {
        std::free(m_catalogData);
        m_catalogData = NULL;
        cCommerce* commerce = (cCommerce*)gApp.GetCommerceInstance();
        commerce->SetDLCCatalogListRetrieved(false);
        commerce->SetDLCCatalogListInfoRetrieved(false);
    }
    SetItemCount(0);
}

void cCommerceNintendoWiiU::tick()
{
    if (GetNetworkConnected())
    {
        SetNetworkConnected(false);
        ClearShoppingCatalogRetrieved();
        SetProcessState((cCommerceNintendo::eCommerce_Process_State)0);
        SetLoggedIn(false);
    }
    else
    {
        SetNetworkConnected(true);
        if (GetProcessState() == 2)
        {
            SetProcessState((cCommerceNintendo::eCommerce_Process_State)3);
            LaunchThread_GetShoppingCatalog();
        }
        else if (GetProcessState() == 6)
        {
            gConsoleUIController.HandleDLCLicenseChange();
            SetProcessState((cCommerceNintendo::eCommerce_Process_State)7);
        }
        else if (GetProcessState() == 11)
        {
            ClearShoppingCatalogRetrieved();
            nn::ec::SynchronizeAocRights();
            C_AOC::getInstance()->Init();
            LaunchThread_GetShoppingCatalog();
            SetProcessState((cCommerceNintendo::eCommerce_Process_State)5);
        }
        else if (GetCommerceInitialised())
        {
            SetProcessState((cCommerceNintendo::eCommerce_Process_State)2);
        }
    }
    if (m_errorOccurred)
    {
        m_errorOccurred = false;
        nn::Result result;
        NNResult rawResult = {static_cast<int32_t>(m_errorCode)};
        result = rawResult;
        gConsoleUIController.RequestErrorMessage(0, 0, NULL, 0, 0, NULL, NULL, NULL, false);
        nn::ec::GetErrorCode(result);
    }
}

unsigned int cCommerceNintendoWiiU::PurchaseDLC(DLC_INFO* info)
{
    if (info == NULL)
        return 0;
    return Purchase(info->CatalogIndex);
}

unsigned int cCommerceNintendoWiiU::PurchaseDLCByIndex(unsigned int index, int controllerId)
{
    cDLCInfo* dlcInfo = (cDLCInfo*)gApp.GetDLCInfoInstance();
    DLC_INFO* info = dlcInfo->GetDLCInfoByIndex(index);
    unsigned int result = PurchaseDLC(info);
    if (result == 0)
    {
        int parameter = 0xb9a1bf4d;
        gConsoleUIController.RequestErrorMessage(0x9da04a9e, 0xcba57cf3, &parameter, 1,
                                                  controllerId, NULL, NULL, NULL, true);
    }
    return result;
}

unsigned int cCommerceNintendoWiiU::PurchaseDLCByPackID(unsigned int packId)
{
    cDLCInfo* dlcInfo = (cDLCInfo*)gApp.GetDLCInfoInstance();
    return PurchaseDLC(dlcInfo->GetDLCInfoForPackID(packId));
}

int cCommerceNintendoWiiU::OrderSortFunction(const void* first, const void* second)
{
    return ((cProductInfo*)second)->GetSortIndex() - ((cProductInfo*)first)->GetSortIndex();
}

unsigned int cCommerceNintendoWiiU::GetDLCProductInfo(cProductInfo** productInfo, unsigned int* count,
                                                      unsigned int category, bool includeUnavailable)
{
    if (GetShoppingCatalogState() == 0)
    {
        *count = 0;
        *productInfo = NULL;
        return 0;
    }

    (void)includeUnavailable;

    CatalogRecord* catalog = GetCommerceInfo();
    unsigned int itemCount = GetItemCount();
    unsigned int productCount = 0;
    cDLCInfo* dlcInfo = static_cast<cDLCInfo*>((void*)gApp.GetDLCInfoInstance());

    if (itemCount != 0)
    {
        g_pDLCProductInfo = new cProductInfo[itemCount];
        for (unsigned int index = 0; index < itemCount; ++index)
        {
            CatalogRecord& entry = catalog[index];
            DLC_INFO* info = dlcInfo->GetDLCInfoByCatalogIndex(entry.catalogIndex);
            if (info != NULL && (category == 0xb || info->ContentType == static_cast<int>(category)))
            {
                g_pDLCProductInfo[productCount].Init(entry.canPurchase, entry.canPurchase, 0, entry.name,
                                                  entry.description, entry.price, "", "",
                                                  info->BannerName, info->PackID, info->SortIndex);
                ++productCount;
            }
        }
        qsort(g_pDLCProductInfo, productCount, sizeof(cProductInfo), OrderSortFunction);
    }

    SetPurchasabilityUpdated(false);
    *count = productCount;
    *productInfo = g_pDLCProductInfo;
    return 1;
}

void cCommerceNintendoWiiU::SetDLCCatalogState(cCommerce::eDLCCatalogState state)
{
    cCommerce::SetDLCCatalogState(state);
    if (state == (cCommerce::eDLCCatalogState)0)
        SetProcessState((cCommerceNintendo::eCommerce_Process_State)0);
}

unsigned int cCommerceNintendoWiiU::AttemptLogIn()
{
    return LogIn();
}

void cCommerceNintendoWiiU::GetShoppingCatalog()
{
    nn::ec::Query query;
    query.SetSalesStatusFilter(nn::ec::SalesStatusFilter_0);
    uint32_t result = m_shoppingCatalog.RetrieveAocs(&query, 0, 100);
    if (result & 0x80000000U)
    {
        m_errorCode = result;
        m_errorOccurred = true;
        return;
    }
    unsigned int total = m_shoppingCatalog.GetTotal();
    m_catalogData = static_cast<CatalogRecord*>(std::calloc(total * sizeof(CatalogRecord), 1));
    SetProcessState((cCommerceNintendo::eCommerce_Process_State)(GetProcessState() == 5 ? 6 : 7));
    SetShoppingCatalogState((cCommerceNintendo::eCommerce_ShoppingCatalogState)2);
    cCommerce* commerce = (cCommerce*)gApp.GetCommerceInstance();
    commerce->SetDLCCatalogListRetrieved(true);
    commerce->SetDLCCatalogListInfoRetrieved(true);
}

cCommerceNintendo::CatalogRecord* cCommerceNintendoWiiU::GetCommerceInfo()
{
    return m_catalogData;
}
