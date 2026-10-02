#include "game/world/block/BannerBlockEntity.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <cwchar>

#include "game/core/BannerPattern.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/L10N.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/BannerItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/core/StringConversion.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"

static const std::wstring TAG_BASE = L"Base";
static const std::wstring TAG_PATTERNS = L"Patterns";
static const std::wstring TAG_PATTERN = L"Pattern";
static const std::wstring TAG_COLOR = L"Color";

BannerBlockEntity::BannerBlockEntity()
    : BlockEntity(), mName(), mBaseColor(DyeColor::BLACK), mPatterns(NULL), mPatternDataSet(false),
      mPatternList(NULL), mColorList(NULL), mTextureHashName(), mCustomName()
{
    mTextureHashName = L"";
    mCustomName = boost::shared_ptr<CensoredString>(new CensoredString());
}

BannerBlockEntity::~BannerBlockEntity()
{
    delete mPatternList;
    delete mColorList;
    delete mPatterns;
}

std::wstring BannerBlockEntity::getName()
{
    return hasCustomName() ? mCustomName->get() : std::wstring(L10N::GetString(IDS_ITEM_BANNER));
}

std::wstring BannerBlockEntity::getCustomName()
{
    return hasCustomName() ? mCustomName->get() : std::wstring(L10N::GetString(IDS_ITEM_BANNER));
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
    if (tag != NULL && tag->contains(L"Base"))
    {
        return DyeColor::byItemData(tag->getInt(L"Base"));
    }
    return DyeColor::BLACK;
}

void BannerBlockEntity::setCustomName(const std::wstring& name, const std::wstring& uuid,
                                      bool chatCensored, bool uuidLocal)
{
    mCustomName->set(name, uuid, chatCensored, uuidLocal);
}

void BannerBlockEntity::setItemValues(not_null_ptr<ItemInstance> item, bool useBannerItemColor)
{
    delete mPatterns;
    mPatterns = NULL;
    CompoundTag* tag = static_cast<CompoundTag*>(item->getTagElement(L"BlockEntityTag"));
    if (tag != NULL && tag->contains(TAG_PATTERNS, Tag::TAG_LIST))
    {
        mPatterns = static_cast<ListTag*>(tag->getList(TAG_PATTERNS, Tag::TAG_COMPOUND)->copy());
    }
    mBaseColor = useBannerItemColor ? getColor(item) : BannerItem::getColor(item);

    delete mPatternList;
    delete mColorList;
    mColorList = NULL;
    mPatternList = NULL;
    mTextureHashName = L"";
    mPatternDataSet = true;

    std::wstring name = item->hasCustomHoverName() ? item->getHoverNameUnfiltered() : L"";
    setCustomName(name, item->getOwnerUUID(), item->wasCreatedByRestrictedPlayer(), item->wasCreatedOnHost());
}

ItemInstance* BannerBlockEntity::addTags(ItemInstance* item, int color, ListTag* patterns)
{
    CompoundTag* tag = static_cast<CompoundTag*>(item->getOrCreateTagElement(L"BlockEntityTag"));
    if (tag == NULL)
    {
        tag = new CompoundTag();
        item->addTagElement(L"BlockEntityTag", tag);
    }
    if (!tag->contains(L"Base"))
    {
        tag->putInt(TAG_BASE, color);
    }
    if (patterns != NULL)
    {
        tag->put(TAG_PATTERNS, patterns->copy());
    }
    return item;
}

void BannerBlockEntity::addTags(not_null_ptr<ItemInstance> item, int color, ListTag* patterns)
{
    addTags(item.get(), color, patterns);
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
    tag->putInt(TAG_BASE, mBaseColor->getItemData());
    if (mPatterns != NULL)
    {
        tag->put(TAG_PATTERNS, mPatterns->copy());
    }
    mCustomName->saveToTag(tag);
    return tag;
}

void BannerBlockEntity::addTags(CompoundTag* tag, int color, ListTag* patterns)
{
    tag->putInt(TAG_BASE, color);
    if (patterns != NULL)
    {
        tag->put(TAG_PATTERNS, patterns->copy());
    }
}

void BannerBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mCustomName->loadFromTag(tag);
    mBaseColor = DyeColor::byItemData(tag->getInt(L"Base"));
    mPatterns = static_cast<ListTag*>(tag->getList(TAG_PATTERNS, Tag::TAG_COMPOUND)->copy());

    delete mPatternList;
    delete mColorList;
    mColorList = NULL;
    mPatternList = NULL;
    mTextureHashName = L"";
    mPatternDataSet = true;
}

boost::shared_ptr<Packet> BannerBlockEntity::getUpdatePacket()
{
    int dimension = mLevel->mDimension->getType()->getId();
    return boost::shared_ptr<Packet>(new ClientboundBlockEntityDataPacket(mPos, 6, dimension, getUpdateTag()));
}

CompoundTag* BannerBlockEntity::getUpdateTag()
{
    CompoundTag* tag = new CompoundTag();
    save(tag);
    return tag;
}

const DyeColor* BannerBlockEntity::getBaseColor(const not_null_ptr<ItemInstance>& item)
{
    return DyeColor::byItemData(item->getAuxValue());
}

int BannerBlockEntity::getPatternCount(const not_null_ptr<ItemInstance>& item)
{
    CompoundTag* tag = static_cast<CompoundTag*>(item->getOrCreateTagElement(L"BlockEntityTag"));
    if (tag != NULL && tag->contains(TAG_PATTERNS))
    {
        return tag->getList(TAG_PATTERNS, Tag::TAG_COMPOUND)->size();
    }
    return 0;
}

void BannerBlockEntity::createPatternList()
{
    if (mPatternList != NULL && mColorList != NULL && !mTextureHashName.empty())
    {
        return;
    }

    if (!mPatternDataSet)
    {
        mTextureHashName = L"";
        return;
    }

    mPatternList = new std::vector<const BannerPattern*>();
    mColorList = new std::vector<const DyeColor*>();
    mPatternList->push_back(BannerPattern::BASE);
    mColorList->push_back(mBaseColor);
    mTextureHashName = static_cast<wchar_t>(mBaseColor->getItemData());
    if (mPatterns == NULL)
    {
        return;
    }

    for (int i = 0; i < mPatterns->size(); ++i)
    {
        CompoundTag* patternTag = mPatterns->getCompound(i);
        const BannerPattern* pattern = BannerPattern::byHash(patternTag->getString(TAG_PATTERN));
        if (pattern != NULL)
        {
            mPatternList->push_back(pattern);
            int color = patternTag->getInt(TAG_COLOR);
            mColorList->push_back(DyeColor::byItemData(color));
            mTextureHashName += pattern->getHashname() + _toString<int>(color);
        }
    }
}

std::vector<const BannerPattern*>* BannerBlockEntity::getPatterns()
{
    createPatternList();
    return mPatternList;
}

std::vector<const DyeColor*>* BannerBlockEntity::getColors()
{
    createPatternList();
    return mColorList;
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
    ListTag* patterns = tag->contains(TAG_PATTERNS, Tag::TAG_LIST) ? tag->getList(TAG_PATTERNS, Tag::TAG_COMPOUND) : new ListTag(0);
    if (!tag->contains(TAG_PATTERNS, Tag::TAG_LIST)) tag->put(TAG_PATTERNS, patterns);
    if (patterns->size() < 6)
    {
        CompoundTag* patternTag = new CompoundTag();
        patternTag->putInt(TAG_COLOR, color->getItemData());
        patternTag->putString(TAG_PATTERN, pattern->getHashname());
        patterns->add(patternTag);
    }
}

void BannerBlockEntity::removeLastPattern(not_null_ptr<ItemInstance> item)
{
    CompoundTag* tag = static_cast<CompoundTag*>(item->getTagElement(L"BlockEntityTag"));
    if (tag == NULL || !tag->contains(TAG_PATTERNS, Tag::TAG_LIST)) return;
    ListTag* patterns = tag->getList(TAG_PATTERNS, Tag::TAG_COMPOUND);
    if (patterns->isEmpty())
    {
        return;
    }
    patterns->remove(patterns->size() - 1);
    if (patterns->isEmpty())
    {
        item->getTag()->remove(L"BlockEntityTag");
        if (item->getTag()->isEmpty())
        {
            item->setTag(NULL);
            addTags(item, DyeColor::byItemData(item->getAuxValue())->getItemData(), NULL);
        }
    }
}

boost::shared_ptr<BlockEntity> BannerBlockEntity::clone()
{
    return boost::shared_ptr<BlockEntity>();
}

unsigned long long BannerBlockEntity::GetType()
{
    return BlockEntityType::BANNER;
}

BlockEntity* BannerBlockEntity::create()
{
    return new BannerBlockEntity();
}
