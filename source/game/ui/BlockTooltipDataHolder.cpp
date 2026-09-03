#include "game/ui/BlockTooltipDataHolder.hpp"

BlockTooltipDataHolder::BlockTooltipDataHolder(boost::shared_ptr<MultiplayerLocalPlayer> pPlayer,
                                               MultiPlayerLevel* pLevel,
                                               not_null_ptr<ItemInstance> pItemInstance,
                                               const BlockState* pBlockState,
                                               BlockPos blockPos,
                                               InteractionResult::EInteractionResult eInteractionResult,
                                               bool bUseItem,
                                               bool bCanPlace,
                                               bool bCanDestroy,
                                               bool bIsInside)
{
    m_pPlayer = pPlayer;
    m_pLevel = pLevel;
    m_pItemInstance = pItemInstance;
    m_pBlockState = pBlockState;
    m_blockPos = blockPos;
    m_eInteractionResult = eInteractionResult;
    m_bUseItem = bUseItem;
    m_bCanPlace = bCanPlace;
    m_bCanDestroy = bCanDestroy;
    m_bIsInside = bIsInside;
}
