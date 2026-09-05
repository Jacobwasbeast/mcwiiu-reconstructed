#include "platform/wiiu/cCommerceNintendoWiiU.hpp"

#include <cstdlib>
#include <cstring>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/C_AOC.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/cDLCInfo.hpp"
#include "game/core/cProductInfo.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

bool NSSLInit();
void curl_global_init(unsigned int flags);
void OSResumeThread(OSThread* thread);
unsigned int GetErrorCode(const nn::Result& result);

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
    if (nn::ec::IsNetworkAccount())
    {
        uint32_t result = nn::ec::Connect();
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
    if (GetCommerceInitialised() == 0 && (nn::ec::Connect() & 0x80000000U) == 0 && NSSLInit())
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
        void* item = m_shoppingCatalog.GetItem(index);
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
    query.SetSalesStatusFilter(0);
    m_shoppingCatalog.RetrieveAocs(&query, 0, 100);
    if (m_shoppingCatalog.GetTotal() != 0)
    {
        void* item = m_shoppingCatalog.GetItem(index);
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
        cCommerce* commerce = (cCommerce*)theApp.GetCommerceInstance();
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
        result.value = m_errorCode;
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
    cDLCInfo* dlcInfo = (cDLCInfo*)theApp.GetDLCInfoInstance();
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
    cDLCInfo* dlcInfo = (cDLCInfo*)theApp.GetDLCInfoInstance();
    return PurchaseDLC(dlcInfo->GetDLCInfoForPackID(packId));
}

int cCommerceNintendoWiiU::OrderSortFunction(const void* first, const void* second)
{
    return ((cProductInfo*)second)->GetSortIndex() - ((cProductInfo*)first)->GetSortIndex();
}

unsigned int cCommerceNintendoWiiU::GetDLCProductInfo(cProductInfo** productInfo, unsigned int* count,
                                                      unsigned int category, bool)
{
    if (GetShoppingCatalogState() == 0)
    {
        *count = 0;
        *productInfo = NULL;
        return 0;
    }
    *count = 0;
    *productInfo = NULL;
    SetPurchasabilityUpdated(false);
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
    query.SetSalesStatusFilter(0);
    uint32_t result = m_shoppingCatalog.RetrieveAocs(&query, 0, 100);
    if (result & 0x80000000U)
    {
        m_errorCode = result;
        m_errorOccurred = true;
        return;
    }
    SetProcessState((cCommerceNintendo::eCommerce_Process_State)(GetProcessState() == 5 ? 6 : 7));
    SetShoppingCatalogState((cCommerceNintendo::eCommerce_ShoppingCatalogState)2);
    cCommerce* commerce = (cCommerce*)theApp.GetCommerceInstance();
    commerce->SetDLCCatalogListRetrieved(true);
    commerce->SetDLCCatalogListInfoRetrieved(true);
}

DLC_INFO* cCommerceNintendoWiiU::GetCommerceInfo()
{
    return (DLC_INFO*)m_catalogData;
}
