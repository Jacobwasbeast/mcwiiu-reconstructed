#include "game/item/ItemInstance.hpp"
#include "game/item/Item.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/Tag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/item/Enchantment.hpp"
#include "game/core/L10N.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/util/ResourceLocation.hpp"
#include <cwchar>
not_null_ptr<ItemInstance> ItemInstance::EMPTY;
ItemInstance::ItemInstance(Block* block)
    : mSelf(), mCount(0), mPopTime(0), mItem(0), mTag(0), mEmptyCache(1),
      mAuxValue(0), mUnknown(0), mFrame(), mCensorshipInfo(), mBreakBlock(0),
      mCanBreak(0), mPlaceBlock(0), mCanPlace(0)
{
_init(Item::byBlock(block), 1, 0);
}
ItemInstance::ItemInstance(Block* block, int count)
    : mSelf(), mCount(0), mPopTime(0), mItem(0), mTag(0), mEmptyCache(1),
      mAuxValue(0), mUnknown(0), mFrame(), mCensorshipInfo(), mBreakBlock(0),
      mCanBreak(0), mPlaceBlock(0), mCanPlace(0)
{
_init(Item::byBlock(block), count, 0);
}
ItemInstance::ItemInstance(Block* block, int count, int auxValue)
    : mSelf(), mCount(0), mPopTime(0), mItem(0), mTag(0), mEmptyCache(1),
      mAuxValue(0), mUnknown(0), mFrame(), mCensorshipInfo(), mBreakBlock(0),
      mCanBreak(0), mPlaceBlock(0), mCanPlace(0)
{
_init(Item::byBlock(block), count, auxValue);
}
ItemInstance::ItemInstance(Item* item)
    : mSelf(), mCount(1), mPopTime(0), mItem(item), mTag(0), mEmptyCache(0),
      mAuxValue(0), mUnknown(0), mFrame(), mCensorshipInfo(), mBreakBlock(0),
      mCanBreak(0), mPlaceBlock(0), mCanPlace(0)
{
}
ItemInstance::ItemInstance(Item* item, int count)
    : mSelf(), mCount(count), mPopTime(0), mItem(item), mTag(0), mEmptyCache(0),
      mAuxValue(0), mUnknown(0), mFrame(), mCensorshipInfo(), mBreakBlock(0),
      mCanBreak(0), mPlaceBlock(0), mCanPlace(0)
{
}
ItemInstance::ItemInstance(Item* item, int count, int auxValue)
    : mSelf(), mCount(count), mPopTime(0), mItem(item), mTag(0), mEmptyCache(0),
      mAuxValue(auxValue), mUnknown(0), mFrame(), mCensorshipInfo(), mBreakBlock(0),
      mCanBreak(0), mPlaceBlock(0), mCanPlace(0)
{
}
ItemInstance::ItemInstance(CompoundTag* tag)
    : mSelf(), mCount(0), mPopTime(0), mItem(0), mTag(tag), mEmptyCache(0),
      mAuxValue(0), mUnknown(0), mFrame(), mCensorshipInfo(), mBreakBlock(0),
      mCanBreak(0), mPlaceBlock(0), mCanPlace(0)
{
}
ItemInstance::ItemInstance(const ItemInstance& other)
    : mSelf(), mCount(other.mCount), mPopTime(other.mPopTime), mItem(other.mItem), mTag(other.mTag),
      mEmptyCache(other.mEmptyCache), mAuxValue(other.mAuxValue), mUnknown(other.mUnknown),
      mFrame(other.mFrame), mCensorshipInfo(other.mCensorshipInfo), mBreakBlock(other.mBreakBlock),
      mCanBreak(other.mCanBreak), mPlaceBlock(other.mPlaceBlock), mCanPlace(other.mCanPlace) {
}
ItemInstance::~ItemInstance() {
delete mTag;
}
ItemInstance& ItemInstance::operator=(const ItemInstance& other) {
if (this != &other) {
mCount=other.mCount;
mPopTime=other.mPopTime;
mItem=other.mItem;
mTag=other.mTag;
mEmptyCache=other.mEmptyCache;
mAuxValue=other.mAuxValue;
mUnknown=other.mUnknown;
mFrame=other.mFrame;
mCensorshipInfo=other.mCensorshipInfo;
mBreakBlock=other.mBreakBlock;
mCanBreak=other.mCanBreak;
mPlaceBlock=other.mPlaceBlock;
mCanPlace=other.mCanPlace;
}
return *this;
}
bool ItemInstance::isEmpty() const {
 return this == EMPTY.get() || mItem == 0 || mCount <= 0;
}
int ItemInstance::getCount() {
 return mEmptyCache != 0 ? 0 : mCount;
}
void ItemInstance::updateEmptyCacheFlag() {
mEmptyCache = isEmpty() ? 1 : 0;
}
void ItemInstance::setCount(int count) {
mCount = count;
updateEmptyCacheFlag();
}
Item* ItemInstance::getItem() const {
 return mEmptyCache != 0 ? 0 : mItem;
}
void ItemInstance::_init(Item* item, int count, int auxValue) {
mItem=item;
mCount=count;
mPopTime=0;
mAuxValue=auxValue;
mTag=0;
mUnknown=0;
mBreakBlock=0;
mCanBreak=0;
mCensorshipInfo.reset(new CensoredString(0xAD36B773, 0x62950B66));
updateEmptyCacheFlag();
}
void ItemInstance::addDataWalkers(DataFixerUpper* fixer) {
if (fixer != 0) {
fixer->addWalker(DataFixTypes::eDataFixTypes_ItemInstance, 0);
fixer->addWalker(DataFixTypes::eDataFixTypes_ItemInstance, 0);
}
}
not_null_ptr<ItemInstance> ItemInstance::copy() const {
ItemInstance* result = new ItemInstance(mItem, mCount, mAuxValue);
if (mTag != 0) result->mTag = mTag->copy();
return not_null_ptr<ItemInstance>(result);
}
void ItemInstance::grow(int amount) {
mCount += amount;
updateEmptyCacheFlag();
}
void ItemInstance::shrink(int amount) {
mCount -= amount;
if (mCount < 0) mCount=0;
updateEmptyCacheFlag();
}
ItemInstance ItemInstance::split(int amount) {
int n = amount < mCount ? amount : mCount;
ItemInstance result(*this);
result.mCount=n;
mCount-=n;
updateEmptyCacheFlag();
return result;
}
int ItemInstance::useOn(boost::shared_ptr<Player> player,
                        Level* level,
                        const BlockPos& pos,
                        InteractionHand::EInteractionHand hand,
                        const Direction* direction,
                        float x,
                        float y,
                        float z,
                        bool flag) {
return mItem != 0 ? mItem->useOn(player, level, pos, hand, direction, x, y, z, flag) == InteractionResult::eInteractionResult_Success : false;
}
float ItemInstance::getDestroySpeed(const BlockState* state) {
return mItem != 0 ? mItem->getDestroySpeed(not_null_ptr<ItemInstance>(const_cast<ItemInstance*>(this)), state) : 0.0f;
}
bool ItemInstance::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand) {
return mItem != 0 && mItem->TestUse(level, player, hand);
}
void ItemInstance::use(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand) {
 if (mItem != 0) {
  mItem->use(level, player, hand);
 }
}
void ItemInstance::finishUsingItem(Level* level, boost::shared_ptr<Player> player) {
if (mItem != 0 && mCount > 0) {
--mCount;
updateEmptyCacheFlag();
}
}
CompoundTag* ItemInstance::save(CompoundTag* tag) {
if (tag == 0) tag = new CompoundTag();
ResourceLocation location = Item::itemRegistry.getKey(mItem);
tag->putString(L"id", location.toString());
tag->putByte(L"Count", (unsigned char)mCount);
tag->putShort(L"Damage", (short)mAuxValue);
if (mTag != 0) tag->put(L"tag", mTag->copy());
return tag;
}
int ItemInstance::getMaxStackSize() {
return mItem != 0 ? mItem->getMaxStackSize() : 64;
}
bool ItemInstance::hasTag() {
return mTag != 0;
}
CompoundTag* ItemInstance::getTag() {
return mTag;
}
bool ItemInstance::isDamageableItem() {
return mItem != 0 && getMaxDamage() > 0;
}
bool ItemInstance::isDamaged() {
return mAuxValue > 0;
}
bool ItemInstance::isStackable() {
return getMaxStackSize() > 1 && !isDamageableItem();
}
bool ItemInstance::isStackedByData() {
return mItem != 0 && mItem->isStackedByData();
}
int ItemInstance::getDamageValue() {
return mAuxValue;
}
int ItemInstance::getAuxValue() const {
return mAuxValue;
}
void ItemInstance::setAuxValue(int auxValue) {
mAuxValue=auxValue;
}
int ItemInstance::getMaxDamage() {
return mItem != 0 ? mItem->getMaxDamage() : 0;
}
bool ItemInstance::hurt(int amount, Random* random) {
if (!isDamageableItem()) return false;
mAuxValue += amount;
return mAuxValue >= getMaxDamage();
}
void ItemInstance::hurtAndBreak(int amount, boost::shared_ptr<LivingEntity> entity) {
hurt(amount, 0);
}
bool ItemInstance::hurtEnemy(boost::shared_ptr<LivingEntity> entity, boost::shared_ptr<Player> player) {
return hurt(1, 0);
}
void ItemInstance::mineBlock(Level* level, const BlockState* state, const BlockPos& pos, boost::shared_ptr<Player> player) {
hurt(1, 0);
}
bool ItemInstance::canDestroySpecial(const BlockState* state) {
return mItem != 0 && mItem->canDestroySpecial(state);
}
bool ItemInstance::interactEnemy(boost::shared_ptr<Player> player, boost::shared_ptr<LivingEntity> entity, InteractionHand::EInteractionHand hand) {
return false;
}
ItemInstance* ItemInstance::copy_not_shared() const {
ItemInstance* result = new ItemInstance(mItem, mCount, mAuxValue);
if (mTag != 0) result->mTag = mTag->copy();
return result;
}
bool ItemInstance::tagMatches(const not_null_ptr<ItemInstance>& first, const not_null_ptr<ItemInstance>& second) {
return first->hasTag() == second->hasTag();
}
bool ItemInstance::matches(const not_null_ptr<ItemInstance>& other, bool compareTags) {
return isSameIgnoreDurability(not_null_ptr<ItemInstance>(this), other) && (!compareTags || tagMatches(not_null_ptr<ItemInstance>(this), other));
}
bool ItemInstance::matches(const not_null_ptr<ItemInstance>& first, const not_null_ptr<ItemInstance>& second, bool compareTags) {
return first->matches(second, compareTags);
}
bool ItemInstance::sameItem(const not_null_ptr<ItemInstance>& other) {
return mItem == other->mItem;
}
bool ItemInstance::isSame(const not_null_ptr<ItemInstance>& first, const not_null_ptr<ItemInstance>& second) {
return first->sameItem(second);
}
bool ItemInstance::sameItemIgnoreDurability(const not_null_ptr<ItemInstance>& other) {
return sameItem(other);
}
bool ItemInstance::isSameIgnoreDurability(const not_null_ptr<ItemInstance>& first, const not_null_ptr<ItemInstance>& second) {
return first->sameItemIgnoreDurability(second);
}
bool ItemInstance::sameItemWithTags(const not_null_ptr<ItemInstance>& other, std::vector<std::wstring>* tags) {
return matches(other, true);
}
bool ItemInstance::sameItem_not_shared(ItemInstance* other) {
return other != 0 && mItem == other->mItem && mAuxValue == other->mAuxValue;
}
int ItemInstance::getUseDescriptionId() {
return mItem != 0 ? mItem->getUseDescriptionId(not_null_ptr<ItemInstance>(this)) : 0;
}
int ItemInstance::getDescriptionId(int auxValue) {
return mItem != 0 ? mItem->getDescriptionId(auxValue) : 0;
}
std::wstring ItemInstance::toString() {
std::wstring value(L"ItemInstance{");
const int descriptionId = getDescriptionId(mAuxValue);
wchar_t description[32];
std::swprintf(description, 32, L"%d", descriptionId);
value += description;
value += L",Count=";
wchar_t digits[16];
std::swprintf(digits, 16, L"%d", mCount);
value += digits;
value += L",Damage=";
std::swprintf(digits, 16, L"%d", mAuxValue);
value += digits;
value += L"}";
return value;
}
void ItemInstance::inventoryTick(Level* level, boost::shared_ptr<Entity> entity, int slot, bool selected) {
if (mPopTime > 0) {
--mPopTime;
}
if (mItem != 0) {
mItem->inventoryTick(not_null_ptr<ItemInstance>(this), level, entity, slot, selected);
}
}
void ItemInstance::onCraftedBy(Level* level, boost::shared_ptr<Player> player, int amount) {
if (mItem != 0) {
mItem->onCraftedBy(not_null_ptr<ItemInstance>(this), level, player);
}
}
int ItemInstance::getUseDuration() {
return mItem != 0 ? mItem->getUseDuration(not_null_ptr<ItemInstance>(this)) : 0;
}
int ItemInstance::getUseAnimation() const {
return mItem != 0 ? mItem->getUseAnimation(not_null_ptr<const ItemInstance>(const_cast<ItemInstance*>(this))) : 0;
}
void ItemInstance::releaseUsing(Level* level, boost::shared_ptr<LivingEntity> entity, int duration) {
if (mItem != 0) {
mItem->releaseUsing(not_null_ptr<ItemInstance>(this), level, entity, duration);
}
}
void ItemInstance::setTag(CompoundTag* tag) {
mTag=tag;
}
void ItemInstance::addTagElement(std::wstring name, Tag* tag) {
if (mTag == 0) mTag = new CompoundTag();
mTag->put(name, tag);
}
Tag* ItemInstance::getOrCreateTagElement(const std::wstring& name) {
if (mTag != 0 && mTag->contains(name, 10)) return mTag->getCompound(name);
CompoundTag* tag = new CompoundTag();
addTagElement(name, tag);
return tag;
}
Tag* ItemInstance::getTagElement(const std::wstring& name) {
return mTag != 0 && mTag->contains(name, 10) ? mTag->getCompound(name) : 0;
}
Tag* ItemInstance::getEnchantmentTags() {
return mTag != 0 && mTag->contains(L"Enchantments", 9) ? mTag->get(L"Enchantments") : 0;
}
bool ItemInstance::wasCreatedByRestrictedPlayer() {
return false;
}
std::wstring ItemInstance::getHoverNameUnfiltered() {
 return mItem != 0 ? mItem->getName(not_null_ptr<ItemInstance>(this)) : std::wstring();
}
std::wstring ItemInstance::getHoverName() {
return getHoverNameUnfiltered();
}
void ItemInstance::setTranslatableName(int name) {
CompoundTag* display = static_cast<CompoundTag*>(getOrCreateTagElement(L"display"));
display->putInt(L"LocName", name);
}
void ItemInstance::setHoverName(const std::wstring& name) {
CompoundTag* display = static_cast<CompoundTag*>(getOrCreateTagElement(L"display"));
display->putString(L"Name", name);
}
void ItemInstance::setCensorshipInfo(const std::wstring& name, bool censored) {
CompoundTag* display = static_cast<CompoundTag*>(getOrCreateTagElement(L"display"));
display->putString(L"Name", name);
display->putBoolean(L"Censored", censored);
}
std::wstring ItemInstance::getOwnerUUID() {
return std::wstring();
}
bool ItemInstance::wasCreatedOnHost() {
return false;
}
void ItemInstance::resetHoverName() {
if (mTag != 0 && mTag->contains(L"display", 10)) {
CompoundTag* display = mTag->getCompound(L"display");
if (display != 0) {
display->remove(L"Name");
}
}
}
bool ItemInstance::hasCustomHoverName() {
if (mTag == 0 || !mTag->contains(L"display", 10)) {
return false;
}
CompoundTag* display = mTag->getCompound(L"display");
return display != 0 && display->contains(L"Name");
}
boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* ItemInstance::getAttributeModifiers(const EquipmentSlot* slot) {
return mItem != 0 ? mItem->getDefaultAttributeModifiers(slot) : 0;
}
std::vector<HtmlString>* ItemInstance::getTooltipLines(boost::shared_ptr<Player> player, bool advanced) {
std::vector<HtmlString>* lines = new std::vector<HtmlString>();
lines->push_back(HtmlString(getHoverName(), eMinecraftColour_HTMLColor_White, false, false, 0));
if (mItem != 0) mItem->appendHoverText(not_null_ptr<ItemInstance>(this), player, lines, advanced);
return lines;
}
std::vector<HtmlString>* ItemInstance::getHoverTextOnly(boost::shared_ptr<Player> player, bool advanced) {
return getTooltipLines(player, advanced);
}
bool ItemInstance::hasFoil() {
return mItem != 0 && mItem->isFoil(not_null_ptr<ItemInstance>(this));
}
int ItemInstance::getRarity() {
return mItem != 0 && mItem->getRarity(not_null_ptr<ItemInstance>(this)) != 0 ? 1 : 0;
}
bool ItemInstance::isEnchanted() {
return getEnchantmentTags() != 0;
}
bool ItemInstance::isEnchantable() {
return mItem != 0 && mItem->isEnchantable(not_null_ptr<ItemInstance>(this));
}
void ItemInstance::enchant(Enchantment* enchantment, int level) {
if (enchantment == 0) {
return;
}
ListTag* enchantments = static_cast<ListTag*>(getOrCreateTagElement(L"Enchantments"));
CompoundTag* entry = new CompoundTag();
entry->putShort(L"id", static_cast<short>(Enchantment::getId(enchantment)));
entry->putShort(L"lvl", static_cast<short>(level));
enchantments->add(entry);
}
bool ItemInstance::mayBePlacedInAdventureMode() {
return mItem != 0 && mItem->mayBePlacedInAdventureMode();
}
bool ItemInstance::isFramed() {
return mFrame.get() != 0;
}
void ItemInstance::setFramed(boost::shared_ptr<ItemFrame> frame) {
mFrame=frame;
}
boost::shared_ptr<ItemFrame> ItemInstance::getFrame() {
return mFrame;
}
int ItemInstance::getBaseRepairCost() {
return mTag != 0 && mTag->contains(L"RepairCost", 3) ? mTag->getInt(L"RepairCost") : 0;
}
void ItemInstance::setRepairCost(int cost) {
if (mTag == 0) mTag = new CompoundTag();
mTag->putInt(L"RepairCost", cost);
}
bool ItemInstance::hasAdventureModeBreakTagForBlock(Block* block) {
if (block != mBreakBlock) {
mBreakBlock=block;
mCanBreak=canDestroySpecial(0);
}
return mCanBreak != 0;
}
bool ItemInstance::hasAdventureModePlaceTagForBlock(Block* block) {
if (block != mPlaceBlock) {
mPlaceBlock=block;
mCanPlace=mayBePlacedInAdventureMode();
}
return mCanPlace != 0;
}
int ItemInstance::getPopTime() const {
return mPopTime;
}
void ItemInstance::setPopTime(int time) {
mPopTime=time;
}
void ItemInstance::set4JData(int data) {
if (mTag == 0) {
 if (data == 0) return;
 mTag = new CompoundTag();
}
mTag->putInt(L"4jdata", data);
}
int ItemInstance::get4JData() {
return mTag != 0 && mTag->contains(L"4jdata") ? mTag->getInt(L"4jdata") : 0;
}
bool ItemInstance::hasPotionStrengthBar() {
return mItem != 0 && mAuxValue != 0;
}
int ItemInstance::GetPotionStrength() {
return (mAuxValue >> 5) & 3;
}
