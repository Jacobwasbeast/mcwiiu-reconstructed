#ifndef SOURCE_PLATFORM_WIIU_FFLIDATABASEMANAGER_HPP
#define SOURCE_PLATFORM_WIIU_FFLIDATABASEMANAGER_HPP
#define private public
#include "platform/wiiu/FFLiDatabaseFileAccessor.hpp"
#undef private
#include "platform/wiiu/FFLiDatabaseRandom.hpp"
enum FFLDataSource { FFL_DATA_SOURCE_OFFICIAL=0, FFL_DATA_SOURCE_DEFAULT=1, FFL_DATA_SOURCE_MIDDLE_DB=2, FFL_DATA_SOURCE_STORE_DATA_OFFICIAL=3, FFL_DATA_SOURCE_STORE_DATA=4, FFL_DATA_SOURCE_BUFFER=5, FFL_DATA_SOURCE_DIRECT_POINTER=6 };
struct FFLiDatabaseDefault { FFLiMiiDataOfficial m_elements[6]; FFLiDatabaseDefault(); ~FFLiDatabaseDefault(); bool Get(FFLiCharInfo*, unsigned int) const; };
class FFLiAllocator; class FFLiFsClient; class FFLiFileWriteBuffer; class FFLiSystemContext; struct FFLiStoreDataCFL; struct FFLiMiddleDB;
class FFLiDatabaseManager { FFLiSystemContext *m_systemContext; FFLiDatabaseFileAccessor m_fileAccessor; FFLiDatabaseDefault m_defaultDatabase; FFLiDatabaseRandom m_randomDatabase; unsigned char m_enabledSpecialMii; unsigned char m_alignment[3]; FFLiAllocator *m_allocator;
public:
 FFLiDatabaseManager(FFLiDatabaseFile*, FFLiFileWriteBuffer*, FFLiSystemContext*, FFLiFsClient*, FFLiAllocator*); ~FFLiDatabaseManager(); unsigned int AfterConstruct(); void BeforeDestruct(); unsigned int FlushQuota(bool); void EnableSpecialMii(unsigned int); bool IsEnabledSpecialMii() const; unsigned int GetCharInfoFromOfficial(FFLiCharInfo*, unsigned short); unsigned int GetCharInfoFromDefault(FFLiCharInfo*, unsigned short); int GetCharInfoFromStoreData(FFLiCharInfo*, FFLDataSource, const FFLiStoreDataCFL*, unsigned short); int PickupCharInfo(FFLiCharInfo*, FFLDataSource, const void*, unsigned short); int GetStoreData(FFLiStoreDataCFL*, FFLDataSource, unsigned short); int SetStoreDataToOfficialForBarista(const FFLiStoreDataCFL*); int SetStoreDataToHiddenForBarista(const FFLiStoreDataCFL*, bool, bool); int UpdateMiddleDB(FFLiMiddleDB*);
};
#endif
