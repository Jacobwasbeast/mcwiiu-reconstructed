#ifndef GAME_WORLD_BLOCK_REDSANDSTONEBLOCK_HPP
#define GAME_WORLD_BLOCK_REDSANDSTONEBLOCK_HPP

#include "game/world/block/Block.hpp"

#include <string>

class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class TextureAtlasSprite;

class RedSandStoneBlock : public Block
{
public:
    class Variant
    {
    public:
        Variant(int data, const std::wstring& name, int translationKey);
        int getData();
        static Variant* byData(int data);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        int getTranslationKey();
        std::wstring getName();

    private:
        int mData;
        std::wstring mName;
        int mTranslationKey;
    };

    RedSandStoneBlock();
    virtual ~RedSandStoneBlock();
    static void blockStaticCtor();
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mTextures[5];
};

#endif
