#include "game/world/block/Block.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/LeadItem.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/world/block/BlockImpl.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/AirBlock.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/ThinFenceBlock.hpp"
#include "game/world/block/HalfTransparentBlock.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/SnowBlock.hpp"
#include "game/world/block/StaticLiquidBlock.hpp"
#include "game/world/block/OreBlock.hpp"
#include "game/world/block/BedBlock.hpp"
#include "game/world/block/ButtonBlock.hpp"
#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/block/PistonHeadBlock.hpp"
#include "game/world/block/NewLeafBlock.hpp"
#include "game/world/block/NewLogBlock.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PoweredRailBlock.hpp"
#include "game/world/block/PrismarineBlock.hpp"
#include "game/world/block/PrismarineSlabBlock.hpp"
#include "game/world/block/QuartzBlock.hpp"
#include "game/world/block/RailBlock.hpp"
#include "game/world/block/RedSandStoneBlock.hpp"
#include "game/world/block/RedStoneWireBlock.hpp"
#include "game/world/block/RotatedPillarBlock.hpp"
#include "game/world/block/SandStoneBlock.hpp"
#include "game/world/block/StainedGlassBlock.hpp"
#include "game/world/block/StainedGlassPaneBlock.hpp"
#include "game/world/block/StoneBlock.hpp"
#include "game/world/block/StoneBrickBlock.hpp"
#include "game/world/block/StoneSlabBlock.hpp"
#include "game/world/block/TorchBlock.hpp"
#include "game/world/block/TrapDoorBlock.hpp"
#include "game/world/block/TripWireHookBlock.hpp"
#include "game/world/block/WallBlock.hpp"
#include "game/world/block/WoodSlabBlock.hpp"
#include "game/world/block/WoolCarpetBlock.hpp"
#include "game/world/block/PurpurSlabBlockFull.hpp"
#include "game/world/block/PurpurSlabBlockHalf.hpp"
#include "game/world/block/StainedHardenedClayBlock.hpp"
#include "game/world/block/StrippedLogBlock.hpp"
#include "game/world/block/RotatedPillarBlockImpl.hpp"
#include "game/world/block/PrismarineSlabBlockFull.hpp"
#include "game/world/block/PrismarineSlabBlockHalf.hpp"
#include "game/world/block/TurtleEggBlock.hpp"
#include "game/world/block/GlazedTerracottaBlock.hpp"
#include "game/world/block/ColoredBlock.hpp"
#include "game/world/block/DriedKelpBlock.hpp"
#include "game/world/block/KelpBlock.hpp"
#include "game/world/block/CoralBlock.hpp"
#include "game/world/block/BarrierBlock.hpp"
#include "game/world/block/BlueIceBlock.hpp"
#include "game/world/block/BoneBlock.hpp"
#include "game/world/block/BookshelfBlock.hpp"
#include "game/world/block/CactusBlock.hpp"
#include "game/world/block/CakeBlock.hpp"
#include "game/world/block/ChorusFlowerBlock.hpp"
#include "game/world/block/ChorusPlantBlock.hpp"
#include "game/world/block/ClayBlock.hpp"
#include "game/world/block/CocoaBlock.hpp"
#include "game/world/block/CraftingTableBlock.hpp"
#include "game/world/block/DetectorRailBlock.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/DragonEggBlock.hpp"
#include "game/world/block/DynamicLiquidBlock.hpp"
#include "game/world/block/EndPortalFrameBlock.hpp"
#include "game/world/block/EndRodBlock.hpp"
#include "game/world/block/FarmBlock.hpp"
#include "game/world/block/FenceBlock.hpp"
#include "game/world/block/FenceGateBlock.hpp"
#include "game/world/block/FireBlock.hpp"
#include "game/world/block/FullNewStoneSlabBlock.hpp"
#include "game/world/block/FullStoneSlabBlock.hpp"
#include "game/world/block/FullWoodSlabBlock.hpp"
#include "game/world/block/GlassBlock.hpp"
#include "game/world/block/Glowstoneblock.hpp"
#include "game/world/block/GrassBlock.hpp"
#include "game/world/block/GrassPathBlock.hpp"
#include "game/world/block/HalfNewStoneSlabBlock.hpp"
#include "game/world/block/HalfStoneSlabBlock.hpp"
#include "game/world/block/HalfWoodSlabBlock.hpp"
#include "game/world/block/HardenedClayBlock.hpp"
#include "game/world/block/HayBlock.hpp"
#include "game/world/block/HugeMushroomBlock.hpp"
#include "game/world/block/IceBlock.hpp"
#include "game/world/block/LadderBlock.hpp"
#include "game/world/block/LeverBlock.hpp"
#include "game/world/block/LogBlock.hpp"
#include "game/world/block/MagmaBlock.hpp"
#include "game/world/block/MelonBlock.hpp"
#include "game/world/block/MonsterEggBlock.hpp"
#include "game/world/block/MyceliumBlock.hpp"
#include "game/world/block/NetherBrickBlock.hpp"
#include "game/world/block/NetherrackBlock.hpp"
#include "game/world/block/NewStoneSlabBlock.hpp"
#include "game/world/block/ObsidianBlock.hpp"
#include "game/world/block/PackedIceBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/PortalBlock.hpp"
#include "game/world/block/PoweredBlock.hpp"
#include "game/world/block/PumpkinBlock.hpp"
#include "game/world/block/PurpurSlabBlock.hpp"
#include "game/world/block/RedFlowerBlock.hpp"
#include "game/world/block/RedstoneLampBlock.hpp"
#include "game/world/block/RedStoneOreBlock.hpp"
#include "game/world/block/RedstoneTorchBlock.hpp"
#include "game/world/block/ReedsBlock.hpp"
#include "game/world/block/SeaLanternBlock.hpp"
#include "game/world/block/SlimeBlock.hpp"
#include "game/world/block/SoulSandBlock.hpp"
#include "game/world/block/SpongeBlock.hpp"
#include "game/world/block/StructureBlock.hpp"
#include "game/world/block/StructureVoidBlock.hpp"
#include "game/world/block/TntBlock.hpp"
#include "game/world/block/TripWireBlock.hpp"
#include "game/world/block/VineBlock.hpp"
#include "game/world/block/WebBlock.hpp"
#include "game/world/block/YellowFlowerBlock.hpp"
#include "game/world/block/AnvilBlock.hpp"
#include "game/world/block/BannerBlock.hpp"
#include "game/world/block/BeaconBlock.hpp"
#include "game/world/block/BeetrootBlock.hpp"
#include "game/world/block/BrewingStandBlock.hpp"
#include "game/world/block/CauldronBlock.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/CommandBlock.hpp"
#include "game/world/block/ComparatorBlock.hpp"
#include "game/world/block/ConcretePowderBlock.hpp"
#include "game/world/block/ConduitBlock.hpp"
#include "game/world/block/CropBlock.hpp"
#include "game/world/block/DaylightDetectorBlock.hpp"
#include "game/world/block/DeadBushBlock.hpp"
#include "game/world/block/DispenserBlock.hpp"
#include "game/world/block/DropperBlock.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/EnchantmentTableBlock.hpp"
#include "game/world/block/EnderChestBlock.hpp"
#include "game/world/block/EndGatewayBlock.hpp"
#include "game/world/block/EndPortalBlock.hpp"
#include "game/world/block/FlowerPotBlock.hpp"
#include "game/world/block/FrostedIceBlock.hpp"
#include "game/world/block/FurnaceBlock.hpp"
#include "game/world/block/GravelBlock.hpp"
#include "game/world/block/HopperBlock.hpp"
#include "game/world/block/JukeboxBlock.hpp"
#include "game/world/block/MobSpawnerBlock.hpp"
#include "game/world/block/MushroomBlock.hpp"
#include "game/world/block/NetherWartBlock.hpp"
#include "game/world/block/NoteBlock.hpp"
#include "game/world/block/PistonMagicBlock.hpp"
#include "game/world/block/PotatoBlock.hpp"
#include "game/world/block/PressurePlateBlock.hpp"
#include "game/world/block/RepeaterBlock.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/SaplingBlock.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/block/SkullBlock.hpp"
#include "game/world/block/SnowLayerBlock.hpp"
#include "game/world/block/StoneButtonBlock.hpp"
#include "game/world/block/StandingBannerBlock.hpp"
#include "game/world/block/StandingSignBlock.hpp"
#include "game/world/block/StemBlock.hpp"
#include "game/world/block/TallGrassBlock.hpp"
#include "game/world/block/WallBannerBlock.hpp"
#include "game/world/block/WallSignBlock.hpp"
#include "game/world/block/WaterlilyBlock.hpp"
#include "game/world/block/WeightedPressurePlateBlock.hpp"
#include "game/world/block/WoodButtonBlock.hpp"
#include "game/core/Coral.hpp"
#include "game/core/CoralFan.hpp"
#include "game/core/CoralFanHang.hpp"
#include "game/core/SeaGrass.hpp"
#include "game/core/SeaPickle.hpp"
#include "game/core/BubbleColumnSegment.hpp"
#include "game/server/ObserverBlock.hpp"

#include "game/client/audio/SoundType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Explosion.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/Item.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/core/Stats.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/StairBlock.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/LevelData.hpp"
#include "game/core/HitResult.hpp"

unsigned int TlsAlloc();
int TlsSetValue(unsigned int index, void* value);

template <class T>
std::wstring _toString(T value);

static const ResourceLocation* AIR_LOCATION = new ResourceLocation(L"air");
DefaultedMappedRegistry<ResourceLocation, Block*>* Block::blockRegistry =
    new DefaultedMappedRegistry<ResourceLocation, Block*>(*AIR_LOCATION);
IdMapper<const BlockState*>* Block::blockStateRegistry = new IdMapper<const BlockState*>();
static const AABB* FULL_BLOCK_SHAPE = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
static const float INDESTRUCTIBLE = -1.0f;
unsigned int THREAD_STORAGE_INDEX = TlsAlloc();

boost::shared_ptr<ItemInstance> createItem(Block* block)
{
    return createItem(block->defaultBlockState());
}

Block::ThreadStorage::ThreadStorage()
{
    mCacheIndex = 0;
    mFastGraphics = false;
    mShapes.data = NULL;
    mShapes.length = 0;

    const AABB** shapes = new const AABB*[3];
    shapes[0] = NULL;
    shapes[1] = NULL;
    shapes[2] = NULL;

    mShapes.length = 3;
    mShapes.data = shapes;
}

Block::ThreadStorage::~ThreadStorage()
{
    delete[] mShapes.data;
}

void Block::CreateNewThreadStorage()
{
    TlsSetValue(THREAD_STORAGE_INDEX, new ThreadStorage());
}

void Block::ReleaseThreadStorage()
{
    delete (ThreadStorage*)TlsGetValue(THREAD_STORAGE_INDEX);
}

void Block::_init(Material* material, const MaterialColor* materialColor)
{
    mMaterialColor = materialColor;
    mParticleGravity = 1.0f;
    mSoundType = SoundType::STONE;
    mFriction = 0.6f;
    mBlockType = EBLOCK_TYPE_NONE;
    mUseNeighbourBrightness = false;
    mLightEmission = 0;
    mMaterial = material;
    mCollectStatistics = true;
    mBlockEntity = false;
    mExplosionResistance = 0.0f;
    mDestroyTime = 0.0f;
    mTicking = false;
    mHasSecondaryIcon = false;
    mBaseItemType = 0;
    mMipmap = true;
    mUseSharedIcon = false;
    mIcon = NULL;
    mIsAlwaysRendered = false;
    mHasTintedFace = false;
    mSemiTransparent = false;
    mBaseItemMaterial = 0;
    mDerivedInitDone = false;
    mTranslucent = !material->blocksLight();
    mIsExtraBlock = false;

    setThickness(0.0f);
}

Block::Block(Material* material)
{
    _init(material, material->getColor());
}

Block::Block(Material* material, const MaterialColor* materialColor)
{
    _init(material, materialColor);
}


Block::~Block()
{
}

void Block::registerBlock(int id, const ResourceLocation& location, Block* block)
{
    blockRegistry->registerMapping(id, ResourceLocation(location), block);
}

void Block::registerBlock(int id, const std::wstring& name, Block* block)
{
    ResourceLocation location(name);
    blockRegistry->registerMapping(id, ResourceLocation(location), block);
}

int Block::getId(Block* block)
{
    return blockRegistry->getId(block);
}

int Block::getElementId(Block* block)
{
    return blockRegistry->getId(block) << 4;
}

int Block::getElementId(const BlockState* state)
{
    Block* block = state->getBlock();
    return (blockRegistry->getId(block) << 4) | block->convertBlockStateToLegacyData(state);
}

int Block::getIdWithData(const BlockState* state)
{
    Block* block = state->getBlock();
    return getId(block) + block->convertBlockStateToLegacyData(state) * 0x1000;
}

Block* Block::byId(int id)
{
    if (id > 0x400)
    {
        id -= 0x300;

        if (id > 0x13E)
        {
            return NULL;
        }
    }
    else if (id > 0x13E)
    {
        return NULL;
    }

    return blockRegistry->byId(id);
}

bool Block::existsWithId(int id)
{
    return blockRegistry->containsId(id);
}

const BlockState* Block::getStateByElementId(unsigned short elementId)
{
    return blockStateRegistry->byId(elementId);
}

const BlockState* Block::stateByIdWithData(int idWithData)
{
    return byId(idWithData & 0xFFF)->getBlockState((idWithData >> 12) & 0xF);
}

Block* Block::byItem(Item* item)
{
    BlockItem* blockItem = dynamic_cast<BlockItem*>(item);

    if (blockItem == NULL)
    {
        return byId(0);
    }

    return blockItem->getBlock();
}

Block* Block::byString(const std::wstring& name)
{
    ResourceLocation location(name);

    if (blockRegistry->containsKey(ResourceLocation(location)))
    {
        return blockRegistry->get(ResourceLocation(location));
    }

    return blockRegistry->byId((int)wcstol(name.c_str(), NULL, 10));
}

Block* Block::setBaseItemTypeAndMaterial(int baseItemType, int baseItemMaterial)
{
    mBaseItemType = baseItemType;
    mBaseItemMaterial = baseItemMaterial;
    return this;
}

int Block::getBaseItemType()
{
    return mBaseItemType;
}

Block* Block::setDestroyTime(float destroyTime)
{
    mDestroyTime = destroyTime;

    if (destroyTime * 5.0f > mExplosionResistance)
    {
        mExplosionResistance = destroyTime * 5.0f;
    }

    return this;
}

bool Block::calculateIsWaterBlocking() const
{
    return mThickness <= 0.0f && (mMaterial->blocksMotion() || mMaterial->isSolid());
}

void Block::setThickness(float thickness)
{
    mThickness = thickness;
    mWaterBlocking = calculateIsWaterBlocking();
}

void Block::registerDefaultState(const BlockState* state)
{
    mDefaultBlockState = state;
}

const BlockState* Block::defaultBlockState()
{
    return mDefaultBlockState;
}

void Block::DerivedInit()
{
    mDerivedInitDone = true;
    mStateDefinition = createBlockStateDefinition();
    registerDefaultState(mStateDefinition->any());

    mFullBlock = defaultBlockState()->isSolidRender();
    mLightBlock = mFullBlock ? 255 : 0;
}

void Block::ClearDerivedInit()
{
    mDerivedInitDone = false;
    delete mStateDefinition;
}

Block* Block::setIconName(const std::wstring& iconName)
{
    mIconName = iconName;
    return this;
}

Block* Block::setSoundType(const SoundType* soundType)
{
    mSoundType = soundType;
    return this;
}

Block* Block::setNameAndDescriptionId(int descriptionId, int useDescriptionId)
{
    mDescriptionId = descriptionId;
    mUseDescriptionId = useDescriptionId;
    return this;
}

Block* Block::setIndestructible()
{
    return setDestroyTime(INDESTRUCTIBLE);
}

Block* Block::sendBlockData(unsigned char blockData)
{
    mBlockData = blockData;
    return this;
}

Block* Block::setLightBlock(int lightBlock)
{
    mLightBlock = lightBlock;
    return this;
}

Block* Block::setTicking(bool ticking)
{
    mTicking = ticking;
    return this;
}

Block* Block::setLightEmission(float lightEmission)
{
    mLightEmission = (int)(lightEmission * 15.0f);
    return this;
}

Block* Block::setExplodeable(float explosionResistance)
{
    mExplosionResistance = explosionResistance * 3.0f;
    return this;
}

bool Block::isTopSolidBlocking(const BlockState* state)
{
    return state->getMaterial()->isSolidBlocking() && state->isCubeShaped();
}

bool Block::canProvideSupport(const BlockState* state, const Direction* direction, EBlockSupportType supportType)
{
    if (supportType != BLOCK_SUPPORT_CENTER)
    {
        return isCubeShaped(state) || isSolidBlockingCube(state);
    }

    if (isValidSpawn(state, boost::shared_ptr<Entity>()) || isCubeShaped(state))
    {
        return true;
    }

    if (HalfSlabBlock::isHalfSlab(state))
    {
        TypedBoxed<const HalfSlabBlock::Half*>* half =
            (TypedBoxed<const HalfSlabBlock::Half*>*)state->getBoxedValue(HalfSlabBlock::TYPE);

        if (half != NULL && half->getValue() == HalfSlabBlock::TOP)
        {
            return true;
        }
    }

    if (StairBlock::isStairs(state))
    {
        TypedBoxed<const StairBlock::Half*>* half =
            (TypedBoxed<const StairBlock::Half*>*)state->getBoxedValue(StairBlock::HALF);

        if (half != NULL && half->getValue() != NULL && half->getValue()->getSerializedName() == L"top")
        {
            return true;
        }
    }

    if (state->getBlock() != byId(78))
    {
        return false;
    }

    return state->getBlock()->convertBlockStateToLegacyData(state) == 7;
}

bool Block::isValidSpawn(const BlockState* state, boost::shared_ptr<Entity> entity)
{
    return true;
}

int Block::getLightEmission(const BlockState* state)
{
    return mLightEmission;
}

bool Block::ParticlesSurviveWithin()
{
    return false;
}

bool Block::shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state)
{
    return true;
}

bool Block::isLiquidBlock()
{
    return false;
}

unsigned int Block::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (!Item::isItemABlock(Item::getId(tooltipData.m_pItemInstance->getItem())))
    {
        int blockId = Item::getBlockId(tooltipData.m_pItemInstance->getItem());

        switch (blockId)
        {
        case 6:
        case 31:
        case 32:
        case 33:
        case 34:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 81:
        case 83:
        case 141:
        case 142:
        case 175:
        case 270:
        case 271:
            if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail && (tooltipData.m_bUseItem || tooltipData.m_bCanPlace))
            {
                return 0x512092D2u;
            }

            return 0xFFFFFFFFu;

        default:
            break;
        }

        if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Success || !(tooltipData.m_bUseItem || tooltipData.m_bCanPlace))
        {
            return 0xFFFFFFFFu;
        }

        return 0x4C36CD96u;
    }

    unsigned int itemId = Item::getId(tooltipData.m_pItemInstance->getItem());

    switch (itemId)
    {
    case 0x103:
    case 0x181:
        if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Fail || !(tooltipData.m_bUseItem || tooltipData.m_bCanPlace))
        {
            return 0xFFFFFFFFu;
        }

        return 0xA3BAC985u;

    case 0x127:
    case 0x152:
    case 0x169:
    case 0x16A:
    case 0x174:
    case 0x187:
    case 0x188:
        if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail && (tooltipData.m_bUseItem || tooltipData.m_bCanPlace))
        {
            return 0x512092D2u;
        }

        return 0xFFFFFFFFu;

    case 0x141:
    case 0x185:
        if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Success)
        {
            return 0xFFFFFFFFu;
        }

        return 0xCE1819A5u;

    case 0x146:
    case 0x147:
    case 0x1CB:
    case 0x1CC:
    case 0x1CD:
    case 0x1CE:
        if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Success || !(tooltipData.m_bUseItem || tooltipData.m_bCanPlace))
        {
            return 0xFFFFFFFFu;
        }

        return 0xE80CCF48u;

    case 0x15F:
        if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Fail)
        {
            return 0xFFFFFFFFu;
        }

        if (DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 12)
        {
            return 0x512092D2u;
        }

        return 0xFFFFFFFFu;

    case 0x191:
        if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Fail || !(tooltipData.m_bUseItem || tooltipData.m_bCanPlace))
        {
            return 0xFFFFFFFFu;
        }

        return 0x568A07BEu;

    case 0x1A4:
        if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success && (tooltipData.m_bUseItem || tooltipData.m_bCanPlace))
        {
            if (LeadItem::bindPlayerMobsTest(tooltipData.m_pPlayer, tooltipData.m_pLevel, BlockPos(tooltipData.m_pPlayer)))
            {
                return 0x7B938F36u;
            }
        }

        return 0xFFFFFFFFu;

    case 0x1B3:
    case 0x1C9:
        if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail && (tooltipData.m_bUseItem || tooltipData.m_bCanPlace))
        {
            return 0x512092D2u;
        }

        return 0xFFFFFFFFu;

    default:
        break;
    }

    return 0xFFFFFFFFu;
}

unsigned int Block::GetHitTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_bCanDestroy)
    {
        return 0xFFFFFFFFu;
    }

    if (getId(tooltipData.m_pBlockState->getBlock()) == 7)
    {
        if (!tooltipData.m_pPlayer->GetGameMode()->isCreative())
        {
            return 0xFFFFFFFFu;
        }

        if (!tooltipData.m_pLevel->mDimension->inPlayerConstraints(tooltipData.m_blockPos))
        {
            return 0xFFFFFFFFu;
        }
    }

    return 0x5FB7A25Au;
}

const MaterialColor* Block::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return mMaterialColor;
}

const BlockState* Block::getBlockState(int data)
{
    return defaultBlockState();
}

int Block::convertBlockStateToLegacyData(const BlockState* state)
{
    state->getProperties();
    return 0;
}

const BlockState* Block::setPropertyValueFromString(const BlockState* state, const Property* property, const std::wstring& value)
{
    Boxed* boxed = property->getValue(value);
    const BlockState* result = state->setBoxedValue(property, boxed);

    delete boxed;

    return result;
}

const BlockState* Block::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return state;
}

const BlockState* Block::rotate(const BlockState* state, Rotation* rotation)
{
    return state;
}

const BlockState* Block::mirror(const BlockState* state, Mirror* mirror)
{
    return state;
}

bool Block::isExceptionBlockForAttaching(Block* block)
{
    if (block == NULL)
    {
        return false;
    }

    if (dynamic_cast<HalfTransparentBlock*>(block) != NULL
        || dynamic_cast<LeafBlock*>(block) != NULL
        || dynamic_cast<ThinFenceBlock*>(block) != NULL)
    {
        return true;
    }

    return block == byId(20) || block == byId(102);
}

bool Block::isExceptionBlockForAttachingPlusPiston(Block* block)
{
    return isExceptionBlockForAttaching(block);
}

bool Block::isExceptionForPistonAttaching(Block* block)
{
    return block == byId(29) || block == byId(33) || block == byId(34);
}

bool Block::isExceptionForTopSolidBlocking(Block* block)
{
    return block == byId(44) || block == byId(126) || block == byId(78)
        || block == byId(2) || block == byId(125);
}

bool Block::isSolidBlockingCube(const BlockState* state)
{
    return mMaterial->blocksMotion() && state->isCubeShaped();
}

bool Block::isSolidBlockingCubeAndNotSignalSource(const BlockState* state)
{
    return mMaterial->isSolidBlocking() && state->isCubeShaped() && !state->isSignalSource();
}

bool Block::isViewBlocking(const BlockState* state)
{
    return mMaterial->blocksMotion() && defaultBlockState()->isCubeShaped();
}

bool Block::isCubeShaped(const BlockState* state)
{
    return true;
}

bool Block::hasCustomBreakingProgress(const BlockState* state)
{
    return false;
}

bool Block::isPathfindable(LevelSource* levelSource, const BlockPos& pos)
{
    return !mMaterial->blocksMotion();
}

EBlockRenderShape Block::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_MODEL;
}

bool Block::hasInHandRenderOffset()
{
    return false;
}

bool Block::mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos)
{
    return false;
}

bool Block::isTransparent(const BlockState* state)
{
    return false;
}

bool Block::isIndestructible()
{
    return mDestroyTime == INDESTRUCTIBLE;
}

float Block::getDestroySpeed(const BlockState* state, Level* level, const BlockPos& pos)
{
    return mDestroyTime;
}

Block* Block::disableMipmap()
{
    mMipmap = false;
    return this;
}

Block* Block::setSemiTransparent()
{
    mSemiTransparent = true;
    return this;
}

bool Block::isTicking()
{
    return mTicking;
}

bool Block::isSlabBlock()
{
    return false;
}

bool Block::isWaterBlocking() const
{
    return mWaterBlocking;
}

bool Block::canContainLiquid(const BlockState* state) const
{
    return !state->isCubeShaped();
}

bool Block::isStrippable()
{
    return false;
}

const BlockState* Block::getStrippedBlock(const BlockState* state)
{
    return state;
}

bool Block::liquidCanFlowIntoFromDirection(LevelSource* levelSource, const BlockPos& pos, const Direction* direction) const
{
    return true;
}

const AABB* Block::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return FULL_BLOCK_SHAPE;
}

int Block::getLightColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, int lightValue)
{
    Block* block = levelSource->getBlockState(pos)->getBlock();
    int lightColor = levelSource->getLightColor(pos, block->getLightEmission(state), -1);

    if (lightColor == 0 && dynamic_cast<HalfSlabBlock*>(block) != NULL)
    {
        BlockPos below = pos.below();
        const BlockState* belowState = levelSource->getBlockState(below);

        return levelSource->getLightColor(below, belowState->getBlock()->getLightEmission(belowState), -1);
    }

    return lightColor;
}

bool Block::neighbourVisibleThroughShape(const AABB* shape, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    switch (direction->get3DDataValue())
    {
    case 0:
        return shape->mMinY > 0.0;

    case 1:
        return shape->mMaxY < 1.0;

    case 2:
        return shape->mMinZ > 0.0;

    case 3:
        return shape->mMaxZ < 1.0;

    case 4:
        return shape->mMinX > 0.0;

    case 5:
        return shape->mMaxX < 1.0;

    default:
        break;
    }

    return false;
}

bool Block::neighbourVisibleThroughShapes(const arrayWithLength<const AABB*>* shapes, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    for (unsigned int index = 0; index < shapes->length; index++)
    {
        const AABB* shape = shapes->data[index];

        if (shape != NULL && neighbourVisibleThroughShape(shape, levelSource, pos, direction))
        {
            return true;
        }
    }

    return false;
}

bool Block::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    if (neighbourVisibleThroughShapes(getShapes(state, levelSource, pos), levelSource, pos, direction))
    {
        return true;
    }

    return !levelSource->isSolidRender(pos.relative(direction));
}

const arrayWithLength<const AABB*>* Block::getShapes(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    ThreadStorage* storage = (ThreadStorage*)TlsGetValue(THREAD_STORAGE_INDEX);

    storage->mShapes.data[0] = getShape(state, levelSource, pos);
    storage->mShapes.data[1] = NULL;
    storage->mShapes.data[2] = NULL;

    return &storage->mShapes;
}

bool Block::isSolidFace(LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    return levelSource->getBlockState(pos)->getMaterial()->isSolid();
}

EBlockFaceShape Block::getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos, const Direction* direction)
{
    return BLOCK_FACE_SOLID;
}

const AABB* Block::getOutlineAABB(const BlockState* state, Level* level, const BlockPos& pos)
{
    return state->getShape(level, pos)->move(pos);
}

void Block::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox, std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity, bool isActualState)
{
    addCollisionAABB(pos, entityBox, collidingBoxes, state->getClipAABB(level, pos));
}

void Block::addCollisionAABB(const BlockPos& pos, const AABB* entityBox, std::vector<AABB*>* collidingBoxes, const AABB* blockBox)
{
    if (blockBox == NULL)
    {
        return;
    }

    AABB* movedBox = blockBox->move(pos);

    if (entityBox->intersects(movedBox->mMinX, movedBox->mMinY, movedBox->mMinZ, movedBox->mMaxX, movedBox->mMaxY, movedBox->mMaxZ))
    {
        collidingBoxes->push_back(movedBox);
    }
}

const AABB* Block::getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return state->getShape(levelSource, pos);
}

bool Block::isSolidRender(const BlockState* state) const
{
    return true;
}

bool Block::mayPick(const BlockState* state, bool includeLiquid)
{
    return mayPick();
}

bool Block::mayPick()
{
    return true;
}

void Block::randomTick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    tick(level, pos, state, random);
}

void Block::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
}

void Block::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
}

void Block::destroy(Level* level, const BlockPos& pos, const BlockState* state)
{
}

void Block::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
}

void Block::addLights(Level* level, const BlockPos& pos)
{
}

int Block::getTickDelay(Level* level)
{
    return 10;
}

void Block::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
}

void Block::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
}

int Block::getResourceCount(Random* random)
{
    return 1;
}

Item* Block::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Item::byBlock(this);
}

float Block::getDestroyProgress(const BlockState* state, boost::shared_ptr<Player> player, Level* level, const BlockPos& pos)
{
    float destroySpeed = state->getDestroySpeed(level, pos);

    if (destroySpeed <= 0.0f)
    {
        return 1.0f;
    }

    if (player->canDestroy(state))
    {
        return player->getDestroySpeed(state) / destroySpeed / 30.0f;
    }

    return player->getDestroySpeed(state) / destroySpeed / 100.0f;
}

void Block::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, int lootBonus)
{
    spawnResources(level, pos, state, 1.0f, lootBonus);
}

void Block::popResource(Level* level, const BlockPos& pos, not_null_ptr<ItemInstance> itemInstance)
{
    if (level->mIsClientSide)
    {
        return;
    }

    if (itemInstance->isEmpty())
    {
        return;
    }

    if (!level->getGameRules()->getBoolean(5))
    {
        return;
    }

    float spread = 0.5f;
    float offset = 0.25f;

    double x = (double)pos.x + (double)(level->mRandom->nextFloat() * spread) + (double)offset;
    double y = (double)pos.y + (double)(level->mRandom->nextFloat() * spread) + (double)offset;
    double z = (double)pos.z + (double)(level->mRandom->nextFloat() * spread) + (double)offset;

    boost::shared_ptr<ItemEntity> itemEntity(new ItemEntity(level, x, y, z, itemInstance));
    itemEntity->setDefaultPickUpDelay();

    level->addEntity(itemEntity);
}

void Block::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance, int lootBonus)
{
    if (level->mIsClientSide)
    {
        return;
    }

    int count = getResourceCountForLootBonus(lootBonus, level->mRandom);

    while (count > 0)
    {
        if (level->mRandom->nextFloat() <= chance)
        {
            Item* item = getResource(state, level->mRandom, lootBonus);

            if (item != NULL)
            {
                popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(item, 1, getSpawnResourcesAuxValue(state))));
            }
        }

        count--;
    }
}

void Block::popExperience(Level* level, const BlockPos& pos, int amount)
{
    if (level->mIsClientSide)
    {
        return;
    }

    if (!level->getGameRules()->getBoolean(5))
    {
        return;
    }

    while (amount > 0)
    {
        int value = ExperienceOrb::getExperienceValue(amount);
        amount -= value;

        boost::shared_ptr<ExperienceOrb> orb(new ExperienceOrb(level,
            (double)pos.x + 0.5, (double)pos.y + 0.5, (double)pos.z + 0.5, value));

        level->addEntity(orb);
    }
}

int Block::getSpawnResourcesAuxValue(const BlockState* state)
{
    return 0;
}

float Block::getExplosionResistance(boost::shared_ptr<Entity> entity)
{
    return mExplosionResistance / 5.0f;
}

HitResult* Block::clip(const BlockState* state, Level* level, const BlockPos& pos, Vec3* from, Vec3* to)
{
    return clip(pos, from, to, state->getShape(level, pos));
}

HitResult* Block::clip(const BlockPos& pos, Vec3* from, Vec3* to, const AABB* box)
{
    Vec3* localFrom = from->subtract((double)pos.x, (double)pos.y, (double)pos.z);
    Vec3* localTo = to->subtract((double)pos.x, (double)pos.y, (double)pos.z);

    HitResult* hit = box->clip(localFrom, localTo);

    if (hit == NULL)
    {
        return NULL;
    }

    HitResult* result = new HitResult(hit->mLocation->add((double)pos.x, (double)pos.y, (double)pos.z),
        hit->mDirection, pos);

    delete hit;

    return result;
}

void Block::wasExploded(Level* level, const BlockPos& pos, Explosion* explosion)
{
    if (level->mIsClientSide)
    {
        return;
    }

    if (!Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        return;
    }

    if (explosion == NULL)
    {
        return;
    }

    boost::shared_ptr<Entity> source = explosion->getDirectSource();

    if (source == NULL)
    {
        return;
    }

    if (source->GetType() == 0x88000005)
    {
        source = boost::static_pointer_cast<FireworksRocketEntity>(source)->getOwner();
    }

    if (source != NULL && (source->GetType() & 0xC800000) == 0xC800000)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(source);
        player->GetGameMode()->RecordBlockDestroyed(pos, getId(this));
    }
}

int Block::getRenderLayer()
{
    return 0;
}

bool Block::mayPlace(Level* level, const BlockPos& pos, const Direction* direction)
{
    return mayPlace(level, pos);
}

bool Block::mayPlace(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos)->getBlock()->mMaterial->isReplaceable();
}

bool Block::mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    return true;
}

bool Block::TestUse()
{
    return false;
}

bool Block::TestUse(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player)
{
    return false;
}

bool Block::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY, float hitZ, bool clientSide)
{
    return false;
}

void Block::stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity)
{
}

const BlockState* Block::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction, float hitX, float hitY, float hitZ, int auxValue, boost::shared_ptr<LivingEntity> placer)
{
    return getBlockState(auxValue);
}

void Block::prepareRender(Level* level, const BlockPos& pos)
{
}

void Block::attack(Level* level, const BlockPos& pos, boost::shared_ptr<Player> player)
{
}

Vec3* Block::handleEntityInside(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, Vec3* motion)
{
    return motion;
}

int Block::getColor() const
{
    return 0xFFFFFF;
}

int Block::getColor(const BlockState* state)
{
    return 0xFFFFFF;
}

int Block::getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex)
{
    return 0xFFFFFF;
}

int Block::getColor(LevelSource* levelSource, const BlockPos& pos)
{
    return getColor();
}

int Block::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    return 0xFFFFFF;
}

int Block::getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    return 0;
}

bool Block::isSignalSource(const BlockState* state)
{
    return false;
}

void Block::entityInside(Level* level, const BlockPos& pos, const BlockState* state, const boost::shared_ptr<Entity>& entity)
{
}

int Block::getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    return 0;
}

bool Block::waterSpreadCausesSpawn()
{
    return false;
}

void Block::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos, const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity, not_null_ptr<ItemInstance> tool)
{
    Block* minedBlock = state->getBlock();

    if (minedBlock != Blocks::wheat && minedBlock != Blocks::carrots && minedBlock != Blocks::potatoes)
    {
        player->awardStat(Stats::blockMinedStats.data[getId(this)], GenericStats::param_blocksMined(state, 1));
    }
    else if (minedBlock->getResource(state, NULL, 0) != NULL)
    {
        player->awardStat(Stats::blockMinedStats.data[getId(this)], GenericStats::param_blocksMined(state, 1));
    }

    player->awardStat(GenericStats::mineBlock(), GenericStats::param_noArgs());
    player->causeFoodExhaustion(FoodConstants::ExhaustionMine());

    if (state->getBlock() == byId(17) || state->getBlock() == byId(18))
    {
        player->awardStat(GenericStats::mineWood(), GenericStats::param_noArgs());
    }

    if (isSilkTouchable()
        && EnchantmentHelper::getItemEnchantmentLevel(Enchantments::SILK_TOUCH, tool) > 0)
    {
        popResource(level, pos, getSilkTouchItemInstance(state));
        return;
    }

    spawnResources(level, pos, state, EnchantmentHelper::getItemEnchantmentLevel(Enchantments::BLOCK_FORTUNE, tool));
}

bool Block::isSilkTouchable()
{
    return defaultBlockState()->isCubeShaped() && !mBlockEntity;
}

not_null_ptr<ItemInstance> Block::getSilkTouchItemInstance(const BlockState* state)
{
    Item* item = Item::byBlock(this);
    int auxValue = 0;

    if (item->isStackedByData())
    {
        auxValue = convertBlockStateToLegacyData(state);
    }

    return not_null_ptr<ItemInstance>(new ItemInstance(item, 1, auxValue));
}

bool Block::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    return true;
}

int Block::getResourceCountForLootBonus(int lootBonus, Random* random)
{
    return getResourceCount(random);
}

void Block::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance)
{
}

bool Block::isPossibleToRespawnInThis()
{
    return !mMaterial->isSolid() && !mMaterial->isLiquid();
}

std::wstring Block::getName()
{
    return std::wstring(L"");
}

int Block::getDescriptionId(int auxValue)
{
    return mDescriptionId;
}

int Block::getUseDescriptionId(int auxValue)
{
    return mUseDescriptionId;
}

bool Block::triggerEvent(const BlockState* state, Level* level, const BlockPos& pos, int eventId, int eventParam)
{
    return false;
}

bool Block::isCollectStatistics()
{
    return mCollectStatistics;
}

Block* Block::setNotCollectStatistics()
{
    mCollectStatistics = false;
    return this;
}

EPistonPushReaction Block::getPistonPushReaction(const BlockState* state)
{
    return (EPistonPushReaction)mMaterial->getPushReaction();
}

float Block::getShadeBrightness(const BlockState* state)
{
    if (state->isSolidRender())
    {
        return 0.2f;
    }

    return 1.0f;
}

void Block::fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance)
{
    entity->causeFallDamage(fallDistance, 1.0f);
}

void Block::updateEntityAfterFallOn(Level* level, boost::shared_ptr<Entity> entity)
{
    entity->mYd = 0.0;
}

not_null_ptr<ItemInstance> Block::getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1, getSpawnResourcesAuxValue(state)));
}

bool Block::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player)
{
    return false;
}

void Block::handleRain(Level* level, const BlockPos& pos)
{
}

bool Block::canInstantlyTick()
{
    return true;
}

bool Block::dropFromExplosion(Explosion* explosion)
{
    return true;
}

bool Block::isMatching(Block* block)
{
    return this == block;
}

bool Block::isMatching(Block* first, Block* second)
{
    if (first == NULL || second == NULL)
    {
        return false;
    }

    if (first == second)
    {
        return true;
    }

    return first->isMatching(second);
}

bool Block::hasAnalogOutputSignal(const BlockState* state)
{
    return false;
}

int Block::getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos)
{
    return 0;
}

std::wstring Block::getIconName()
{
    if (mIconName.empty())
    {
        return std::wstring(L"MISSING_ICON_TILE_") + _toString(getId(this)) + L"_" + _toString(mDescriptionId);
    }

    return mIconName;
}

void Block::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(getIconName());
}

std::wstring Block::getTileItemIconName()
{
    return std::wstring(L"");
}

TextureAtlasSprite* Block::getIconTexture(const Direction* direction, const BlockState* state)
{
    return getTexture(direction, state);
}

TextureAtlasSprite* Block::getTexture(LevelSource* levelSource, const BlockPos& pos, const Direction* direction, const BlockState* state)
{
    if (!LeafBlock::isLeaves(state->getBlock()))
    {
        return getTexture(direction, state);
    }

    static const int OFFSETS_X[6] = { 1, 0, 0, 0, 0, -1 };
    static const int OFFSETS_Y[6] = { 0, 1, 0, 0, -1, 0 };
    static const int OFFSETS_Z[6] = { 0, 0, 1, -1, 0, 0 };

    for (int index = 0; index < 6; index++)
    {
        const BlockState* neighbour = levelSource->getBlockState(pos.offset(OFFSETS_X[index], OFFSETS_Y[index], OFFSETS_Z[index]));

        if (!LeafBlock::isLeaves(neighbour->getBlock()) && !neighbour->isSolidRender())
        {
            return getTexture(direction, state);
        }
    }

    static_cast<LeafBlock*>(Blocks::leaves)->setFancy(false);
    static_cast<LeafBlock*>(Blocks::leaves2)->setFancy(false);

    TextureAtlasSprite* texture = getTexture(direction, state);

    static_cast<LeafBlock*>(Blocks::leaves)->setFancy(true);
    static_cast<LeafBlock*>(Blocks::leaves2)->setFancy(true);

    return texture;
}

TextureAtlasSprite* Block::getTexture(const Direction* direction, const BlockState* state)
{
    return mIcon;
}

TextureAtlasSprite* Block::getTexture(const Direction* direction)
{
    return getTexture(direction, defaultBlockState());
}

int Block::getFaceFlagsFast(LevelSource* levelSource, const BlockPos& pos)
{
    const AABB* shape = levelSource->getBlockState(pos)->getShape(levelSource, pos);
    int flags = 0;

    if (shape->mMinY > 0.0 || !levelSource->getBlockState(pos.below())->isSolidRender())
    {
        flags = 1;
    }

    if (shape->mMaxY < 1.0 || !levelSource->getBlockState(pos.above())->isSolidRender())
    {
        flags |= 2;
    }

    if (shape->mMinZ > 0.0 || !levelSource->getBlockState(pos.north())->isSolidRender())
    {
        flags |= 4;
    }

    if (shape->mMaxZ < 1.0 || !levelSource->getBlockState(pos.south())->isSolidRender())
    {
        flags |= 8;
    }

    if (shape->mMinX > 0.0 || !levelSource->getBlockState(pos.west())->isSolidRender())
    {
        flags |= 16;
    }

    if (shape->mMaxX < 1.0 || !levelSource->getBlockState(pos.east())->isSolidRender())
    {
        flags |= 32;
    }

    return flags;
}

int Block::getFaceFlags(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, bool forceAll)
{
    if (forceAll)
    {
        return 0x3F;
    }

    Block* block = state->getBlock();
    int id = getId(block);

    if (id < 8 || (unsigned int)(id - 12) < 6)
    {
        return block->getFaceFlagsFast(levelSource, pos);
    }

    int flags = block->shouldRenderFace(state, levelSource, pos, Direction::DOWN) ? 1 : 0;

    if (block->shouldRenderFace(state, levelSource, pos, Direction::UP))
    {
        flags |= 2;
    }

    if (block->shouldRenderFace(state, levelSource, pos, Direction::NORTH))
    {
        flags |= 4;
    }

    if (block->shouldRenderFace(state, levelSource, pos, Direction::SOUTH))
    {
        flags |= 8;
    }

    if (block->shouldRenderFace(state, levelSource, pos, Direction::WEST))
    {
        flags |= 16;
    }

    if (block->shouldRenderFace(state, levelSource, pos, Direction::EAST))
    {
        flags |= 32;
    }

    return flags;
}

const BlockState* Block::getInventoryRenderState(const BlockState* state)
{
    return state;
}

BlockStateDefinition* Block::createBlockStateDefinition()
{
    return new BlockStateDefinition(this);
}

BlockStateDefinition* Block::getBlockStateDefinition()
{
    return mStateDefinition;
}

EBlockOffsetType Block::getOffsetType()
{
    return BLOCK_OFFSET_NONE;
}

Vec3* Block::getOffset(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    EBlockOffsetType offsetType = getOffsetType();

    if (offsetType == BLOCK_OFFSET_NONE)
    {
        return Vec3::newTemp(0.0, 0.0, 0.0);
    }

    long seed = Mth::getSeed(pos.x, 0, pos.z);

    double offsetX = ((double)(float)((double)((seed >> 16) & 0xF) / 15.0) - 0.5) * 0.5;
    double offsetZ = ((double)(float)((double)((seed >> 24) & 0xF) / 15.0) - 0.5) * 0.5;

    if (offsetType == BLOCK_OFFSET_XYZ)
    {
        double offsetY = ((double)(float)((double)((seed >> 20) & 0xF) / 15.0) - 1.0) * 0.2;

        return Vec3::newTemp(offsetX, offsetY, offsetZ);
    }

    return Vec3::newTemp(offsetX, 0.0, offsetZ);
}

const SoundType* Block::getSoundType()
{
    return mSoundType;
}

std::wstring Block::toString()
{
    return std::wstring(L"Block{") + blockRegistry->getKey(this).toString() + L"}";
}

void Block::appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player, std::vector<HtmlString>* lines, bool advanced)
{
}

bool Block::instanceof(EBlockType blockType)
{
    return mBlockType == blockType;
}

void Block::staticCtor()
{
    DirectionalBlock::FACING = DirectionProperty::create(L"facing");
    HorizontalDirectionalBlock::FACING = DirectionProperty::create(L"facing", Direction::Plane::HORIZONTAL);

    DirectionalBlock::blockStaticCtor();
    HorizontalDirectionalBlock::blockStaticCtor();
    AnvilBlock::blockStaticCtor();
    BannerBlock::blockStaticCtor();
    BedBlock::blockStaticCtor();
    ButtonBlock::blockStaticCtor();
    ChestBlock::blockStaticCtor();
    ColoredBlock::blockStaticCtor();
    ComparatorBlock::blockStaticCtor();
    DetectorRailBlock::blockStaticCtor();
    DirtBlock::blockStaticCtor();
    DispenserBlock::blockStaticCtor();
    DoorBlock::blockStaticCtor();
    DoublePlantBlock::blockStaticCtor();
    EnderChestBlock::blockStaticCtor();
    EndPortalFrameBlock::blockStaticCtor();
    FlowerPotBlock::blockStaticCtor();
    FurnaceBlock::blockStaticCtor();
    HalfSlabBlock::blockStaticCtor();
    HopperBlock::blockStaticCtor();
    HugeMushroomBlock::blockStaticCtor();
    LadderBlock::blockStaticCtor();
    LeverBlock::blockStaticCtor();
    LogBlock::blockStaticCtor();
    MonsterEggBlock::blockStaticCtor();
    PistonBaseBlock::blockStaticCtor();
    PistonHeadBlock::blockStaticCtor();
    PistonMagicBlock::blockStaticCtor();
    PlanksBlock::blockStaticCtor();
    NewLeafBlock::blockStaticCtor();
    NewLogBlock::blockStaticCtor();
    SandBlock::blockStaticCtor();
    NewStoneSlabBlock::blockStaticCtor();
    OldLeafBlock::blockStaticCtor();
    OldLogBlock::blockStaticCtor();
    PortalBlock::blockStaticCtor();
    PoweredRailBlock::blockStaticCtor();
    PrismarineBlock::blockStaticCtor();
    PrismarineSlabBlock::blockStaticCtor();
    QuartzBlock::blockStaticCtor();
    RailBlock::blockStaticCtor();
    RedSandStoneBlock::blockStaticCtor();
    RedStoneWireBlock::blockStaticCtor();
    RotatedPillarBlock::blockStaticCtor();
    SandStoneBlock::blockStaticCtor();
    SaplingBlock::blockStaticCtor();
    SkullBlock::blockStaticCtor();
    StainedGlassBlock::blockStaticCtor();
    StainedGlassPaneBlock::blockStaticCtor();
    StairBlock::blockStaticCtor();
    StoneBlock::blockStaticCtor();
    StoneBrickBlock::blockStaticCtor();
    StoneSlabBlock::blockStaticCtor();
    TallGrassBlock::blockStaticCtor();
    TorchBlock::blockStaticCtor();
    StemBlock::blockStaticCtor();
    TrapDoorBlock::blockStaticCtor();
    TripWireHookBlock::blockStaticCtor();
    WallBlock::blockStaticCtor();
    WallSignBlock::blockStaticCtor();
    WoodSlabBlock::blockStaticCtor();
    WoolCarpetBlock::blockStaticCtor();
    PurpurSlabBlock::blockStaticCtor();
    NoteBlock::blockStaticCtor();
    ConcretePowderBlock::blockStaticCtor();
    StainedHardenedClayBlock::staticCtor();
    CommandBlock::blockStaticCtor();

    registerBlock(0, *AIR_LOCATION,
        (new AirBlock())->setNameAndDescriptionId(-1, -1));

    registerBlock(1, L"stone",
        (new StoneBlock())->setDestroyTime(1.5f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setIconName(L"stone")->setNameAndDescriptionId(-1951576660, 1182278201)->setBaseItemTypeAndMaterial(14, 2));

    registerBlock(2, L"grass",
        (new GrassBlock())->setDestroyTime(0.6f)->setSoundType(SoundType::GRASS)->setIconName(L"grass")->setNameAndDescriptionId(-1011953045, 1347151022)->setBaseItemTypeAndMaterial(13, 58));

    registerBlock(3, L"dirt",
        (new DirtBlock())->setDestroyTime(0.5f)->setSoundType(SoundType::GRAVEL)->setIconName(L"dirt")->setNameAndDescriptionId(1730991212, 1412954865)->setBaseItemTypeAndMaterial(13, 58));

    Block* cobblestone = (new BlockImpl(Material::STONE))->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setIconName(L"cobblestone")->setNameAndDescriptionId(1660683698, 1587890167)->setBaseItemTypeAndMaterial(14, 2);
    registerBlock(4, L"cobblestone", cobblestone);

    Block* planks = (new PlanksBlock())->setIconName(L"planks")->setDestroyTime(2.0f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(318988197, 478421857)->setBaseItemTypeAndMaterial(50, 1);
    registerBlock(5, L"planks", planks);

    registerBlock(6, L"sapling",
        (new SaplingBlock())->setDestroyTime(0.0f)->setSoundType(SoundType::GRASS)->setIconName(L"sapling")->sendBlockData(15)->setNameAndDescriptionId(1867374529, -1103634204)->disableMipmap());

    registerBlock(7, L"bedrock",
        (new BlockImpl(Material::STONE))->setIndestructible()->setExplodeable(6000000.0f)->setSoundType(SoundType::STONE)->setIconName(L"bedrock")->setNotCollectStatistics()->setNameAndDescriptionId(-366796655, 179614580));

    registerBlock(8, L"flowing_water",
        (new DynamicLiquidBlock(Material::WATER))->setDestroyTime(100.0f)->setLightBlock(2)->setIconName(L"water_flow")->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(886002090, -1593126065));

    registerBlock(9, L"water",
        (new StaticLiquidBlock(Material::WATER))->setDestroyTime(100.0f)->setLightBlock(2)->setIconName(L"water_still")->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(886002090, -1593126065));

    registerBlock(10, L"flowing_lava",
        (new DynamicLiquidBlock(Material::LAVA))->setDestroyTime(100.0f)->setLightEmission(1.0f)->setIconName(L"lava_flow")->setLightBlock(14)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-285958635, 791466680));

    registerBlock(11, L"lava",
        (new StaticLiquidBlock(Material::LAVA))->setDestroyTime(100.0f)->setLightEmission(1.0f)->setIconName(L"lava_still")->setLightBlock(14)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-285958635, 791466680));

    registerBlock(12, L"sand",
        (new SandBlock())->setDestroyTime(0.5f)->setSoundType(SoundType::SAND)->setIconName(L"sand")->setNameAndDescriptionId(-461941713, 738590338)->setBaseItemTypeAndMaterial(13, 13));

    registerBlock(13, L"gravel",
        (new GravelBlock())->setDestroyTime(0.6f)->setSoundType(SoundType::GRAVEL)->setIconName(L"gravel")->setNameAndDescriptionId(1789965892, -1090874487));

    registerBlock(14, L"gold_ore",
        (new OreBlock())->setDestroyTime(3.0f)->setExplodeable(5.0f)->setSoundType(SoundType::STONE)->setIconName(L"gold_ore")->setNameAndDescriptionId(87729608, 1159503909));

    registerBlock(15, L"iron_ore",
        (new OreBlock())->setDestroyTime(3.0f)->setExplodeable(5.0f)->setSoundType(SoundType::STONE)->setIconName(L"iron_ore")->setNameAndDescriptionId(-1157619462, -651919785));

    registerBlock(16, L"coal_ore",
        (new OreBlock())->setDestroyTime(3.0f)->setExplodeable(5.0f)->setSoundType(SoundType::STONE)->setIconName(L"coal_ore")->setNameAndDescriptionId(-798334967, -309588396));

    registerBlock(17, L"log",
        (new OldLogBlock())->setIconName(L"log")->sendBlockData(15)->setNameAndDescriptionId(1652869627, 1299556926));

    registerBlock(18, L"leaves",
        (new OldLeafBlock())->setIconName(L"leaves")->sendBlockData(3)->setNameAndDescriptionId(-806242921, -636152246));

    registerBlock(19, L"sponge",
        (new SpongeBlock())->setDestroyTime(0.6f)->setSoundType(SoundType::GRASS)->setIconName(L"sponge")->setNameAndDescriptionId(-1164385321, 1450219226));

    registerBlock(20, L"glass",
        (new GlassBlock(Material::GLASS, 0))->setDestroyTime(0.3f)->setSoundType(SoundType::GLASS)->setIconName(L"glass")->setNameAndDescriptionId(1743648693, -208402928)->setBaseItemTypeAndMaterial(42, 34));

    registerBlock(21, L"lapis_ore",
        (new OreBlock())->setDestroyTime(3.0f)->setExplodeable(5.0f)->setSoundType(SoundType::STONE)->setIconName(L"lapis_ore")->setNameAndDescriptionId(-1756036801, -1391530246));

    registerBlock(22, L"lapis_block",
        (new BlockImpl(Material::METAL))->setDestroyTime(3.0f)->setExplodeable(5.0f)->setSoundType(SoundType::STONE)->setIconName(L"lapis_block")->setNameAndDescriptionId(1241032974, -1933971317)->setBaseItemTypeAndMaterial(32, 10));

    registerBlock(23, L"dispenser",
        (new DispenserBlock())->setDestroyTime(3.5f)->setSoundType(SoundType::STONE)->setIconName(L"dispenser")->sendBlockData(15)->setNameAndDescriptionId(1725322846, -1023739365)->setBaseItemTypeAndMaterial(51, 2));

    Block* sandstone = (new SandStoneBlock())->setSoundType(SoundType::STONE)->setIconName(L"sandstone")->setDestroyTime(0.8f)->sendBlockData(15)->sendBlockData(15)->setNameAndDescriptionId(-803557604, 1826546953)->setBaseItemTypeAndMaterial(13, 13);
    registerBlock(24, L"sandstone", sandstone);

    registerBlock(25, L"noteblock",
        (new NoteBlock())->setSoundType(SoundType::WOOD)->setDestroyTime(0.8f)->setIconName(L"noteblock")->sendBlockData(15)->setBaseItemTypeAndMaterial(56, 0)->setNameAndDescriptionId(573932120, 251253259));

    registerBlock(26, L"bed",
        (new BedBlock())->setSoundType(SoundType::WOOD)->setDestroyTime(0.2f)->setIconName(L"bed")->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-2044341404, -1192892383));

    registerBlock(27, L"golden_rail",
        (new PoweredRailBlock())->setDestroyTime(0.7f)->setSoundType(SoundType::METAL)->setIconName(L"rail_golden")->sendBlockData(15)->setNameAndDescriptionId(1765197957, 1191188682)->setBaseItemTypeAndMaterial(12, 9)->disableMipmap());

    registerBlock(28, L"detector_rail",
        (new DetectorRailBlock())->setDestroyTime(0.7f)->setSoundType(SoundType::METAL)->setIconName(L"rail_detector")->sendBlockData(15)->setNameAndDescriptionId(-920002828, -688423498)->disableMipmap()->setBaseItemTypeAndMaterial(12, 8));

    registerBlock(29, L"pistonStickyBase",
        (new PistonBaseBlock(1))->sendBlockData(15)->setNameAndDescriptionId(-1070671485, -524444667)->setBaseItemTypeAndMaterial(27, 28)->setIconName(L"sticky_piston"));

    registerBlock(30, L"web",
        (new WebBlock())->setLightBlock(1)->setDestroyTime(4.0f)->setIconName(L"web")->setNameAndDescriptionId(-1091729841, -1572113478)->disableMipmap());

    registerBlock(31, L"tallgrass",
        (new TallGrassBlock())->setDestroyTime(0.0f)->setSoundType(SoundType::GRASS)->setIconName(L"tallgrass")->setNameAndDescriptionId(1678129041, -692404068)->disableMipmap());

    registerBlock(32, L"deadbush",
        (new DeadBushBlock())->setDestroyTime(0.0f)->setSoundType(SoundType::GRASS)->setIconName(L"deadbush")->setNameAndDescriptionId(-252434952, 1558910013)->disableMipmap());

    registerBlock(33, L"piston",
        (new PistonBaseBlock(0))->sendBlockData(15)->setNameAndDescriptionId(-1724938342, 678207717)->setBaseItemTypeAndMaterial(27, 28)->setIconName(L"pistonBase"));

    registerBlock(34, L"piston_head",
        (new PistonHeadBlock())->setNameAndDescriptionId(-1724938342, 678207717)->sendBlockData(15));

    registerBlock(35, L"wool",
        (new ColoredBlock(Material::WOOL))->setDestroyTime(0.8f)->setSoundType(SoundType::CLOTH)->setIconName(L"wool_colored")->sendBlockData(15)->setNameAndDescriptionId(-1090398773, 163579633)->setBaseItemTypeAndMaterial(49, 55));

    registerBlock(36, L"piston_extension",
        (new PistonMagicBlock())->setIconName(L"pistonBase")->setNameAndDescriptionId(-1724938342, -1));

    registerBlock(37, L"yellow_flower",
        (new YellowFlowerBlock())->setDestroyTime(0.0f)->setSoundType(SoundType::GRASS)->setIconName(L"flower_dandelion")->setNameAndDescriptionId(1134989826, 1194834415)->disableMipmap());

    registerBlock(38, L"red_flower",
        (new RedFlowerBlock())->setDestroyTime(0.0f)->setSoundType(SoundType::GRASS)->setIconName(L"flower_rose")->setNameAndDescriptionId(-1991483335, 1194834415)->disableMipmap());

    registerBlock(39, L"brown_mushroom",
        (new MushroomBlock())->setDestroyTime(0.0f)->setSoundType(SoundType::GRASS)->setLightEmission(0.125f)->setIconName(L"mushroom_brown")->setNameAndDescriptionId(192600811, 1857369046)->disableMipmap());

    registerBlock(40, L"red_mushroom",
        (new MushroomBlock())->setDestroyTime(0.0f)->setSoundType(SoundType::GRASS)->setIconName(L"mushroom_red")->setNameAndDescriptionId(192600811, 1857369046)->disableMipmap());

    registerBlock(41, L"gold_block",
        (new BlockImpl(Material::METAL, MaterialColor::GOLD))->setDestroyTime(3.0f)->setExplodeable(10.0f)->setSoundType(SoundType::METAL)->setIconName(L"gold_block")->setNameAndDescriptionId(-537198959, -267065668)->setBaseItemTypeAndMaterial(32, 4));

    registerBlock(42, L"iron_block",
        (new BlockImpl(Material::METAL, MaterialColor::METAL))->setDestroyTime(5.0f)->setExplodeable(10.0f)->setSoundType(SoundType::METAL)->setIconName(L"iron_block")->setNameAndDescriptionId(941478371, -146969842)->setBaseItemTypeAndMaterial(32, 3));

    registerBlock(43, L"double_stone_slab",
        (new FullStoneSlabBlock())->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setIconName(L"stoneSlab")->setNameAndDescriptionId(1625160352, -2103709410)->setBaseItemTypeAndMaterial(21, 2));

    registerBlock(44, L"stone_slab",
        (new HalfStoneSlabBlock())->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setIconName(L"stoneSlab")->setNameAndDescriptionId(1625160352, 859572981)->setBaseItemTypeAndMaterial(22, 2));

    Block* brickBlock = (new BlockImpl(Material::STONE, MaterialColor::COLOR_RED))->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setIconName(L"brick")->setNameAndDescriptionId(1699884934, 71287859)->setBaseItemTypeAndMaterial(13, 14);
    registerBlock(45, L"brick_block", brickBlock);

    registerBlock(46, L"tnt",
        (new TntBlock())->setDestroyTime(0.0f)->setSoundType(SoundType::GRASS)->setIconName(L"tnt")->setNameAndDescriptionId(-382113911, -1145801876)->setBaseItemTypeAndMaterial(28, 0));

    registerBlock(47, L"bookshelf",
        (new BookshelfBlock())->setDestroyTime(1.5f)->setSoundType(SoundType::WOOD)->setIconName(L"bookshelf")->setIconName(L"bookshelf")->setNameAndDescriptionId(940001922, -655217577)->setBaseItemTypeAndMaterial(46, 52));

    registerBlock(48, L"mossy_cobblestone",
        (new BlockImpl(Material::STONE))->setIconName(L"cobblestone_mossy")->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(1763304729, 1357145140)->setBaseItemTypeAndMaterial(14, 2));

    registerBlock(49, L"obsidian",
        (new ObsidianBlock())->setDestroyTime(50.0f)->setExplodeable(2000.0f)->setTicking(1)->setSoundType(SoundType::STONE)->setIconName(L"obsidian")->setNameAndDescriptionId(75283994, -1953294233)->setBaseItemTypeAndMaterial(13, 2));

    registerBlock(50, L"torch",
        (new TorchBlock())->setDestroyTime(0.0f)->setLightEmission(0.9375f)->setSoundType(SoundType::WOOD)->setIconName(L"torch_on")->setNameAndDescriptionId(-422971867, -1189036672)->disableMipmap()->setBaseItemTypeAndMaterial(23, 1));

    registerBlock(51, L"fire",
        (new FireBlock())->setDestroyTime(0.0f)->setLightEmission(1.0f)->setSoundType(SoundType::CLOTH)->setIconName(L"fire")->setNotCollectStatistics()->setNameAndDescriptionId(-1247941401, -1)->disableMipmap());

    registerBlock(52, L"mob_spawner",
        (new MobSpawnerBlock())->setDestroyTime(5.0f)->setSoundType(SoundType::METAL)->setIconName(L"mob_spawner")->setNotCollectStatistics()->setNameAndDescriptionId(1689012380, 4403401));

    registerBlock(53, L"oak_stairs",
        (new StairBlock(planks->defaultBlockState()->setBoxedValue(PlanksBlock::VARIANT, new TypedBoxed<PlanksBlock::Variant*>(PlanksBlock::Variant::OAK))))->setIconName(L"stairsWood")->sendBlockData(15)->setNameAndDescriptionId(-779521723, 1962448236)->setBaseItemTypeAndMaterial(16, 1));

    registerBlock(54, L"chest",
        (new ChestBlock(ChestBlock::TYPE_SINGLE))->setDestroyTime(2.5f)->setSoundType(SoundType::WOOD)->setIconName(L"chest")->sendBlockData(15)->setNameAndDescriptionId(1888250110, -1487670933)->setBaseItemTypeAndMaterial(36, 1));

    registerBlock(55, L"redstone_wire",
        (new RedStoneWireBlock())->setDestroyTime(0.0f)->setSoundType(SoundType::STONE)->setIconName(L"redstone_dust")->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(733836782, -2085079141)->disableMipmap());

    registerBlock(56, L"diamond_ore",
        (new OreBlock())->setDestroyTime(3.0f)->setExplodeable(5.0f)->setSoundType(SoundType::STONE)->setIconName(L"diamond_ore")->setNameAndDescriptionId(-2051872732, 526693633));

    registerBlock(57, L"diamond_block",
        (new BlockImpl(Material::METAL, MaterialColor::DIAMOND))->setDestroyTime(5.0f)->setExplodeable(10.0f)->setSoundType(SoundType::METAL)->setIconName(L"diamond_block")->setNameAndDescriptionId(1557210581, 1044427824)->setBaseItemTypeAndMaterial(32, 5));

    registerBlock(58, L"crafting_table",
        (new CraftingTableBlock())->setDestroyTime(2.5f)->setSoundType(SoundType::WOOD)->setIconName(L"crafting_table")->setNameAndDescriptionId(-488183524, -405433066)->setBaseItemTypeAndMaterial(30, 1));

    registerBlock(59, L"wheat",
        (new CropBlock(0))->setIconName(L"wheat")->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-1565208526, -355421273)->disableMipmap());

    Block* farmland = (new FarmBlock())->setDestroyTime(0.6f)->setSoundType(SoundType::GRAVEL)->setIconName(L"farmland")->sendBlockData(15)->setNameAndDescriptionId(934190936, 635505253);
    registerBlock(60, L"farmland", farmland);

    registerBlock(61, L"furnace",
        (new FurnaceBlock(false))->setDestroyTime(3.5f)->setSoundType(SoundType::STONE)->setIconName(L"furnace")->sendBlockData(15)->setNameAndDescriptionId(-2146068145, 339052362)->setBaseItemTypeAndMaterial(30, 2));

    registerBlock(62, L"lit_furnace",
        (new FurnaceBlock(true))->setDestroyTime(3.5f)->setSoundType(SoundType::STONE)->setIconName(L"furnace")->setLightEmission(0.875f)->sendBlockData(15)->setNameAndDescriptionId(-2146068145, 339052362));

    registerBlock(63, L"standing_sign",
        (new StandingSignBlock())->setDestroyTime(1.0f)->setSoundType(SoundType::WOOD)->setIconName(L"sign")->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-922556236, -1238865095));

    registerBlock(64, L"wooden_door",
        (new DoorBlock(Material::WOOD))->setDestroyTime(3.0f)->setSoundType(SoundType::WOOD)->setIconName(L"door_wood")->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(1964228784, 716140400));

    registerBlock(65, L"ladder",
        (new LadderBlock())->setDestroyTime(0.4f)->setSoundType(SoundType::LADDER)->setIconName(L"ladder")->sendBlockData(15)->setNameAndDescriptionId(-1253079859, -2140469840)->disableMipmap()->setBaseItemTypeAndMaterial(48, 1));

    registerBlock(66, L"rail",
        (new RailBlock())->setDestroyTime(0.7f)->setSoundType(SoundType::METAL)->setIconName(L"rail_normal")->sendBlockData(15)->setNameAndDescriptionId(2060256657, 207750380)->setBaseItemTypeAndMaterial(12, 9)->disableMipmap());

    registerBlock(67, L"stone_stairs",
        (new StairBlock(cobblestone->defaultBlockState()))->setIconName(L"stairsStone")->sendBlockData(15)->setNameAndDescriptionId(883201333, 1962448236)->setBaseItemTypeAndMaterial(16, 2));

    registerBlock(68, L"wall_sign",
        (new WallSignBlock())->setDestroyTime(1.0f)->setSoundType(SoundType::WOOD)->setIconName(L"sign")->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-922556236, -1238865095));

    registerBlock(69, L"lever",
        (new LeverBlock())->setDestroyTime(0.5f)->setSoundType(SoundType::WOOD)->setIconName(L"lever")->sendBlockData(15)->setNameAndDescriptionId(1523807903, 423055482)->setBaseItemTypeAndMaterial(45, 1));

    registerBlock(70, L"stone_pressure_plate",
        (new PressurePlateBlock(L"stone", Material::STONE, PressurePlateBlock::MOBS))->setDestroyTime(0.5f)->setSoundType(SoundType::STONE)->sendBlockData(15)->setNameAndDescriptionId(-1688694459, 2139474008)->setBaseItemTypeAndMaterial(15, 2));

    registerBlock(71, L"iron_door",
        (new DoorBlock(Material::METAL))->setIconName(L"door_iron")->setDestroyTime(5.0f)->setSoundType(SoundType::METAL)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-1764856892, -458439679));

    registerBlock(72, L"wooden_pressure_plate",
        (new PressurePlateBlock(L"planks_oak", Material::WOOD, PressurePlateBlock::EVERYTHING))->setDestroyTime(0.5f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(3680245, -413015488)->setBaseItemTypeAndMaterial(15, 1));

    registerBlock(73, L"redstone_ore",
        (new RedStoneOreBlock(0))->setIconName(L"redstone_ore")->setDestroyTime(3.0f)->setExplodeable(5.0f)->setSoundType(SoundType::STONE)->sendBlockData(15)->setNameAndDescriptionId(-1282273712, 1472513597));

    registerBlock(74, L"lit_redstone_ore",
        (new RedStoneOreBlock(1))->setIconName(L"redstone_ore")->setLightEmission(0.625f)->setDestroyTime(3.0f)->setExplodeable(5.0f)->setSoundType(SoundType::STONE)->sendBlockData(15)->setNameAndDescriptionId(-1282273712, 1472513597));

    registerBlock(75, L"unlit_redstone_torch",
        (new RedstoneTorchBlock(0))->setIconName(L"redstone_torch_off")->setDestroyTime(0.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(2037557570, 780434656)->disableMipmap());

    registerBlock(76, L"redstone_torch",
        (new RedstoneTorchBlock(1))->setIconName(L"redstone_torch_on")->setDestroyTime(0.0f)->setLightEmission(0.5f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(2037557570, 780434656)->disableMipmap());

    registerBlock(77, L"stone_button",
        (new StoneButtonBlock())->setIconName(L"stone")->setDestroyTime(0.5f)->setSoundType(SoundType::STONE)->sendBlockData(15)->setNameAndDescriptionId(1464423445, -753144148)->setBaseItemTypeAndMaterial(35, 2));

    registerBlock(78, L"snow_layer",
        (new SnowLayerBlock())->setIconName(L"snow")->setDestroyTime(0.1f)->setSoundType(SoundType::SNOW)->sendBlockData(15)->setLightBlock(0)->setNameAndDescriptionId(-226754302, -1353574661)->setBaseItemTypeAndMaterial(13, 16));

    registerBlock(79, L"ice",
        (new IceBlock(0))->setIconName(L"ice")->setDestroyTime(0.5f)->setLightBlock(3)->setSoundType(SoundType::GLASS)->setSemiTransparent()->setNameAndDescriptionId(-1972979160, 1699472637));

    registerBlock(80, L"snow",
        (new SnowBlock())->setIconName(L"snow")->setDestroyTime(0.2f)->setSoundType(SoundType::CLOTH)->setNameAndDescriptionId(-226754302, 1621205791)->setBaseItemTypeAndMaterial(13, 16));

    registerBlock(81, L"cactus",
        (new CactusBlock())->setIconName(L"cactus")->setDestroyTime(0.4f)->setSoundType(SoundType::CLOTH)->setNameAndDescriptionId(-1222532308, -1996497743)->disableMipmap());

    registerBlock(82, L"clay",
        (new ClayBlock())->setIconName(L"clay")->setDestroyTime(0.6f)->setSoundType(SoundType::GRAVEL)->setNameAndDescriptionId(556408288, -1246404842)->setBaseItemTypeAndMaterial(13, 15));

    registerBlock(83, L"reeds",
        (new ReedsBlock())->setIconName(L"reeds")->setDestroyTime(0.0f)->setSoundType(SoundType::GRASS)->setNotCollectStatistics()->setNameAndDescriptionId(-1754411966, 697296055)->disableMipmap());

    registerBlock(84, L"jukebox",
        (new JukeboxBlock())->setIconName(L"jukebox")->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->sendBlockData(15)->setNameAndDescriptionId(676972475, -2057803490)->setBaseItemTypeAndMaterial(53, 0));

    registerBlock(85, L"fence",
        (new FenceBlock(L"planks_oak", Material::WOOD, PlanksBlock::Variant::OAK->getMaterialColor()))->setDestroyTime(2.0f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(1463281045, 1128628177)->setBaseItemTypeAndMaterial(29, 1));

    registerBlock(86, L"carved_pumpkin",
        (new PumpkinBlock())->setIconName(L"pumpkin")->setDestroyTime(1.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(-235109589, 278425254));

    registerBlock(257, L"pumpkin",
        (new PumpkinBlock())->setIconName(L"pumpkin")->setDestroyTime(1.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(-1271821741, 1522582022));

    registerBlock(87, L"netherrack",
        (new NetherrackBlock())->setIconName(L"netherrack")->setDestroyTime(0.4f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-916997000, -407860275));

    registerBlock(88, L"soul_sand",
        (new SoulSandBlock())->setIconName(L"soul_sand")->setDestroyTime(0.5f)->setSoundType(SoundType::SAND)->setNameAndDescriptionId(-871834659, -1364743816));

    registerBlock(89, L"glowstone",
        (new Glowstoneblock(Material::GLASS))->setIconName(L"glowstone")->setDestroyTime(0.3f)->setSoundType(SoundType::GLASS)->setLightEmission(1.0f)->setNameAndDescriptionId(1524807217, -2004119014)->setBaseItemTypeAndMaterial(23, 23));

    registerBlock(90, L"portal",
        (new PortalBlock())->setDestroyTime(-1.0f)->setIconName(L"portal")->setSoundType(SoundType::GLASS)->setLightEmission(0.75f)->setNameAndDescriptionId(2020085891, 361110974));

    registerBlock(91, L"lit_pumpkin",
        (new PumpkinBlock())->setIconName(L"pumpkin")->setDestroyTime(1.0f)->setSoundType(SoundType::WOOD)->setLightEmission(1.0f)->sendBlockData(15)->setNameAndDescriptionId(-144849667, 211640248)->setBaseItemTypeAndMaterial(23, 22));

    registerBlock(92, L"cake",
        (new CakeBlock())->setIconName(L"cake")->setDestroyTime(0.5f)->setSoundType(SoundType::CLOTH)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-1614580205, 464030782));

    registerBlock(93, L"unpowered_repeater",
        (new RepeaterBlock(0))->setIconName(L"repeater_off")->setDestroyTime(0.0f)->setSoundType(SoundType::WOOD)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-1183081123, -128546302)->disableMipmap()->setBaseItemTypeAndMaterial(59, 0));

    registerBlock(94, L"powered_repeater",
        (new RepeaterBlock(1))->setIconName(L"repeater_on")->setDestroyTime(0.0f)->setSoundType(SoundType::WOOD)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-1183081123, -128546302)->disableMipmap()->setBaseItemTypeAndMaterial(59, 0));

    registerBlock(95, L"stained_glass",
        (new StainedGlassBlock(Material::GLASS))->setIconName(L"glass")->setDestroyTime(0.3f)->setSoundType(SoundType::GLASS)->setNameAndDescriptionId(-2141183110, 1295116847)->setSemiTransparent()->setBaseItemTypeAndMaterial(42, 34));

    registerBlock(96, L"trapdoor",
        (new TrapDoorBlock(Material::WOOD, MaterialColor::WOOD))->setIconName(L"trapdoor")->setDestroyTime(3.0f)->setSoundType(SoundType::WOOD)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-579730614, 1969487947)->setBaseItemTypeAndMaterial(6, 25));

    registerBlock(97, L"monster_egg",
        (new MonsterEggBlock())->setIconName(L"monsterStoneEgg")->setDestroyTime(0.75f)->setNameAndDescriptionId(786740680, 1095399893));

    Block* stonebrick = (new StoneBrickBlock())->setIconName(L"stonebrick")->setDestroyTime(1.5f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-1018384393, -1881711158)->setBaseItemTypeAndMaterial(14, 31);
    registerBlock(98, L"stonebrick", stonebrick);

    registerBlock(99, L"brown_mushroom_block",
        (new HugeMushroomBlock(Material::WOOD, MaterialColor::DIRT, byId(39)))->setIconName(L"mushroom_block")->setDestroyTime(0.2f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(-335533579, 1445448480)->sendBlockData(15));

    registerBlock(100, L"red_mushroom_block",
        (new HugeMushroomBlock(Material::WOOD, MaterialColor::COLOR_RED, byId(40)))->setIconName(L"mushroom_block")->setDestroyTime(0.2f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(302163038, 1445448480)->sendBlockData(15));

    registerBlock(101, L"iron_bars",
        (new ThinFenceBlock(L"iron_bars", L"iron_bars", Material::METAL, true))->setDestroyTime(5.0f)->setExplodeable(10.0f)->setSoundType(SoundType::METAL)->setNameAndDescriptionId(-88007431, -318588764)->setBaseItemTypeAndMaterial(29, 3));

    registerBlock(102, L"glass_pane",
        (new ThinFenceBlock(L"glass", L"glass_pane_top", Material::GLASS, false))->setDestroyTime(0.3f)->setSoundType(SoundType::GLASS)->setNameAndDescriptionId(1607245545, 736707028)->setBaseItemTypeAndMaterial(42, 34));

    registerBlock(103, L"melon_block",
        (new MelonBlock())->setIconName(L"melon")->setDestroyTime(1.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(-591549446, -34103255));

    registerBlock(104, L"pumpkin_stem",
        (new StemBlock())->setIconName(L"pumpkin_stem")->setDestroyTime(0.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(-1022325989, -1)->disableMipmap());

    registerBlock(105, L"melon_stem",
        (new StemBlock())->setIconName(L"melon_stem")->setDestroyTime(0.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(630381882, -1)->disableMipmap());

    registerBlock(106, L"vine",
        (new VineBlock())->setIconName(L"vine")->setDestroyTime(0.2f)->setSoundType(SoundType::GRASS)->sendBlockData(15)->setNameAndDescriptionId(1791100347, -334192122)->disableMipmap());

    registerBlock(107, L"fence_gate",
        (new FenceGateBlock(L"planks_oak", PlanksBlock::Variant::OAK))->setIconName(L"fenceGate")->setDestroyTime(2.0f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(-1761600827, -1927890567)->setBaseItemTypeAndMaterial(47, 1));

    registerBlock(108, L"brick_stairs",
        (new StairBlock(brickBlock->defaultBlockState()))->setIconName(L"stairsBrick")->sendBlockData(15)->setNameAndDescriptionId(-1097401796, 1962448236)->setBaseItemTypeAndMaterial(16, 14));

    registerBlock(109, L"stone_brick_stairs",
        (new StairBlock(stonebrick->defaultBlockState()->setBoxedValue(StoneBrickBlock::VARIANT, new TypedBoxed<StoneBrickBlock::Variant*>(StoneBrickBlock::Variant::DEFAULT))))->setIconName(L"stairsStoneBrickSmooth")->sendBlockData(15)->setNameAndDescriptionId(-251417459, 1962448236)->setBaseItemTypeAndMaterial(16, 31));

    registerBlock(110, L"mycelium",
        (new MyceliumBlock())->setIconName(L"mycelium")->setDestroyTime(0.6f)->setSoundType(SoundType::GRASS)->setNameAndDescriptionId(1212229689, 1645782764));

    registerBlock(111, L"waterlily",
        (new WaterlilyBlock())->setIconName(L"waterlily")->setDestroyTime(0.0f)->setSoundType(SoundType::GRASS)->setNameAndDescriptionId(-1831372102, -709402049)->disableMipmap());

    Block* netherBrick = (new NetherBrickBlock())->setIconName(L"nether_brick")->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(579553374, -759882693)->setBaseItemTypeAndMaterial(13, 32);
    registerBlock(112, L"nether_brick", netherBrick);

    registerBlock(113, L"nether_brick_fence",
        (new FenceBlock(L"nether_brick", Material::STONE, MaterialColor::NETHER))->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-561299988, 2016476681)->setBaseItemTypeAndMaterial(29, 32));

    registerBlock(114, L"nether_brick_stairs",
        (new StairBlock(netherBrick->defaultBlockState()))->setIconName(L"stairsNetherBrick")->sendBlockData(15)->setNameAndDescriptionId(-360518601, 1962448236)->setBaseItemTypeAndMaterial(16, 32));

    registerBlock(115, L"nether_wart",
        (new NetherWartBlock())->setIconName(L"nether_wart")->sendBlockData(15)->setNameAndDescriptionId(-1730502778, 1370132419)->disableMipmap());

    registerBlock(116, L"enchanting_table",
        (new EnchantmentTableBlock())->setIconName(L"enchanting_table")->setDestroyTime(5.0f)->setExplodeable(2000.0f)->setNameAndDescriptionId(1317837992, -2025668523)->setBaseItemTypeAndMaterial(30, 36));

    registerBlock(117, L"brewing_stand",
        (new BrewingStandBlock())->setIconName(L"brewing_stand")->setDestroyTime(0.5f)->setLightEmission(0.125f)->sendBlockData(15)->setNameAndDescriptionId(539335289, -1918013109)->setBaseItemTypeAndMaterial(30, 35));

    registerBlock(118, L"cauldron",
        (new CauldronBlock())->setIconName(L"cauldron")->setDestroyTime(2.0f)->sendBlockData(15)->setNameAndDescriptionId(-1383143349, -1833942746));

    registerBlock(119, L"end_portal",
        (new EndPortalBlock(Material::PORTAL))->setDestroyTime(0.0f)->setExplodeable(6000000.0f)->setNameAndDescriptionId(1238979323, -666504698));

    registerBlock(120, L"end_portal_frame",
        (new EndPortalFrameBlock())->setIconName(L"endframe")->setSoundType(SoundType::GLASS)->setLightEmission(0.125f)->setDestroyTime(0.0f)->sendBlockData(15)->setExplodeable(6000000.0f)->setNameAndDescriptionId(-561531743, 1304651180));

    registerBlock(121, L"end_stone",
        (new BlockImpl(Material::STONE, MaterialColor::SAND))->setIconName(L"end_stone")->setDestroyTime(3.0f)->setExplodeable(15.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(1896130883, -482475298));

    registerBlock(122, L"dragon_egg",
        (new DragonEggBlock())->setDestroyTime(3.0f)->setIconName(L"dragon_egg")->setExplodeable(15.0f)->setSoundType(SoundType::STONE)->setLightEmission(0.125f)->setNameAndDescriptionId(-2096307741, 2006994726));

    registerBlock(123, L"redstone_lamp",
        (new RedstoneLampBlock(0))->setIconName(L"redstone_lamp_off")->setDestroyTime(0.3f)->setSoundType(SoundType::GLASS)->setBaseItemTypeAndMaterial(56, 0)->setNameAndDescriptionId(1627139062, -1622610869));

    registerBlock(124, L"lit_redstone_lamp",
        (new RedstoneLampBlock(1))->setIconName(L"redstone_lamp_on")->setDestroyTime(0.3f)->setSoundType(SoundType::GLASS)->setBaseItemTypeAndMaterial(56, 0)->setNameAndDescriptionId(1627139062, -1622610869));

    registerBlock(125, L"double_wooden_slab",
        (new FullWoodSlabBlock())->setIconName(L"woodSlab")->setDestroyTime(2.0f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(754256853, 754256853)->setBaseItemTypeAndMaterial(21, 1));

    registerBlock(126, L"wooden_slab",
        (new HalfWoodSlabBlock())->setIconName(L"woodSlab")->setDestroyTime(2.0f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(754256853, 754256853)->setBaseItemTypeAndMaterial(22, 1));

    registerBlock(127, L"cocoa",
        (new CocoaBlock())->setIconName(L"cocoa")->setDestroyTime(0.2f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(-344420786, -1677943605));

    registerBlock(128, L"sandstone_stairs",
        (new StairBlock(sandstone->defaultBlockState()->setBoxedValue(SandStoneBlock::TYPE, new TypedBoxed<SandStoneBlock::Type*>(SandStoneBlock::SMOOTH))))->setIconName(L"stairsSandstone")->sendBlockData(15)->setNameAndDescriptionId(-1842581131, 1962448236)->setBaseItemTypeAndMaterial(16, 13));

    registerBlock(129, L"emerald_ore",
        (new OreBlock())->setIconName(L"emerald_ore")->setDestroyTime(3.0f)->setExplodeable(5.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(1525740761, 1075886516));

    registerBlock(130, L"ender_chest",
        (new EnderChestBlock())->setDestroyTime(22.5f)->setIconName(L"enderChest")->setExplodeable(1000.0f)->setSoundType(SoundType::STONE)->sendBlockData(15)->setLightEmission(0.5f)->setNameAndDescriptionId(-621298906, 1270672747)->setBaseItemTypeAndMaterial(36, 33));

    registerBlock(131, L"tripwire_hook",
        (new TripWireHookBlock())->setIconName(L"trip_wire_source")->sendBlockData(15)->setNameAndDescriptionId(1949301963, -1376884498)->setBaseItemTypeAndMaterial(45, 0));

    registerBlock(132, L"tripwire",
        (new TripWireBlock())->setIconName(L"trip_wire")->sendBlockData(15)->setNameAndDescriptionId(57442505, -189186572)->disableMipmap());

    registerBlock(133, L"emerald_block",
        (new BlockImpl(Material::METAL, MaterialColor::EMERALD))->setIconName(L"emerald_block")->setDestroyTime(5.0f)->setExplodeable(10.0f)->setSoundType(SoundType::METAL)->setNameAndDescriptionId(1707855608, 1799850453)->setBaseItemTypeAndMaterial(32, 44));

    registerBlock(134, L"spruce_stairs",
        (new StairBlock(planks->defaultBlockState()->setBoxedValue(PlanksBlock::VARIANT, new TypedBoxed<PlanksBlock::Variant*>(PlanksBlock::Variant::SPRUCE))))->setIconName(L"stairsWoodSpruce")->sendBlockData(15)->setNameAndDescriptionId(666001927, 1962448236)->setBaseItemTypeAndMaterial(16, 39));

    registerBlock(135, L"birch_stairs",
        (new StairBlock(planks->defaultBlockState()->setBoxedValue(PlanksBlock::VARIANT, new TypedBoxed<PlanksBlock::Variant*>(PlanksBlock::Variant::BIRCH))))->setIconName(L"stairsWoodBirch")->sendBlockData(15)->setNameAndDescriptionId(2140340631, 1962448236)->setBaseItemTypeAndMaterial(16, 40));

    registerBlock(136, L"jungle_stairs",
        (new StairBlock(planks->defaultBlockState()->setBoxedValue(PlanksBlock::VARIANT, new TypedBoxed<PlanksBlock::Variant*>(PlanksBlock::Variant::JUNGLE))))->setIconName(L"stairsWoodJungle")->sendBlockData(15)->setNameAndDescriptionId(1700333602, 1962448236)->setBaseItemTypeAndMaterial(16, 41));

    registerBlock(137, L"command_block",
        (new CommandBlock(MaterialColor::COLOR_BROWN, CommandBlock::IMPULSE))->setIconName(L"command_block")->setIndestructible()->setExplodeable(6000000.0f)->setNameAndDescriptionId(1574311038, -79610325));

    registerBlock(138, L"beacon",
        (new BeaconBlock())->setIconName(L"beacon")->setLightEmission(1.0f)->setSoundType(SoundType::GLASS)->setNameAndDescriptionId(-63126613, -1056354586)->setBaseItemTypeAndMaterial(61, 34));

    registerBlock(139, L"cobblestone_wall",
        (new WallBlock())->setIconName(L"cobbleWall")->setNameAndDescriptionId(726532432, -1616412963)->setBaseItemTypeAndMaterial(29, 2));

    registerBlock(140, L"flower_pot",
        (new FlowerPotBlock())->setDestroyTime(0.0f)->setIconName(L"flower_pot")->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-691080247, -1237969044));

    registerBlock(141, L"carrots",
        (new CropBlock(0))->setIconName(L"carrots")->setNameAndDescriptionId(344335317, 1476904080)->disableMipmap());

    registerBlock(142, L"potatoes",
        (new PotatoBlock())->setIconName(L"potatoes")->setNameAndDescriptionId(-1607323512, 1877161723)->disableMipmap());

    registerBlock(143, L"wooden_button",
        (new WoodButtonBlock())->setIconName(L"planks_oak")->setBaseItemTypeAndMaterial(35, 1)->setDestroyTime(0.5f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(-1792100991, -753144148));

    registerBlock(144, L"skull",
        (new SkullBlock())->setIconName(L"skull")->setDestroyTime(1.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-31596006, -1989482401));

    registerBlock(145, L"anvil",
        (new AnvilBlock())->setIconName(L"anvil")->setBaseItemTypeAndMaterial(30, 3)->setDestroyTime(5.0f)->setSoundType(SoundType::ANVIL)->setExplodeable(2000.0f)->sendBlockData(15)->setNameAndDescriptionId(366579523, -2146312138));

    registerBlock(146, L"trapped_chest",
        (new ChestBlock(ChestBlock::TYPE_LEFT))->setBaseItemTypeAndMaterial(36, 25)->setDestroyTime(2.5f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(845811998, -1767515453));

    registerBlock(147, L"light_weighted_pressure_plate",
        (new WeightedPressurePlateBlock(L"gold_block", Material::METAL, 15, MaterialColor::GOLD))->setBaseItemTypeAndMaterial(15, 4)->setDestroyTime(0.5f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(1888521428, 1823420681));

    registerBlock(148, L"heavy_weighted_pressure_plate",
        (new WeightedPressurePlateBlock(L"iron_block", Material::METAL, 150, MaterialColor::METAL))->setBaseItemTypeAndMaterial(15, 3)->setDestroyTime(0.5f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(506712369, 986861820));

    registerBlock(149, L"unpowered_comparator",
        (new ComparatorBlock(0))->setIconName(L"comparator_off")->setDestroyTime(0.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(1442887949, -150634528)->disableMipmap()->setBaseItemTypeAndMaterial(59, 0));

    registerBlock(150, L"powered_comparator",
        (new ComparatorBlock(1))->setIconName(L"comparator_on")->setDestroyTime(0.0f)->setLightEmission(0.625f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(1442887949, -150634528)->disableMipmap()->setBaseItemTypeAndMaterial(59, 0));

    registerBlock(151, L"daylight_detector",
        (new DaylightDetectorBlock(0))->setIconName(L"daylight_detector")->setNameAndDescriptionId(1065283032, -379601475));

    registerBlock(152, L"redstone_block",
        (new PoweredBlock(Material::METAL, MaterialColor::FIRE))->setIconName(L"redstone_block")->setDestroyTime(5.0f)->setExplodeable(10.0f)->setSoundType(SoundType::METAL)->setNameAndDescriptionId(-2072282455, 963824100)->setBaseItemTypeAndMaterial(32, 48));

    registerBlock(153, L"quartz_ore",
        (new OreBlock(MaterialColor::NETHER))->setIconName(L"quartz_ore")->setDestroyTime(3.0f)->setExplodeable(5.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(1018040569, -1084182693));

    registerBlock(154, L"hopper",
        (new HopperBlock())->setIconName(L"hopper")->setDestroyTime(3.0f)->setExplodeable(8.0f)->setSoundType(SoundType::METAL)->setNameAndDescriptionId(2089418295, 2098570490)->setBaseItemTypeAndMaterial(51, 0));

    Block* quartzBlock = (new QuartzBlock())->setIconName(L"quartz_block")->setSoundType(SoundType::STONE)->setDestroyTime(0.8f)->setNameAndDescriptionId(1404172104, -1571399371)->setBaseItemTypeAndMaterial(14, 45);
    registerBlock(155, L"quartz_block", quartzBlock);

    registerBlock(156, L"quartz_stairs",
        (new StairBlock(quartzBlock->defaultBlockState()->setBoxedValue(QuartzBlock::VARIANT, new TypedBoxed<QuartzBlock::Variant*>(QuartzBlock::Variant::DEFAULT))))->setIconName(L"stairsQuartz")->setNameAndDescriptionId(2021542239, 1962448236)->setBaseItemTypeAndMaterial(16, 45));

    registerBlock(157, L"activator_rail",
        (new PoweredRailBlock())->setIconName(L"rail_activator")->setDestroyTime(0.7f)->setSoundType(SoundType::METAL)->setNameAndDescriptionId(-8062537, 238648458)->setBaseItemTypeAndMaterial(12, 9));

    registerBlock(158, L"dropper",
        (new DropperBlock())->setIconName(L"dropper")->setDestroyTime(3.5f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(634103265, -1693458140)->setBaseItemTypeAndMaterial(51, 0));

    registerBlock(159, L"stained_hardened_clay",
        (new StainedHardenedClayBlock())->setIconName(L"hardened_clay_stained")->setDestroyTime(1.25f)->setExplodeable(7.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(1192910743, 946027994)->setBaseItemTypeAndMaterial(41, 15));

    registerBlock(160, L"stained_glass_pane",
        (new StainedGlassPaneBlock())->setIconName(L"glass")->setDestroyTime(0.3f)->setSoundType(SoundType::GLASS)->setSemiTransparent()->setNameAndDescriptionId(-1809051305, 1981321514)->setBaseItemTypeAndMaterial(42, 34));

    registerBlock(161, L"leaves2",
        (new NewLeafBlock())->setIconName(L"leaves2")->sendBlockData(3)->setNameAndDescriptionId(-806242921, -636152246));

    registerBlock(162, L"log2",
        (new NewLogBlock())->setIconName(L"log")->sendBlockData(15)->setNameAndDescriptionId(1652869627, 1299556926));

    registerBlock(163, L"acacia_stairs",
        (new StairBlock(planks->defaultBlockState()->setBoxedValue(PlanksBlock::VARIANT, new TypedBoxed<PlanksBlock::Variant*>(PlanksBlock::Variant::ACACIA))))->setIconName(L"stairsWoodAcacia")->sendBlockData(15)->setNameAndDescriptionId(-2082414187, 1962448236)->setBaseItemTypeAndMaterial(16, 42));

    registerBlock(164, L"dark_oak_stairs",
        (new StairBlock(planks->defaultBlockState()->setBoxedValue(PlanksBlock::VARIANT, new TypedBoxed<PlanksBlock::Variant*>(PlanksBlock::Variant::DARK_OAK))))->setIconName(L"stairsWoodDarkOak")->sendBlockData(15)->setNameAndDescriptionId(1820864892, 1962448236)->setBaseItemTypeAndMaterial(16, 43));

    registerBlock(165, L"slime",
        (new SlimeBlock())->setSoundType(SoundType::SLIME)->setNameAndDescriptionId(1273472087, 208588370)->setBaseItemTypeAndMaterial(32, 57)->setSemiTransparent());

    registerBlock(166, L"barrier",
        (new BarrierBlock())->setIconName(L"barrier")->setNameAndDescriptionId(70640450, 711746119));

    registerBlock(167, L"iron_trapdoor",
        (new TrapDoorBlock(Material::METAL))->setIconName(L"iron_trapdoor")->setDestroyTime(5.0f)->setSoundType(SoundType::METAL)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-93457941, 361384478)->setBaseItemTypeAndMaterial(6, 25));

    Block* prismarine = (new PrismarineBlock())->setIconName(L"prismarine_rough")->setDestroyTime(1.5f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-1016146117, 1294842838)->setBaseItemTypeAndMaterial(13, 13);
    registerBlock(168, L"prismarine", prismarine);

    registerBlock(169, L"sea_lantern",
        (new SeaLanternBlock(Material::GLASS))->setIconName(L"sea_lantern")->setDestroyTime(0.3f)->setSoundType(SoundType::GLASS)->setLightEmission(1.0f)->setNameAndDescriptionId(2073873113, 2134025212)->setBaseItemTypeAndMaterial(23, 54));

    registerBlock(170, L"hay_block",
        (new HayBlock())->setDestroyTime(0.5f)->setIconName(L"hay_block")->setSoundType(SoundType::GRASS)->setNameAndDescriptionId(-628091201, 1790302378)->setBaseItemTypeAndMaterial(32, 53));

    registerBlock(171, L"carpet",
        (new WoolCarpetBlock())->setIconName(L"woolCarpet")->setDestroyTime(0.1f)->setSoundType(SoundType::CLOTH)->setLightBlock(0)->setNameAndDescriptionId(-67474618, 29708315)->setBaseItemTypeAndMaterial(40, 6));

    registerBlock(172, L"hardened_clay",
        (new HardenedClayBlock())->setIconName(L"hardened_clay")->setDestroyTime(1.25f)->setExplodeable(7.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(2142500442, 954439535)->setBaseItemTypeAndMaterial(41, 15));

    registerBlock(173, L"coal_block",
        (new BlockImpl(Material::STONE, MaterialColor::COLOR_BLACK))->setIconName(L"coal_block")->setDestroyTime(5.0f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(2112358854, 2042591469)->setBaseItemTypeAndMaterial(32, 49));

    registerBlock(174, L"packed_ice",
        (new PackedIceBlock())->setIconName(L"ice_packed")->setDestroyTime(0.5f)->setSoundType(SoundType::GLASS)->setNameAndDescriptionId(474586919, -1694722534));

    registerBlock(175, L"double_plant",
        (new DoublePlantBlock())->setNameAndDescriptionId(1348410234, -31749657)->disableMipmap());

    registerBlock(176, L"standing_banner",
        (new StandingBannerBlock())->setIconName(L"planks_oak")->setDestroyTime(1.0f)->setSoundType(SoundType::WOOD)->setNotCollectStatistics()->setNameAndDescriptionId(445928624, 1713023493));

    registerBlock(177, L"wall_banner",
        (new WallBannerBlock())->setIconName(L"planks_oak")->setDestroyTime(1.0f)->setSoundType(SoundType::WOOD)->setNotCollectStatistics()->setNameAndDescriptionId(631422002, 1567917047));

    registerBlock(178, L"daylight_detector_inverted",
        (new DaylightDetectorBlock(1))->setIconName(L"daylight_detector")->setNameAndDescriptionId(1065283032, -379601475));

    Block* redSandstone = (new RedSandStoneBlock())->setIconName(L"red_sandstone")->setSoundType(SoundType::STONE)->setDestroyTime(0.8f)->setNameAndDescriptionId(1891675551, -1229413198)->setBaseItemTypeAndMaterial(13, 48);
    registerBlock(179, L"red_sandstone", redSandstone);

    registerBlock(180, L"red_sandstone_stairs",
        (new StairBlock(redSandstone->defaultBlockState()->setBoxedValue(RedSandStoneBlock::TYPE, new TypedBoxed<RedSandStoneBlock::Type*>(RedSandStoneBlock::Type::SMOOTH))))->setIconName(L"red_sandstone")->setNameAndDescriptionId(1845194718, 786142283)->setBaseItemTypeAndMaterial(16, 48));

    registerBlock(181, L"double_stone_slab2",
        (new FullNewStoneSlabBlock())->setIconName(L"double_stone_slab2")->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(1316829447, 406008282)->setBaseItemTypeAndMaterial(21, 48));

    registerBlock(182, L"stone_slab2",
        (new HalfNewStoneSlabBlock())->setIconName(L"stone_slab2")->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(1316829447, 406008282)->setBaseItemTypeAndMaterial(22, 48));

    registerBlock(183, L"spruce_fence_gate",
        (new FenceGateBlock(L"planks_spruce", PlanksBlock::Variant::SPRUCE))->setIconName(L"fenceGate")->setDestroyTime(2.0f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(-474640214, -1927890567)->setBaseItemTypeAndMaterial(47, 39));

    registerBlock(184, L"planks_birch",
        (new FenceGateBlock(L"planks_birch", PlanksBlock::Variant::BIRCH))->setIconName(L"fenceGate")->setDestroyTime(2.0f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(-707596766, -1927890567)->setBaseItemTypeAndMaterial(47, 40));

    registerBlock(185, L"jungle_fence_gate",
        (new FenceGateBlock(L"planks_jungle", PlanksBlock::Variant::JUNGLE))->setIconName(L"fenceGate")->setDestroyTime(2.0f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(-1452719473, -1927890567)->setBaseItemTypeAndMaterial(47, 41));

    registerBlock(186, L"planks_big_oak",
        (new FenceGateBlock(L"planks_big_oak", PlanksBlock::Variant::DARK_OAK))->setIconName(L"fenceGate")->setDestroyTime(2.0f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(-424434903, -1927890567)->setBaseItemTypeAndMaterial(47, 42));

    registerBlock(187, L"acacia_fence_gate",
        (new FenceGateBlock(L"planks_acacia", PlanksBlock::Variant::ACACIA))->setIconName(L"fenceGate")->setDestroyTime(2.0f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(-1397533096, -1927890567)->setBaseItemTypeAndMaterial(47, 43));

    registerBlock(188, L"planks_spruce",
        (new FenceBlock(L"planks_spruce", Material::WOOD, PlanksBlock::Variant::SPRUCE->getMaterialColor()))->setDestroyTime(2.0f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(1571971706, 1128628177)->setBaseItemTypeAndMaterial(29, 39));

    registerBlock(189, L"birch_fence",
        (new FenceBlock(L"planks_birch", Material::WOOD, PlanksBlock::Variant::BIRCH->getMaterialColor()))->setDestroyTime(2.0f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(-1758441134, 1128628177)->setBaseItemTypeAndMaterial(29, 40));

    registerBlock(190, L"planks_jungle",
        (new FenceBlock(L"planks_jungle", Material::WOOD, PlanksBlock::Variant::JUNGLE->getMaterialColor()))->setDestroyTime(2.0f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(445729087, 1128628177)->setBaseItemTypeAndMaterial(29, 41));

    registerBlock(191, L"dark_oak_fence",
        (new FenceBlock(L"planks_big_oak", Material::WOOD, PlanksBlock::Variant::DARK_OAK->getMaterialColor()))->setDestroyTime(2.0f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(511023769, 1128628177)->setBaseItemTypeAndMaterial(29, 42));

    registerBlock(192, L"planks_acacia",
        (new FenceBlock(L"planks_acacia", Material::WOOD, PlanksBlock::Variant::ACACIA->getMaterialColor()))->setDestroyTime(2.0f)->setExplodeable(5.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(-1887801560, 1128628177)->setBaseItemTypeAndMaterial(29, 43));

    registerBlock(193, L"spruce_door",
        (new DoorBlock(Material::WOOD))->setIconName(L"door_spruce")->setDestroyTime(3.0f)->setSoundType(SoundType::WOOD)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(791032551, 716140400));

    registerBlock(194, L"birch_door",
        (new DoorBlock(Material::WOOD))->setIconName(L"door_birch")->setDestroyTime(3.0f)->setSoundType(SoundType::WOOD)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-47248137, 716140400));

    registerBlock(195, L"jungle_door",
        (new DoorBlock(Material::WOOD))->setIconName(L"door_jungle")->setDestroyTime(3.0f)->setSoundType(SoundType::WOOD)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-1744743806, 716140400));

    registerBlock(196, L"acacia_door",
        (new DoorBlock(Material::WOOD))->setIconName(L"door_acacia")->setDestroyTime(3.0f)->setSoundType(SoundType::WOOD)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(1678692597, 716140400));

    registerBlock(197, L"dark_oak_door",
        (new DoorBlock(Material::WOOD))->setIconName(L"door_dark_oak")->setDestroyTime(3.0f)->setSoundType(SoundType::WOOD)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-547368868, 716140400));

    registerBlock(198, L"end_rod",
        (new EndRodBlock())->setIconName(L"end_rod")->setDestroyTime(0.0f)->setLightEmission(0.9375f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(-1150000506, 1875247091));

    registerBlock(199, L"chorus_plant",
        (new ChorusPlantBlock())->setIconName(L"chorus_flower_plant")->setDestroyTime(0.4f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(662828135, -1659105222));

    registerBlock(200, L"chorus_flower",
        (new ChorusFlowerBlock())->setIconName(L"chorus_flower")->setDestroyTime(0.4f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(-777779883, -1873306112));

    Block* purpurBlock = (new BlockImpl(Material::STONE, MaterialColor::COLOR_MAGENTA))->setIconName(L"purpur_block")->setDestroyTime(1.5f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-1451163449, 1780598970)->setBaseItemTypeAndMaterial(32, 62);
    registerBlock(201, L"purpur_block", purpurBlock);

    registerBlock(202, L"purpur_pillar",
        (new RotatedPillarBlockImpl(Material::STONE, MaterialColor::COLOR_MAGENTA))->setIconTopName(L"purpur_pillar_top")->setIconName(L"purpur_pillar")->setDestroyTime(1.5f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(175836970, -1656134881)->setBaseItemTypeAndMaterial(14, 62));

    registerBlock(203, L"purpur_stairs",
        (new StairBlock(purpurBlock->defaultBlockState()))->setNameAndDescriptionId(2022113926, 232393875)->setBaseItemTypeAndMaterial(16, 62));

    registerBlock(204, L"purpur_double_slab",
        (new PurpurSlabBlockFull())->setDestroyTime(2.0f)->setIconName(L"purpur_block")->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-1321483148, -234598575)->setBaseItemTypeAndMaterial(21, 62));

    registerBlock(205, L"purpur_slab",
        (new PurpurSlabBlockHalf())->setDestroyTime(2.0f)->setIconName(L"purpur_block")->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-1321483148, -234598575)->setBaseItemTypeAndMaterial(22, 62));

    registerBlock(206, L"end_bricks",
        (new BlockImpl(Material::STONE, MaterialColor::SAND))->setSoundType(SoundType::STONE)->setDestroyTime(0.8f)->setIconName(L"end_bricks")->setNameAndDescriptionId(637472533, -1647411429)->setBaseItemTypeAndMaterial(14, 33));

    registerBlock(207, L"beetroots",
        (new BeetrootBlock())->setIconName(L"beetroots")->disableMipmap()->setNameAndDescriptionId(-185865156, 2110888562));

    Block* grassPath = (new GrassPathBlock())->setDestroyTime(0.65f)->setSoundType(SoundType::GRASS)->setIconName(L"grass_path")->setNotCollectStatistics()->setNameAndDescriptionId(2077997905, 388250431);
    registerBlock(208, L"grass_path", grassPath);

    registerBlock(209, L"end_gateway",
        (new EndGatewayBlock(Material::PORTAL))->setIconName(L"stone")->setDestroyTime(0.0f)->setExplodeable(6000000.0f)->setNameAndDescriptionId(929597140, -308337414));

    registerBlock(210, L"repeating_command_block",
        (new CommandBlock(MaterialColor::COLOR_PURPLE, CommandBlock::REPEAT))->setIconName(L"repeating_command_block")->setIndestructible()->setExplodeable(6000000.0f)->setNameAndDescriptionId(1574311038, -79610325));

    registerBlock(211, L"chain_command_block",
        (new CommandBlock(MaterialColor::COLOR_GREEN, CommandBlock::CHAIN))->setIconName(L"chain_command_block")->setIndestructible()->setExplodeable(6000000.0f)->setNameAndDescriptionId(1574311038, -79610325));

    registerBlock(212, L"frosted_ice",
        (new FrostedIceBlock())->setDestroyTime(0.5f)->setLightBlock(3)->setSoundType(SoundType::GLASS)->setNameAndDescriptionId(-1819286078, -832368980));

    registerBlock(213, L"magma",
        (new MagmaBlock())->setDestroyTime(0.5f)->setSoundType(SoundType::STONE)->setIconName(L"magma")->setNameAndDescriptionId(378681848, -1833708938)->setBaseItemTypeAndMaterial(32, 61));

    registerBlock(214, L"nether_wart_block",
        (new BlockImpl(Material::GRASS, MaterialColor::COLOR_RED))->setDestroyTime(1.0f)->setSoundType(SoundType::WOOD)->setIconName(L"nether_wart_block")->setNameAndDescriptionId(-1017084682, -746937032)->setBaseItemTypeAndMaterial(32, 60));

    registerBlock(215, L"red_nether_brick",
        (new NetherBrickBlock())->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setIconName(L"red_nether_brick")->setNameAndDescriptionId(394564789, -1270652069)->setBaseItemTypeAndMaterial(14, 32));

    registerBlock(216, L"bone_block",
        (new BoneBlock())->setIconName(L"bone_block")->setNameAndDescriptionId(-2045584929, -1381471327)->setBaseItemTypeAndMaterial(13, 59));

    registerBlock(217, L"structure_void",
        (new StructureVoidBlock())->setIconName(L"structure_block")->setNameAndDescriptionId(-1, -1));

    registerBlock(218, L"observer",
        (new ObserverBlock())->setDestroyTime(3.0f)->setNameAndDescriptionId(919115967, -450986799));

    registerBlock(219, L"white_shulker_box",
        (new ShulkerBoxBlock(DyeColor::WHITE))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-1852278165, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(220, L"orange_shulker_box",
        (new ShulkerBoxBlock(DyeColor::ORANGE))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-181944388, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(221, L"magenta_shulker_box",
        (new ShulkerBoxBlock(DyeColor::MAGENTA))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-1397207335, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(222, L"light_blue_shulker_box",
        (new ShulkerBoxBlock(DyeColor::LIGHT_BLUE))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-993600685, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(223, L"yellow_shulker_box",
        (new ShulkerBoxBlock(DyeColor::YELLOW))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-1378120288, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(224, L"lime_shulker_box",
        (new ShulkerBoxBlock(DyeColor::LIME))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(1291125785, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(225, L"pink_shulker_box",
        (new ShulkerBoxBlock(DyeColor::PINK))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(1406224920, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(226, L"gray_shulker_box",
        (new ShulkerBoxBlock(DyeColor::GRAY))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-2127590495, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(227, L"silver_shulker_box",
        (new ShulkerBoxBlock(DyeColor::SILVER))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(632612827, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(228, L"cyan_shulker_box",
        (new ShulkerBoxBlock(DyeColor::CYAN))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(269607033, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(229, L"purple_shulker_box",
        (new ShulkerBoxBlock(DyeColor::PURPLE))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-915929846, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(230, L"blue_shulker_box",
        (new ShulkerBoxBlock(DyeColor::BLUE))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(1370616402, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(231, L"brown_shulker_box",
        (new ShulkerBoxBlock(DyeColor::BROWN))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(837000442, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(232, L"green_shulker_box",
        (new ShulkerBoxBlock(DyeColor::GREEN))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-334831233, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(233, L"red_shulker_box",
        (new ShulkerBoxBlock(DyeColor::RED))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(715884903, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(234, L"black_shulker_box",
        (new ShulkerBoxBlock(DyeColor::BLACK))->setIconName(L"shulker_top")->setDestroyTime(2.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(202884659, 1964547301)->setBaseItemTypeAndMaterial(36, 70));

    registerBlock(235, L"white_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::WHITE))->setIconName(L"glazed_terracotta_white")->setNameAndDescriptionId(1628058156, -151425670));

    registerBlock(236, L"orange_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::ORANGE))->setIconName(L"glazed_terracotta_orange")->setNameAndDescriptionId(-1280717229, -1670944035));

    registerBlock(237, L"magenta_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::MAGENTA))->setIconName(L"glazed_terracotta_magenta")->setNameAndDescriptionId(902019870, 129555800));

    registerBlock(238, L"light_blue_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::LIGHT_BLUE))->setIconName(L"glazed_terracotta_light_blue")->setNameAndDescriptionId(624388556, -502732646));

    registerBlock(239, L"yellow_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::YELLOW))->setIconName(L"glazed_terracotta_yellow")->setNameAndDescriptionId(-1601647889, 1923313889));

    registerBlock(240, L"lime_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::LIME))->setIconName(L"glazed_terracotta_lime")->setNameAndDescriptionId(978734374, 2046805872));

    registerBlock(241, L"pink_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::PINK))->setIconName(L"glazed_terracotta_pink")->setNameAndDescriptionId(1194519607, -956349351));

    registerBlock(242, L"gray_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::GRAY))->setIconName(L"glazed_terracotta_gray")->setNameAndDescriptionId(10555198, 1945017848));

    registerBlock(243, L"silver_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::SILVER))->setIconName(L"glazed_terracotta_silver")->setNameAndDescriptionId(1704134580, -680897838));

    registerBlock(244, L"cyan_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::CYAN))->setIconName(L"glazed_terracotta_cyan")->setNameAndDescriptionId(36664886, -1432293952));

    registerBlock(245, L"purple_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::PURPLE))->setIconName(L"glazed_terracotta_purple")->setNameAndDescriptionId(1800023349, -55405333));

    registerBlock(246, L"blue_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::BLUE))->setIconName(L"glazed_terracotta_blue")->setNameAndDescriptionId(-1882284179, 1902553715));

    registerBlock(247, L"brown_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::BROWN))->setIconName(L"glazed_terracotta_brown")->setNameAndDescriptionId(-2072210035, 1267838099));

    registerBlock(248, L"green_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::GREEN))->setIconName(L"glazed_terracotta_green")->setNameAndDescriptionId(135251048, 1879648542));

    registerBlock(249, L"red_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::RED))->setIconName(L"glazed_terracotta_red")->setNameAndDescriptionId(467194480, 1904104694));

    registerBlock(250, L"black_glazed_terracotta",
        (new GlazedTerracottaBlock(DyeColor::BLACK))->setIconName(L"glazed_terracotta_black")->setNameAndDescriptionId(-1105669692, 883395810));

    registerBlock(251, L"concrete",
        (new ColoredBlock(Material::STONE))->setDestroyTime(1.8f)->setSoundType(SoundType::STONE)->setIconName(L"concrete")->setNameAndDescriptionId(1021483312, -1002471842));

    registerBlock(252, L"concrete_powder",
        (new ConcretePowderBlock())->setDestroyTime(0.5f)->setSoundType(SoundType::SAND)->setIconName(L"concrete_powder")->setNameAndDescriptionId(-282780732, 1414336874)->setBaseItemTypeAndMaterial(57, 69));

    registerBlock(255, L"structure_block",
        (new StructureBlock())->setIconName(L"structure_block")->setIndestructible()->setExplodeable(6000000.0f)->setNameAndDescriptionId(-1, -1));

    registerBlock(256, L"conduit",
        (new ConduitBlock())->setDestroyTime(3.0f)->setLightEmission(1.0f)->setNameAndDescriptionId(-635688620, 590787882)->setBaseItemTypeAndMaterial(61, 2));

    registerBlock(258, L"kelp",
        (new KelpBlock())->setIconName(L"kelp_top")->setSoundType(SoundType::GRASS)->setDestroyTime(0.0f)->setNameAndDescriptionId(68990709, 1293354491));

    registerBlock(259, L"coral_block",
        (new CoralBlock())->setIconName(L"coral_red")->setDestroyTime(7.0f)->setNameAndDescriptionId(-1854932677, 2047656660));

    registerBlock(263, L"coral",
        (new Coral())->setIconName(L"coral")->setDestroyTime(0.0f)->setNameAndDescriptionId(634798269, 577391330)->disableMipmap());

    registerBlock(264, L"coral_fan",
        (new CoralFan())->setIconName(L"coral_fan")->setDestroyTime(0.0f)->setExplodeable(1.5f)->setNameAndDescriptionId(-1592619875, 955602852)->disableMipmap());

    registerBlock(265, L"coral_fan_dead",
        (new CoralFan())->setIconName(L"coral_fan_dead")->setDestroyTime(0.0f)->setExplodeable(1.5f)->setNameAndDescriptionId(-1287380242, -491243345)->disableMipmap());

    registerBlock(266, L"coral_fan_hang",
        (new CoralFanHang())->setIconName(L"coral_fan_hang")->setDestroyTime(0.0f)->setExplodeable(1.5f)->setNameAndDescriptionId(-1592619875, 955602852)->disableMipmap());

    registerBlock(267, L"coral_fan_hang2",
        (new CoralFanHang())->setIconName(L"coral_fan_hang2")->setDestroyTime(0.0f)->setExplodeable(1.5f)->setNameAndDescriptionId(1418867847, 955602852)->disableMipmap());

    registerBlock(268, L"coral_fan_hang3",
        (new CoralFanHang())->setIconName(L"coral_fan_hang3")->setDestroyTime(0.0f)->setExplodeable(1.5f)->setNameAndDescriptionId(1544104069, 955602852)->disableMipmap());

    registerBlock(269, L"dried_kelp_block",
        (new DriedKelpBlock(Material::BAMBOO))->setSoundType(SoundType::GRASS)->setIconName(L"dried_kelp_block")->setDestroyTime(0.6f)->setNameAndDescriptionId(2079230276, 506381546)->setBaseItemTypeAndMaterial(32, 73));

    registerBlock(270, L"sea_grass",
        (new SeaGrass())->setIconName(L"seagrass")->setSoundType(SoundType::GRASS)->setDestroyTime(0.0f)->setNameAndDescriptionId(-835292333, 726461));

    registerBlock(271, L"sea_pickle",
        (new SeaPickle())->setIconName(L"sea_pickle")->setSoundType(SoundType::SLIME)->setDestroyTime(0.4f)->setNameAndDescriptionId(-1602384073, 644018585));

    registerBlock(272, L"bubble_column_segment",
        (new BubbleColumnSegment(Material::BUBBLE_COLUMN))->setIconName(L"bubble_column_segment")->setTicking(1)->setNameAndDescriptionId(-478126772, -22685454));

    registerBlock(273, L"blue_ice",
        (new BlueIceBlock(Material::ICE_SOLID))->setIconName(L"blue_ice")->setDestroyTime(2.8f)->setSoundType(SoundType::GLASS)->setLightEmission(0.3f)->setNameAndDescriptionId(1371904713, 1737388871)->setBaseItemTypeAndMaterial(32, 74));

    registerBlock(274, L"spruce_trapdoor",
        (new TrapDoorBlock(Material::WOOD, MaterialColor::PODZOL))->setIconName(L"spruce_trapdoor")->setDestroyTime(3.0f)->setSoundType(SoundType::WOOD)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(1918030883, 1969487947)->setBaseItemTypeAndMaterial(6, 25));

    registerBlock(275, L"birch_trapdoor",
        (new TrapDoorBlock(Material::WOOD, MaterialColor::SAND))->setIconName(L"birch_trapdoor")->setDestroyTime(3.0f)->setSoundType(SoundType::WOOD)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(2013809035, 1969487947)->setBaseItemTypeAndMaterial(6, 25));

    registerBlock(276, L"jungle_trapdoor",
        (new TrapDoorBlock(Material::WOOD, MaterialColor::DIRT))->setIconName(L"jungle_trapdoor")->setDestroyTime(3.0f)->setSoundType(SoundType::WOOD)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-1491120218, 1969487947)->setBaseItemTypeAndMaterial(6, 25));

    registerBlock(277, L"acacia_trapdoor",
        (new TrapDoorBlock(Material::WOOD, MaterialColor::COLOR_ORANGE))->setIconName(L"acacia_trapdoor")->setDestroyTime(3.0f)->setSoundType(SoundType::WOOD)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(1557027153, 1969487947)->setBaseItemTypeAndMaterial(6, 25));

    registerBlock(278, L"dark_oak_trapdoor",
        (new TrapDoorBlock(Material::WOOD, MaterialColor::COLOR_BROWN))->setIconName(L"dark_oak_trapdoor")->setDestroyTime(3.0f)->setSoundType(SoundType::WOOD)->setNotCollectStatistics()->sendBlockData(15)->setNameAndDescriptionId(-1951682489, 1969487947)->setBaseItemTypeAndMaterial(6, 25));

    registerBlock(279, L"turtle_egg",
        (new TurtleEggBlock())->setSoundType(SoundType::STONE)->setIconName(L"turtle_egg_hatch_0")->setDestroyTime(0.4f)->setNameAndDescriptionId(912101067, -46237355));

    registerBlock(291, L"prismarine_stairs",
        (new StairBlock(prismarine->defaultBlockState()->setBoxedValue(PrismarineBlock::VARIANT, new TypedBoxed<PrismarineBlock::Variant*>(PrismarineBlock::Variant::ROUGH))))->setIconName(L"prismarine_stairs")->setNameAndDescriptionId(-587055318, 1258585647)->setBaseItemTypeAndMaterial(16, 54));

    registerBlock(292, L"prismarine_bricks_stairs",
        (new StairBlock(prismarine->defaultBlockState()->setBoxedValue(PrismarineBlock::VARIANT, new TypedBoxed<PrismarineBlock::Variant*>(PrismarineBlock::Variant::BRICKS))))->setIconName(L"prismarine_bricks")->setNameAndDescriptionId(240124639, 103047266)->setBaseItemTypeAndMaterial(16, 54));

    registerBlock(293, L"dark_prismarine_stairs",
        (new StairBlock(prismarine->defaultBlockState()->setBoxedValue(PrismarineBlock::VARIANT, new TypedBoxed<PrismarineBlock::Variant*>(PrismarineBlock::Variant::DARK))))->setIconName(L"dark_prismarine")->setNameAndDescriptionId(-891724559, -1052410340)->setBaseItemTypeAndMaterial(16, 54));

    registerBlock(295, L"stripped_log_spruce",
        (new StrippedLogBlock())->setIconTopName(L"stripped_log_spruce_top")->setIconName(L"stripped_log_spruce")->setDestroyTime(2.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(875624916, -1597509807));

    registerBlock(296, L"stripped_log_birch",
        (new StrippedLogBlock())->setIconTopName(L"stripped_log_birch_top")->setIconName(L"stripped_log_birch")->setDestroyTime(2.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(-393264956, 48531913));

    registerBlock(297, L"stripped_log_jungle",
        (new StrippedLogBlock())->setIconTopName(L"stripped_log_jungle_top")->setIconName(L"stripped_log_jungle")->setDestroyTime(2.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(-4201887, 1054734580));

    registerBlock(298, L"stripped_log_acacia",
        (new StrippedLogBlock())->setIconTopName(L"stripped_log_acacia_top")->setIconName(L"stripped_log_acacia")->setDestroyTime(2.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(577563830, -1871963005));

    registerBlock(299, L"stripped_log_dark_oak",
        (new StrippedLogBlock())->setIconTopName(L"stripped_log_dark_oak_top")->setIconName(L"stripped_log_dark_oak")->setDestroyTime(2.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(1879620976, -1504211787));

    registerBlock(300, L"stripped_log_oak",
        (new StrippedLogBlock())->setIconTopName(L"stripped_log_oak_top")->setIconName(L"stripped_log_oak")->setDestroyTime(2.0f)->setSoundType(SoundType::WOOD)->setNameAndDescriptionId(-1162676829, 822856878));

    registerBlock(301, L"acacia_pressure_plate",
        (new PressurePlateBlock(L"planks_acacia", Material::WOOD, PressurePlateBlock::EVERYTHING))->setDestroyTime(0.5f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(-581175314, 136821440)->setBaseItemTypeAndMaterial(15, 42));

    registerBlock(302, L"birch_pressure_plate",
        (new PressurePlateBlock(L"planks_birch", Material::WOOD, PressurePlateBlock::EVERYTHING))->setDestroyTime(0.5f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(409645100, -1702551582)->setBaseItemTypeAndMaterial(15, 40));

    registerBlock(303, L"dark_oak_pressure_plate",
        (new PressurePlateBlock(L"planks_big_oak", Material::WOOD, PressurePlateBlock::EVERYTHING))->setDestroyTime(0.5f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(-714459144, 1585240838)->setBaseItemTypeAndMaterial(15, 43));

    registerBlock(304, L"jungle_pressure_plate",
        (new PressurePlateBlock(L"planks_jungle", Material::WOOD, PressurePlateBlock::EVERYTHING))->setDestroyTime(0.5f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(919836665, 1227514775)->setBaseItemTypeAndMaterial(15, 41));

    registerBlock(305, L"spruce_pressure_plate",
        (new PressurePlateBlock(L"planks_spruce", Material::WOOD, PressurePlateBlock::EVERYTHING))->setDestroyTime(0.5f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(-1105754404, 507335842)->setBaseItemTypeAndMaterial(15, 39));

    registerBlock(306, L"acacia_button",
        (new WoodButtonBlock())->setIconName(L"planks_acacia")->setBaseItemTypeAndMaterial(35, 42)->setDestroyTime(0.5f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(2070353022, -753144148));

    registerBlock(307, L"birch_button",
        (new WoodButtonBlock())->setIconName(L"planks_birch")->setBaseItemTypeAndMaterial(35, 40)->setDestroyTime(0.5f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(102998108, -753144148));

    registerBlock(308, L"dark_oak_button",
        (new WoodButtonBlock())->setIconName(L"planks_big_oak")->setBaseItemTypeAndMaterial(35, 43)->setDestroyTime(0.5f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(238880712, -753144148));

    registerBlock(309, L"jungle_button",
        (new WoodButtonBlock())->setIconName(L"planks_jungle")->setBaseItemTypeAndMaterial(35, 41)->setDestroyTime(0.5f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(1565863721, -753144148));

    registerBlock(310, L"spruce_button",
        (new WoodButtonBlock())->setIconName(L"planks_spruce")->setBaseItemTypeAndMaterial(35, 39)->setDestroyTime(0.5f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(-1815731156, -753144148));

    registerBlock(311, L"prismarine_double_slab",
        (new PrismarineSlabBlockFull())->setDestroyTime(2.0f)->setIconName(L"prismarine_block")->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-636643208, 1416464845)->setBaseItemTypeAndMaterial(21, 54));

    registerBlock(312, L"prismarine_slab",
        (new PrismarineSlabBlockHalf())->setDestroyTime(2.0f)->setIconName(L"prismarine_block")->setExplodeable(10.0f)->setSoundType(SoundType::STONE)->setNameAndDescriptionId(-636643208, 1416464845)->setBaseItemTypeAndMaterial(22, 54));

    registerBlock(313, L"bark_spruce",
        (new RotatedPillarBlockImpl(Material::WOOD))->setIconTopName(L"log_spruce")->setIconName(L"log_spruce")->setDestroyTime(2.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(1652869627, 1299556926));

    registerBlock(314, L"bark_birch",
        (new RotatedPillarBlockImpl(Material::WOOD))->setIconTopName(L"log_birch")->setIconName(L"log_birch")->setDestroyTime(2.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(1652869627, 1299556926));

    registerBlock(315, L"bark_jungle",
        (new RotatedPillarBlockImpl(Material::WOOD))->setIconTopName(L"log_jungle")->setIconName(L"log_jungle")->setDestroyTime(2.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(1652869627, 1299556926));

    registerBlock(316, L"bark_acacia",
        (new RotatedPillarBlockImpl(Material::WOOD))->setIconTopName(L"log_acacia")->setIconName(L"log_acacia")->setDestroyTime(2.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(1652869627, 1299556926));

    registerBlock(317, L"bark_dark_oak",
        (new RotatedPillarBlockImpl(Material::WOOD))->setIconTopName(L"log_big_oak")->setIconName(L"log_big_oak")->setDestroyTime(2.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(1652869627, 1299556926));

    registerBlock(318, L"bark_oak",
        (new RotatedPillarBlockImpl(Material::WOOD))->setIconTopName(L"log_oak")->setIconName(L"log_oak")->setDestroyTime(2.0f)->setSoundType(SoundType::WOOD)->sendBlockData(15)->setNameAndDescriptionId(1652869627, 1299556926));

    DefaultedMappedRegistry<ResourceLocation, Block*>::iterator it;

    for (it = blockRegistry->begin(); it != blockRegistry->end(); ++it)
    {
        Block* block = it->first;

        if (block->mMaterial == Material::AIR)
        {
            block->mUseNeighbourBrightness = false;
            continue;
        }

        block->mUseNeighbourBrightness = dynamic_cast<StairBlock*>(block) != NULL ||
                                         dynamic_cast<HalfSlabBlock*>(block) != NULL ||
                                         block == farmland || block == grassPath ||
                                         block->mTranslucent || block->mLightBlock == 0;

        for (int data = 0; data < 8; data++)
        {
            block->getBlockState(data);
        }
    }

    for (it = blockRegistry->begin(); it != blockRegistry->end(); ++it)
    {
        Block* block = it->first;
        const std::vector<const BlockState*>& states = block->getBlockStateDefinition()->getPossibleBlockStates();

        for (std::vector<const BlockState*>::const_iterator state = states.begin(); state != states.end(); ++state)
        {
            blockStateRegistry->addMapping(*state, (getId(block) << 4) | block->convertBlockStateToLegacyData(*state));
        }
    }

    Stats::buildItemStats();
}
