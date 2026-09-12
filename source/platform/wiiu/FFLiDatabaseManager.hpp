#ifndef SOURCE_PLATFORM_WIIU_FFLIDATABASEMANAGER_HPP
#define SOURCE_PLATFORM_WIIU_FFLIDATABASEMANAGER_HPP

#include "game/client/render/Renderer.hpp"

#define private public
#include "platform/wiiu/FFLiDatabaseFileAccessor.hpp"
#undef private
#include "platform/wiiu/FFLiDatabaseDefault.hpp"
#include "platform/wiiu/FFLiDatabaseRandom.hpp"

class FFLiAllocator;
class FFLiFsClient;
class FFLiFileWriteBuffer;
class FFLiSystemContext;
struct FFLiStoreDataCFL;
class FFLiMiddleDB;

class FFLiDatabaseManager
{
private:
    FFLiSystemContext *m_systemContext;
    FFLiDatabaseFileAccessor m_fileAccessor;
    FFLiDatabaseDefault m_defaultDatabase;
    FFLiDatabaseRandom m_randomDatabase;
    unsigned char m_enabledSpecialMii;
    unsigned char m_alignment[3];
    FFLiAllocator *m_allocator;

public:
    FFLiDatabaseManager(FFLiDatabaseFile *database, FFLiFileWriteBuffer *writeBuffer, FFLiSystemContext *systemContext, FFLiFsClient *client, FFLiAllocator *allocator);
    ~FFLiDatabaseManager();
    unsigned int AfterConstruct();
    int BeforeDestruct();
    unsigned int FlushQuota(bool force);
    void EnableSpecialMii(unsigned int enabled);
    bool IsEnabledSpecialMii() const;
    unsigned int GetCharInfoFromOfficial(FFLiCharInfo *charInfo, unsigned short index);
    unsigned int GetCharInfoFromDefault(FFLiCharInfo *charInfo, unsigned short index);
    int GetCharInfoFromStoreData(FFLiCharInfo *charInfo, FFLDataSource source, const FFLiStoreDataCFL *storeData, unsigned short index);
    int PickupCharInfo(FFLiCharInfo *charInfo, FFLDataSource source, const void *data, unsigned short index);
    int GetStoreData(FFLiStoreDataCFL *storeData, FFLDataSource source, unsigned short index);
    int SetStoreDataToOfficialForBarista(const FFLiStoreDataCFL *storeData);
    int SetStoreDataToHiddenForBarista(const FFLiStoreDataCFL *storeData, bool temporary, bool force);
    int UpdateMiddleDB(FFLiMiddleDB *middleDatabase);
};

#endif
