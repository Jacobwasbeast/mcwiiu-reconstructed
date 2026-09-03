#include "game/core/DLCColourTableFile.hpp"

#include "game/core/Minecraft.hpp"

class ColourTable
{
public:
    ColourTable(ColourTable* source, unsigned char* data, unsigned int size);
};

DLCColourTableFile::DLCColourTableFile(const std::wstring& path)
    : DLCFile(DLCManager::eDLCType_ColourTable, path)
    , m_pColourTable(NULL)
{
}

DLCColourTableFile::~DLCColourTableFile()
{
    delete m_pColourTable;
}

void DLCColourTableFile::addData(unsigned char* pData, unsigned int size)
{
    m_pColourTable = new ColourTable(Minecraft::GetInstance()->getColourTable(), pData, size);
}
