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
    ListTag* tileEntities = level->getList(L"TileEntities", Tag::TAG_COMPOUND);
    ListTag* sections = level->getList(L"Sections", Tag::TAG_COMPOUND);

    for (int sectionIndex = 0; sectionIndex < sections->size(); sectionIndex++)
    {
        CompoundTag* section = sections->getCompound(sectionIndex);
        unsigned char sectionY = section->getByte(L"Y");
        arrayWithLength<unsigned char> blockData = section->getByteArray(L"Blocks");

        for (unsigned int blockIndex = 0; blockIndex < blockData.length; blockIndex++)
        {
            if (blockData.data[blockIndex] == 26)
            {
                CompoundTag* bedEntity = new CompoundTag();

                bedEntity->putString(L"id", L"bed");
                bedEntity->putInt(L"x", (blockIndex & 0xF) + chunkX * 16);
                bedEntity->putInt(L"y", ((blockIndex >> 8) & 0xF) + sectionY * 16);
                bedEntity->putInt(L"z", ((blockIndex >> 4) & 0xF) + chunkZ * 16);

                tileEntities->add(bedEntity);
            }
        }
    }

    return tag;
}
