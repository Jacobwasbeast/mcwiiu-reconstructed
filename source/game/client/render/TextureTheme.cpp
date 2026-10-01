#include "game/client/render/TextureTheme.hpp"

#include "game/core/BufferedImage.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockTransaction.hpp"
#include "game/world/generation/RandomPatchesLayerTheme.hpp"

template <class T>
T _fromHEXString(const std::wstring& text);

TextureTheme::BlockStack::BlockStack()
    : mOffset(0)
{
}

TextureTheme::BlockStack::BlockStack(const BlockState* state, unsigned int count, int offset)
    : mOffset(offset)
{
    for (unsigned int i = 0; i < count; ++i)
        mStates.push_back(state);
    static const BlockState* air = Blocks::air->defaultBlockState();
    mStates.push_back(air);
}

TextureTheme::BlockStack::BlockStack(std::vector<const BlockState*>& states, int offset)
    : mOffset(offset)
    , mStates(states)
{
}

const BlockState* TextureTheme::BlockStack::at(int index) const
{
    index += mOffset;
    if (mStates.empty())
        return NULL;
    if (index < 0)
        return Blocks::air->defaultBlockState();
    if (index >= (int)mStates.size() - 1)
        index = (int)mStates.size() - 1;
    return mStates.at(index);
}

TextureTheme::TextureTheme()
{
}

const TextureTheme::BlockStack& TextureTheme::getStack(BufferedImage* image, int x, int y) const
{
    static BlockStack defaultStack;
    static BlockStack invalidStack(Blocks::air->defaultBlockState(), 1, 0);
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
    int value = _fromHEXString<int>(key);
    mStacks.emplace(std::make_pair(value, stack));
}

void TextureTheme::Apply(Random&, BlockTransaction* transaction, int&, int&) const
{
    const int width = transaction->mSize.x;
    const int depth = transaction->mSize.z;
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
            if (top == -1)
                continue;
            for (int y = transaction->mSize.y - 1; y > 0; --y)
            {
                const BlockState* state = stack.at(top - y);
                if (state != NULL)
                    transaction->setBlock(BlockPos(x, y, z), state);
            }
        }
    }
}
