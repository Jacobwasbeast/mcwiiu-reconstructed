#ifndef GAME_WORLD_BLOCK_PLANKSBLOCK_HPP
#define GAME_WORLD_BLOCK_PLANKSBLOCK_HPP

#include <string>

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class LevelSource;
class MaterialColor;
class TextureAtlasSprite;

class PlanksBlock : public Block
{
public:
    class Variant
    {
    public:
        Variant(int data, const std::wstring& name, int translationKey, const MaterialColor* materialColor);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring getName();
        int getData();
        int getTranslationKey();
        const MaterialColor* getMaterialColor();
        static Variant* byData(int data);

        int mData;
        std::wstring mName;
        int mTranslationKey;
        const MaterialColor* mMaterialColor;
    };

    PlanksBlock();
    virtual ~PlanksBlock();
    static void blockStaticCtor();
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual int getDescriptionId(int auxValue);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

    TextureAtlasSprite* mIcons[6];
};

#endif
