#ifndef GAME_ITEM_ITEMCOLORS_HPP
#define GAME_ITEM_ITEMCOLORS_HPP
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include <boost/unordered_map.hpp>
#include "game/world/block/IdMapper.hpp"
class Block; class BlockColors; class Item; class ItemInstance;
class ItemColors {
public:
 class ItemColor { public: virtual ~ItemColor() {} virtual int getColor(not_null_ptr<ItemInstance> item, int layer) = 0; };
 ItemColors();
 void registerColor(ItemColor* color,arrayWithLength<Item*> items);
 void registerColor(ItemColor* color,arrayWithLength<Block*> blocks);
 static ItemColors* createDefault(BlockColors* blockColors);
 int getColor(not_null_ptr<ItemInstance> item,int layer);
private:
    IdMapper<ItemColor*>* mItemColorIds;
    boost::unordered_map<Item*, int> mColors;
};
#endif
