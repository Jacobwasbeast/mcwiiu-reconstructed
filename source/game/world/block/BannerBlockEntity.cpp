#include "game/world/block/BannerBlockEntity.hpp"

#include <cwchar>

#include "game/core/BannerPattern.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/L10N.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/BannerItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"

BannerBlockEntity::BannerBlockEntity()
    : BlockEntity(), mBaseColor(DyeColor::WHITE), mPatterns(NULL), mPatternListCreated(false),
      mPatternList(), mColorList(), mTextureHashName(L""), mCustomName(new CensoredString())
{
}

BannerBlockEntity::~BannerBlockEntity()
{
}

std::wstring BannerBlockEntity::getName()
{
    return hasCustomName() ? mCustomName->get() : std::wstring(L10N::GetString(0x2c090b72));
}

std::wstring BannerBlockEntity::getCustomName()
{
    return hasCustomName() ? mCustomName->get() : std::wstring(L10N::GetString(0x2c090b72));
}

bool BannerBlockEntity::hasCustomName()
{
    return mCustomName->checkParentalFilter() != 0 && !mCustomName->raw().empty();
}

std::wstring BannerBlockEntity::getDisplayName()
{
    return hasCustomName() ? getCustomName() : getName();
}

const DyeColor* BannerBlockEntity::getColor(const not_null_ptr<ItemInstance>& item)
{
    CompoundTag* tag = static_cast<CompoundTag*>(item->getTagElement(L"BlockEntityTag"));
    return tag != NULL && tag->contains(L"Base") ? DyeColor::byItemData(tag->getInt(L"Base")) : DyeColor::WHITE;
}

void BannerBlockEntity::setCustomName(const std::wstring& name, const std::wstring& uuid,
                                      bool chatCensored, bool uuidLocal)
{
    mCustomName->set(name, uuid, chatCensored, uuidLocal);
}

void BannerBlockEntity::setItemValues(not_null_ptr<ItemInstance> item, bool useBannerItemColor)
{
    mPatterns = NULL;
    CompoundTag* tag = static_cast<CompoundTag*>(item->getTagElement(L"BlockEntityTag"));
    if (tag != NULL && tag->contains(L"Patterns", 9))
        mPatterns = tag->getList(L"Patterns", 10);
    mBaseColor = useBannerItemColor ? getColor(item) : BannerItem::getColor(item);
    mPatternList.clear();
    mColorList.clear();
    mTextureHashName = L"";
    mPatternListCreated = true;
    std::wstring name = item->hasCustomHoverName() ? item->getHoverNameUnfiltered() : L"";
    setCustomName(name, item->getOwnerUUID(), item->wasCreatedByRestrictedPlayer(), item->wasCreatedOnHost());
}

ItemInstance* BannerBlockEntity::addTags(ItemInstance* item, int color, ListTag* patterns)
{
    addTags(not_null_ptr<ItemInstance>(item), color, patterns);
    return item;
}

void BannerBlockEntity::addTags(not_null_ptr<ItemInstance> item, int color, ListTag* patterns)
{
    CompoundTag* tag = static_cast<CompoundTag*>(item->getOrCreateTagElement(L"BlockEntityTag"));
    if (!tag->contains(L"Base")) tag->putInt(L"Base", color);
    if (patterns != NULL) tag->put(L"Patterns", patterns);
}

not_null_ptr<ItemInstance> BannerBlockEntity::getItem()
{
    not_null_ptr<ItemInstance> item(BannerItem::makeBanner(mBaseColor, mPatterns));
    addTags(item, mBaseColor->getItemData(), NULL);
    if (hasCustomName()) item->setHoverName(getName());
    return item;
}

CompoundTag* BannerBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putInt(L"Base", mBaseColor->getItemData());
    if (mPatterns != NULL) tag->put(L"Patterns", mPatterns);
    mCustomName->saveToTag(tag);
    return tag;
}

void BannerBlockEntity::addTags(CompoundTag* tag, int color, ListTag* patterns)
{
    tag->putInt(L"Base", color);
    if (patterns != NULL) tag->put(L"Patterns", patterns);
}

void BannerBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mCustomName->loadFromTag(tag);
    mBaseColor = DyeColor::byItemData(tag->getInt(L"Base"));
    mPatterns = tag->getList(L"Patterns", 10);
    mPatternList.clear();
    mColorList.clear();
    mTextureHashName = L"";
    mPatternListCreated = true;
}

boost::shared_ptr<Packet> BannerBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>(new ClientboundBlockEntityDataPacket(getBlockPos(), 6, 0, getUpdateTag()));
}

CompoundTag* BannerBlockEntity::getUpdateTag()
{
    CompoundTag* tag = new CompoundTag();
    save(tag);
    return tag;
}

const DyeColor* BannerBlockEntity::getBaseColor(const not_null_ptr<ItemInstance>& item)
{
    return getColor(item);
}

int BannerBlockEntity::getPatternCount(const not_null_ptr<ItemInstance>& item)
{
    CompoundTag* tag = static_cast<CompoundTag*>(item->getTagElement(L"BlockEntityTag"));
    return tag == NULL || !tag->contains(L"Patterns", 9) ? 0 : tag->getList(L"Patterns", 10)->size();
}

void BannerBlockEntity::createPatternList()
{
    if (mPatternListCreated) return;
    mPatternList.clear();
    mColorList.clear();
    mPatternList.push_back(BannerPattern::byHash(L"b"));
    mColorList.push_back(mBaseColor);
    mTextureHashName = L"b";
    if (mPatterns != NULL)
    {
        for (int i = 0; i < mPatterns->size(); ++i)
        {
            CompoundTag* tag = mPatterns->getCompound(i);
            BannerPattern* pattern = BannerPattern::byHash(tag->getString(L"Pattern"));
            if (pattern != NULL)
            {
                mPatternList.push_back(pattern);
                mColorList.push_back(DyeColor::byItemData(tag->getInt(L"Color")));
                mTextureHashName += pattern->getHashname();
                wchar_t number[16];
                swprintf(number, 16, L"%d", tag->getInt(L"Color"));
                mTextureHashName += number;
            }
        }
    }
    mPatternListCreated = true;
}

std::vector<const BannerPattern*>* BannerBlockEntity::getPatterns()
{
    createPatternList();
    return &mPatternList;
}

std::vector<const DyeColor*>* BannerBlockEntity::getColors()
{
    createPatternList();
    return &mColorList;
}

std::wstring BannerBlockEntity::getTextureHashName()
{
    createPatternList();
    return mTextureHashName;
}

void BannerBlockEntity::fjAddPatternToItem(const not_null_ptr<ItemInstance>& item,
                                           const BannerPattern* pattern, const DyeColor* color)
{
    CompoundTag* tag = static_cast<CompoundTag*>(item->getOrCreateTagElement(L"BlockEntityTag"));
    ListTag* patterns = tag->contains(L"Patterns", 9) ? tag->getList(L"Patterns", 10) : new ListTag(10);
    if (!tag->contains(L"Patterns", 9)) tag->put(L"Patterns", patterns);
    if (patterns->size() < 6)
    {
        CompoundTag* patternTag = new CompoundTag();
        patternTag->putInt(L"Color", color->getItemData());
        patternTag->putString(L"Pattern", pattern->getHashname());
        patterns->add(patternTag);
    }
}

void BannerBlockEntity::removeLastPattern(not_null_ptr<ItemInstance> item)
{
    CompoundTag* tag = static_cast<CompoundTag*>(item->getTagElement(L"BlockEntityTag"));
    if (tag == NULL || !tag->contains(L"Patterns", 9)) return;
    ListTag* patterns = tag->getList(L"Patterns", 10);
    if (patterns->size() == 0) return;
    patterns->remove(patterns->size() - 1);
    if (patterns->isEmpty()) item->getTag()->remove(L"BlockEntityTag");
    addTags(item, getBaseColor(item)->getItemData(), NULL);
}

boost::shared_ptr<BlockEntity> BannerBlockEntity::clone()
{
    boost::shared_ptr<BannerBlockEntity> result(new BannerBlockEntity());
    BlockEntity::clone(result);
    result->mBaseColor = mBaseColor;
    result->mPatterns = mPatterns;
    result->mPatternListCreated = mPatternListCreated;
    result->mPatternList = mPatternList;
    result->mColorList = mColorList;
    result->mTextureHashName = mTextureHashName;
    result->mCustomName = mCustomName;
    return result;
}

unsigned long long BannerBlockEntity::GetType()
{
    return 0x000000000100020cULL;
}

BlockEntity* BannerBlockEntity::create()
{
    return new BannerBlockEntity();
}
