#ifndef GAME_CORE_DLCFILE_HPP
#define GAME_CORE_DLCFILE_HPP

#include <string>

#include "game/core/DLCManager.hpp"

class DLCPack;

class DLCFile
{
    friend class DLCPack;

public:
    DLCFile(DLCManager::EDLCType type, const std::wstring& path);
    virtual ~DLCFile();

    virtual void addData(unsigned char* pData, unsigned int size);
    virtual unsigned char* getData(unsigned int& size);
    virtual void addParameter(DLCManager::EDLCParameterType parameter, const std::wstring& value);
    virtual std::wstring getParameterAsString(DLCManager::EDLCParameterType parameter, const std::wstring& name);
    virtual bool getParameterAsBool(DLCManager::EDLCParameterType parameter);

protected:
    DLCPack* m_pPack;
    DLCManager::EDLCType m_type;
    std::wstring m_path;
    unsigned int m_fileID;
};

#endif
