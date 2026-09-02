#include "game/core/InteractionResultHolder.hpp"

InteractionResultHolder::InteractionResultHolder(InteractionResult::EInteractionResult result, const not_null_ptr<ItemInstance>& object)
    : result(result)
    , object(object)
{
}
