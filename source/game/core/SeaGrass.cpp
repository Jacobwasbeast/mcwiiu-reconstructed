#include "game/core/SeaGrass.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

static AbstractProperty<int>* SEAGRASS_TYPE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"type", 0, 2));

SeaGrass::SeaGrass()
    : Bush(Material::PLANT), mCarriedIcon(NULL), mDoubleTallTopIcon(NULL), mDoubleTallBottomIcon(NULL)
{
    DerivedInit();
    TypedBoxed<int> type(0);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(SEAGRASS_TYPE, &type));
    setTicking(true);
}
SeaGrass::~SeaGrass() {}
bool SeaGrass::isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool) { return true; }
bool SeaGrass::isBonemealSuccess(Level* l, Random* r, const BlockPos& p, const BlockState* s) { return trySpawnSeaGrass(true, l, r, p, s); }
void SeaGrass::performBonemeal(Level* l, Random* r, const BlockPos& p, const BlockState* s) { trySpawnSeaGrass(false, l, r, p, s); }
void SeaGrass::tick(Level* level, const BlockPos& pos, const BlockState*, Random*)
{
    if (!level->mIsClientSide)
        level->addToTickNextTick(pos, defaultBlockState()->getBlock(), 1);
}
void SeaGrass::registerIcons(IconRegister* r)
{
    mIcon = r->registerIcon(L"seagrass");
    mCarriedIcon = r->registerIcon(L"seagrass_carried");
    mDoubleTallTopIcon = r->registerIcon(L"seagrass_doubletall_top");
    mDoubleTallBottomIcon = r->registerIcon(L"seagrass_doubletall_bottom");
}
TextureAtlasSprite* SeaGrass::getTexture(const Direction* d, const BlockState* s)
{
    if (d == Direction::UP)
        return mCarriedIcon;

    int n = convertBlockStateToLegacyData(s);
    if (n == 1)
        return mDoubleTallTopIcon;
    if (n == 2)
        return mDoubleTallBottomIcon;
    return mIcon;
}
EBlockRenderShape SeaGrass::getRenderShape(const BlockState*) { return static_cast<EBlockRenderShape>(52); }
int SeaGrass::getColor(const BlockState* s) { return Block::getColor(s); }
int SeaGrass::getColor(LevelSource* l, const BlockPos& p, const BlockState*) { return Block::getColor(l, p); }
const MaterialColor* SeaGrass::getMapColor(const BlockState* s, LevelSource* l, const BlockPos& p) { return Block::getMapColor(s, l, p); }
Item* SeaGrass::getResource(const BlockState*, Random*, int) { return NULL; }
const BlockState* SeaGrass::getBlockState(int data) { TypedBoxed<int> type(data); return defaultBlockState()->setBoxedValue(SEAGRASS_TYPE, &type); }
int SeaGrass::convertBlockStateToLegacyData(const BlockState* s) { return s->getValue(SEAGRASS_TYPE); }
unsigned int SeaGrass::GetInteractTooltip(const BlockTooltipDataHolder& h) { return Block::GetInteractTooltip(h); }
EBlockOffsetType SeaGrass::getOffsetType() { return BLOCK_OFFSET_XZ; }
const AABB* SeaGrass::getShape(const BlockState*, LevelSource*, const BlockPos&) { return NULL; }
void SeaGrass::neighborChanged(const BlockState* s, Level* l, const BlockPos& p, Block*, const BlockPos&)
{
    if (!l->mIsClientSide && !canSurvive(l, p, s))
        l->addToTickNextTick(p, defaultBlockState()->getBlock(), 2);
}
bool SeaGrass::mayPlace(Level* l, const BlockPos& p, bool client)
{
    const BlockState* s = l->getBlockState(p.below());
    if (s->getBlock() == NULL || s->getBlock()->isLiquidBlock()
        || !l->isSolidBlockingBlock(p.below()))
        return false;
    if (client)
        return true;
    if (!l->isUnderWater(p, 270, 0))
        return false;
    const BlockState* e = l->getExtraBlockState(p);
    return e->mElementId == 0 || e->mElementId > 7;
}

bool SeaGrass::canSurvive(Level* l, const BlockPos& p, const BlockState* s, bool client)
{
    if (l->getBlockState(p)->getBlock() != this)
        return mayPlace(l, p, client);
    if (!l->isUnderWater(p, 270, convertBlockStateToLegacyData(s)))
        return false;

    int n = convertBlockStateToLegacyData(s);
    if (n == 1)
    {
        return l->getBlockState(p.below())->getBlock() == this
            && convertBlockStateToLegacyData(l->getBlockState(p.below())) == 2;
    }
    if (n == 2)
    {
        return l->getBlockState(p.above())->getBlock() == this
            && convertBlockStateToLegacyData(l->getBlockState(p.above())) == 1
            && l->isSolidBlockingBlock(p.below());
    }
    return mayPlace(l, p, client);
}
bool SeaGrass::canSurvive(Level* l, const BlockPos& p, const BlockState* s) { return canSurvive(l, p, s, false); }
bool SeaGrass::mayPlace(Level* l, const BlockPos& p) { return mayPlace(l, p, false); }
void SeaGrass::onPlace(Level* l, const BlockPos& p, const BlockState*)
{
    if (l->getBlockState(p)->getBlock() == Blocks::double_plant)
        l->setExtraBlock(p, Blocks::double_plant->defaultBlockState(), 3, false);
}
void SeaGrass::playerDestroy(Level* l, boost::shared_ptr<Player>, const BlockPos& p, const BlockState* s,
                             boost::shared_ptr<BlockEntity>, not_null_ptr<ItemInstance> t)
{
    if (t->isEmpty() || Item::getId(t->getItem()) != 0x167)
        return;

    int n = convertBlockStateToLegacyData(s) == 0 ? 1 : 2;
    not_null_ptr<ItemInstance> drop(new ItemInstance(Item::byBlock(Blocks::double_plant), 2, n));
    Block::popResource(l, p, drop);
}

bool SeaGrass::trySpawnSeaGrass(bool check, Level* l, Random* r, const BlockPos& p, const BlockState*)
{
    if (!l->isUnderWater(p.above(), 270, 0))
        return false;

    BlockPos target = p.above();
    for (int i = 0; i < 3 && l->getBlockState(target)->getBlock() == Blocks::sea_grass; ++i)
        target = target.above();
    if (!l->isEmptyWaterBlock(target))
        return false;

    SeaGrass* seaGrass = static_cast<SeaGrass*>(Blocks::sea_grass);
    const BlockState* state = seaGrass->getBlockState(1 + (r->nextInt(8) == 0 ? 1 : 0));
    if (check)
        return true;

    l->setBlockAndUpdate(target, state);
    l->setBlockAndUpdate(target.above(), seaGrass->getBlockState(2));
    return true;
}
BlockStateDefinition* SeaGrass::createBlockStateDefinition()
{
    const Property* properties[] = { SEAGRASS_TYPE };
    return new BlockStateDefinition(this, properties, 1);
}
bool SeaGrass::isSilkTouchable() { return false; }
