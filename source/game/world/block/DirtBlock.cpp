#include "game/world/block/DirtBlock.hpp"

#include "game/core/DyeColor.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"

class SeaGrass
{
public:
    static bool trySpawnSeaGrass(bool checkOnly, Level* level, Random* random,
                                 const BlockPos& pos, const BlockState* state);
};

static DirtBlock::Variant* gVariants[3] = { NULL, NULL, NULL };
static const Property* gVariantProperty = NULL;
static bool gStateVariantsReady = false;

DirtBlock::Variant::Variant(int data, const std::wstring& name, int nameId, int descriptionId,
                            const MaterialColor* materialColor, const std::wstring& texture,
                            const std::wstring& sideTexture, const std::wstring& topTexture)
    : mData(data), mName(name), mNameId(nameId), mDescriptionId(descriptionId),
      mMaterialColor(materialColor), mTexture(texture), mSideTexture(sideTexture),
      mTopTexture(topTexture), mIcon(NULL), mSideIcon(NULL), mTopIcon(NULL)
{
}

const MaterialColor* DirtBlock::Variant::getMaterialColor()
{
    return mMaterialColor;
}

int DirtBlock::Variant::getData()
{
    return mData;
}

DirtBlock::Variant* DirtBlock::Variant::byData(int data)
{
    if (data < 0 || data > 2)
    {
        data = 0;
    }

    return gVariants[data];
}

TextureAtlasSprite* DirtBlock::Variant::getTexture(const Direction* direction)
{
    if (mTopIcon != NULL && direction == Direction::UP)
    {
        return mTopIcon;
    }
    if (mSideIcon != NULL && direction != Direction::UP && direction != Direction::DOWN)
    {
        return mSideIcon;
    }
    return mIcon;
}

void DirtBlock::Variant::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(mTexture);
    if (!mSideTexture.empty())
    {
        mSideIcon = iconRegister->registerIcon(mSideTexture);
    }
    if (!mTopTexture.empty())
    {
        mTopIcon = iconRegister->registerIcon(mTopTexture);
    }
}

int DirtBlock::Variant::getNameId()
{
    return mNameId;
}

int DirtBlock::Variant::getDescriptionId()
{
    return mDescriptionId;
}

std::wstring DirtBlock::Variant::toString() const
{
    return mName;
}

std::wstring DirtBlock::Variant::getSerializedName() const
{
    return mName;
}

DirtBlock::DirtBlock()
    : Block(Material::DIRT)
{
    DerivedInit();
    if (gVariants[0] != NULL)
    {
        registerDefaultState(defaultBlockState());
    }
}

bool DirtBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool)
{
    return true;
}

bool DirtBlock::isBonemealSuccess(Level* level, Random* random, const BlockPos& pos,
                                  const BlockState* state)
{
    return SeaGrass::trySpawnSeaGrass(true, level, random, pos, state);
}

void DirtBlock::performBonemeal(Level* level, Random* random, const BlockPos& pos,
                                const BlockState* state)
{
    SeaGrass::trySpawnSeaGrass(false, level, random, pos, state);
}

void DirtBlock::blockStaticCtor()
{
    if (gVariants[0] != NULL)
    {
        return;
    }

    gVariants[0] = new Variant(0, L"dirt", 0x672CD46C, 0x5437FAF1, MaterialColor::DIRT,
                               L"dirt", L"", L"");
    gVariants[1] = new Variant(1, L"coarse_dirt", 0xC79DB172, 0x935970C7, MaterialColor::DIRT,
                               L"dirt", L"", L"");
    gVariants[2] = new Variant(2, L"podzol", 0x64186D51, 0x8FB42134, MaterialColor::PODZOL,
                               L"dirt", L"dirt_podzol_side", L"dirt_podzol_top");
}

const MaterialColor* DirtBlock::getMapColor(const BlockState* state, LevelSource* levelSource,
                                            const BlockPos& pos)
{
    return getBlockState(0) == state ? gVariants[0]->getMaterialColor()
                                     : gVariants[0]->getMaterialColor();
}

const BlockState* DirtBlock::fillVirtualBlockStateProperties(const BlockState* state,
                                                              LevelSource*, const BlockPos&)
{
    return state;
}

not_null_ptr<ItemInstance> DirtBlock::getCloneItemInstance(Level*, const BlockPos&,
                                                           const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(this, 1, convertBlockStateToLegacyData(state)));
}

const BlockState* DirtBlock::getBlockState(int data)
{
    if (data < 0 || data > 2)
    {
        data = 0;
    }
    return defaultBlockState();
}

int DirtBlock::convertBlockStateToLegacyData(const BlockState*)
{
    return 0;
}

unsigned int DirtBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) > 0x121
        && Item::getId(tooltipData.m_pItemInstance->getItem()) < 0x127
        && tooltipData.m_pBlockState != NULL)
    {
        return 0x70889480u;
    }
    return Block::GetInteractTooltip(tooltipData);
}

BlockStateDefinition* DirtBlock::createBlockStateDefinition()
{
    blockStaticCtor();
    return new BlockStateDefinition(this);
}

int DirtBlock::getSpawnResourcesAuxValue(const BlockState*)
{
    return 0;
}

TextureAtlasSprite* DirtBlock::getTexture(const Direction*, const BlockState*)
{
    return gVariants[0]->getTexture(Direction::UP);
}

void DirtBlock::registerIcons(IconRegister* iconRegister)
{
    blockStaticCtor();
    gVariants[0]->registerIcons(iconRegister);
    gVariants[1]->registerIcons(iconRegister);
    gVariants[2]->registerIcons(iconRegister);
}

DirtBlock::~DirtBlock()
{
}
