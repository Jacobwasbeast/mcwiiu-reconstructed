#include "game/world/block/BedBlockEntityInjecter.hpp"

#include "game/data/ListTag.hpp"
#include "game/world/biome/arrayWithLength.hpp"

BedBlockEntityInjecter::~BedBlockEntityInjecter()
{
}

int BedBlockEntityInjecter::getVersion()
{
    return 1125;
}

CompoundTag* BedBlockEntityInjecter::fix(CompoundTag* tag)
{
    CompoundTag* level = tag->getCompound(L"Level");
    int chunkX = level->getInt(L"xPos");
    int chunkZ = level->getInt(L"zPos");
    ListTag* tileEntities = level->getList(L"TileEntities", 10);
    ListTag* sections = level->getList(L"Sections", 10);

    for (int sectionIndex = 0; sectionIndex < sections->size(); sectionIndex++)
    {
        CompoundTag* section = sections->getCompound(sectionIndex);
        unsigned char sectionY = section->getByte(L"Y");
        arrayWithLength<unsigned char> blocks = section->getByteArray(L"Blocks");

        for (unsigned int index = 0; index < blocks.length; index++)
        {
            if (blocks.data[index] == 26)
            {
                CompoundTag* bedTag = new CompoundTag();

                bedTag->putString(L"id", L"bed");
                bedTag->putInt(L"x", (index & 0xF) + chunkX * 16);
                bedTag->putInt(L"y", ((index >> 8) & 0xF) + sectionY * 16);
                bedTag->putInt(L"z", ((index >> 4) & 0xF) + chunkZ * 16);

                tileEntities->add(bedTag);
            }
        }
    }

    return tag;
}
