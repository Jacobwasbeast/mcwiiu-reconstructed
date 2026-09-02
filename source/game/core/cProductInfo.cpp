#include "game/core/cProductInfo.hpp"

#include <string.h>

cProductInfo::cProductInfo()
{
    m_pName = NULL;
    m_pProductIdString = NULL;
    m_pDescription = NULL;
    m_pPriceString = NULL;
    m_pSKUId = NULL;
    m_SortIndex = 0;
    m_bCanDownload = false;
    m_pBanner = NULL;
    m_PackID = 0;
    m_bCanPurchase = false;
}

cProductInfo::~cProductInfo()
{
    delete m_pDescription;
    delete m_pName;
    delete m_pPriceString;
    delete m_pProductIdString;
    delete m_pSKUId;
    delete m_pBanner;
}

void cProductInfo::Init(bool canPurchase, bool canDownload, unsigned int price, char* pName, char* pDescription, char* pPriceString, char* pProductIdString, char* pSKUId, char* pBanner, unsigned int packID, unsigned int sortIndex)
{
    m_bCanDownload = canDownload;
    m_bCanPurchase = canPurchase;

    unsigned int nameLength = strlen(pName);
    m_pName = new char[nameLength + 1]();
    if (nameLength != 0)
    {
        memcpy(m_pName, pName, nameLength);
    }

    m_Price = price;

    unsigned int priceStringLength = strlen(pPriceString);
    m_pPriceString = new char[priceStringLength + 1]();
    if (priceStringLength != 0)
    {
        memcpy(m_pPriceString, pPriceString, priceStringLength);
    }

    unsigned int productIdStringLength = strlen(pProductIdString);
    unsigned int productIdPrefixLength = 0;
    if (productIdStringLength == 36)
    {
        productIdPrefixLength = 20;
    }
    unsigned int productIdLength = productIdStringLength - productIdPrefixLength;
    m_pProductIdString = new char[productIdLength + 1]();
    if (productIdStringLength != 0)
    {
        memcpy(m_pProductIdString, pProductIdString + productIdPrefixLength, productIdLength);
    }

    unsigned int skuIdLength = strlen(pSKUId);
    m_pSKUId = new char[skuIdLength + 1]();
    if (skuIdLength != 0)
    {
        memcpy(m_pSKUId, pSKUId, skuIdLength);
    }

    m_pDescription = new char[4001]();
    memcpy(m_pDescription, pDescription, 4000);

    m_pBanner = new char[17]();
    memcpy(m_pBanner, pBanner, 16);

    m_PackID = packID;
    m_SortIndex = sortIndex;
}

bool cProductInfo::CanPurchase()
{
    return m_bCanPurchase;
}

unsigned int cProductInfo::GetSortIndex()
{
    return m_SortIndex;
}

char* cProductInfo::GetName()
{
    return m_pName;
}

char* cProductInfo::GetDescription()
{
    return m_pDescription;
}

char* cProductInfo::GetPriceString()
{
    return m_pPriceString;
}

char* cProductInfo::GetBanner()
{
    return m_pBanner;
}

unsigned int cProductInfo::GetPackID()
{
    return m_PackID;
}
