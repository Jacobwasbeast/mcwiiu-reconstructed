#include "game/item/RecordItem.hpp"

#include "game/client/audio/SoundEvent.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/item/ItemInstance.hpp"

RecordItem::RecordItem(const std::wstring& name, int recordId, const SoundEvent* sound)
    : Item(), mRecordId(recordId), mSound(sound)
{
    setIconName(name);
}

RecordItem::~RecordItem()
{
}

InteractionResult::EInteractionResult RecordItem::useOn(boost::shared_ptr<Player>, Level*, const BlockPos&, InteractionHand::EInteractionHand, const Direction*, float, float, float, bool)
{
    return InteractionResult::eInteractionResult_Success;
}

void RecordItem::appendHoverText(not_null_ptr<ItemInstance> item, boost::shared_ptr<Player>, std::vector<HtmlString>* lines, bool)
{
    lines->push_back(HtmlString(L10N::GetString(mRecordId),
                                static_cast<eMinecraftColour>(getColor(item, 0)), false, false, 0));
}

std::wstring RecordItem::getDisplayName()
{
    return mIconName;
}

const Rarity* RecordItem::getRarity(not_null_ptr<ItemInstance>)
{
    return Item::getRarity(ItemInstance::EMPTY);
}

RecordItem* RecordItem::getBySound(const SoundEvent* sound)
{
    (void)sound;
    return 0;
}

const SoundEvent* RecordItem::getSound()
{
    return mSound;
}

TextureAtlasSprite* RecordItem::getIcon(int)
{
    return mIcon;
}

void RecordItem::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(mIconName);
}
