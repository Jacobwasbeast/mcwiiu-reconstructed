#ifndef GAME_WORLD_BLOCK_PURPURSLABBLOCK_HPP
#define GAME_WORLD_BLOCK_PURPURSLABBLOCK_HPP

#include <string>

#include "game/world/block/HalfSlabBlock.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Boxed;
class Level;
class Random;
class Property;
class ItemInstance;

class PurpurSlabBlock : public HalfSlabBlock
{
public:
    class Variant
    {
    public:
        Variant(std::wstring name);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;

        std::wstring mName;
    };

    PurpurSlabBlock();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual ~PurpurSlabBlock();
    virtual void DerivedInit();
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual int getAuxName(int auxValue);
    virtual Property* getTypeProperty();
    virtual Boxed* getTypeForItem(not_null_ptr<ItemInstance> itemInstance);
    static void blockStaticCtor();
};

#endif
