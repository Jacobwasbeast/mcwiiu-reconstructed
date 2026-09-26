#include "game/util/NbtUtils.hpp"

#include <vector>

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/Tag.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/SandStoneBlock.hpp"
#include "game/world/block/StoneBlock.hpp"
#include "game/world/block/StoneBrickBlock.hpp"

bool NbtUtils::compareNbt(Tag* first, Tag* second, bool compareLists)
{
    if (first == second) return true;
    if (first == NULL || second == NULL) return false;
    CompoundTag* firstCompound = dynamic_cast<CompoundTag*>(first);
    if (firstCompound != NULL)
    {
        CompoundTag* secondCompound = dynamic_cast<CompoundTag*>(second);
        if (secondCompound == NULL) return false;
        std::vector<std::wstring>* keys = firstCompound->getAllKeys();
        std::vector<std::wstring>::const_iterator it;
        for (it = keys->begin(); it != keys->end(); ++it)
        {
            if (!compareNbt(firstCompound->get(*it), secondCompound->get(*it), compareLists)) { delete keys; return false; }
        }
        delete keys;
        return true;
    }
    ListTag* firstList = dynamic_cast<ListTag*>(first);
    if (firstList != NULL && compareLists)
    {
        ListTag* secondList = dynamic_cast<ListTag*>(second);
        if (secondList == NULL) return false;
        int i;
        for (i = 0; i < firstList->size(); ++i)
        {
            bool found = false;
            int j;
            for (j = 0; j < secondList->size(); ++j)
            {
                if (compareNbt(firstList->get(i), secondList->get(j), compareLists)) { found = true; break; }
            }
            if (!found) return false;
        }
        return true;
    }
    return first->equals(second);
}

BlockPos NbtUtils::readBlockPos(CompoundTag* tag)
{
    return BlockPos(tag->getInt(L"X"), tag->getInt(L"Y"), tag->getInt(L"Z"));
}

CompoundTag* NbtUtils::writeBlockPos(const BlockPos& pos)
{
    CompoundTag* tag = new CompoundTag();
    tag->putInt(L"X", pos.x);
    tag->putInt(L"Y", pos.y);
    tag->putInt(L"Z", pos.z);
    return tag;
}

void NbtUtils::mapToBlock(std::wstring& name, int& data)
{
    if (name == L"minecraft:stone_bricks") { name = L"minecraft:stonebrick"; data = 0; return; }
    if (name == L"minecraft:bricks") { name = L"minecraft:brick_block"; data = 0; return; }
    if (name == L"minecraft:chiseled_stone_bricks") { name = L"minecraft:stonebrick"; data |= StoneBrickBlock::Variant::byData(3)->getData(); return; }
    if (name == L"minecraft:cracked_stone_bricks") { name = L"minecraft:stonebrick"; data |= StoneBrickBlock::Variant::byData(2)->getData(); return; }
    if (name == L"minecraft:mossy_stone_bricks") { name = L"minecraft:stonebrick"; data |= StoneBrickBlock::Variant::byData(1)->getData(); return; }
    if (name == L"minecraft:polished_granite") { name = L"minecraft:stone"; data |= StoneBlock::Variant::byData(2)->getData(); return; }
    if (name == L"minecraft:diorite") { name = L"minecraft:stone"; data |= StoneBlock::Variant::byData(3)->getData(); return; }
    if (name == L"minecraft:polished_diorite") { name = L"minecraft:stone"; data |= StoneBlock::Variant::byData(4)->getData(); return; }
    if (name == L"minecraft:dark_oak_planks") { name = L"minecraft:planks"; data |= PlanksBlock::Variant::byData(5)->getData(); return; }
    if (name == L"minecraft:spruce_planks") { name = L"minecraft:planks"; data |= PlanksBlock::Variant::byData(1)->getData(); return; }
    if (name == L"minecraft:cut_sandstone") { name = L"minecraft:sandstone"; data |= SandStoneBlock::Variant::byData(2)->getData(); return; }
    if (name == L"minecraft:light_blue_terracotta") { name = L"minecraft:stained_hardened_clay"; data |= 3; return; }
    if (name == L"minecraft:magma_block") { name = L"minecraft:magma"; return; }
    if (name == L"minecraft:stripped_oak_log") { name = L"minecraft:stripped_log_oak"; return; }
    if (name == L"minecraft:stripped_spruce_log") { name = L"minecraft:stripped_log_spruce"; return; }
    if (name == L"minecraft:stripped_birch_log") { name = L"minecraft:stripped_log_birch"; return; }
    if (name == L"minecraft:stripped_jungle_log") { name = L"minecraft:stripped_log_jungle"; return; }
    if (name == L"minecraft:stripped_acacia_log") { name = L"minecraft:stripped_log_acacia"; return; }
    if (name == L"minecraft:stripped_dark_oak_log") { name = L"minecraft:stripped_log_dark_oak"; return; }
    if (name == L"minecraft:chiseled_sandstone") { name = L"minecraft:sandstone"; data |= SandStoneBlock::Variant::byData(1)->getData(); }
}

const BlockState* NbtUtils::readBlockState(CompoundTag* tag)
{
    if (!tag->contains(L"Name", Tag::TAG_STRING)) return Blocks::air->defaultBlockState();
    std::wstring name = tag->getString(L"Name");
    int data = 0;
    mapToBlock(name, data);
    Block* block = Block::byString(name);
    const BlockState* state = block->defaultBlockState();
    if (tag->contains(L"Properties", Tag::TAG_COMPOUND))
    {
        CompoundTag* properties = tag->getCompound(L"Properties");
        const std::vector<const Property*>& blockProperties = block->getBlockStateDefinition()->getProperties();
        std::vector<std::wstring>* keys = properties->getAllKeys();
        std::vector<std::wstring>::const_iterator key;
        for (key = keys->begin(); key != keys->end(); ++key)
        {
            std::vector<const Property*>::const_iterator property;
            for (property = blockProperties.begin(); property != blockProperties.end(); ++property)
            {
                if ((*property)->getName() == *key)
                {
                    state = block->setPropertyValueFromString(state, *property, properties->getString(*key));
                    break;
                }
            }
        }
        delete keys;
    }
    return state;
}
