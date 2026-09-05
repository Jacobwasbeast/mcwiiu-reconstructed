#include "game/world/block/LogBlock.hpp"

#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Rotation.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/item/Item.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

extern Item* ItemInstance_getItem(const ItemInstance* itemInstance) __asm__("getItem__12ItemInstanceCFv");

static LogBlock::LogAxis* gLogAxisOak = NULL;
static LogBlock::LogAxis* gLogAxisSpruce = NULL;
static LogBlock::LogAxis* gLogAxisBirch = NULL;
static LogBlock::LogAxis* gLogAxisJungle = NULL;
static EnumProperty<LogBlock::LogAxis*>* gLogBlockAxisProperty = NULL;
static const int gLogDescriptionIds[4] = { 0x0B851C01, (int)0x7D86FA86, (int)0xBB86EB26, 0x635D7533 };

LogBlock::LogAxis::LogAxis(const std::wstring& name)
    : mName(name)
{
}

unsigned int LogBlock::LogAxis::getId() const
{
    if (this == gLogAxisOak) return 0;
    if (this == gLogAxisSpruce) return 1;
    if (this == gLogAxisBirch) return 2;
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
    if (id == Direction::Axis::eAxis_X) return gLogAxisOak;
    if (id == Direction::Axis::eAxis_Y) return gLogAxisSpruce;
    if (id == Direction::Axis::eAxis_Z) return gLogAxisBirch;
    return gLogAxisJungle;
}

void LogBlock::blockStaticCtor()
{
    gLogAxisOak = new LogAxis(L"x");
    gLogAxisSpruce = new LogAxis(L"y");
    gLogAxisBirch = new LogAxis(L"z");
    gLogAxisJungle = new LogAxis(L"none");

    boost::unordered_set<LogAxis*> values;
    values.insert(gLogAxisOak);
    values.insert(gLogAxisSpruce);
    values.insert(gLogAxisBirch);
    values.insert(gLogAxisJungle);
    gLogBlockAxisProperty = EnumProperty<LogAxis*>::create(L"axis", typeid(LogAxis*), values, Predicates<LogAxis*>::alwaysTrue);
}

LogBlock::LogBlock()
    : RotatedPillarBlock(Material::WOOD)
{
    setDestroyTime(2.0f);
    setSoundType(SoundType::WOOD);
}

LogBlock::~LogBlock()
{
}

void LogBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!level->hasChunksAt(pos.offset(-5, -5, -5), pos.offset(5, 5, 5))) return;
    int x;
    int y;
    int z;
    for (x = -4; x <= 4; ++x)
    {
        for (y = -4; y <= 4; ++y)
        {
            for (z = -4; z <= 4; ++z)
            {
                BlockPos neighbor = pos.offset(x, y, z);
                const BlockState* neighborState = level->getBlockState(neighbor);
                if (neighborState->getMaterial() == Material::LEAVES)
                    level->setBlock(neighbor, neighborState, 4, false);
            }
        }
    }
}

const BlockState* LogBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction, float hitX, float hitY, float hitZ, int auxValue, boost::shared_ptr<LivingEntity> placer)
{
    const BlockState* result = getBlockState(auxValue);
    TypedBoxed<LogAxis*> axis(LogAxis::fromAxis(direction->getAxis()));
    return result->setBoxedValue(gLogBlockAxisProperty, &axis);
}

const BlockState* LogBlock::rotate(const BlockState* state, Rotation* rotation)
{
    if (rotation->getType() == Rotation::eTYPE_Clockwise90 || rotation->getType() == Rotation::eTYPE_CounterClockwise90)
    {
        LogAxis* axis = state->getValue(gLogBlockAxisProperty);
        if (axis->getId() == 0)
        {
            TypedBoxed<LogAxis*> value(gLogAxisBirch);
            return state->setBoxedValue(gLogBlockAxisProperty, &value);
        }
        if (axis->getId() == 2)
        {
            TypedBoxed<LogAxis*> value(gLogAxisOak);
            return state->setBoxedValue(gLogBlockAxisProperty, &value);
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
    unsigned int id = convertBlockStateToLegacyData(state);
    Block* stripped = id == 0 ? Blocks::stripped_log_oak : id == 1 ? Blocks::stripped_log_spruce : id == 2 ? Blocks::stripped_log_birch : Blocks::stripped_log_jungle;
    if (stripped == NULL) return state;
    TypedBoxed<LogAxis*> axis(state->getValue(gLogBlockAxisProperty));
    return stripped->defaultBlockState()->setBoxedValue(gLogBlockAxisProperty, &axis);
}

unsigned int LogBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    unsigned int id = Item::getId(ItemInstance_getItem(tooltipData.m_pItemInstance.get()));
    if ((id == 0x102 || id == 0x10f || id == 0x113 || id == 0x117 || id == 0x11e) && !tooltipData.m_bCanPlace && !Minecraft::InMiniGame((EMiniGameId)15, true)) return 0x2c6a27a9;
    return Block::GetInteractTooltip(tooltipData);
}

TextureAtlasSprite* LogBlock::getTypeTexture(const BlockState* state)
{
    return mTypeTextures[state->getValue(gLogBlockAxisProperty)->getId()];
}

TextureAtlasSprite* LogBlock::getTopTexture(const BlockState* state)
{
    return mTopTextures[state->getValue(gLogBlockAxisProperty)->getId()];
}

void LogBlock::registerIcons(IconRegister* iconRegister)
{
    int i;
    for (i = 0; i < 6; ++i)
    {
        std::wstring name = getIconName() + L"_" + (i == 0 ? L"oak" : i == 1 ? L"spruce" : i == 2 ? L"birch" : i == 3 ? L"jungle" : i == 4 ? L"acacia" : L"dark_oak");
        mTypeTextures[i] = iconRegister->registerIcon(name);
        mTopTextures[i] = iconRegister->registerIcon(name + L"_top");
    }
}

Direction::Axis* LogBlock::getAxis(const BlockState* state)
{
    return state->getValue(gLogBlockAxisProperty)->getAxis();
}
