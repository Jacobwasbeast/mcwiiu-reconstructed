#include "platform/wiiu/FFLiDatabaseManager.hpp"
#include "platform/wiiu/FFLiMiddleDB.hpp"
#include "platform/wiiu/FFLiStoreDataCFL.hpp"
#include "platform/wiiu/FFLiSystemContext.hpp"
#include "vendor/__N_23_FFLiDatabaseManager_cpp_875c9ca8.hpp"

#include <string.h>

int FFLiStoreDataCFLToCharInfo(FFLiCharInfo *charInfo, const FFLiStoreDataCFL &storeData);
bool FFLiIsNullMiiID(const FFLCreateID *createId);
bool FFLiiVerifyCharInfo(const FFLiCharInfo *charInfo, int verifyName);

FFLiDatabaseManager::FFLiDatabaseManager(FFLiDatabaseFile *database, FFLiFileWriteBuffer *writeBuffer, FFLiSystemContext *systemContext, FFLiFsClient *client, FFLiAllocator *allocator)
    : m_systemContext(systemContext),
      m_fileAccessor(client, database, writeBuffer, allocator),
      m_defaultDatabase(),
      m_randomDatabase(systemContext),
      m_enabledSpecialMii(0),
      m_allocator(allocator)
{
    m_fileAccessor.Init();
}

FFLiDatabaseManager::~FFLiDatabaseManager()
{
}

unsigned int FFLiDatabaseManager::AfterConstruct()
{
    unsigned long long titleId = m_systemContext->TitleID();
    unsigned int result = m_fileAccessor.AfterConstruct(titleId);
    if (result == 0)
        m_fileAccessor.BootLoad();
    return result;
}

int FFLiDatabaseManager::BeforeDestruct()
{
    return m_fileAccessor.BeforeDestruct();
}

unsigned int FFLiDatabaseManager::FlushQuota(bool force)
{
    unsigned int result = m_fileAccessor.BeforeFlushQuota();
    if (result == 0)
        result = m_fileAccessor.FlushQuota(force);
    return result;
}

bool FFLiDatabaseManager::IsEnabledSpecialMii() const
{
    return m_enabledSpecialMii != 0;
}

unsigned int FFLiDatabaseManager::GetCharInfoFromOfficial(FFLiCharInfo *charInfo, unsigned short index)
{
    return m_fileAccessor.m_database->Get(charInfo, index, true, IsEnabledSpecialMii()) ? 0u : 3u;
}

unsigned int FFLiDatabaseManager::GetCharInfoFromDefault(FFLiCharInfo *charInfo, unsigned short index)
{
    return m_defaultDatabase.Get(charInfo, index) ? 0u : 3u;
}

int FFLiDatabaseManager::GetCharInfoFromStoreData(FFLiCharInfo *charInfo, FFLDataSource source, const FFLiStoreDataCFL *storeData, unsigned short index)
{
    int result = FFLiStoreDataCFLToCharInfo(charInfo, storeData[index]);
    unsigned short officialIndex = 0;
    if (result == 0 && source == FFL_DATA_SOURCE_STORE_DATA_OFFICIAL &&
        m_fileAccessor.m_database->Search(&officialIndex, *(const FFLCreateID *)(charInfo->data + 0x108)))
        result = GetCharInfoFromOfficial(charInfo, officialIndex);
    return result;
}

int FFLiDatabaseManager::PickupCharInfo(FFLiCharInfo *charInfo, FFLDataSource source, const void *data, unsigned short index)
{
    int result = 1;
    unsigned int magic = 0;
    if (data != 0)
        memcpy(&magic, data, sizeof(magic));

    if (source == FFL_DATA_SOURCE_OFFICIAL) {
        result = GetCharInfoFromOfficial(charInfo, index);
    } else if (source == FFL_DATA_SOURCE_DEFAULT) {
        result = GetCharInfoFromDefault(charInfo, index);
    } else if (source == FFL_DATA_SOURCE_MIDDLE_DB && magic == 0x46464D41) {
        result = static_cast<const FFLiMiddleDB *>(data)->GetCharInfo(charInfo, index);
    } else if (source == FFL_DATA_SOURCE_STORE_DATA_OFFICIAL || source == FFL_DATA_SOURCE_STORE_DATA) {
        result = GetCharInfoFromStoreData(charInfo, source, static_cast<const FFLiStoreDataCFL *>(data), index);
    } else if (source == FFL_DATA_SOURCE_BUFFER) {
        memcpy(charInfo, data, sizeof(FFLiCharInfo));
        result = FFLiIsNullMiiID((const FFLCreateID *)(charInfo->data + 0x108)) ? 3 : 0;
    }

    __N_23_FFLiDatabaseManager_cpp_875c9ca8::ReplaceNameAndCreatorIfAccord(charInfo);
    if (result != 0)
        return result;
    return FFLiIsNullMiiID((const FFLCreateID *)(charInfo->data + 0x108)) || !FFLiiVerifyCharInfo(charInfo, 1) ? 3 : 0;
}
