#ifndef SOURCE_PLATFORM_WIIU_FFLIDATABASEFILEACCESSOR_HPP
#define SOURCE_PLATFORM_WIIU_FFLIDATABASEFILEACCESSOR_HPP

#include <stdint.h>
#include "platform/wiiu/FFLiFsCommand.hpp"
#include "platform/wiiu/FFLiDatabaseFileOfficial.hpp"

class FFLiFsClient;
class FFLiAllocator;
class FFLiFileWriteBuffer;
struct FFLiMiiDataCore;
struct FSCmdBlock;
enum FFLpDatabaseFileType
{
    FFLP_DATABASE_FILE_OFFICIAL = 0,
    FFLP_DATABASE_FILE_HIDDEN = 1,
    FFLP_DATABASE_FILE_BACKUP = 2
};
typedef FFLiDatabaseFileOfficial FFLiDatabaseFile;

class FFLiDatabaseFileAccessor
{
private:
    FFLiFsClient *m_client;
    FFLiFsCommand m_command;
    FFLiDatabaseFileOfficial *m_database;
    FFLiFileWriteBuffer *m_writeBuffer;
    FFLiAllocator *m_allocator;
    unsigned int m_databaseState;
    char m_pathOfficial[256];
    char m_pathBackup[256];
    char m_pathHidden[256];
    unsigned char m_pathsReady;
    unsigned char m_officialBackupPending;
    unsigned char m_hiddenFlushPending;
    unsigned char m_databaseChanged;
    unsigned char m_accessorState;
    unsigned char m_hiddenSavePending;
    unsigned char m_alignment[2];

public:
    FFLiDatabaseFileAccessor(FFLiFsClient *client, FFLiDatabaseFile *database, FFLiFileWriteBuffer *writeBuffer, FFLiAllocator *allocator);
    ~FFLiDatabaseFileAccessor();
    void Init();
    int AfterConstruct(unsigned long long titleId);
    const char *GetPathHidden() const;
    const char *GetPathOfficial() const;
    int IsExistFile(const char *path);
    const char *GetPathBackup() const;
    int BootLoadImpl(FFLiFsCommand *command);
    int AdjustRegularListOfficial();
    void BootLoad();
    int BeforeDestruct();
    int SaveHidden();
    int BackupOfficial();
    int BeforeFlushQuota();
    int FlushQuota(bool force);
    int SaveOfficial();
    int SaveIfUpdated();
    int FormatOfficial();
    bool SetOfficial(const FFLiCharInfo *charInfo, unsigned short index);
    int DeleteOfficial(unsigned short index);
    int IsUpdatedFileOfficial(int *updated);
    int UpdateOfficialFromFileImpl();
    int UpdateOfficialFromFile();
    int SetRegularOfficial(unsigned short index, bool regular);
    int FormatHidden();
    bool AddHidden(const FFLiMiiDataCore &data, bool temporary);
    int DeleteHidden(const FFLCreateID &createId);
    int DeleteFileOfficial();
    int DeleteFileHidden();
    int ImportFileOfficial(const void *data, unsigned int size);
    int ImportFileHidden(const void *data, unsigned int size);
    int ExportFileOfficial(void *data, unsigned int *size);
    int ExportFileHidden(void *data, unsigned int *size);
    int AfterConstructByAccessor(const FFLiDatabaseFileAccessor *accessor);
    int FormatFileOfficial();
    int FormatFileHidden();
    int DestroyFileOfficial();
    int DestroyFileHidden();
    void ResetAllAuthorID(const FFLiAuthorID &authorId);
    void ResetAuthorID(unsigned int index, const FFLiAuthorID &authorId);
    static const char *GetPath(FFLpDatabaseFileType type);
    static int IsExistFiles(int *result, FSClient *client, FSCmdBlock *command);
};

#endif
