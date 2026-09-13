#include "platform/wiiu/FFLiDatabaseFileAccessor.hpp"
#include "platform/wiiu/FFLiFsFile.hpp"
#include "platform/wiiu/FFLiAllocator.hpp"
#define PLATFORM_WIIU_FFLIMIIDATAOFFICIAL_HPP
struct FFLiMiiDataCore;
#include "platform/wiiu/FFLiDatabaseFileHidden.hpp"
#include "vendor/__N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a.hpp"
#include <string.h>

class FFLiFileWriteBuffer;

FFLiDatabaseFileAccessor::FFLiDatabaseFileAccessor(FFLiFsClient *client, FFLiDatabaseFile *database, FFLiFileWriteBuffer *writeBuffer, FFLiAllocator *allocator)
    : m_client(client), m_command(client), m_database(database), m_writeBuffer(writeBuffer), m_allocator(allocator), m_databaseState(0), m_pathsReady(0), m_officialBackupPending(0), m_hiddenFlushPending(0), m_databaseChanged(0), m_accessorState(0), m_hiddenSavePending(0)
{
    memset(m_pathOfficial, 0, sizeof(m_pathOfficial));
    memset(m_pathBackup, 0, sizeof(m_pathBackup));
    memset(m_pathHidden, 0, sizeof(m_pathHidden));
}

FFLiDatabaseFileAccessor::~FFLiDatabaseFileAccessor()
{
}

void FFLiDatabaseFileAccessor::Init()
{
    m_database->Init();
    static_cast<FFLiDatabaseFileHidden *>(static_cast<void *>(static_cast<unsigned char *>(static_cast<void *>(m_database)) + 0x43840))->Init();
}

int FFLiDatabaseFileAccessor::AfterConstruct(unsigned long long titleId)
{
    int result = __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::GetDatabasePath(m_pathOfficial, 0x100, titleId, "db/FFL_ODB.dat");
    if (result == 0)
        result = __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::GetDatabasePath(m_pathBackup, 0x100, titleId, "db/FFL_ODB_OLD.dat");
    if (result == 0)
        result = __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::GetDatabasePath(m_pathHidden, 0x100, titleId, "db/FFL_HDB.dat");
    if (result == 0) {
        m_pathsReady = 1;
    }
    return result;
}

const char *FFLiDatabaseFileAccessor::GetPathHidden() const { return m_pathHidden; }
const char *FFLiDatabaseFileAccessor::GetPathOfficial() const { return m_pathOfficial; }
const char *FFLiDatabaseFileAccessor::GetPathBackup() const { return m_pathBackup; }

int FFLiDatabaseFileAccessor::IsExistFile(const char *path)
{
    FFLiFsFile file(&m_command);
    if (file.Open(path, "r", 8) == 0 && file.Close(0) == 0)
        return 1;
    return 0;
}

int FFLiDatabaseFileAccessor::BootLoadImpl(FFLiFsCommand *command)
{
    FFLiDatabaseFileHidden *hidden = static_cast<FFLiDatabaseFileHidden *>(static_cast<void *>(static_cast<unsigned char *>(static_cast<void *>(m_database)) + 0x43840));
    int result = __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::LoadDatabaseHidden(hidden, command, GetPathHidden());
    if (result == 0) {
        if (!hidden->IsValid()) {
            hidden->Init();
            m_databaseChanged = 1;
            result = __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::SaveDatabaseHidden(*hidden, command, m_writeBuffer, GetPathHidden());
            if (result != 0)
                return result == 4 ? 0x13 : (result == 3 ? 0x0c : 5);
        }
    } else {
        if (result == 4)
            return 0x13;
        hidden->Init();
        m_databaseChanged = 1;
        result = __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::SaveDatabaseHidden(*hidden, command, m_writeBuffer, GetPathHidden());
        if (result != 0)
            return result == 4 ? 0x13 : (result == 3 ? 0x0c : 5);
    }
    result = __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::LoadDatabaseOfficial(m_database, command, GetPathOfficial());
    if (result == 0 && m_database->IsValid()) {
        if (IsExistFile(GetPathBackup()))
            return 0;
    } else {
        if (result != 0 && result == 4)
            return 0x13;
        result = __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::LoadDatabaseOfficial(m_database, command, GetPathBackup());
        if (result == 0 && m_database->IsValid()) {
            m_databaseChanged = 1;
            result = __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::SaveDatabaseOfficial(*m_database, command, m_writeBuffer, GetPathOfficial());
            if (result == 0)
                return 0;
        } else {
            if (result != 0 && result == 4)
                return 0x13;
            Init();
            m_databaseChanged = 1;
            result = __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::SaveDatabaseOfficial(*m_database, command, m_writeBuffer, GetPathBackup());
            if (result == 0) {
                result = __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::SaveDatabaseOfficial(*m_database, command, m_writeBuffer, GetPathOfficial());
                if (result == 0)
                    return 10;
            }
        }
    }
    if (result == 4) return 0x13;
    if (result == 3) return 0x0c;
    return 5;
}

int FFLiDatabaseFileAccessor::AdjustRegularListOfficial()
{
    void *buffer = m_allocator->Allocate(0x7534, 4);
    if (buffer == 0) return 0x0c;
    m_database->AdjustRegularList(static_cast<FFLiDatabaseFileOfficial::AdjustRegularBuffer *>(buffer));
    m_allocator->Free(buffer);
    return 0;
}

void FFLiDatabaseFileAccessor::BootLoad()
{
    if (BootLoadImpl(&m_command) == 0)
        AdjustRegularListOfficial();
}

int FFLiDatabaseFileAccessor::BeforeDestruct() { return m_pathsReady ? 0 : 5; }

int FFLiDatabaseFileAccessor::SaveHidden()
{
    if (!m_client->IsValid()) return 0x12;
    FFLiDatabaseFileHidden *hidden = static_cast<FFLiDatabaseFileHidden *>(static_cast<void *>(static_cast<unsigned char *>(static_cast<void *>(m_database)) + 0x43840));
    hidden->UpdateCrc();
    int result = __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::SaveDatabaseHidden(*hidden, &m_command, m_writeBuffer, GetPathHidden());
    if (result == 0) { m_hiddenSavePending = 0; m_hiddenFlushPending = 1; return 0; }
    if (result == 4) return 0x13;
    if (result == 3) return 0x0c;
    return 7;
}

int FFLiDatabaseFileAccessor::BackupOfficial()
{
    if (!m_client->IsValid()) return 0x12;
    int result = __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::CopyDatabaseOfficial(GetPathBackup(), GetPathOfficial(), &m_command, m_writeBuffer, m_allocator);
    if (result == 0) return 0;
    if (result == 4) return 0x13;
    if (result == 3) return 0x0c;
    return 7;
}

int FFLiDatabaseFileAccessor::BeforeFlushQuota()
{
    int result = 0;
    if ((m_hiddenSavePending == 0 || (result = SaveHidden()) == 0) && (m_officialBackupPending == 0 || (result = BackupOfficial()) == 0)) result = 0;
    return result;
}

int FFLiDatabaseFileAccessor::FlushQuota(bool force)
{
    bool updated = m_databaseChanged != 0 || m_officialBackupPending != 0 || m_hiddenFlushPending != 0;
    if (force || updated) {
        if (FFLiFsFile::FlushQuota(&m_command, GetPathOfficial(), 0) != 0) return 0x12;
        m_officialBackupPending = 0;
        m_databaseChanged = 0;
        m_hiddenFlushPending = 0;
    }
    return 0;
}

int FFLiDatabaseFileAccessor::SaveOfficial()
{
    if (!m_client->IsValid()) return 0x12;
    m_database->UpdateCrc();
    int result = __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::SaveDatabaseOfficial(*m_database, &m_command, m_writeBuffer, GetPathOfficial());
    if (result == 0) { m_officialBackupPending = 0; return 0; }
    if (result == 4) return 0x13;
    if (result == 3) return 0x0c;
    return 7;
}

int FFLiDatabaseFileAccessor::SaveIfUpdated() { return m_officialBackupPending ? SaveOfficial() : 0; }
int FFLiDatabaseFileAccessor::FormatOfficial() { m_database->Init(); m_officialBackupPending = 1; return 0; }
bool FFLiDatabaseFileAccessor::SetOfficial(const FFLiCharInfo *data, unsigned short index) { bool result = m_database->Set(*data, index); if (result) m_officialBackupPending = 1; return result; }
int FFLiDatabaseFileAccessor::DeleteOfficial(unsigned short index) { m_database->DeleteMii(m_database->GetImpl(index), index); m_officialBackupPending = 1; return 0; }
int FFLiDatabaseFileAccessor::IsUpdatedFileOfficial(int *updated) { if (updated) *updated = m_officialBackupPending != 0; return 0; }
int FFLiDatabaseFileAccessor::UpdateOfficialFromFileImpl()
{
    int result = __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::LoadDatabaseOfficial(m_database, &m_command, GetPathOfficial());
    if (result == 0 && !m_database->IsValid())
        result = 1;
    return result;
}
int FFLiDatabaseFileAccessor::UpdateOfficialFromFile()
{
    if (!m_client->IsValid())
        return 0x12;
    int result = UpdateOfficialFromFileImpl();
    if (result == 0)
    {
        m_officialBackupPending = 0;
        m_databaseChanged = 1;
        return 0;
    }
    if (result == 4)
        return 0x13;
    if (result == 3)
        return 0x0c;
    return 7;
}
int FFLiDatabaseFileAccessor::SetRegularOfficial(unsigned short index, bool regular) { if (m_database->SetRegular(index, regular)) { m_officialBackupPending = 1; return 0; } return 5; }
int FFLiDatabaseFileAccessor::FormatHidden() { static_cast<FFLiDatabaseFileHidden *>(static_cast<void *>(static_cast<unsigned char *>(static_cast<void *>(m_database)) + 0x43840))->Init(); m_hiddenSavePending = 1; return 0; }
bool FFLiDatabaseFileAccessor::AddHidden(const FFLiMiiDataCore &data, bool temporary) { bool result = static_cast<FFLiDatabaseFileHidden *>(static_cast<void *>(static_cast<unsigned char *>(static_cast<void *>(m_database)) + 0x43840))->Add(data, temporary); if (result) m_hiddenSavePending = 1; return result; }
int FFLiDatabaseFileAccessor::DeleteHidden(const FFLCreateID &createId) { FFLiDatabaseFileHidden *hidden = static_cast<FFLiDatabaseFileHidden *>(static_cast<void *>(static_cast<unsigned char *>(static_cast<void *>(m_database)) + 0x43840)); hidden->DeleteMii(createId); m_hiddenSavePending = 1; return 0; }
int FFLiDatabaseFileAccessor::DeleteFileOfficial() { return FFLiFsFile::Remove(&m_command, GetPathOfficial(), 0); }
int FFLiDatabaseFileAccessor::DeleteFileHidden() { return FFLiFsFile::Remove(&m_command, GetPathHidden(), 0); }
int FFLiDatabaseFileAccessor::ImportFileOfficial(const void *data, unsigned int size) { return __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::WriteFile(data, size, &m_command, m_writeBuffer, GetPathOfficial()); }
int FFLiDatabaseFileAccessor::ImportFileHidden(const void *data, unsigned int size) { return __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::WriteFile(data, size, &m_command, m_writeBuffer, GetPathHidden()); }
int FFLiDatabaseFileAccessor::ExportFileOfficial(void *data, unsigned int *size) { return __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::ReadFile(data, size ? *size : 0, &m_command, GetPathOfficial()); }
int FFLiDatabaseFileAccessor::ExportFileHidden(void *data, unsigned int *size) { return __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::ReadFile(data, size ? *size : 0, &m_command, GetPathHidden()); }
int FFLiDatabaseFileAccessor::AfterConstructByAccessor(const FFLiDatabaseFileAccessor *accessor) { if (accessor == 0) return 5; memcpy(m_pathOfficial, accessor->m_pathOfficial, sizeof(m_pathOfficial)); memcpy(m_pathBackup, accessor->m_pathBackup, sizeof(m_pathBackup)); memcpy(m_pathHidden, accessor->m_pathHidden, sizeof(m_pathHidden)); m_pathsReady = accessor->m_pathsReady; return 0; }
int FFLiDatabaseFileAccessor::FormatFileOfficial()
{
    if (!m_client->IsValid())
        return 0x12;
    int result = FormatOfficial();
    if (result != 0)
        return result;
    result = SaveOfficial();
    if (result == 0)
        m_databaseChanged = 1;
    return result;
}
int FFLiDatabaseFileAccessor::FormatFileHidden()
{
    if (!m_client->IsValid())
        return 0x12;
    int result = FormatHidden();
    if (result != 0)
        return result;
    result = SaveHidden();
    if (result == 0)
        m_databaseChanged = 1;
    return result;
}
int FFLiDatabaseFileAccessor::DestroyFileOfficial()
{
    if (!m_client->IsValid())
        return 0x12;
    int result = DeleteFileOfficial();
    if (result == 0)
    {
        m_database->Init();
        m_officialBackupPending = 0;
        m_databaseChanged = 1;
        return 0;
    }
    if (result == -6 || result == -10)
        return 4;
    return result;
}
int FFLiDatabaseFileAccessor::DestroyFileHidden()
{
    if (!m_client->IsValid())
        return 0x12;
    int result = DeleteFileHidden();
    if (result == 0)
    {
        FFLiDatabaseFileHidden *hidden = static_cast<FFLiDatabaseFileHidden *>(static_cast<void *>(static_cast<unsigned char *>(static_cast<void *>(m_database)) + 0x43840));
        hidden->Init();
        m_hiddenSavePending = 0;
        m_hiddenFlushPending = 0;
        m_databaseChanged = 1;
        return 0;
    }
    if (result == -6 || result == -10)
        return 4;
    return result;
}
void FFLiDatabaseFileAccessor::ResetAllAuthorID(const FFLiAuthorID &authorId) { m_database->ResetAllAuthorID(authorId); m_officialBackupPending = 1; }
void FFLiDatabaseFileAccessor::ResetAuthorID(unsigned int index, const FFLiAuthorID &authorId) { m_database->ResetAuthorID(static_cast<unsigned short>(index), authorId); m_officialBackupPending = 1; }
const char *FFLiDatabaseFileAccessor::GetPath(FFLpDatabaseFileType type) { if (type == FFLP_DATABASE_FILE_HIDDEN) return "db_FFL_HDB.dat"; if (type == FFLP_DATABASE_FILE_BACKUP) return "db_FFL_ODB_OLD.dat"; return "db_FFL_ODB.dat"; }
int FFLiDatabaseFileAccessor::IsExistFiles(int *result, FSClient *, FSCmdBlock *) { if (result) *result = 0; return 0; }
