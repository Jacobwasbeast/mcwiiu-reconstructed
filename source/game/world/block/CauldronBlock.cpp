#include "game/world/block/CauldronBlock.hpp"
#include "game/item/ItemId.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/GenericStats.hpp"
#include "game/entity/mob/HorseArmorItem.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionItem.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/Potions.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mth.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
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
#include "game/world/generation/MultiPlayerLevel.hpp"

const std::wstring CauldronBlock::TEXTURE_INNER = L"cauldron_inner";
const std::wstring CauldronBlock::TEXTURE_BOTTOM = L"cauldron_bottom";
const std::wstring CauldronBlock::TEXTURE_WATER = L"cauldron_water";
AbstractProperty<int>* CauldronBlock::LEVEL =
    static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"level", 0, 6));

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
    addCollisionAABB(pos, box, boxes, &inner);
    addCollisionAABB(pos, box, boxes, &north);
    addCollisionAABB(pos, box, boxes, &south);
    addCollisionAABB(pos, box, boxes, &west);
    addCollisionAABB(pos, box, boxes, &east);
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
    int waterLevel = convertBlockStateToLegacyData(state);
    float waterHeight = ((float)(waterLevel * 2) + 5.0f) * 0.0625f;
    if (!level->mIsClientSide && entity->isOnFire() && 0 < waterLevel)
    {
        if (entity->mBoundingBox->mMinY <= (double)(waterHeight + (float)pos.y))
        {
            entity->clearFire();
            setWaterLevel(level, pos, state, waterLevel - 1);
        }
    }
}

bool CauldronBlock::TestUse()
{
    return false;
}

void CauldronBlock::useInventory(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item,
                                 not_null_ptr<ItemInstance> replacement, int count)
{
    if (!player->mAbilities.mInstabuild)
        item->shrink(count);
    if (!player->addItem(replacement))
        player->drop(replacement, false);
}

void CauldronBlock::spawnCauldronEvent(Level* level, const BlockState*, const BlockPos& pos, int event) const
{
    if (level->mIsClientSide)
        return;
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity != NULL && blockEntity->GetType() == BlockEntityType::CAULDRON)
    {
        boost::shared_ptr<CauldronBlockEntity> cauldron = boost::static_pointer_cast<CauldronBlockEntity>(blockEntity);
        level->levelEvent(event, pos, cauldron->getColor());
    }
}

void CauldronBlock::explodeCauldronContents(Level* level, const BlockState* state, const BlockPos& pos)
{
    setWaterLevel(level, pos, state, 0);
    spawnCauldronEvent(level, state, pos, LevelEvent::CAULDRON_EXPLODE);
}

void CauldronBlock::decrementStack(not_null_ptr<ItemInstance> item)
{
    item->shrink(1);
}

bool CauldronBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                        InteractionHand::EInteractionHand hand, const Direction*, float, float, float, bool clientSide)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity == NULL || blockEntity->GetType() != BlockEntityType::CAULDRON)
        return false;
    boost::shared_ptr<CauldronBlockEntity> cauldron = boost::static_pointer_cast<CauldronBlockEntity>(blockEntity);
    if (clientSide)
        return false;

    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (itemInstance->isEmpty())
        return true;

    int waterLevel = state->getValue(LEVEL);
    bool full = waterLevel > 5;
    bool empty = waterLevel == 0;
    bool hasPotion = !empty && cauldron->getPotionId().compare(L"") != 0;
    bool hasWater = !empty && !hasPotion;
    bool hasClearWater = hasWater && !cauldron->mHasCustomColor;
    bool used = false;

    Item* item = itemInstance->getItem();
    if (item == Items::waterBucket)
    {
        if (hasWater || empty)
        {
            if (full && hasClearWater)
                return true;
            if (!player->mAbilities.mInstabuild)
                useInventory(player, itemInstance, not_null_ptr<ItemInstance>(new ItemInstance(Items::bucket)), 1);
            cauldron->setPotionId(L"");
            cauldron->setCustomColor(0);
            setWaterLevel(level, pos, state, 5);
            setWaterLevel(level, pos, state, 6);
            spawnCauldronEvent(level, state, pos, LevelEvent::CAULDRON_FILL_WATER);
            used = true;
        }
        else if (hasPotion)
        {
            if (!player->mAbilities.mInstabuild)
                useInventory(player, itemInstance, not_null_ptr<ItemInstance>(new ItemInstance(Items::bucket)), 1);
            explodeCauldronContents(level, state, pos);
            used = true;
        }
        return used;
    }

    if (item == Items::bucket)
    {
        if (full && hasWater)
        {
            useInventory(player, itemInstance, not_null_ptr<ItemInstance>(new ItemInstance(Items::waterBucket)), 1);
            cauldron->setPotionId(L"");
            cauldron->setCustomColor(0);
            setWaterLevel(level, pos, state, 0);
            spawnCauldronEvent(level, state, pos, LevelEvent::CAULDRON_TAKE_WATER);
            used = true;
        }
        return used;
    }

    if (item == Items::glassBottle && waterLevel > 1)
    {
        if (hasWater)
        {
            not_null_ptr<ItemInstance> waterPotion(new ItemInstance(Items::potion));
            PotionUtils::setPotion(waterPotion, Potions::WATER);
            useInventory(player, itemInstance, waterPotion, 1);
            spawnCauldronEvent(level, state, pos, LevelEvent::CAULDRON_TAKE_WATER);
        }
        else if (hasPotion)
        {
            not_null_ptr<ItemInstance> potion(new ItemInstance(Potion::getBasePotion(cauldron->getPotionType())));
            PotionUtils::setPotion(potion, Potion::byName(cauldron->getPotionId()));
            useInventory(player, itemInstance, potion, 1);
            spawnCauldronEvent(level, state, pos, LevelEvent::CAULDRON_TAKE_POTION);
        }
        if (waterLevel < 3)
        {
            cauldron->setPotionId(L"");
            cauldron->setCustomColor(0);
        }
        setWaterLevel(level, pos, state, waterLevel - 2);
        return true;
    }

    if (item == Items::arrow && itemInstance->getAuxValue() == 0 && !empty && hasPotion
        && cauldron->getPotionId().compare(L"") != 0 && cauldron->getPotionId().compare(L"empty") != 0
        && cauldron->getPotionId().compare(L"water") != 0 && cauldron->getPotionId().compare(L"mundane") != 0
        && cauldron->getPotionId().compare(L"thick") != 0 && cauldron->getPotionId().compare(L"awkward") != 0)
    {
        int stacks = 1;
        if (waterLevel > 2)
            stacks = waterLevel - 2;
        if (stacks < 0)
            stacks = 0;
        int count = stacks * 16;
        if (itemInstance->getCount() < count)
            count = itemInstance->getCount();
        waterLevel -= (count + 15) / 16;

        not_null_ptr<ItemInstance> tippedArrows(new ItemInstance(Items::tippedArrow, count));
        PotionUtils::setPotion(tippedArrows, Potion::byName(cauldron->getPotionId()));
        useInventory(player, itemInstance, tippedArrows, count);
        if (waterLevel < 3)
            cauldron->setPotionId(L"");
        if (waterLevel < 3)
            waterLevel = 0;
        setWaterLevel(level, pos, state, waterLevel);
        return true;
    }

    if (item == Items::potion || item == Items::splashPotion || item == Items::lingeringPotion)
    {
        std::wstring potionId = PotionUtils::getPotionID(itemInstance);
        if (potionId.compare(L"water") == 0)
        {
            if (hasWater || empty)
            {
                if (!full || !hasClearWater)
                {
                    if (!player->mAbilities.mInstabuild)
                        useInventory(player, itemInstance,
                                     not_null_ptr<ItemInstance>(new ItemInstance(Items::glassBottle, 1)), 1);
                    cauldron->setPotionId(L"");
                    cauldron->setCustomColor(0);
                    setWaterLevel(level, pos, state, waterLevel + 2);
                    spawnCauldronEvent(level, state, pos, LevelEvent::CAULDRON_FILL_WATER);
                    used = true;
                }
            }
            else if (hasPotion)
            {
                if (!player->mAbilities.mInstabuild)
                    useInventory(player, itemInstance,
                                 not_null_ptr<ItemInstance>(new ItemInstance(Items::glassBottle, 1)), 1);
                explodeCauldronContents(level, state, pos);
                used = true;
            }
        }
        else if (potionId.compare(L"") != 0 && potionId.compare(L"empty") != 0)
        {
            bool samePotion = hasPotion && cauldron->getPotionId().compare(potionId) == 0;
            if (empty || (samePotion && !full))
            {
                cauldron->setPotionId(potionId);
                cauldron->setPotionType(static_cast<Potion::EType>(static_cast<PotionItem*>(item)->getPotionType()));
                setWaterLevel(level, pos, state, waterLevel + 2);
                if (!player->mAbilities.mInstabuild)
                    useInventory(player, itemInstance,
                                 not_null_ptr<ItemInstance>(new ItemInstance(Items::glassBottle, 1)), 1);
                spawnCauldronEvent(level, state, pos, LevelEvent::CAULDRON_FILL_POTION);
                used = true;
            }
            else if (!samePotion)
            {
                if (!player->mAbilities.mInstabuild)
                    useInventory(player, itemInstance,
                                 not_null_ptr<ItemInstance>(new ItemInstance(Items::glassBottle, 1)), 1);
                explodeCauldronContents(level, state, pos);
                cauldron->setPotionId(L"");
                cauldron->setCustomColor(0);
                used = true;
            }
        }
        return used;
    }

    if (empty || hasPotion)
        return false;

    ArmorItem* armor = dynamic_cast<ArmorItem*>(item);
    if (armor != NULL)
    {
        if (armor->getMaterial() == ArmorItem::ArmorMaterial::LEATHER && armor->mRenderIndex == 0)
        {
            if (hasClearWater)
            {
                bool drained = false;
                if (armor->hasCustomColor(itemInstance))
                {
                    armor->clearColor(itemInstance);
                    setWaterLevel(level, pos, state, waterLevel - 1);
                    drained = waterLevel < 2;
                    spawnCauldronEvent(level, state, pos, LevelEvent::CAULDRON_CLEAN_ARMOR);
                    used = true;
                }
                if (!drained)
                    return used;
            }
            else
            {
                armor->setColor(itemInstance, cauldron->getCustomColor());
                setWaterLevel(level, pos, state, waterLevel - 1);
                spawnCauldronEvent(level, state, pos, LevelEvent::CAULDRON_DYE_ARMOR);
                used = true;
                player->awardStat(gStats->get_armourDyed(Item::VALIDgetId(item)), GenericStats::param_noArgs());
                if (waterLevel > 1)
                    return used;
            }
            cauldron->setPotionId(L"");
            cauldron->setCustomColor(0);
        }
        return used;
    }

    if (Item::getId(item) == ItemId::LEATHER_HORSE_ARMOR)
    {
        HorseArmorItem* horseArmor = static_cast<HorseArmorItem*>(item);
        if (hasClearWater)
        {
            if (!horseArmor->hasCustomColor(itemInstance))
                return false;
            horseArmor->clearColor(itemInstance);
            setWaterLevel(level, pos, state, waterLevel - 1);
            spawnCauldronEvent(level, state, pos, LevelEvent::CAULDRON_CLEAN_ARMOR);
        }
        else
        {
            horseArmor->setColor(itemInstance, cauldron->getCustomColor());
            setWaterLevel(level, pos, state, waterLevel - 1);
            spawnCauldronEvent(level, state, pos, LevelEvent::CAULDRON_DYE_ARMOR);
        }
        if (waterLevel < 2)
        {
            cauldron->setPotionId(L"");
            cauldron->setCustomColor(0);
        }
        return true;
    }

    if (item == Items::dye)
    {
        not_null_ptr<ItemInstance> dye = itemInstance->copy();
        dye->setCount(1);
        if (!player->isCreative())
            decrementStack(itemInstance);
        cauldron->setItem(2, dye);
        cauldron->mixDyes();
        setWaterLevel(level, pos, state, waterLevel - 1);
        setWaterLevel(level, pos, state, waterLevel);
        spawnCauldronEvent(level, state, pos, LevelEvent::CAULDRON_ADD_DYE);
        return true;
    }

    if (item != Items::banner || BannerBlockEntity::getPatternCount(itemInstance) <= 0 || level->mIsClientSide)
        return false;

    not_null_ptr<ItemInstance> banner = itemInstance->copy();
    banner->setCount(1);
    BannerBlockEntity::removeLastPattern(banner);
    if (!player->mAbilities.mInstabuild)
    {
        itemInstance->shrink(1);
        setWaterLevel(level, pos, state, waterLevel - 1);
    }
    if (!itemInstance->isEmpty())
    {
        if (player->mInventory->add(banner))
        {
            if (player->GetType() == EntityType::SERVER_PLAYER)
                boost::static_pointer_cast<ServerPlayer>(player)->refreshContainer(player->mInventoryMenu);
        }
        else
        {
            player->drop(banner, false);
        }
    }
    else
    {
        player->setItemInHand(hand, banner);
    }
    spawnCauldronEvent(level, state, pos, LevelEvent::CAULDRON_CLEAN_BANNER);
    return true;
}

void CauldronBlock::setWaterLevel(Level* level, const BlockPos& pos, const BlockState*, int value)
{
    const BlockState* next = getBlockState(Mth::clamp(value, 0, 6));
    if (level != NULL && next != NULL) level->setBlock(pos, next, Block::UPDATE_CLIENTS, false);
}
void CauldronBlock::handleRain(Level* level, const BlockPos& pos)
{
    if (level->mRandom->nextInt(20) == 1)
    {
        float temperature = level->getBiome(pos)->getTemperature(pos);
        if (level->getBiomeSource()->scaleTemp(temperature, pos.y) >= 0.15f)
        {
            const BlockState* state = level->getBlockState(pos);
            if (state->getValue(LEVEL) < 6)
            {
                level->setBlock(pos, state->cycle(LEVEL), Block::UPDATE_CLIENTS, false);
            }
        }
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
    boost::shared_ptr<BlockEntity> blockEntity = data.m_pLevel->getBlockEntity(data.m_blockPos);
    if (blockEntity == NULL || blockEntity->GetType() != BlockEntityType::CAULDRON)
        return IDS_NONE;

    boost::shared_ptr<CauldronBlockEntity> cauldron = boost::static_pointer_cast<CauldronBlockEntity>(blockEntity);
    int waterLevel = data.m_pBlockState->getValue(LEVEL);
    bool full = waterLevel > 5;
    bool empty = waterLevel == 0;
    bool hasPotion = !empty && cauldron->getPotionId().compare(L"") != 0;
    bool hasWater = !empty && !hasPotion;
    bool hasClearWater = hasWater && !cauldron->mHasCustomColor;
    unsigned int defaultTooltip = Block::GetInteractTooltip(data);

    switch (Item::getId(data.m_pItemInstance->getItem()))
    {
    case 262:
    {
        bool canTip = false;
        if (data.m_bUseBlock && data.m_bCanPlace && data.m_pItemInstance->getAuxValue() == 0 && !empty && hasPotion
            && cauldron->getPotionId().compare(L"") != 0 && cauldron->getPotionId().compare(L"empty") != 0
            && cauldron->getPotionId().compare(L"water") != 0 && cauldron->getPotionId().compare(L"mundane") != 0
            && cauldron->getPotionId().compare(L"thick") != 0 && cauldron->getPotionId().compare(L"awkward") != 0)
        {
            canTip = true;
        }
        if (!canTip)
            return IDS_NONE;
        return data.m_pItemInstance->getCount() == 1 ? 0x3E95FA83u : 0x5401ED31u;
    }
    case 298:
    case 299:
    case 300:
    case 301:
        if (data.m_bUseBlock && data.m_bCanPlace && !empty && hasWater)
        {
            ArmorItem* armor = static_cast<ArmorItem*>(data.m_pItemInstance->getItem());
            if (armor->mRenderIndex != 0)
                return IDS_NONE;
            if (!hasClearWater)
                return IDS_TOOLTIPS_DYE;
            if (armor->hasCustomColor(data.m_pItemInstance))
                return IDS_TOOLTIPS_CLEAN;
        }
        return IDS_NONE;
    case 325:
        if (data.m_bUseBlock && data.m_bCanPlace && full && hasWater)
            return IDS_TOOLTIPS_COLLECT;
        return IDS_NONE;
    case 326:
        if (!data.m_bUseBlock || !data.m_bCanPlace
            || data.m_eInteractionResult == InteractionResult::eInteractionResult_Fail || defaultTooltip != IDS_TOOLTIPS_EMPTY)
        {
            return defaultTooltip;
        }
        if (hasWater || empty)
        {
            if (!hasClearWater && !empty)
                return IDS_TOOLTIPS_FLUSH;
            if (!full)
                return IDS_TOOLTIPS_FILL;
        }
        else if (hasPotion)
        {
            return IDS_TOOLTIPS_MIX;
        }
        return IDS_NONE;
    case 351:
        if (data.m_bUseBlock && data.m_bCanPlace && !empty && hasWater)
            return IDS_TOOLTIPS_MIX;
        return IDS_NONE;
    case 374:
        if (data.m_bUseBlock && data.m_bCanPlace && waterLevel >= 2)
            return IDS_TOOLTIPS_COLLECT;
        return IDS_NONE;
    case 425:
        if (data.m_bUseBlock && data.m_bCanPlace && !empty && hasWater
            && BannerBlockEntity::getPatternCount(data.m_pItemInstance) >= 1)
        {
            return IDS_TOOLTIPS_CLEAN;
        }
        return defaultTooltip;
    case 453:
        if (data.m_bUseBlock && data.m_bCanPlace && !empty && hasWater)
        {
            HorseArmorItem* horseArmor = static_cast<HorseArmorItem*>(data.m_pItemInstance->getItem());
            if (!hasClearWater)
                return IDS_TOOLTIPS_DYE;
            if (horseArmor->hasCustomColor(data.m_pItemInstance))
                return IDS_TOOLTIPS_CLEAN;
        }
        return IDS_NONE;
    case 459:
    case 460:
    case 461:
    case 462:
        return IDS_NONE;
    case 373:
    case 438:
    case 441:
    {
        if (!data.m_bUseBlock || !data.m_bCanPlace)
            return IDS_NONE;
        std::wstring potionId = PotionUtils::getPotionID(data.m_pItemInstance);
        if (potionId.compare(L"water") == 0)
        {
            if (hasWater || empty)
            {
                if (!hasClearWater && !empty)
                    return IDS_TOOLTIPS_FLUSH;
                if (!full)
                    return IDS_TOOLTIPS_FILL;
            }
            else if (hasPotion)
            {
                return IDS_TOOLTIPS_MIX;
            }
        }
        else if (potionId.compare(L"") != 0 && potionId.compare(L"empty") != 0)
        {
            bool samePotion = hasPotion && cauldron->getPotionId().compare(potionId) == 0;
            if (!empty)
            {
                if (!samePotion)
                    return IDS_TOOLTIPS_MIX;
                if (full)
                    return IDS_NONE;
            }
            return IDS_TOOLTIPS_FILL;
        }
        return IDS_NONE;
    }
    default:
        return defaultTooltip;
    }
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
    if (name == TEXTURE_INNER) return cauldron->mInnerIcon;
    if (name == TEXTURE_BOTTOM) return cauldron->mBottomIcon;
    if (name == TEXTURE_WATER) return cauldron->mWaterIcon;
    return NULL;
}
EBlockRenderShape CauldronBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_CAULDRON;
}

void CauldronBlock::spawnPotionParticles(Level* level, double x, double y, double z,
                                         Random* random, int color, int count)
{
    const BlockState* state = level->getBlockState(BlockPos(x, y, z));
    if (state->getBlock() != Blocks::cauldron)
        return;
    const double surfaceHeight = (state->getValue(LEVEL) * 2 + 6.0) / 16.0;
    for (int i = 0; i < count; ++i)
        level->addParticle(PARTICLE_MOB_SPELL, x + random->nextDouble(), y + surfaceHeight, z + random->nextDouble(),
                           ((color >> 16) & 0xff) / 255.0, ((color >> 8) & 0xff) / 255.0, (color & 0xff) / 255.0,
                           arrayWithLength<int>());
}
void CauldronBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity.get() != NULL && blockEntity->GetType() == BlockEntityType::CAULDRON)
    {
        int waterLevel = convertBlockStateToLegacyData(state);
        boost::shared_ptr<CauldronBlockEntity> cauldronEntity = boost::static_pointer_cast<CauldronBlockEntity>(blockEntity);
        if (0 < waterLevel && cauldronEntity->getPotionId() != L"")
        {
            int color = cauldronEntity->getPotionColor();
            spawnPotionParticles(level, (double)pos.x, (double)pos.y, (double)pos.z, random, color, 1);
        }
    }
}
void CauldronBlock::spawnSplashParticles(Level* level, double x, double y, double z,
                                         Random* random, int color, int count)
{
    const BlockState* state = level->getBlockState(BlockPos(x, y, z));
    if (state->getBlock() != Blocks::cauldron)
        return;
    const double surfaceHeight = (state->getValue(LEVEL) * 2 + 3.0) / 16.0 + 0.5;
    for (int i = 0; i < count; ++i)
    {
        const double particleX = x + 0.5 + (random->nextDouble() - 0.5) * 0.5;
        const double particleZ = z + 0.5 + (random->nextDouble() - 0.5) * 0.5;
        arrayWithLength<int> tint(4, true);
        tint.data[0] = static_cast<int>(((color >> 16) & 0xff) * 1.3f);
        tint.data[1] = static_cast<int>(((color >> 8) & 0xff) * 1.3f);
        tint.data[2] = static_cast<int>((color & 0xff) * 1.3f);
        tint.data[3] = 255;
        level->addParticle(PARTICLE_SPLASH, particleX, y + surfaceHeight - 0.25, particleZ, 0.0, 0.0, 0.0, tint);
    }
}

void CauldronBlock::spawnBubbleParticles(Level* level, double x, double y, double z,
                                         Random* random, int color, int count)
{
    const BlockState* state = level->getBlockState(BlockPos(x, y, z));
    if (state->getBlock() != Blocks::cauldron)
        return;
    const double surfaceHeight = (state->getValue(LEVEL) * 2 + 6.0) / 16.0;
    for (int i = 0; i < count; ++i)
    {
        const double particleX = x + 0.5 + (random->nextDouble() - 0.5) * 0.7;
        const double particleZ = z + 0.5 + (random->nextDouble() - 0.5) * 0.7;
        level->addParticle(PARTICLE_BUBBLE, particleX, y + surfaceHeight - 0.15, particleZ, 0.0, 0.0, 0.0,
                           arrayWithLength<int>());
    }
}

void CauldronBlock::spawnExplosionParticles(Level* level, double x, double y, double z,
                                            Random* random, int color, int count)
{
    const BlockState* state = level->getBlockState(BlockPos(x, y, z));
    if (state->getBlock() != Blocks::cauldron)
        return;
    const double surfaceHeight = (state->getValue(LEVEL) * 2 + 6.0) / 16.0;
    for (int i = 0; i < count; ++i)
    {
        const double particleX = x + random->nextDouble();
        const double particleZ = z + random->nextDouble();
        const double xa = random->nextDouble() * 0.02;
        const double ya = random->nextDouble() * 0.02;
        const double za = random->nextDouble() * 0.02;
        level->addParticle(PARTICLE_EXPLODE, particleX, y + surfaceHeight, particleZ, xa, ya, za,
                           arrayWithLength<int>());
    }
}
EPistonPushReaction CauldronBlock::getPistonPushReaction(const BlockState*) { return PISTON_PUSH_NORMAL; }
bool CauldronBlock::ParticlesSurviveWithin() { return true; }
