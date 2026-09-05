#ifndef GAME_ENTITY_MOB_HANGINGENTITYITEM_HPP
#define GAME_ENTITY_MOB_HANGINGENTITYITEM_HPP

#include <boost/shared_ptr.hpp>
#include <vector>

#include "game/core/Direction.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class BlockPos;
class Level;
class Player;

class HangingEntityItem : public Item
{
public:
    explicit HangingEntityItem(unsigned long long entityType);
    virtual ~HangingEntityItem();

    void createEntity(Level* level, const BlockPos& pos, const Direction* direction, int auxValue);
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player,
                                                        Level* level,
                                                        const BlockPos& pos,
                                                        InteractionHand::EInteractionHand hand,
                                                        const Direction* direction,
                                                        float hitX,
                                                        float hitY,
                                                        float hitZ,
                                                        bool simulate);
    virtual void appendHoverText(not_null_ptr<ItemInstance> itemInstance,
                                 boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines,
                                 bool advanced);

private:
    unsigned long long mEntityType;
};

#endif
