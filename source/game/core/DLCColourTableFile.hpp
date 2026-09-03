#ifndef GAME_CORE_DLCCOLOURTABLEFILE_HPP
#define GAME_CORE_DLCCOLOURTABLEFILE_HPP

#include <string>

#include "game/core/DLCFile.hpp"

class ColourTable;

class DLCColourTableFile : public DLCFile
{
public:
    DLCColourTableFile(const std::wstring& path);
    virtual ~DLCColourTableFile();

    virtual void addData(unsigned char* pData, unsigned int size);

protected:
    ColourTable* m_pColourTable;
};

#endif
