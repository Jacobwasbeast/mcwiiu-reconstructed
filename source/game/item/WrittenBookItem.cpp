#include "game/item/WrittenBookItem.hpp"
#include "game/core/StringIds.hpp"

#include <string>

#include "game/network/CGameNetworkManager.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/StringTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/WritableBookItem.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

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
    if (!WritableBookItem::makeSureTagIsValid(tag))
    {
        return false;
    }

    if (!tag->contains(L"title", Tag::TAG_STRING))
    {
        return false;
    }

    std::wstring title = tag->getString(L"title");
    if (title.empty() || title.size() > 32)
    {
        return false;
    }

    return tag->contains(L"author", Tag::TAG_STRING);
}

int WrittenBookItem::getGeneration(not_null_ptr<ItemInstance> itemInstance)
{
    return itemInstance->getTag()->getInt(L"generation");
}

std::wstring WrittenBookItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    if (itemInstance->hasTag())
    {
        CompoundTag* tag = itemInstance->getTag();

        bool ownedByLocalPlayer = false;
        if (tag->getString(L"ownerUUID").compare(L"") != 0)
        {
            for (int i = 0; i < 4; ++i)
            {
                boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[i];
                if (localPlayer != NULL && localPlayer->getUUID() == tag->getString(L"ownerUUID"))
                {
                    ownedByLocalPlayer = true;
                    break;
                }
            }
        }

        bool chatCensored = g_pApp->IsConsoleChatCensored();
        bool createdByRestrictedPlayer =
            tag->contains(L"createdByRestrictedPlayer") && tag->getByte(L"createdByRestrictedPlayer") != 0;
        bool createdByHost = true;
        if (tag->contains(L"createdByHost") && tag->getByte(L"createdByHost") == 0)
            createdByHost = false;

        if ((chatCensored || createdByRestrictedPlayer) && !ownedByLocalPlayer &&
            (!CGameNetworkManager::sInstance.IsHost() || !createdByHost))
        {
            return Item::getName(itemInstance);
        }

        std::wstring title = tag->getString(L"title");
        if (!title.empty())
            return title;
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
    std::wstring author = tag->getString(L"author");
    if (!author.empty())
    {
        std::wstring line = L10N::GetString(0xf68c61db);
        line.replace(line.find(L"%s"), 2, author);
        lines->push_back(HtmlString(line, eMinecraftColour_NOTSET, false, false, 0));
    }

    unsigned int generationIds[4];
    generationIds[0] = 0x4af696b3u;
    generationIds[1] = 0x24f41c4au;
    generationIds[2] = 0xfef1a1e1u;
    generationIds[3] = 0xd8ef2778u;
    lines->push_back(HtmlString(L10N::GetString(generationIds[tag->getInt(L"generation")]),
                                eMinecraftColour_NOTSET, false, false, 0));
}

void WrittenBookItem::resolveBookComponents(not_null_ptr<ItemInstance> itemInstance,
                                            boost::shared_ptr<Player> player)
{
    if (itemInstance->getTag() == NULL)
    {
        return;
    }

    CompoundTag* tag = itemInstance->getTag();
    if (tag->getBoolean(L"resolved"))
    {
        return;
    }

    tag->putBoolean(L"resolved", true);
    if (!makeSureTagIsValid(tag))
    {
        return;
    }

    ListTag* pages = tag->getList(L"pages", Tag::TAG_STRING);
    for (int i = 0; i < pages->size(); ++i)
    {
        pages->set(i, new StringTag(pages->getString(i)));
    }
    tag->put(L"pages", pages);
}

InteractionResultHolder WrittenBookItem::use(Level* level,
                                             boost::shared_ptr<Player> player,
                                             InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!level->mIsClientSide)
    {
        resolveBookComponents(itemInstance, player);
    }

    player->openItemInstanceGui(itemInstance, hand);
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

bool WrittenBookItem::TestUse(Level* level,
                              boost::shared_ptr<Player> player,
                              InteractionHand::EInteractionHand hand)
{
    return true;
}

unsigned int WrittenBookItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return IDS_TOOLTIPS_READ;
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

    return itemInstance->getTag()->contains(L"pages") || itemInstance->getTag()->contains(L"title");
}
