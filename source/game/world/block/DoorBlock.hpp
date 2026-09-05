#ifndef GAME_WORLD_BLOCK_DOORBLOCK_HPP
#define GAME_WORLD_BLOCK_DOORBLOCK_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include "game/core/Direction.hpp"
#include "game/world/block/Block.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Material;
class MaterialColor;
class Mirror;
class Player;
class Random;
class Rotation;
class TextureAtlasSprite;

class DoorBlock : public Block
{
public:
    class DoorHalf : public StringRepresentable
    {
    public:
        enum { HALF_COUNT = 2 };
        DoorHalf(const std::wstring& name) : mName(name) {}
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring mName;
    };

    class HingeSide : public StringRepresentable
    {
    public:
        enum { HINGE_COUNT = 2 };
        HingeSide(const std::wstring& name) : mName(name) {}
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring mName;
    };

    static AbstractProperty<DoorHalf*>* HALF;
    static AbstractProperty<HingeSide*>* HINGE;
    static Property* OPEN;
    static Property* POWERED;
    static AbstractProperty<Direction*>* FACING;
    static DoorHalf* LOWER;
    static DoorHalf* UPPER;
    static HingeSide* LEFT;
    static HingeSide* RIGHT;

    DoorBlock(Material* material);
    virtual ~DoorBlock();
    static void blockStaticCtor();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual std::wstring getName();
    virtual bool isSolidRender(const BlockState* state) const;
    static bool isOpenFromData(int data);
    static bool isUpperFromData(int data);
    static int getLowerData(int data);
    static int getCompositeData(LevelSource* levelSource, const BlockPos& pos);
    virtual bool isPathfindable(LevelSource* levelSource, const BlockPos& pos);
    virtual bool isCubeShaped(const BlockState* state);
    int getCloseSound();
    int getOpenSound();
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool TestUse();
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY, float hitZ, bool clientSide);
    void setOpen(Level* level, const BlockPos& pos, bool open);
    bool CanSurvive(const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block, const BlockPos& fromPos);
    Item* getItem();
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player);
    virtual int getRenderLayer();
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    static bool isOpen(LevelSource* levelSource, const BlockPos& pos);
    static const Direction* getFacingFromData(int data);
    static const Direction* getFacing(LevelSource* levelSource, const BlockPos& pos);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos, const Direction* direction);
    static bool isDoor(Block* block);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual TextureAtlasSprite* getTexture(LevelSource* levelSource, const BlockPos& pos, const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    static bool isPoweredFromData(int data);
    static bool isPowered(LevelSource* levelSource, const BlockPos& pos);
    static bool hasRightHingeFromData(int data);

    int mDoorData;
    TextureAtlasSprite* mUpperIcon;
    TextureAtlasSprite* mFlippedUpperIcon;
    TextureAtlasSprite* mLowerIcon;
    TextureAtlasSprite* mFlippedLowerIcon;
};

#endif
