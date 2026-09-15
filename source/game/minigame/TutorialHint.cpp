#include "game/minigame/TutorialHint.hpp"

#include "game/core/Minecraft.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

TutorialHint::TutorialHint(eTutorial_Hint hintValue, Tutorial* tutorialValue, int description,
                           eHintType hintType, bool repeatMessage, bool allowFadeValue,
                           int minimumDisplayTime)
    : hint(hintValue), descriptionId(description), tutorial(tutorialValue), type(hintType),
      consecutiveTicks(0), lastBlock(NULL), active(true), fadeAllowed(allowFadeValue),
      minimumTime(minimumDisplayTime)
{
    tutorial->addMessage(description, repeatMessage && hintType != HINT_CREATE_ITEM_SELECTED, 3);
}

TutorialHint::~TutorialHint()
{
}

int TutorialHint::startDestroyBlock(not_null_ptr<ItemInstance> item, const BlockState* state)
{
    if (type == HINT_START_DESTROY_BLOCK)
    {
        Block* block = state->getBlock();
        if (block == lastBlock && active)
        {
            ++consecutiveTicks;
            if (consecutiveTicks > 20)
            {
                lastBlock = block;
                return descriptionId;
            }
        }
        else
        {
            consecutiveTicks = 0;
        }
        lastBlock = block;
    }
    return -1;
}

unsigned int TutorialHint::destroyBlock(Block* block)
{
    if (type == HINT_START_DESTROY_BLOCK && block == lastBlock && consecutiveTicks > 0)
    {
        active = false;
    }
    return 0xffffffffU;
}

unsigned int TutorialHint::attack(not_null_ptr<ItemInstance> item, boost::shared_ptr<Entity> entity)
{
    return 0xffffffffU;
}

int TutorialHint::createItemSelected(not_null_ptr<ItemInstance> item, bool selected)
{
    if (type == HINT_CREATE_ITEM_SELECTED && !selected)
    {
        return descriptionId;
    }
    return -1;
}

int TutorialHint::itemDamaged(not_null_ptr<ItemInstance> item)
{
    if (type == HINT_ITEM_DAMAGED)
    {
        return descriptionId;
    }
    return -1;
}

unsigned int TutorialHint::onTake(not_null_ptr<ItemInstance> item)
{
    return 0;
}

unsigned int TutorialHint::onLookAt(const BlockState* state)
{
    return 0;
}

unsigned int TutorialHint::onLookAtSkull(Level* level, const BlockPos& pos, const BlockState* state)
{
    return 0;
}

unsigned int TutorialHint::onLookAtEntity(boost::shared_ptr<Entity> entity)
{
    return 0;
}

unsigned int TutorialHint::onGameModeChange(eTutorial_GameMode gameMode)
{
    return 0xffffffffU;
}

int TutorialHint::tick()
{
    if (type == HINT_WATER_MOVEMENT)
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        MultiplayerLocalPlayer* player = minecraft->mLocalPlayers[tutorial->playerId].get();
        if (player->isUnderLiquid(Material::WATER) && !player->PositionLocked())
        {
            return descriptionId;
        }
    }
    return -1;
}

unsigned int TutorialHint::getMinimumDisplayTime()
{
    return (unsigned int)minimumTime;
}

unsigned char TutorialHint::allowFade()
{
    return (unsigned char)fadeAllowed;
}
