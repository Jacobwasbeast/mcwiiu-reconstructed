#include "game/ui/ItemTooltipDataHolder.hpp"

ItemTooltipDataHolder::ItemTooltipDataHolder(boost::shared_ptr<MultiplayerLocalPlayer> pPlayer,
                                             not_null_ptr<ItemInstance> pItemInstance,
                                             MultiPlayerLevel* pLevel,
                                             InteractionResult::EInteractionResult eInteractionResult)
{
    m_pPlayer = pPlayer;
    m_pItemInstance = pItemInstance;
    m_pLevel = pLevel;
    m_eInteractionResult = eInteractionResult;
}
