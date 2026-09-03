#include "platform/wiiu/FFLiDatabaseManager.hpp"
#include "platform/wiiu/FFLiMiddleDB.hpp"
#include "platform/wiiu/FFLiSystemContext.hpp"
#include "vendor/__N_23_FFLiDatabaseManager_cpp_875c9ca8.hpp"
#include <string.h>
struct FFLiStoreDataCFL { unsigned char data[96]; };
int FFLiStoreDataCFLToCharInfo(FFLiCharInfo*, const FFLiStoreDataCFL&); void FFLiCharInfoToStoreDataCFL(FFLiStoreDataCFL*, const FFLiCharInfo*); bool FFLiIsNullMiiID(const unsigned char*); int FFLiiVerifyCharInfo(const FFLiCharInfo*, int);
FFLiDatabaseManager::FFLiDatabaseManager(FFLiDatabaseFile* d, FFLiFileWriteBuffer* w, FFLiSystemContext* s, FFLiFsClient* c, FFLiAllocator* a):m_systemContext(s),m_fileAccessor(c,d,w,a),m_defaultDatabase(),m_randomDatabase(s),m_enabledSpecialMii(0),m_allocator(a)
{
    m_fileAccessor.Init();
}
FFLiDatabaseManager::~FFLiDatabaseManager(){}
unsigned int FFLiDatabaseManager::AfterConstruct()
{
    unsigned long long t=m_systemContext->TitleID();
    if(m_fileAccessor.AfterConstruct(t)==0)
        m_fileAccessor.BootLoad();
    return 0;
}
void FFLiDatabaseManager::BeforeDestruct(){m_fileAccessor.BeforeDestruct();}
unsigned int FFLiDatabaseManager::FlushQuota(bool f)
{
    if(m_fileAccessor.BeforeFlushQuota()==0)
        m_fileAccessor.FlushQuota(f);
    return 0;
}
void FFLiDatabaseManager::EnableSpecialMii(unsigned int e){m_enabledSpecialMii=e!=0;}
bool FFLiDatabaseManager::IsEnabledSpecialMii()const{return m_enabledSpecialMii!=0;}
unsigned int FFLiDatabaseManager::GetCharInfoFromOfficial(FFLiCharInfo* c,unsigned short i){return m_fileAccessor.m_database->Get(c,i,true,IsEnabledSpecialMii())?0u:3u;}
unsigned int FFLiDatabaseManager::GetCharInfoFromDefault(FFLiCharInfo* c,unsigned short i){return m_defaultDatabase.Get(c,i)?0u:3u;}
int FFLiDatabaseManager::GetCharInfoFromStoreData(FFLiCharInfo* c,FFLDataSource s,const FFLiStoreDataCFL* d,unsigned short i)
{
    int r=FFLiStoreDataCFLToCharInfo(c,*(d+i));
    unsigned short n=0;
    if(r==0&&s==FFL_DATA_SOURCE_STORE_DATA_OFFICIAL&&m_fileAccessor.m_database->Search(&n,*(const FFLCreateID*)(c->data+0x108)))
        r=GetCharInfoFromOfficial(c,n);
    return r;
}
int FFLiDatabaseManager::PickupCharInfo(FFLiCharInfo* c,FFLDataSource s,const void* d,unsigned short i)
{
    int r=1; unsigned int m=0;
    if(d!=0) memcpy(&m,d,4);
    if(s==FFL_DATA_SOURCE_OFFICIAL) r=GetCharInfoFromOfficial(c,i);
    else if(s==FFL_DATA_SOURCE_DEFAULT) r=GetCharInfoFromDefault(c,i);
    else if(s==FFL_DATA_SOURCE_MIDDLE_DB&&m==0x46464D41) r=static_cast<const FFLiMiddleDB*>(d)->GetCharInfo(c,i)?0:3;
    else if(s==FFL_DATA_SOURCE_STORE_DATA_OFFICIAL||s==FFL_DATA_SOURCE_STORE_DATA) r=GetCharInfoFromStoreData(c,s,static_cast<const FFLiStoreDataCFL*>(d),i);
    else if(s==FFL_DATA_SOURCE_BUFFER){memcpy(c,d,sizeof(FFLiCharInfo));r=FFLiIsNullMiiID(c->data+0x108)?3:0;}
    __N_23_FFLiDatabaseManager_cpp_875c9ca8::ReplaceNameAndCreatorIfAccord(c);
    if(r!=0) return r;
    return(FFLiIsNullMiiID(c->data+0x108)||!FFLiiVerifyCharInfo(c,1))?3:0;
}
int FFLiDatabaseManager::GetStoreData(FFLiStoreDataCFL* d,FFLDataSource s,unsigned short i){FFLiCharInfo c;int r=PickupCharInfo(&c,s,0,i);if(r==0)FFLiCharInfoToStoreDataCFL(d,&c);return r;}
int FFLiDatabaseManager::SetStoreDataToOfficialForBarista(const FFLiStoreDataCFL* d){FFLiCharInfo c;FFLiStoreDataCFLToCharInfo(&c,*d);return m_fileAccessor.SetOfficial(&c,0)?0:3;}
int FFLiDatabaseManager::SetStoreDataToHiddenForBarista(const FFLiStoreDataCFL* d,bool t,bool f){if(d==0)return 3;if(f)return m_fileAccessor.FlushQuota(t)==0?0:3;return 0;}
int FFLiDatabaseManager::UpdateMiddleDB(FFLiMiddleDB* d){m_randomDatabase.UpdateMiddleDB(d);return 0;}
