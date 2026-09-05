#include "game/world/block/LookAtBlockHint.hpp"

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Property.hpp"

LookAtBlockHint::LookAtBlockHint(eTutorial_Hint hint, Tutorial* tutorial,
                                 const BlockState** states, unsigned int stateCount,
                                 const Property** properties, unsigned int propertyCount,
                                 int descriptionId, int auxValue, bool useState)
    : TutorialHint(hint, tutorial, -1, (TutorialHint::eHintType)6, true, true, -1),
      mStateCount(stateCount), mStates(new const BlockState*[stateCount]),
      mStateIds(new Block*[stateCount]), mPropertyCount(propertyCount),
      mPropertyValues(new const Property*[propertyCount]), mDescriptionId(descriptionId),
      mAuxValue(auxValue), mUseState(useState)
{
    unsigned int i;
    for (i = 0; i < mStateCount; ++i)
    {
        mStates[i] = states[i];
        mStateIds[i] = states[i] == NULL ? NULL : states[i]->getBlock();
    }
    for (i = 0; i < mPropertyCount; ++i)
    {
        mPropertyValues[i] = properties[i];
    }
}

LookAtBlockHint::~LookAtBlockHint()
{
    delete[] mStates;
    delete[] mStateIds;
    delete[] mPropertyValues;
}

unsigned int LookAtBlockHint::onLookAt(const BlockState* state)
{
    const BlockState* matchingState = NULL;
    Block* block;
    Item* item;
    Tutorial::PopupMessageDetails* details;
    int auxValue;
    unsigned int i;

    if (state == NULL || state->getBlock() == NULL)
    {
        return 0;
    }
    block = state->getBlock();
    for (i = 0; i < mStateCount; ++i)
    {
        if (mStates[i] != NULL && mStateIds[i] == block)
        {
            matchingState = mStates[i];
            break;
        }
    }
    if (matchingState == NULL)
    {
        return 0;
    }
    if (!mUseState)
    {
        matchingState = state;
    }

    item = Item::byBlock(block);
    details = new Tutorial::PopupMessageDetails();
    details->allowFade = true;
    details->messageId = mDescriptionId == -2 ? -1 :
        (mDescriptionId < 0 ? (item == NULL ? Block::getId(block) : Item::getId(item)) : mDescriptionId);
    auxValue = mAuxValue;
    if (auxValue < 0)
    {
        auxValue = block->getDescriptionId(-1);
    }
    details->promptId = auxValue;
    if (item == NULL)
    {
        details->titleId = block->getUseDescriptionId(-1);
    }
    else
    {
        details->titleId = Item::getId(item);
        details->promptId = auxValue;
    }
    return tutorial->setMessage(this, details);
}

unsigned int LookAtBlockHint::onLookAtSkull(Level* level, const BlockPos& pos,
                                             const BlockState* state)
{
    ItemInstance* itemInstance;
    Tutorial::PopupMessageDetails* details;

    if (state == NULL || state->getBlock() == NULL || level == NULL)
    {
        return 0;
    }
    itemInstance = state->getBlock()->getCloneItemInstance(level, pos, state).get();
    if (itemInstance == NULL || itemInstance->getAuxValue() == -1)
    {
        return 0;
    }
    details = new Tutorial::PopupMessageDetails();
    details->titleId = itemInstance->getItem() == NULL ? -1 : Item::VALIDgetId(itemInstance->getItem());
    details->message = itemInstance->getDescriptionId(-1);
    details->promptId = itemInstance->getAuxValue();
    details->allowFade = true;
    details->force = true;
    return tutorial->setMessage(this, details);
}
