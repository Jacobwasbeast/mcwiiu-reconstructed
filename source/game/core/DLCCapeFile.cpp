#include "game/core/DLCCapeFile.hpp"

#include "game/core/CMinecraftApp.hpp"

DLCCapeFile::DLCCapeFile(const std::wstring& path)
    : DLCFile(DLCManager::eDLCType_Cape, path)
{
}

DLCCapeFile::~DLCCapeFile()
{
}

void DLCCapeFile::addData(unsigned char* data, unsigned int size)
{
    CMinecraftApp* app = (CMinecraftApp*)0x10528dc0u;
    app->AddMemoryTextureFile((int*)&m_path, (int*)data, size);
}
