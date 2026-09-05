#ifndef GAME_WORLD_BLOCK_MONSTEREGGBLOCK_HPP
#define GAME_WORLD_BLOCK_MONSTEREGGBLOCK_HPP

#include <string>

#include "game/world/block/Block.hpp"

class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class Random;
class TextureAtlasSprite;

class MonsterEggBlock : public Block
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
        const BlockState* getHostBlockState();
        static Variant* byData(int data);
        static Variant* byHostBlock(const BlockState* state);

    private:
        int mData;
        std::wstring mName;
        int mTranslationKey;
    };

    MonsterEggBlock();
    virtual ~MonsterEggBlock();
    static void blockStaticCtor();
    virtual int getResourceCount(Random* random);
    static bool isCompatibleHostBlock(const BlockState* state);
    virtual not_null_ptr<ItemInstance> getSilkTouchItemInstance(const BlockState* state);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
};

#endif
