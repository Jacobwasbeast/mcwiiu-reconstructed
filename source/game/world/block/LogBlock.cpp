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

LogBlock::LogAxis* LogBlock::X = new LogBlock::LogAxis(L"x");
LogBlock::LogAxis* LogBlock::Y = new LogBlock::LogAxis(L"y");
LogBlock::LogAxis* LogBlock::Z = new LogBlock::LogAxis(L"z");
LogBlock::LogAxis* LogBlock::NONE = new LogBlock::LogAxis(L"none");
LogBlock::LogAxis* LogBlock::LogAxis::X = LogBlock::X;
LogBlock::LogAxis* LogBlock::LogAxis::Y = LogBlock::Y;
LogBlock::LogAxis* LogBlock::LogAxis::Z = LogBlock::Z;
LogBlock::LogAxis* LogBlock::LogAxis::NONE = LogBlock::NONE;
EnumProperty<LogBlock::LogAxis*>* LogBlock::AXIS = NULL;
static std::wstring TEXTURE_NAMES[6] = {
    std::wstring(L"oak"),
    std::wstring(L"spruce"),
    std::wstring(L"birch"),
    std::wstring(L"jungle"),
    std::wstring(L"acacia"),
    std::wstring(L"big_oak")
};
static const int gLogDescriptionIds[4] = { IDS_TILE_LOG_OAK, (int)IDS_TILE_LOG_SPRUCE, (int)IDS_TILE_LOG_BIRCH, IDS_TILE_LOG_JUNGLE };

LogBlock::LogAxis::LogAxis(const std::wstring& name)
    : mName(name)
{
}

unsigned int LogBlock::LogAxis::getId() const
{
    if (this == X) return 0;
    if (this == Y) return 1;
    if (this == Z) return 2;
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
    LogAxis* result = LogBlock::Z;
    if (id != Direction::Axis::eAxis_Z) result = LogBlock::NONE;
    return result;
}

void LogBlock::blockStaticCtor()
{
    std::wstring name(L"axis");
    EnumProperty<LogAxis*>* axis;
    {
        boost::unordered_set<LogAxis*> values;
        values.insert(LogAxis::X);
        values.insert(LogAxis::Y);
        values.insert(LogAxis::Z);
        values.insert(LogAxis::NONE);
        axis = EnumProperty<LogAxis*>::create(name, typeid(LogAxis*), values, Predicates<LogAxis*>::alwaysTrue);
    }
    AXIS = axis;
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
            level->setBlock(neighbour, neighbourState->setValue(LeafBlock::CHECK_DECAY, true), Block::UPDATE_INVISIBLE, false);
        }
    }
    delete positions;
}

const BlockState* LogBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction, float hitX, float hitY, float hitZ, int auxValue, boost::shared_ptr<LivingEntity> placer)
{
    return getBlockState(auxValue)->setValue(AXIS, LogAxis::fromAxis(direction->getAxis()));
}

const BlockState* LogBlock::rotate(const BlockState* state, Rotation* rotation)
{
    if (rotation->getType() == Rotation::eTYPE_Clockwise90 || rotation->getType() == Rotation::eTYPE_CounterClockwise90)
    {
        LogAxis* axis = state->getValue(AXIS);
        if (axis->getId() == 0)
            return state->setValue(AXIS, Z);
        if (axis->getId() == 2)
            return state->setValue(AXIS, X);
    }
    return state;
}

int LogBlock::getDescriptionId(int auxValue)
{
    const BlockState* state = auxValue == -1 ? defaultBlockState() : getBlockState(auxValue);
    return gLogDescriptionIds[getWoodType(state)];
}

bool LogBlock::isStrippable()
{
    return true;
}

const BlockState* LogBlock::getStrippedBlock(const BlockState* state)
{
    unsigned int woodType = getWoodType(state);
    const BlockState* stripped = NULL;
    if (woodType == 0)
        stripped = Blocks::stripped_log_oak->defaultBlockState();
    else if (woodType == 1)
        stripped = Blocks::stripped_log_spruce->defaultBlockState();
    else if (woodType < 3)
        stripped = Blocks::stripped_log_birch->defaultBlockState();
    else if (woodType == 3)
        stripped = Blocks::stripped_log_jungle->defaultBlockState();
    if (stripped == NULL)
        return state;

    unsigned int axisId = state->getValue(AXIS)->getId();
    if (axisId == 0)
        stripped = stripped->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::X);
    else if (axisId == 1)
        stripped = stripped->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::Y);
    else
    {
        if (axisId != 2)
            return stripped;
        stripped = stripped->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::Z);
    }
    return stripped;
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
        mTypeTextures[i] = iconRegister->registerIcon(getIconName() + L"_" + TEXTURE_NAMES[i]);
        mTopTextures[i] = iconRegister->registerIcon(getIconName() + L"_" + TEXTURE_NAMES[i] + L"_top");
    }
}

Direction::Axis* LogBlock::getAxis(const BlockState* state)
{
    return state->getValue(AXIS)->getAxis();
}
