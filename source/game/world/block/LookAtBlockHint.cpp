#include "game/world/block/LookAtBlockHint.hpp"

#include <algorithm>

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
      mStateIds(new Block*[mStateCount])
{
    unsigned int i;
    for (i = 0; i < mStateCount; ++i)
    {
        mStates[i] = states[i];
        mStateIds[i] = states[i]->getBlock();
    }
    mPropertyCount = propertyCount;
    const Property** values = new const Property*[propertyCount];
    mPropertyValues = values;
    const unsigned int count = mPropertyCount;
    for (unsigned int j = 0; j < count; ++j)
    {
        values[j] = properties[j];
    }
    mDescriptionId = descriptionId;
    mAuxValue = auxValue;
    mUseState = useState;
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
    Block* matchedBlock = NULL;
    Item* item;
    Tutorial::PopupMessageDetails* details;
    int auxValue;
    unsigned int i;
    unsigned int j;

    block = state->getBlock();
    if (block == NULL)
    {
        return 0;
    }
    for (i = 0; i < mStateCount; ++i)
    {
        if (mStateIds[i] != block)
        {
            continue;
        }
        matchedBlock = mStateIds[i];
        if (mPropertyCount == 0)
        {
            matchingState = mStates[i];
            break;
        }
        for (j = 0; j < mPropertyCount; ++j)
        {
            const Property* property = mPropertyValues[j];
            const BlockState* candidate = mStates[i];
            if (std::find(candidate->getProperties().begin(), candidate->getProperties().end(), property)
                    == candidate->getProperties().end())
            {
                continue;
            }
            if (std::find(state->getProperties().begin(), state->getProperties().end(), property)
                    == state->getProperties().end())
            {
                continue;
            }
            if (candidate->getBoxedValue(property) == state->getBoxedValue(property))
            {
                matchingState = candidate;
                break;
            }
        }
        if (matchingState != NULL)
        {
            break;
        }
    }
    if (matchedBlock == NULL || matchingState == NULL)
    {
        return 0;
    }
    if (!mUseState)
    {
        matchingState = state;
    }

    item = Item::byBlock(matchedBlock) != NULL ? Item::byBlock(matchedBlock)
                                                : matchedBlock->getResource(matchingState, NULL, 0);
    details = new Tutorial::PopupMessageDetails();
    details->isItem = true;
    if (mDescriptionId < 0)
    {
        if (mDescriptionId == -2)
        {
            details->itemId = -1;
        }
        else if (item == NULL)
        {
            details->itemId = Block::getId(matchedBlock);
        }
        else
        {
            details->itemId = Item::getId(item);
        }
    }
    else
    {
        details->itemId = mDescriptionId;
    }
    auxValue = mAuxValue;
    if (auxValue < 0)
    {
        auxValue = matchedBlock->convertBlockStateToLegacyData(matchingState);
    }
    details->itemAuxValue = auxValue;
    if (item == NULL)
    {
        details->titleId = matchedBlock->getUseDescriptionId(-1);
        details->itemDescriptionId = matchedBlock->getDescriptionId(auxValue);
    }
    else
    {
        not_null_ptr<ItemInstance> itemInstance(new ItemInstance(item, 1, auxValue));
        if (item == Items::banner)
        {
            BannerBlockEntity::addTags(itemInstance, 0xf, NULL);
        }
        if (mDescriptionId < 0)
        {
            details->itemInstance = itemInstance;
        }
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
        }
    }
    if (!found)
    {
        return 0;
    }
    not_null_ptr<ItemInstance> itemInstance = block->getCloneItemInstance(level, pos, state);
    if (itemInstance->getAuxValue() == -1)
    {
        return 0;
    }
    Tutorial::PopupMessageDetails* details = new Tutorial::PopupMessageDetails;
    details->titleId = itemInstance->getUseDescriptionId();
    if (itemInstance->getItem() != NULL)
    {
        details->itemDescription = itemInstance->getHoverName();
    }
    details->itemDescriptionId = itemInstance->getDescriptionId(-1);
    details->itemInstance = itemInstance;
    details->itemId = Item::VALIDgetId(itemInstance->getItem());
    details->itemAuxValue = itemInstance->getAuxValue();
    details->allowFade = true;
    details->force = true;
    return tutorial->setMessage(this, details);
}
