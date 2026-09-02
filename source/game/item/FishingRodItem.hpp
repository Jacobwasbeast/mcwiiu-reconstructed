#ifndef GAME_ITEM_FISHINGRODITEM_HPP
#define GAME_ITEM_FISHINGRODITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/ItemPropertyFunction.hpp"

class IconRegister;
class ItemInstance;
class ItemTooltipDataHolder;
class Level;
class LivingEntity;
class Player;
class TextureAtlasSprite;

class FishingRodItem : public Item
{
public:
    class FishingRodItem_ItemPropertyFunction : public ItemPropertyFunction
    {
    public:
        virtual float call(not_null_ptr<ItemInstance> itemInstance,
                           Level* level,
                           boost::shared_ptr<LivingEntity> entity);
    };

    FishingRodItem();

    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
    virtual ~FishingRodItem();
    virtual InteractionResultHolder use(Level* level,
                                        boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual bool isHandEquipped();
    virtual bool isMirroredArt();
    virtual int getEnchantmentValue();
    virtual void registerIcons(IconRegister* iconRegister);

    TextureAtlasSprite* getEmptyIcon();

private:
    TextureAtlasSprite* mEmptyIcon;
};

#endif
