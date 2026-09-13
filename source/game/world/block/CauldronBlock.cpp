#include "game/world/block/CauldronBlock.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mth.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/CauldronBlockEntity.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"

static AbstractProperty<int>* LEVEL = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"level", 0, 6));

CauldronBlock::CauldronBlock()
    : BaseEntityBlock(Material::METAL, MaterialColor::STONE),
      mInnerIcon(NULL),
      mTopIcon(NULL),
      mBottomIcon(NULL),
      mWaterIcon(NULL)
{
    DerivedInit();
    TypedBoxed<int> level(0);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(LEVEL, &level));
}
CauldronBlock::~CauldronBlock()
{
}

boost::shared_ptr<BlockEntity> CauldronBlock::newBlockEntity(Level*, int data)
{
    boost::shared_ptr<CauldronBlockEntity> result(new CauldronBlockEntity());
    result->setData(data);
    return result;
}

void CauldronBlock::addCollisionAABBs(const BlockState*, Level*, const BlockPos& pos, const AABB* box, std::vector<AABB*>* boxes, boost::shared_ptr<Entity>, bool)
{
    static const AABB inner(0.0, 0.0, 0.0, 1.0, 0.3125, 1.0);
    static const AABB north(0.0, 0.0, 0.0, 1.0, 1.0, 0.125);
    static const AABB south(0.0, 0.0, 0.875, 1.0, 1.0, 1.0);
    static const AABB west(0.0, 0.0, 0.125, 0.125, 1.0, 0.875);
    static const AABB east(0.875, 0.0, 0.125, 1.0, 1.0, 0.875);
    addCollisionAABB(pos, box, boxes, &inner); addCollisionAABB(pos, box, boxes, &north); addCollisionAABB(pos, box, boxes, &south); addCollisionAABB(pos, box, boxes, &west); addCollisionAABB(pos, box, boxes, &east);
}
const AABB* CauldronBlock::getShape(const BlockState* state, LevelSource* levelSource,
                                    const BlockPos& pos)
{
    return Block::getShape(state, levelSource, pos);
}

bool CauldronBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool CauldronBlock::isCubeShaped(const BlockState*)
{
    return false;
}

void CauldronBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                                 const boost::shared_ptr<Entity>& entity)
{
    Block::entityInside(level, pos, state, entity);
}

bool CauldronBlock::TestUse()
{
    return false;
}

void CauldronBlock::useInventory(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item, not_null_ptr<ItemInstance> replacement, int count)
{
    if (item.get() != NULL && replacement.get() != NULL)
        item->shrink(count);

    if (player.get() != NULL && replacement.get() != NULL && !player->addItem(replacement))
        player->drop(replacement, false);
}

void CauldronBlock::spawnCauldronEvent(Level* level, const BlockState*, const BlockPos& pos, int event) const
{
    if (level != NULL)
        level->levelEvent(event, pos, 0);
}

void CauldronBlock::explodeCauldronContents(Level* level, const BlockState* state, const BlockPos& pos)
{
    spawnCauldronEvent(level, state, pos, 0xdad);
}

void CauldronBlock::decrementStack(not_null_ptr<ItemInstance> item)
{
    if (item.get() != NULL)
        item->shrink(1);
}

bool CauldronBlock::use(Level* level, const BlockPos&, const BlockState* state,
                        boost::shared_ptr<Player>, InteractionHand::EInteractionHand,
                        const Direction*, float, float, float, bool)
{
    return level != NULL && state != NULL;
}

void CauldronBlock::setWaterLevel(Level* level, const BlockPos& pos, const BlockState*, int value)
{
    const BlockState* next = getBlockState(Mth::clamp(value, 0, 6));
    if (level != NULL && next != NULL) level->setBlock(pos, next, 2, false);
}
void CauldronBlock::handleRain(Level* level, const BlockPos& pos)
{
    if (level != NULL && level->mRandom != NULL && level->mRandom->nextInt(20) == 1)
    {
        const BlockState* state = level->getBlockState(pos);
        if (state != NULL && convertBlockStateToLegacyData(state) < 6)
            setWaterLevel(level, pos, state, convertBlockStateToLegacyData(state) + 1);
    }
}

Item* CauldronBlock::getResource(const BlockState*, Random*, int)
{
    return Items::cauldron;
}

not_null_ptr<ItemInstance> CauldronBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::cauldron));
}

bool CauldronBlock::hasAnalogOutputSignal(const BlockState*)
{
    return true;
}

int CauldronBlock::getAnalogOutputSignal(const BlockState* state, Level*, const BlockPos&)
{
    return (convertBlockStateToLegacyData(state) + 1) * 3;
}

const BlockState* CauldronBlock::getBlockState(int data)
{
    TypedBoxed<int> level(data);
    return defaultBlockState()->setBoxedValue(LEVEL, &level);
}

int CauldronBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(LEVEL);
}

unsigned int CauldronBlock::GetInteractTooltip(const BlockTooltipDataHolder& data)
{
    return Block::GetInteractTooltip(data);
}

bool CauldronBlock::canProvideSupport(const BlockState*, const Direction* direction, EBlockSupportType type)
{
    return direction == Direction::DOWN && (type == BLOCK_SUPPORT_EDGE || type == BLOCK_SUPPORT_ANY);
}

BlockStateDefinition* CauldronBlock::createBlockStateDefinition()
{
    const Property* properties[] = { LEVEL };
    return new BlockStateDefinition(this, properties, 1);
}

bool CauldronBlock::isPathfindable(LevelSource*, const BlockPos&)
{
    return true;
}
EBlockFaceShape CauldronBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&,
                                                 const Direction* direction)
{
    if (direction == Direction::DOWN)
        return BLOCK_FACE_BOWL;
    if (direction == Direction::UP)
        return BLOCK_FACE_UNDEFINED;
    return BLOCK_FACE_SOLID;
}

TextureAtlasSprite* CauldronBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction == Direction::UP)
        return mTopIcon;
    if (direction == Direction::DOWN)
        return mBottomIcon;
    return mIcon;
}

void CauldronBlock::registerIcons(IconRegister* icons)
{
    if (icons == NULL)
        return;

    mInnerIcon = icons->registerIcon(L"cauldron_inner");
    mTopIcon = icons->registerIcon(L"cauldron_top");
    mBottomIcon = icons->registerIcon(L"cauldron_bottom");
    mIcon = icons->registerIcon(L"cauldron_side");
    mWaterIcon = icons->registerIcon(L"cauldron_water");
}
TextureAtlasSprite* CauldronBlock::getTexture(const std::wstring& name)
{
    CauldronBlock* cauldron = dynamic_cast<CauldronBlock*>(Blocks::cauldron);
    if (cauldron == NULL) return NULL;
    if (name == L"cauldron_inner") return cauldron->mInnerIcon;
    if (name == L"cauldron_bottom") return cauldron->mBottomIcon;
    if (name == L"cauldron_water") return cauldron->mWaterIcon;
    return NULL;
}
EBlockRenderShape CauldronBlock::getRenderShape(const BlockState*) { return (EBlockRenderShape)24; }
void CauldronBlock::spawnPotionParticles(Level* level, double x, double y, double z, Random*, int color, int count) { if (level != NULL) level->levelEvent(count + 2000, BlockPos((int)x, (int)y, (int)z), color); }
void CauldronBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random) { if (level != NULL && random != NULL && random->nextInt(10) == 0) level->levelEvent(2000, pos, 0); }
void CauldronBlock::spawnSplashParticles(Level* level, double x, double y, double z, Random*, int color, int count) { if (level != NULL) level->levelEvent(count + 2001, BlockPos((int)x, (int)y, (int)z), color); }
void CauldronBlock::spawnBubbleParticles(Level* level, double x, double y, double z, Random*, int color, int count) { if (level != NULL) level->levelEvent(count + 2002, BlockPos((int)x, (int)y, (int)z), color); }
void CauldronBlock::spawnExplosionParticles(Level* level, double x, double y, double z, Random*, int color, int count) { if (level != NULL) level->levelEvent(count + 2003, BlockPos((int)x, (int)y, (int)z), color); }
EPistonPushReaction CauldronBlock::getPistonPushReaction(const BlockState*) { return PISTON_PUSH_NORMAL; }
bool CauldronBlock::ParticlesSurviveWithin() { return true; }
