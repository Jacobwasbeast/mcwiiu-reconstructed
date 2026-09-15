#include "game/core/DLCGameRulesHeader.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"

#include "game/core/GameRuleManager.hpp"


DLCGameRulesHeader::DLCGameRulesHeader(const std::wstring& path)
    : DLCFile(DLCManager::eDLCType_GameRulesHeader, path),
      m_data(NULL),
      m_dataSize(0),
      m_ready(false),
      m_levelGenerationOptions(NULL)
{
    std::wstring grfPath = path.substr(0, path.size() - 4) + L".grf";
    m_gameRulesSource.setGrfPath(grfPath);
}

DLCGameRulesHeader::~DLCGameRulesHeader() = default;

bool DLCGameRulesHeader::requiresTexturePack()
{
    return m_gameRulesSource.requiresTexturePack();
}

unsigned int DLCGameRulesHeader::getTexturePackId()
{
    return m_gameRulesSource.getTexturePackId();
}

std::wstring DLCGameRulesHeader::getDefaultSaveName()
{
    return m_gameRulesSource.getDefaultSaveName();
}

const wchar_t* DLCGameRulesHeader::getWorldName()
{
    return m_gameRulesSource.getWorldName();
}

const wchar_t* DLCGameRulesHeader::getDisplayName()
{
    return m_gameRulesSource.getDisplayName();
}

std::wstring DLCGameRulesHeader::getGrfPath()
{
    return m_gameRulesSource.getGrfPath();
}

void DLCGameRulesHeader::setRequiresTexturePack(bool required)
{
    m_gameRulesSource.setRequiresTexturePack(required);
}

void DLCGameRulesHeader::setTexturePackId(unsigned int id)
{
    m_gameRulesSource.setTexturePackId(id);
}

void DLCGameRulesHeader::setDefaultSaveName(const std::wstring& name)
{
    m_gameRulesSource.setDefaultSaveName(name);
}

void DLCGameRulesHeader::setWorldName(const std::wstring& name)
{
    m_gameRulesSource.setWorldName(name);
}

void DLCGameRulesHeader::setDisplayName(const std::wstring& name)
{
    m_gameRulesSource.setDisplayName(name);
}

void DLCGameRulesHeader::setGrfPath(const std::wstring& path)
{
    m_gameRulesSource.setGrfPath(path);
}

bool DLCGameRulesHeader::ready()
{
    return m_ready;
}

void DLCGameRulesHeader::addData(unsigned char* data, unsigned int size)
{
    m_data = data;
    m_dataSize = size;
}

unsigned char* DLCGameRulesHeader::getData(unsigned int& size)
{
    size = m_dataSize;
    return m_data;
}

void DLCGameRulesHeader::setGrfData(unsigned char* data, unsigned int size, StringTable* stringTable)
{
    if (m_ready)
        return;

    m_ready = true;
    gApp.mGameRuleManager.readRuleFile(m_levelGenerationOptions, data, size, stringTable);
}
