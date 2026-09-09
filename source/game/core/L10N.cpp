#include "game/core/L10N.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/data/StringTable.hpp"

static StringTable* g_stringTable = NULL;
extern CConsoleMinecraftApp gApp;

const wchar_t* L10N::GetString(int id)
{
    if (g_stringTable == NULL)
        return L"";
    return g_stringTable->getString(id);
}

void L10N::loadStringTable()
{
    delete g_stringTable;
    g_stringTable = NULL;

    std::wstring path = L"languages.loc";
    if (gApp.hasArchiveFile(path, true))
    {
        arrayWithLength<unsigned char> data = gApp.getArchiveFile(path, true);
        g_stringTable = new StringTable(data, false, false);
        delete[] data.data;
    }
}
