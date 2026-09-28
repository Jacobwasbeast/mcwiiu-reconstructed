#include "game/world/generation/FlatLayerInfo.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/core/L10N.hpp"
#include "game/core/StringConversion.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/core/StringFormat.hpp"
FlatLayerInfo::FlatLayerInfo() : mVersion(3), mBlockState(NULL), mHeight(0), mStart(0) {}
FlatLayerInfo::FlatLayerInfo(int height, Block* block)
{
    _init(height, block);
}
FlatLayerInfo::FlatLayerInfo(int version, int height, int blockId)
{
    Block* block = Block::byId(blockId);
    _init(height, block);
    mBlockState = block->getBlockState(0);
    mVersion = version;
}
FlatLayerInfo::FlatLayerInfo(int version, int height, Block* block)
{
    _init(height, block);
    mVersion = version;
}
FlatLayerInfo::FlatLayerInfo(int version, int height, Block* block, int data)
{
    _init(height, block);
    mVersion = version;
    mBlockState = block->getBlockState(data);
}
unsigned int FlatLayerInfo::getStart() const { return (unsigned int)mStart; }
const BlockState* FlatLayerInfo::getBlockState() const { return mBlockState; }
unsigned int FlatLayerInfo::getHeight() const { return (unsigned int)mHeight; }
int FlatLayerInfo::getEnd() const { return getStart() + getHeight() - 1; }
void FlatLayerInfo::_init(int height, Block* block) { mHeight = height; mVersion = 3; mBlockState = block->defaultBlockState(); mStart = 0; }
void FlatLayerInfo::setHeight(int height) { mHeight = height; }
void FlatLayerInfo::setBlockState(const BlockState* state) { mBlockState = state; }
Block* FlatLayerInfo::getBlock() const { return mBlockState ? mBlockState->getBlock() : NULL; }
int FlatLayerInfo::getId() const { return Block::getId(getBlock()); }
int FlatLayerInfo::getData() const { Block* block = getBlock(); return block ? block->convertBlockStateToLegacyData(mBlockState) : 0; }
void FlatLayerInfo::setStart(int start) { mStart = start; }
std::wstring FlatLayerInfo::toString()
{
    std::wstring result;
    if (mVersion < 3)
    {
        result = _toString<int>(Block::getId(getBlock()));
        if (mHeight > 1)
            result = _toString<int>(mHeight) + L"x" + result;
    }
    else
    {
        result = Block::blockRegistry->getKey(getBlock()).toString();
        if (mHeight > 1)
            result = _toString<int>(mHeight) + L"*" + result;
    }
    int data = getData();
    if (data > 0)
        result += L":" + _toString<int>(data);
    return result;
}

std::wstring FlatLayerInfo::getHeightText() const
{
    return formatwstr(L"%ix", getHeight());
}

std::wstring FlatLayerInfo::getNameText() const
{
    Block* block = getBlock();
    if (block == Blocks::air)
        return std::wstring(L10N::GetString(IDS_SUPERFLAT_AIR_LAYER));
    if (block == NULL)
        return std::wstring();
    ItemInstance item(block);
    item.setAuxValue(getData());
    return item.getHoverName();
}

std::wstring FlatLayerInfo::getDescriptionText() const
{
    Block* block = getBlock();
    if (block == Blocks::air)
        return std::wstring(L10N::GetString(IDS_SUPERFLAT_DESC_AIR_LAYER));
    if (block == NULL)
        return std::wstring();
    return std::wstring(L10N::GetString(block->getDescriptionId(-1)));
}
