#include "game/item/FireworkChargeItem.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/core/IconRegister.hpp"
#include "game/data/Tag.hpp"
#include "game/core/HtmlString.hpp"
#include "game/item/ItemInstance.hpp"

FireworkChargeItem::FireworkChargeItem()
    : Item(), mOverlayIcon(NULL)
{
}

FireworkChargeItem::~FireworkChargeItem()
{
}

Tag* FireworkChargeItem::getExplosionTagField(not_null_ptr<ItemInstance> item, const std::wstring& name)
{
    if (!item || !item->hasTag())
    {
        return NULL;
    }
    CompoundTag* tag = item->getTag();
    if (tag == NULL)
    {
        return NULL;
    }
    CompoundTag* explosion = tag->getCompound(L"Explosion");
    return explosion == NULL ? NULL : explosion->get(name);
}

void FireworkChargeItem::appendHoverText(CompoundTag* tag, std::vector<HtmlString>* lines)
{
    if (tag == NULL || lines == NULL)
    {
        return;
    }
    static const wchar_t* shapeNames[] = { L"Small Ball", L"Large Ball", L"Star", L"Creeper", L"Burst" };
    unsigned char shape = tag->getByte(L"Type");
    std::wstring shapeText = shape < 5 ? shapeNames[shape] : L"Unknown";
    lines->push_back(HtmlString(shapeText, eMinecraftColour_NOTSET, false, false, 0));
    if (tag->getIntArray(L"Colors").data != NULL)
    {
        lines->push_back(HtmlString(L"Colors", eMinecraftColour_NOTSET, false, false, 0));
    }
    if (tag->getIntArray(L"FadeColors").data != NULL)
    {
        lines->push_back(HtmlString(L"Fade Colors", eMinecraftColour_NOTSET, false, false, 0));
    }
    if (tag->getBoolean(L"Trail"))
    {
        lines->push_back(HtmlString(L"Trail", eMinecraftColour_NOTSET, false, false, 0));
    }
    if (tag->getBoolean(L"Flicker"))
    {
        lines->push_back(HtmlString(L"Twinkle", eMinecraftColour_NOTSET, false, false, 0));
    }
}

void FireworkChargeItem::appendHoverText(not_null_ptr<ItemInstance> item, boost::shared_ptr<Player> player, std::vector<HtmlString>* lines, bool advanced)
{
    if (!item || !item->hasTag() || lines == NULL)
    {
        return;
    }
    CompoundTag* tag = item->getTag();
    if (tag != NULL)
    {
        CompoundTag* explosion = tag->getCompound(L"Explosion");
        if (explosion != NULL)
        {
            appendHoverText(explosion, lines);
        }
    }
}

TextureAtlasSprite* FireworkChargeItem::getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> item)
{
    if (layer < 1)
    {
        return Item::getLayerIcon(auxValue, layer, item);
    }
    return mOverlayIcon;
}

bool FireworkChargeItem::hasMultipleSpriteLayers()
{
    return true;
}

void FireworkChargeItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    if (iconRegister != NULL)
    {
        std::wstring overlay = getIconName() + L"_overlay";
        mOverlayIcon = iconRegister->registerIcon(overlay);
    }
}
