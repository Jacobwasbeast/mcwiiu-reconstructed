#ifndef GAME_WORLD_BLOCK_CHORUSFLOWERBLOCK_HPP
#define GAME_WORLD_BLOCK_CHORUSFLOWERBLOCK_HPP

#include "game/world/block/Block.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Random;
class TextureAtlasSprite;

class ChorusFlowerBlock : public Block
{
public:
    ChorusFlowerBlock();
    virtual ~ChorusFlowerBlock();
    static const AABB* getUpShape();
    static const AABB* getDownShape();
    static const AABB* getSouthShape();
    static const AABB* getNorthShape();
    static const AABB* getEastShape();
    static const AABB* getWestShape();
    virtual void DerivedInit();
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    bool isDead(int age);
    static bool allNeighborsEmpty(Level* level, const BlockPos& pos, const Direction* excluded);
    void placeGrownFlower(Level* level, const BlockPos& pos, int age);
    void placeDeadFlower(Level* level, const BlockPos& pos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block, const BlockPos& fromPos);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos, const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity, not_null_ptr<ItemInstance> tool);
    virtual int getRenderLayer();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    static void growTreeRecursive(Level* level, const BlockPos& pos, Random* random, const BlockPos& origin, int radius, int depth);
    static void generatePlant(Level* level, BlockPos pos, Random* random, int radius);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction);
    void setCulledFace(const Direction* direction);
    virtual bool isWaterBlocking() const;
    virtual bool waterSpreadCausesSpawn();
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual bool canProvideSupport(const BlockState* state, const Direction* direction, EBlockSupportType supportType);
    virtual not_null_ptr<ItemInstance> getSilkTouchItemInstance(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos, const Direction* direction);

private:
    TextureAtlasSprite* mDeadIcon;
    TextureAtlasSprite* mPlantIcon;
    const Direction* mCulledFace;
};

#endif
