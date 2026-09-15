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
template <class T> class EnumProperty;

class PlanksBlock : public Block
{
public:
    class Variant
    {
    public:
        enum Data
        {
            eDataOak = 0,
            eDataSpruce = 1,
            eDataBirch = 2,
            eDataJungle = 3,
            eDataAcacia = 4,
            eDataDarkOak = 5
        };

        enum TranslationKey
        {
            eTranslationKeyOak = 0x13035FA5,
            eTranslationKeySpruce = 0x7A4DF272,
            eTranslationKeyBirch = 0x9BC6104A,
            eTranslationKeyJungle = 0x8CE369FC,
            eTranslationKeyAcacia = 0x674A2380,
            eTranslationKeyDarkOak = 0x0458BC59
        };

        Variant(int data, const std::wstring& name, int translationKey, const MaterialColor* materialColor);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring getName();
        int getData();
        int getTranslationKey();
        const MaterialColor* getMaterialColor();
        static Variant* byData(int data);

        static Variant* OAK;
        static Variant* SPRUCE;
        static Variant* BIRCH;
        static Variant* JUNGLE;
        static Variant* ACACIA;
        static Variant* DARK_OAK;

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

    static EnumProperty<Variant*>* VARIANT;

    TextureAtlasSprite* mIcons[6];
};

extern PlanksBlock::Variant* gPlankVariants[6];

#endif
