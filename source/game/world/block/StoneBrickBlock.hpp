#ifndef GAME_WORLD_BLOCK_STONEBRICKBLOCK_HPP
#define GAME_WORLD_BLOCK_STONEBRICKBLOCK_HPP

#include <string>

#include "game/world/block/Block.hpp"

class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class LevelSource;
class TextureAtlasSprite;

class StoneBrickBlock : public Block
{
public:
    class Variant
    {
    public:
        Variant(int data, const std::wstring& name, int translationKey);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring getName();
        int getData();
        int getTranslationKey();
        static Variant* byData(int data);

    private:
        int mData;
        std::wstring mName;
        int mTranslationKey;
    };

    StoneBrickBlock();
    virtual ~StoneBrickBlock();
    static void blockStaticCtor();
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual int getDescriptionId(int auxValue);

private:
    TextureAtlasSprite* mTextures[4];
};

#endif
