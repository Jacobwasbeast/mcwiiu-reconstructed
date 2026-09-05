#include "game/item/WrittenBookItem.hpp"

#include <string>

#include "game/core/L10N.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/StringTag.hpp"
#include "game/item/ItemInstance.hpp"

WrittenBookItem::WrittenBookItem()
    : Item()
{
    setMaxStackSize(1);
}

WrittenBookItem::~WrittenBookItem()
{
}

bool WrittenBookItem::makeSureTagIsValid(CompoundTag* tag)
{
    if (tag == NULL || !tag->contains(L"pages", 8))
    {
        return false;
    }

    ListTag* pages = tag->getList(L"pages", 8);
    if (pages == NULL)
    {
        return false;
    }

    for (int i = 0; i < pages->size(); ++i)
    {
        if (pages->getString(i).size() > 256)
        {
            return false;
        }
    }

    if (!tag->contains(L"title", 8))
    {
        return false;
    }

    std::wstring title = tag->getString(L"title");
    return !title.empty() && title.size() < 33 && tag->contains(L"author", 8);
}

int WrittenBookItem::getGeneration(not_null_ptr<ItemInstance> itemInstance)
{
    CompoundTag* tag = itemInstance->getTag();
    if (tag == NULL)
    {
        return 0;
    }
    return tag->getInt(L"generation");
}

std::wstring WrittenBookItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    if (itemInstance->hasTag())
    {
        CompoundTag* tag = itemInstance->getTag();
        if (tag != NULL)
        {
            std::wstring title = tag->getString(L"title");
            if (!title.empty())
            {
                return title;
            }
        }
    }
    return Item::getName(itemInstance);
}

void WrittenBookItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance,
                                      boost::shared_ptr<Player> player,
                                      std::vector<HtmlString>* lines,
                                      bool advanced)
{
    if (!itemInstance->hasTag())
    {
        return;
    }

    CompoundTag* tag = itemInstance->getTag();
    if (tag == NULL)
    {
        return;
    }

    std::wstring author = tag->getString(L"author");
    if (!author.empty())
    {
        std::wstring line = L10N::GetString(0xf68c61db);
        std::wstring::size_type marker = line.find(L"%s");
        if (marker != std::wstring::npos)
        {
            line.replace(marker, 2, author);
        }
        lines->push_back(HtmlString(line, eMinecraftColour_HTMLColor_White, false, false, 0));
    }

    static const unsigned int generationIds[4] =
    {
        0x4af696b3u, 0x24f41c4au, 0xfef1a1e1u, 0xd8ef2778u
    };
    int generation = getGeneration(itemInstance);
    if (generation < 0 || generation >= 4)
    {
        generation = 0;
    }
    lines->push_back(HtmlString(L10N::GetString(generationIds[generation]),
                                eMinecraftColour_HTMLColor_White, false, false, 0));
}

void WrittenBookItem::resolveBookComponents(not_null_ptr<ItemInstance> itemInstance,
                                            boost::shared_ptr<Player> player)
{
    CompoundTag* tag = itemInstance->getTag();
    if (tag == NULL || tag->getBoolean(L"resolved"))
    {
        return;
    }

    tag->putBoolean(L"resolved", true);
    if (!makeSureTagIsValid(tag))
    {
        return;
    }

    ListTag* pages = tag->getList(L"pages", 8);
    if (pages == NULL)
    {
        return;
    }

    for (int i = 0; i < pages->size(); ++i)
    {
        std::wstring page = pages->getString(i);
        pages->set(i, new StringTag(page));
    }
    tag->put(L"pages", pages);
}

InteractionResultHolder WrittenBookItem::use(Level* level,
                                             boost::shared_ptr<Player> player,
                                             InteractionHand::EInteractionHand hand)
{
    return Item::use(level, player, hand);
}

bool WrittenBookItem::TestUse(Level* level,
                              boost::shared_ptr<Player> player,
                              InteractionHand::EInteractionHand hand)
{
    return true;
}

unsigned int WrittenBookItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return 0x80699fffu;
}

bool WrittenBookItem::isFoil(not_null_ptr<ItemInstance> itemInstance)
{
    return true;
}

bool WrittenBookItem::HasBookTags(not_null_ptr<ItemInstance> itemInstance)
{
    if (!itemInstance->hasTag())
    {
        return false;
    }

    CompoundTag* tag = itemInstance->getTag();
    return tag != NULL && (tag->contains(L"title") || tag->contains(L"pages"));
}
