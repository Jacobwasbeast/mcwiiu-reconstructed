#include "game/world/block/LookAtBlockHint.hpp"

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Property.hpp"

LookAtBlockHint::LookAtBlockHint(eTutorial_Hint hint, Tutorial* tutorial,
                                 const BlockState** states, unsigned int stateCount,
                                 const Property* const* properties, unsigned int propertyCount,
                                 int descriptionId, int auxValue, bool useState)
    : TutorialHint(hint, tutorial, -1, TutorialHint::HINT_LOOK_AT_BLOCK, true, true, -1),
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

    block = state->getBlock();
    if (block == NULL)
    {
        return 0;
    }
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
    details->isItem = true;
    details->itemId = mDescriptionId == -2 ? -1 :
        (mDescriptionId < 0 ? (item == NULL ? Block::getId(block) : Item::getId(item)) : mDescriptionId);
    auxValue = mAuxValue;
    if (auxValue < 0)
    {
        auxValue = block->convertBlockStateToLegacyData(matchingState);
    }
    details->itemAuxValue = auxValue;
    if (item == NULL)
    {
        details->titleId = block->getUseDescriptionId(-1);
        details->itemDescriptionId = block->getDescriptionId(auxValue);
    }
    else
    {
        not_null_ptr<ItemInstance> itemInstance(new ItemInstance(item, 1, auxValue));
        if (item == Items::banner)
        {
            BannerBlockEntity::addTags(itemInstance, 0xf, NULL);
        }
        details->itemInstance = itemInstance;
        details->titleId = itemInstance->getUseDescriptionId();
        details->itemDescriptionId = itemInstance->getDescriptionId(-1);
    }
    return tutorial->setMessage(this, details);
}

unsigned int LookAtBlockHint::onLookAtSkull(Level* level, const BlockPos& pos,
                                             const BlockState* state)
{
    Block* block;
    bool found;
    unsigned int i;
    not_null_ptr<ItemInstance> itemInstance;
    Tutorial::PopupMessageDetails* details;

    block = state->getBlock();
    if (block == NULL || block != Blocks::skull)
    {
        return 0;
    }
    found = false;
    for (i = 0; i < mStateCount; ++i)
    {
        if (mStateIds[i] == block)
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        return 0;
    }
    itemInstance = block->getCloneItemInstance(level, pos, state);
    if (itemInstance->getAuxValue() == -1)
    {
        return 0;
    }
    details = new Tutorial::PopupMessageDetails();
    details->titleId = itemInstance->getUseDescriptionId();
    details->itemDescriptionId = itemInstance->getDescriptionId(-1);
    if (itemInstance->getItem() != NULL)
    {
        details->itemDescription = itemInstance->getHoverName();
    }
    details->itemInstance = itemInstance;
    details->itemId = Item::VALIDgetId(itemInstance->getItem());
    details->itemAuxValue = itemInstance->getAuxValue();
    details->allowFade = true;
    details->force = true;
    return tutorial->setMessage(this, details);
}
