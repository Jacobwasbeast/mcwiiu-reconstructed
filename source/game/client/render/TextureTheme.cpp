#include "game/client/render/TextureTheme.hpp"

#include <cstdlib>

#include "game/core/BufferedImage.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockTransaction.hpp"
#include "game/world/generation/RandomPatchesLayerTheme.hpp"

TextureTheme::BlockStack::BlockStack()
    : mOffset(0)
    , mStates()
{
}

TextureTheme::BlockStack::BlockStack(const BlockState* state, unsigned int count, int offset)
    : mOffset(offset)
    , mStates()
{
    for (unsigned int i = 0; i < count; ++i)
        mStates.push_back(state);
    mStates.push_back(Block::byId(0)->defaultBlockState());
}

TextureTheme::BlockStack::BlockStack(std::vector<const BlockState*>& states, int offset)
    : mOffset(offset)
    , mStates(states)
{
}

TextureTheme::BlockStack::~BlockStack()
{
}

const BlockState* TextureTheme::BlockStack::at(int index) const
{
    index += mOffset;
    if (index < 0)
        return Block::byId(0)->defaultBlockState();
    if (mStates.empty())
        return NULL;
    if (index >= (int)mStates.size() - 1)
        index = (int)mStates.size() - 1;
    return mStates.at(index);
}

TextureTheme::TextureTheme()
    : mAssets()
    , mStacks()
{
}

TextureTheme::~TextureTheme()
{
}

const TextureTheme::BlockStack& TextureTheme::getStack(BufferedImage* image, int x, int y) const
{
    static BlockStack defaultStack;
    static BlockStack invalidStack(NULL, 1, 0);
    if (x > 0 && y > 0 && x < (int)image->getWidth() && y < (int)image->getHeight())
    {
        int key = image->getData()[(x * image->getWidth() + y)] & 0xFFFFFF;
        boost::unordered_map<int, BlockStack>::const_iterator it = mStacks.find(key);
        if (it != mStacks.end())
            return it->second;
        return defaultStack;
    }
    return invalidStack;
}

void TextureTheme::addStack(const std::wstring& key, const BlockStack& stack)
{
    int value = (int)std::wcstol(key.c_str(), NULL, 16);
    mStacks[value] = stack;
}

void TextureTheme::Apply(Random&, BlockTransaction* transaction, int& width, int& depth) const
{
    unsigned int size = width > depth ? width : depth;
    BufferedImage* image = mAssets.getAssetForSize(size);
    int xOffset = ((int)image->getWidth() - width) / 2;
    int zOffset = ((int)image->getHeight() - depth) / 2;
    for (int x = 0; x < width; ++x)
    {
        for (int z = 0; z < depth; ++z)
        {
            const BlockStack& stack = getStack(image, x + xOffset, z + zOffset);
            int top = RandomPatchesLayerTheme::HighestSolidBlock(transaction, x, z);
            if (top < 0)
                continue;
            for (int y = 1; y < top; ++y)
            {
                const BlockState* state = stack.at(top - y);
                if (state != NULL)
                    transaction->setBlock(BlockPos(x, y, z), state);
            }
        }
    }
}
