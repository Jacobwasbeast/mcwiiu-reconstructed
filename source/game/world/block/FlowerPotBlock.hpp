#ifndef GAME_WORLD_BLOCK_FLOWERPOTBLOCK_HPP
#define GAME_WORLD_BLOCK_FLOWERPOTBLOCK_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BaseEntityBlock.hpp"

class BlockEntity;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class Item;
class Level;
class LevelSource;
class Random;

template <class T> class EnumProperty;

class FlowerPotBlock : public BaseEntityBlock
{
public:
    class Contents : public StringRepresentable
    {
    public:
        enum EContents
        {
            EMPTY = 0,
            POPPY = 1,
            DANDELION = 2,
            OAK_SAPLING = 3,
            SPRUCE_SAPLING = 4,
            BIRCH_SAPLING = 5,
            JUNGLE_SAPLING = 6,
            ACACIA_SAPLING = 7,
            DARK_OAK_SAPLING = 8,
            MUSHROOM_RED = 9,
            MUSHROOM_BROWN = 10,
            DEAD_BUSH = 11,
            FERN = 12,
            GRASS = 13,
            BLUE_ORCHID = 14,
            ALLIUM = 15,
            HOUSTONIA = 16,
            RED_TULIP = 17,
            ORANGE_TULIP = 18,
            WHITE_TULIP = 19,
            PINK_TULIP = 20,
            OXEYE_DAISY = 21,
            CACTUS = 22
        };

        Contents(EContents contents, const std::wstring& name);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;

        std::wstring mName;
        EContents mContents;
    };

    FlowerPotBlock();
    virtual ~FlowerPotBlock();
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    static void blockStaticCtor();
    virtual std::wstring getName();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex);
    boost::shared_ptr<class FlowerPotBlockEntity> getFlowerPotEntity(Level* level, const BlockPos& pos);
    bool canBePotted(const not_null_ptr<ItemInstance>& item);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state,
                     boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand,
                     const Direction* direction, float hitX, float hitY, float hitZ, bool clientSide);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos,
                                                             const BlockState* state);
    void CanSurvive(const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                 Block* block, const BlockPos& fromPos);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                   boost::shared_ptr<Player> player);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state,
                                                               LevelSource* levelSource,
                                                               const BlockPos& pos);
    virtual int getRenderLayer();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state,
                                               const BlockPos& pos, const Direction* direction);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual bool waterSpreadCausesSpawn();

    static EnumProperty<Contents*>* CONTENTS;
};

#endif
