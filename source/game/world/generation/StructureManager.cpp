#include "game/world/generation/StructureManager.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/world/generation/StructureTemplate.hpp"

StructureManager::StructureManager(DataFixerUpper* fixer)
    : boost::unordered_map<int, const StructureTemplate*>(), fixerUpper(fixer)
{
}

StructureManager::~StructureManager()
{
    iterator it;
    for (it = begin(); it != end(); ++it)
    {
        delete it->second;
    }
    clear();
}

void StructureManager::staticCtor()
{
}

std::wstring StructureManager::fjGetStructureTemplatePath(eMinecraftStructureTemplate type) const
{
    static const wchar_t* names[] =
    {
        L"igloo/igloo_top",
        L"endcity/end_city",
        L"shipwreck/shipwreck",
        L"ocean_ruin/ocean_ruin",
        L"village/village",
        L"pillager_outpost/outpost",
        L"woodland_mansion/mansion",
        L"monument/monument",
        L"buried_treasure/treasure",
        L"desert_pyramid/desert_pyramid",
        L"jungle_temple/jungle_temple",
        L"swamp_hut/swamp_hut"
    };
    const int index = static_cast<int>(type);
    if (index < 0 || index >= static_cast<int>(sizeof(names) / sizeof(names[0])))
    {
        return std::wstring(L"Structures/");
    }
    return std::wstring(L"Structures/") + names[index] + L".nbt";
}

void StructureManager::readStructure(eMinecraftStructureTemplate type, ByteArrayInputStream* input)
{
    if (input == 0)
    {
        return;
    }
    if (find(static_cast<int>(type)) == end())
    {
        (*this)[static_cast<int>(type)] = new StructureTemplate();
    }
}

bool StructureManager::load(eMinecraftStructureTemplate type)
{
    return find(static_cast<int>(type)) != end();
}

const StructureTemplate* StructureManager::get(eMinecraftStructureTemplate type)
{
    iterator it = find(static_cast<int>(type));
    if (it != end())
    {
        return it->second;
    }
    if (load(type))
    {
        it = find(static_cast<int>(type));
        if (it != end())
        {
            return it->second;
        }
    }
    return 0;
}

const StructureTemplate* StructureManager::getOrCreate(eMinecraftStructureTemplate type)
{
    const StructureTemplate* existing = get(type);
    if (existing != 0)
    {
        return existing;
    }
    StructureTemplate* created = new StructureTemplate();
    (*this)[static_cast<int>(type)] = created;
    return created;
}
