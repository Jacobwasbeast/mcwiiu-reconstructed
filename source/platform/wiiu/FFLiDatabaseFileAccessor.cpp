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

int FFLiDatabaseFileAccessor::IsExistFile(const char* path)
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

int FFLiDatabaseFileAccessor::BeforeDestruct() { return (m_pathsReady != 0) ? 0 : 5; }

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

