#ifndef GAME_WORLD_BLOCK_LEVERBLOCK_HPP
#define GAME_WORLD_BLOCK_LEVERBLOCK_HPP
#include <string>
#include <boost/shared_ptr.hpp>
#include "game/world/block/Block.hpp"
#include "game/core/Direction.hpp"
class AABB; class BlockPos; class BlockState; class BlockStateDefinition; class BlockTooltipDataHolder; class Direction; class Level; class LevelSource; class LivingEntity; class Material; class Mirror; class Player; class Rotation;
template <class T> class AbstractProperty;
class LeverBlock : public Block
{
public:
    class LeverDirection : public StringRepresentable
    {
    public:
        static LeverDirection* DOWN_X; static LeverDirection* EAST; static LeverDirection* WEST; static LeverDirection* SOUTH; static LeverDirection* NORTH; static LeverDirection* UP_Z; static LeverDirection* UP_X; static LeverDirection* DOWN_Z; static LeverDirection* BY_DATA[8];
        LeverDirection(int data, const std::wstring& name, const Direction* direction);
        int getData(); static void staticCtor(); static LeverDirection* fromFacing(const Direction* face, const Direction* direction); const Direction* getDirection(); static LeverDirection* byData(int data);
        virtual std::wstring toString() const; virtual std::wstring getSerializedName() const; std::wstring getName();
    private: int mData; std::wstring mName; const Direction* mDirection;
    };
    static AbstractProperty<LeverDirection*>* FACING; static AbstractProperty<bool>* POWERED;
    static const AABB* AABB_DOWN_X; static const AABB* AABB_UP_X; static const AABB* AABB_NORTH; static const AABB* AABB_SOUTH; static const AABB* AABB_WEST; static const AABB* AABB_EAST; static const AABB* AABB_UP_Z; static const AABB* AABB_DOWN_Z;
    LeverBlock(); virtual ~LeverBlock(); static void blockStaticCtor();
    virtual const AABB* getClipAABB(const BlockState*, LevelSource*, const BlockPos&); virtual bool isSolidRender(const BlockState*) const; virtual bool isCubeShaped(const BlockState*); static bool canAttachTo(Level*, const BlockPos&, const Direction*); virtual bool mayPlace(Level*, const BlockPos&, const Direction*); virtual bool mayPlace(Level*, const BlockPos&);
    virtual const BlockState* getStateForPlacement(Level*, const BlockPos&, const Direction*, float, float, float, int, boost::shared_ptr<LivingEntity>); static int getLeverFacing(const Direction*); virtual void neighborChanged(const BlockState*, Level*, const BlockPos&, Block*, const BlockPos&); virtual bool canSurvive(Level*, const BlockPos&, const BlockState*); virtual int getSpawnResourcesAuxValue(const BlockState*); virtual const AABB* getShape(const BlockState*, LevelSource*, const BlockPos&); virtual bool TestUse();
    virtual bool use(Level*, const BlockPos&, const BlockState*, boost::shared_ptr<Player>, InteractionHand::EInteractionHand, const Direction*, float, float, float, bool); virtual void onRemove(Level*, const BlockPos&, const BlockState*); virtual int getSignal(const BlockState*, LevelSource*, const BlockPos&, const Direction*); virtual int getDirectSignal(const BlockState*, LevelSource*, const BlockPos&, const Direction*); virtual bool isSignalSource(const BlockState*); virtual const BlockState* getBlockState(int); virtual int convertBlockStateToLegacyData(const BlockState*); virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder&); virtual const BlockState* mirror(const BlockState*, Mirror*); virtual const BlockState* rotate(const BlockState*, Rotation*); virtual BlockStateDefinition* createBlockStateDefinition(); virtual EBlockFaceShape getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*); virtual EBlockRenderShape getRenderShape(const BlockState*);
};
#endif
