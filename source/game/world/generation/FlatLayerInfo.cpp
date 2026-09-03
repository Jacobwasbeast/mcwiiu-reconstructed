#include "game/world/generation/FlatLayerInfo.hpp"
#include "game/core/L10N.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"
#include "game/util/ResourceLocation.hpp"
std::wstring formatwstr(const wchar_t* format, ...);
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
    std::wstring result = formatwstr(L"%i", getId());
    if (mHeight > 1)
        result = formatwstr(L"%i", mHeight) + L"x" + result;
    if (getData() > 0)
        result += formatwstr(L":%i", getData());
    return result;
}

std::wstring FlatLayerInfo::getHeightText() const
{
    return formatwstr(L"%ix", getHeight());
}

std::wstring FlatLayerInfo::getNameText() const
{
    Block* block = getBlock();
    if (block == NULL)
        return std::wstring();
    ItemInstance item(block);
    item.setAuxValue(getData());
    return item.getHoverName();
}

std::wstring FlatLayerInfo::getDescriptionText() const
{
    Block* block = getBlock();
    if (block == NULL)
        return std::wstring();
    return std::wstring(L10N::GetString(block->getDescriptionId(-1)));
}
