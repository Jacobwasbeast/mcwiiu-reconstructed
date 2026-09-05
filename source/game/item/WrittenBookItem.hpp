#ifndef GAME_ITEM_WRITTENBOOKITEM_HPP
#define GAME_ITEM_WRITTENBOOKITEM_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/item/Item.hpp"

class CompoundTag;
class ItemInstance;
class ItemTooltipDataHolder;
class Level;
class Player;

class WrittenBookItem : public Item
{
public:
    WrittenBookItem();
    virtual ~WrittenBookItem();

    static bool makeSureTagIsValid(CompoundTag* tag);
    static int getGeneration(not_null_ptr<ItemInstance> itemInstance);
    virtual std::wstring getName(not_null_ptr<ItemInstance> itemInstance);
    virtual void appendHoverText(not_null_ptr<ItemInstance> itemInstance,
                                 boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines,
                                 bool advanced);
    void resolveBookComponents(not_null_ptr<ItemInstance> itemInstance,
                               boost::shared_ptr<Player> player);
    virtual InteractionResultHolder use(Level* level,
                                        boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual bool TestUse(Level* level,
                         boost::shared_ptr<Player> player,
                         InteractionHand::EInteractionHand hand);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
    virtual bool isFoil(not_null_ptr<ItemInstance> itemInstance);
    static bool HasBookTags(not_null_ptr<ItemInstance> itemInstance);
};

#endif
