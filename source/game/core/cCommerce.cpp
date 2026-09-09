#include "game/core/cCommerce.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"

#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/cProductInfo.hpp"

unsigned char g_bPurchasabilityUpdated = 0;
extern unsigned char g_bDLCCatalogListRetrieved;
extern unsigned char g_bDLCCatalogListInfoRetrieved;
extern cCommerce::eDLCCatalogState g_eDLCCatalogState;

unsigned char g_bDLCCatalogListRetrieved = 0;
unsigned char g_bDLCCatalogListInfoRetrieved = 0;
cCommerce::eDLCCatalogState g_eDLCCatalogState = cCommerce::eDLCCatalogState_Unknown;
cProductInfo* g_pDLCProductInfo = NULL;

cCommerce::cCommerce()
{
}

cCommerce::~cCommerce()
{
}

void cCommerce::ClearProductInfo()
{
    g_bPurchasabilityUpdated = false;
    if (g_pDLCProductInfo != NULL)
    {
        delete[] g_pDLCProductInfo;
    }
    g_pDLCProductInfo = NULL;
}

unsigned int cCommerce::PurchaseDLCByIndex(unsigned int index, int contentType)
{
    return 0;
}

bool cCommerce::OwnsBundleContainingPack(unsigned int packId)
{
    DLCPack* pack = NULL;

    if (packId < 0x409)
    {
        if (packId < 0x21e)
        {
            if (packId != 0x206)
            {
                if (packId == 0x214 || packId == 0x219)
                {
                    pack = gApp.mDLCManager.getPackFromPackID(0x1197);
                }
                else if (packId == 0x21a)
                {
                    pack = gApp.mDLCManager.getPackFromPackID(0x1195);
                }
                else
                {
                    return false;
                }
            }
            else
            {
                pack = gApp.mDLCManager.getPackFromPackID(0x1196);
            }
        }
        else if (packId == 0x21e)
        {
            pack = gApp.mDLCManager.getPackFromPackID(0x1194);
        }
        else if (packId == 0x22b)
        {
            pack = gApp.mDLCManager.getPackFromPackID(0x1010);
        }
        else if (packId == 0x405)
        {
            pack = gApp.mDLCManager.getPackFromPackID(0x1196);
        }
        else if (packId == 0x407)
        {
            pack = gApp.mDLCManager.getPackFromPackID(0x1195);
        }
        else
        {
            return false;
        }
    }
    else if (packId < 0x1002)
    {
        if (packId == 0x409)
        {
            pack = gApp.mDLCManager.getPackFromPackID(0x1194);
        }
        else if (packId >= 0x801 && (packId == 0x801 || packId == 0x802 || packId == 0x807 || packId == 0x809))
        {
            pack = gApp.mDLCManager.getPackFromPackID(0x1195);
        }
        else
        {
            return false;
        }
    }
    else if (packId < 0x1006)
    {
        pack = gApp.mDLCManager.getPackFromPackID(0x1006);
    }
    else if (packId == 0x100b)
    {
        pack = gApp.mDLCManager.getPackFromPackID(0x1194);
    }
    else if (packId == 0x100c)
    {
        pack = gApp.mDLCManager.getPackFromPackID(0x1196);
    }
    else if (packId >= 0x1011 && packId <= 0x1013)
    {
        pack = gApp.mDLCManager.getPackFromPackID(0x1010);
    }
    else
    {
        return false;
    }

    return pack != NULL && pack->hasPurchasedPack();
}

unsigned int cCommerce::PurchaseDLCByPackID(unsigned int packId)
{
    return 0;
}

unsigned int cCommerce::HasPurchased(unsigned int index, bool* purchased, bool* available)
{
    if (g_pDLCProductInfo != NULL)
    {
        *available = true;
        *purchased = !g_pDLCProductInfo[index].CanPurchase();
        return 1;
    }
    *purchased = false;
    *available = false;
    return 0;
}

unsigned int cCommerce::GetDLCProductInfo(cProductInfo** productInfo, unsigned int* count, unsigned int index, bool includeUnavailable)
{
    return 0;
}

unsigned int cCommerce::CheckForEmptyStore(int state)
{
    return 0;
}

unsigned int cCommerce::ProductListAvailable()
{
    return 0;
}

void cCommerce::GetProductTextureName(std::wstring& textureName, char* productName)
{
}

void cCommerce::ShowStoreIcon()
{
}

void cCommerce::HideStoreIcon()
{
}

unsigned char cCommerce::GetPurchasabilityUpdated()
{
    return g_bPurchasabilityUpdated;
}

void cCommerce::SetPurchasabilityUpdated(bool updated)
{
    g_bPurchasabilityUpdated = updated;
}

unsigned char cCommerce::GetDLCCatalogListRetrieved()
{
    return g_bDLCCatalogListRetrieved;
}

unsigned char cCommerce::GetDLCCatalogListInfoRetrieved()
{
    return g_bDLCCatalogListInfoRetrieved;
}

unsigned int cCommerce::GetDLCCatalogState()
{
    return static_cast<unsigned int>(g_eDLCCatalogState);
}

void cCommerce::SetDLCCatalogListRetrieved(bool retrieved)
{
    g_bDLCCatalogListRetrieved = retrieved;
    if (!retrieved)
    {
        return;
    }
    this->SetDLCCatalogState(static_cast<eDLCCatalogState>(1));
}

void cCommerce::SetDLCCatalogListInfoRetrieved(bool retrieved)
{
    g_bDLCCatalogListInfoRetrieved = retrieved;
}

void cCommerce::SetDLCCatalogState(eDLCCatalogState state)
{
    g_eDLCCatalogState = state;
}

unsigned int cCommerce::AttemptLogIn()
{
    return 1;
}



