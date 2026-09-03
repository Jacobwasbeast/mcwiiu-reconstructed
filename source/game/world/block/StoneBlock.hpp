#ifndef GAME_WORLD_BLOCK_STONEBLOCK_HPP
#define GAME_WORLD_BLOCK_STONEBLOCK_HPP

#include <string>

#include "game/world/block/Block.hpp"

class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Item;
class LevelSource;
class MaterialColor;
class Random;
class TextureAtlasSprite;

class StoneBlock : public Block
{
public:
    class Variant
    {
    public:
        Variant(int data, const MaterialColor* materialColor, const std::wstring& name, int nameId, int descriptionId, const std::wstring& serializedName, bool natural);
        const MaterialColor* getMaterialColor();
        int getData();
        static Variant* byData(int data);
        TextureAtlasSprite* getTexture();
        void registerIcons(IconRegister* iconRegister);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        int getNameId();
        int getDescriptionId();
        bool isNatural() const;
        std::wstring getName();
    private:
        int mData;
        std::wstring mName;
        int mNameId;
        int mDescriptionId;
        const MaterialColor* mMaterialColor;
        std::wstring mSerializedName;
        bool mNatural;
        TextureAtlasSprite* mTexture;
    };

    StoneBlock();
    virtual ~StoneBlock();
    static void blockStaticCtor();
    virtual std::wstring getName();
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual Item* getResource(const BlockState* state, Random* random, int fortune);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
};

#endif
