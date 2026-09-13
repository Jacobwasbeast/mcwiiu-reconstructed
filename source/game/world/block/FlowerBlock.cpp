#include "game/world/block/FlowerBlock.hpp"

#include <cstddef>
#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/TypedBoxed.hpp"

FlowerBlock::FlowerCategory* FlowerBlock::FlowerCategory::YELLOW = new FlowerBlock::FlowerCategory();
FlowerBlock::FlowerCategory* FlowerBlock::FlowerCategory::RED = new FlowerBlock::FlowerCategory();

FlowerBlock::Flower* FlowerBlock::Flower::DANDELION = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::YELLOW, 0, L"dandelion", 115721561, 1847906812,
    L"flower_dandelion", FlowerBlock::Flower::FLOWER_DANDELION);

FlowerBlock::Flower* FlowerBlock::Flower::POPPY = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 0, L"poppy", -1991483335, -651465812,
    L"flower_rose", FlowerBlock::Flower::FLOWER_POPPY);

static FlowerBlock::Flower* gBlueOrchid = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 1, L"blue_orchid", -1075794771, 65534745,
    L"flower_blue_orchid", FlowerBlock::Flower::FLOWER_BLUE_ORCHID);

static FlowerBlock::Flower* gAllium = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 2, L"allium", -459188977, 1381974082,
    L"flower_alium", FlowerBlock::Flower::FLOWER_BLUE_ORCHID);

static FlowerBlock::Flower* gHoustonia = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 3, L"houstonia", 151996011, -1874500818,
    L"flower_houstonia", FlowerBlock::Flower::FLOWER_HOUSTONIA);

static FlowerBlock::Flower* gRedTulip = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 4, L"red_tulip", -237699297, 1013643162,
    L"flower_tulip_red", FlowerBlock::Flower::FLOWER_RED_TULIP);

static FlowerBlock::Flower* gOrangeTulip = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 5, L"orange_tulip", -222227692, -1169810583,
    L"flower_tulip_orange", FlowerBlock::Flower::FLOWER_ORANGE_TULIP);

static FlowerBlock::Flower* gWhiteTulip = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 6, L"white_tulip", -727005853, -2136162442,
    L"flower_tulip_white", FlowerBlock::Flower::FLOWER_WHITE_TULIP);

static FlowerBlock::Flower* gPinkTulip = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 7, L"pink_tulip", 439136352, -152482691,
    L"flower_tulip_pink", FlowerBlock::Flower::FLOWER_PINK_TULIP);

static FlowerBlock::Flower* gOxeyeDaisy = new FlowerBlock::Flower(
    FlowerBlock::FlowerCategory::RED, 8, L"oxeye_daisy", -1503237240, 484087620,
    L"flower_oxeye_daisy", FlowerBlock::Flower::FLOWER_OXEYE_DAISY);

FlowerBlock::Flower* FlowerBlock::Flower::FLOWERS[FlowerBlock::Flower::FLOWER_COUNT] =
{
    FlowerBlock::Flower::DANDELION,
    FlowerBlock::Flower::POPPY,
    gBlueOrchid,
    gAllium,
    gHoustonia,
    gRedTulip,
    gOrangeTulip,
    gWhiteTulip,
    gPinkTulip,
    gOxeyeDaisy
};

FlowerBlock::Flower* FlowerBlock::Flower::YELLOW_FLOWERS[FlowerBlock::Flower::YELLOW_FLOWER_COUNT] =
{
    FlowerBlock::Flower::DANDELION
};

FlowerBlock::Flower* FlowerBlock::Flower::RED_FLOWERS[FlowerBlock::Flower::RED_FLOWER_COUNT] =
{
    FlowerBlock::Flower::POPPY,
    gBlueOrchid,
    gAllium,
    gHoustonia,
    gRedTulip,
    gOrangeTulip,
    gWhiteTulip,
    gPinkTulip,
    gOxeyeDaisy
};

FlowerBlock::Flower::Flower(FlowerCategory* category, int data, const std::wstring& name, int nameId,
                            int descriptionId, const std::wstring& iconName, EFlower id)
    : mCategory(category),
      mData(data),
      mName(name),
      mNameId(nameId),
      mDescriptionId(descriptionId),
      mIconName(iconName),
      mIcon(NULL),
      mId(id)
{
}

std::wstring FlowerBlock::Flower::toString() const
{
    return mName;
}

std::wstring FlowerBlock::Flower::getSerializedName() const
{
    return mName;
}

FlowerBlock::FlowerCategory* FlowerBlock::Flower::getCategory() const
{
    return mCategory;
}

int FlowerBlock::Flower::getData() const
{
    return mData;
}

FlowerBlock::Flower::EFlower FlowerBlock::Flower::getId() const
{
    return mId;
}

int FlowerBlock::Flower::getNameId() const
{
    return mNameId;
}

int FlowerBlock::Flower::getDescriptionId() const
{
    return mDescriptionId;
}

TextureAtlasSprite* FlowerBlock::Flower::getTexture() const
{
    return mIcon;
}

void FlowerBlock::Flower::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(mIconName);
}

std::pair<FlowerBlock::Flower**, int> FlowerBlock::Flower::byCategory(const FlowerCategory* category)
{
    if (category == FlowerCategory::YELLOW)
    {
        return std::make_pair(YELLOW_FLOWERS, (int)YELLOW_FLOWER_COUNT);
    }

    return std::make_pair(RED_FLOWERS, (int)RED_FLOWER_COUNT);
}

FlowerBlock::Flower* FlowerBlock::Flower::byData(const FlowerCategory* category, int data)
{
    std::pair<Flower**, int> flowers = byCategory(category);

    if (data < 0 || data >= flowers.second)
    {
        data = 0;
    }

    return flowers.first[data];
}

Block* FlowerBlock::FlowerCategory::getBlock()
{
    return Block::byId(this == YELLOW ? 37 : 38);
}

FlowerBlock::FlowerCategoryPredicate::FlowerCategoryPredicate(FlowerBlock* block)
    : mBlock(block)
{
}

bool FlowerBlock::FlowerCategoryPredicate::apply(FlowerBlock::Flower* flower) const
{
    return flower->getCategory() == mBlock->getFlowerCategory();
}

FlowerBlock::FlowerCategoryPredicate::~FlowerCategoryPredicate()
{
}

FlowerBlock::FlowerBlock()
    : mTypeProperty(NULL)
{
}

FlowerBlock::~FlowerBlock()
{
}

void FlowerBlock::DerivedInit()
{
    Block::DerivedInit();

    TypedBoxed<Flower*> flower(getFlowerCategory() == FlowerCategory::RED ? Flower::POPPY : Flower::DANDELION);

    registerDefaultState(mStateDefinition->any()->setBoxedValue(getFlowerProperty(), &flower));
}

AbstractProperty<FlowerBlock::Flower*>* FlowerBlock::getFlowerProperty()
{
    if (mTypeProperty == NULL)
    {
        std::wstring name = L"type";

        FlowerCategoryPredicate* filter = new FlowerCategoryPredicate(this);

        boost::unordered_set<Flower*> values;
        for (int i = 0; i < Flower::FLOWER_COUNT; ++i)
        {
            values.insert(Flower::FLOWERS[i]);
        }

        mTypeProperty = EnumProperty<Flower*>::create(name, typeid(Flower), values, filter);
    }

    return mTypeProperty;
}

BlockStateDefinition* FlowerBlock::createBlockStateDefinition()
{
    const Property* properties[] = { getFlowerProperty() };

    return new BlockStateDefinition(this, properties, 1);
}

const BlockState* FlowerBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();

    TypedBoxed<Flower*> flower(Flower::byData(getFlowerCategory(), data));

    return state->setBoxedValue(getFlowerProperty(), &flower);
}

int FlowerBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(getFlowerProperty())->getData();
}

int FlowerBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(getFlowerProperty())->getData();
}

EBlockOffsetType FlowerBlock::getOffsetType()
{
    return BLOCK_OFFSET_XZ;
}

const AABB* FlowerBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    const Vec3* offset = state->getOffset(levelSource, pos);

    return Bush::getShape(state, levelSource, pos)->move(*offset);
}

bool FlowerBlock::canContainLiquid(const BlockState* state) const
{
    return false;
}

bool FlowerBlock::mayPlace(Level* level, const BlockPos& pos)
{
    Block* liquidBlock = level->getLiquidBlock(pos);

    if (liquidBlock->mMaterial == Material::WATER || liquidBlock->mMaterial == Material::LAVA)
    {
        return false;
    }

    return Bush::mayPlace(level, pos);
}

TextureAtlasSprite* FlowerBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return state->getValue(mTypeProperty)->getTexture();
}

void FlowerBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < Flower::FLOWER_COUNT; ++i)
    {
        Flower::FLOWERS[i]->registerIcons(iconRegister);
    }
}

unsigned int FlowerBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == 351
        && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail)
    {
        if (DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0)
        {
            return 0x9F5FDC40u;
        }
    }

    return Block::GetInteractTooltip(tooltipData);
}

bool FlowerBlock::isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool isClientSide)
{
    return true;
}

bool FlowerBlock::isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    return performBonemeal(true, level, random, pos, state);
}

void FlowerBlock::performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    performBonemeal(false, level, random, pos, state);
}

bool FlowerBlock::performBonemeal(bool checkOnly, Level* level, Random* random, const BlockPos& pos,
                                  const BlockState* state)
{
    Random* levelRandom = level->mRandom;
    bool success = false;

    for (int distance = 16; distance < 64; ++distance)
    {
        MutableBlockPos flowerPos(pos);

        if (!randomWalk(level, flowerPos, distance)
            || level->getBlockId(flowerPos.x, flowerPos.y, flowerPos.z) != 0)
        {
            continue;
        }

        bool keepCategory = levelRandom->nextInt(8) != 0;
        Block* redFlower = FlowerCategory::RED->getBlock();
        Block* yellowFlower = FlowerCategory::YELLOW->getBlock();
        Block* flowerBlock = (this == redFlower) == keepCategory ? redFlower : yellowFlower;

        if (flowerBlock != NULL && flowerBlock->canSurvive(level, flowerPos, state))
        {
            if (!checkOnly)
            {
                level->setBlockAndData(flowerPos.x, flowerPos.y, flowerPos.z, Block::getId(flowerBlock), 0, 3);
            }

            success = true;
        }
    }

    return success;
}

bool FlowerBlock::randomWalk(Level* level, MutableBlockPos& pos, int distance) const
{
    Random* random = level->mRandom;
    int steps = distance / 16;

    for (int step = 0; step < steps; ++step)
    {
        pos.addX(random->nextInt(3) - 1);
        pos.addY((random->nextInt(3) - 1) * random->nextInt(3) / 2);
        pos.addZ(random->nextInt(3) - 1);

        if (level->getBlockId(pos.x, pos.y - 1, pos.z) != 2)
        {
            return false;
        }

        if (level->isSolidBlockingBlockInLoadedChunk(pos, true))
        {
            return false;
        }
    }

    return steps > 0;
}
