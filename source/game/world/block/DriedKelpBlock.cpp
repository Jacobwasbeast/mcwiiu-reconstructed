#include "game/world/block/DriedKelpBlock.hpp"

#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/core/Direction.hpp"
#include "game/core/FlippedIcon.hpp"
#include "game/core/IconRegister.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

DriedKelpBlock::DriedKelpBlock(Material* material)
    : Block(material)
{
    DerivedInit();
    setTicking(true);
    mSideIcon = NULL;
    mBottomIcon = NULL;
}

DriedKelpBlock::~DriedKelpBlock()
{
}

void DriedKelpBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos& fromPos)
{
    if (!level->mIsClientSide
        && level->getBlockId(fromPos.x, fromPos.y + 1, fromPos.z) == 51
        && level->isUnderWater(pos, 269, 0))
    {
        level->removeBlock(pos.above());
    }
}

void DriedKelpBlock::tick(Level*, const BlockPos&, const BlockState*, Random*)
{
}

EPistonPushReaction DriedKelpBlock::getPistonPushReaction(const BlockState*)
{
    return PISTON_PUSH_NORMAL;
}

TextureAtlasSprite* DriedKelpBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction == Direction::NORTH || direction == Direction::WEST)
    {
        return mSideIcon;
    }
    if (direction == Direction::SOUTH || direction == Direction::EAST)
    {
        return mBottomIcon;
    }
    return mTopIcon;
}

void DriedKelpBlock::registerIcons(IconRegister* iconRegister)
{
    mSideIcon = iconRegister->registerIcon(L"dried_kelp_side");
    mTopIcon = iconRegister->registerIcon(L"dried_kelp_top");
    mBottomIcon = new FlippedIcon(mSideIcon, true, false);
}
