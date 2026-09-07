#ifndef GAME_WORLD_BLOCK_DIRTBLOCK_HPP
#define GAME_WORLD_BLOCK_DIRTBLOCK_HPP

#include <string>

#include "game/world/block/Block.hpp"
#include "game/world/block/BonemealableBlock.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class MaterialColor;
class Random;
class TextureAtlasSprite;

template <class T> class EnumProperty;

class DirtBlock : public Block, public BonemealableBlock
{
public:
    class Variant
    {
    public:
        Variant(int data, const std::wstring& name, int nameId, int descriptionId,
                const MaterialColor* materialColor, const std::wstring& texture,
                const std::wstring& topTexture, const std::wstring& sideTexture);

        const MaterialColor* getMaterialColor();
        int getData();
        static Variant* byData(int data);
        TextureAtlasSprite* getTexture(const Direction* direction);
        void registerIcons(IconRegister* iconRegister);
        int getNameId();
        int getDescriptionId();
        std::wstring toString() const;
        std::wstring getSerializedName() const;
        std::wstring getName();

    private:
        int mData;
        std::wstring mName;
        int mNameId;
        int mDescriptionId;
        const MaterialColor* mMaterialColor;
        std::wstring mTexture;
        std::wstring mTopTexture;
        std::wstring mSideTexture;
        TextureAtlasSprite* mIcon;
        TextureAtlasSprite* mSideIcon;
        TextureAtlasSprite* mTopIcon;
    };

    DirtBlock();
    virtual bool isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state,
                                       bool isClientSide);
    virtual bool isBonemealSuccess(Level* level, Random* random, const BlockPos& pos,
                                   const BlockState* state);
    virtual void performBonemeal(Level* level, Random* random, const BlockPos& pos,
                                 const BlockState* state);
    static void blockStaticCtor();
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource,
                                             const BlockPos& pos);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state,
                                                               LevelSource* levelSource,
                                                               const BlockPos& pos);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos,
                                                            const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual ~DirtBlock();
};

extern DirtBlock::Variant* gVariants[3];
extern EnumProperty<DirtBlock::Variant*>* gVariantProperty;

#endif
