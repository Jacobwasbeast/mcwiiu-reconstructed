#include "game/world/block/LogBlock.hpp"
#include "game/world/block/RotatedPillarBlock.hpp"
#include "game/core/StringIds.hpp"

#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Rotation.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

LogBlock::LogAxis* LogBlock::X = NULL;
LogBlock::LogAxis* LogBlock::Y = NULL;
LogBlock::LogAxis* LogBlock::Z = NULL;
LogBlock::LogAxis* LogBlock::NONE = NULL;
EnumProperty<LogBlock::LogAxis*>* LogBlock::AXIS = NULL;
static const int gLogDescriptionIds[4] = { IDS_TILE_LOG_OAK, (int)IDS_TILE_LOG_SPRUCE, (int)IDS_TILE_LOG_BIRCH, IDS_TILE_LOG_JUNGLE };

LogBlock::LogAxis::LogAxis(const std::wstring& name)
    : mName(name)
{
}

unsigned int LogBlock::LogAxis::getId() const
{
    if (this == LogBlock::X) return 0;
    if (this == LogBlock::Y) return 1;
    if (this == LogBlock::Z) return 2;
    return 3;
}

Direction::Axis* LogBlock::LogAxis::getAxis()
{
    unsigned int id = getId();
    if (id == 0) return Direction::Axis::X;
    if (id == 1) return Direction::Axis::Y;
    if (id == 2) return Direction::Axis::Z;
    return NULL;
}

std::wstring LogBlock::LogAxis::toString() const
{
    return mName;
}

std::wstring LogBlock::LogAxis::getSerializedName() const
{
    return mName;
}

LogBlock::LogAxis* LogBlock::LogAxis::fromAxis(Direction::Axis* axis)
{
    Direction::Axis::EAxis id = axis->getId();
    if (id == Direction::Axis::eAxis_X) return LogBlock::X;
    if (id == Direction::Axis::eAxis_Y) return LogBlock::Y;
    if (id == Direction::Axis::eAxis_Z) return LogBlock::Z;
    return LogBlock::NONE;
}

void LogBlock::blockStaticCtor()
{
    X = new LogAxis(L"x");
    Y = new LogAxis(L"y");
    Z = new LogAxis(L"z");
    NONE = new LogAxis(L"none");

    boost::unordered_set<LogAxis*> values;
    values.insert(X);
    values.insert(Y);
    values.insert(Z);
    values.insert(NONE);
    AXIS = EnumProperty<LogAxis*>::create(L"axis", typeid(LogAxis*), values, Predicates<LogAxis*>::alwaysTrue);
}

LogBlock::LogBlock()
    : RotatedPillarBlock(Material::WOOD)
{
    setDestroyTime(2.0f);
    setSoundType(SoundType::WOOD);
}

void LogBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!level->hasChunksAt(pos.offset(-5, -5, -5), pos.offset(5, 5, 5)))
        return;

    Iterator<BlockPos*>* positions = BlockPos::between(pos.offset(-4, -4, -4), pos.offset(4, 4, 4));
    while (positions->hasNext())
    {
        BlockPos neighbour = *positions->next();
        const BlockState* neighbourState = level->getBlockState(neighbour);
        if (neighbourState->getBlock()->mMaterial == Material::LEAVES && !neighbourState->getValue(LeafBlock::CHECK_DECAY))
        {
            TypedBoxed<bool> checkDecay(true);
            level->setBlock(neighbour, neighbourState->setBoxedValue(LeafBlock::CHECK_DECAY, &checkDecay), Block::UPDATE_INVISIBLE, false);
        }
    }
    delete positions;
}

const BlockState* LogBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction, float hitX, float hitY, float hitZ, int auxValue, boost::shared_ptr<LivingEntity> placer)
{
    const BlockState* result = getBlockState(auxValue);
    TypedBoxed<LogAxis*> axis(LogAxis::fromAxis(direction->getAxis()));
    return result->setBoxedValue(AXIS, &axis);
}

const BlockState* LogBlock::rotate(const BlockState* state, Rotation* rotation)
{
    if (rotation->getType() == Rotation::eTYPE_Clockwise90 || rotation->getType() == Rotation::eTYPE_CounterClockwise90)
    {
        LogAxis* axis = state->getValue(AXIS);
        if (axis->getId() == 0)
        {
            TypedBoxed<LogAxis*> value(Z);
            return state->setBoxedValue(AXIS, &value);
        }
        if (axis->getId() == 2)
        {
            TypedBoxed<LogAxis*> value(X);
            return state->setBoxedValue(AXIS, &value);
        }
    }
    return state;
}

int LogBlock::getDescriptionId(int auxValue)
{
    const BlockState* state = auxValue == -1 ? defaultBlockState() : getBlockState(auxValue);
    return gLogDescriptionIds[convertBlockStateToLegacyData(state)];
}

bool LogBlock::isStrippable()
{
    return true;
}

const BlockState* LogBlock::getStrippedBlock(const BlockState* state)
{
    unsigned int woodType = getWoodType(state);
    const BlockState* stripped;
    if (woodType == 0)
        stripped = Blocks::stripped_log_oak->defaultBlockState();
    else if (woodType == 1)
        stripped = Blocks::stripped_log_spruce->defaultBlockState();
    else if (woodType < 3)
        stripped = Blocks::stripped_log_birch->defaultBlockState();
    else
        stripped = Blocks::stripped_log_jungle->defaultBlockState();

    unsigned int axisId = state->getValue(AXIS)->getId();
    Direction::Axis* axis;
    if (axisId == 0)
        axis = Direction::Axis::X;
    else if (axisId == 1)
        axis = Direction::Axis::Y;
    else
        axis = Direction::Axis::Z;
    TypedBoxed<Direction::Axis*> boxedAxis(axis);
    return stripped->setBoxedValue(gRotatedPillarBlockAxisProperty, &boxedAxis);
}

unsigned int LogBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    unsigned int id = Item::getId(tooltipData.m_pItemInstance->getItem());
    if ((id == 0x102 || id == 0x10f || id == 0x113 || id == 0x117 || id == 0x11e) && tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success && !Minecraft::InMiniGame(eMiniGameId_Any, true)) return IDS_TOOLTIPS_STRIP;
    return Block::GetInteractTooltip(tooltipData);
}

TextureAtlasSprite* LogBlock::getTypeTexture(const BlockState* state)
{
    return mTypeTextures[getWoodType(state)];
}

TextureAtlasSprite* LogBlock::getTopTexture(const BlockState* state)
{
    return mTopTextures[getWoodType(state)];
}

void LogBlock::registerIcons(IconRegister* iconRegister)
{
    int i;
    for (i = 0; i < 6; ++i)
    {
        std::wstring name = getIconName() + L"_" + (i == 0 ? L"oak" : i == 1 ? L"spruce" : i == 2 ? L"birch" : i == 3 ? L"jungle" : i == 4 ? L"acacia" : L"big_oak");
        mTypeTextures[i] = iconRegister->registerIcon(name);
        mTopTextures[i] = iconRegister->registerIcon(name + L"_top");
    }
}

Direction::Axis* LogBlock::getAxis(const BlockState* state)
{
    return state->getValue(AXIS)->getAxis();
}
