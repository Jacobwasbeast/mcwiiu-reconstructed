#ifndef GAME_WORLD_BLOCK_DLCMATERIALFILE_HPP
#define GAME_WORLD_BLOCK_DLCMATERIALFILE_HPP

#include <string>

#include "game/core/DLCFile.hpp"
#include "game/world/block/MaterialTable.hpp"

class DLCMaterialFile : public DLCFile
{
public:
    DLCMaterialFile(const std::wstring& path);
    virtual ~DLCMaterialFile();

    virtual void addData(unsigned char* pData, unsigned int size);

    MaterialTable* getMaterialTable() { return m_pMaterialTable; }

protected:
    MaterialTable* m_pMaterialTable;
};

#endif
