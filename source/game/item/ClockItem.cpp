#include "game/item/ClockItem.hpp"

#include "game/core/IconRegister.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/ClockItem_ItemPropertyFunction.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

ClockItem::ClockItem()
    : Item()
{
    mIcons[0] = NULL;
    addProperty(ResourceLocation(L"time"), new ClockItem_ItemPropertyFunction());
}

ClockItem::~ClockItem()
{
}

TextureAtlasSprite* ClockItem::getIcon(int auxValue)
{
    TextureAtlasSprite* icon = Item::getIcon(auxValue);
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mPlayer && auxValue == 0)
        icon = mIcons[static_cast<LocalPlayer*>(minecraft->mPlayer.get())->mUserIndex];
    return icon;
}

void ClockItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    mIcons[0] = iconRegister->registerIcon(L"clockP0");
    mIcons[1] = iconRegister->registerIcon(L"clockP1");
    mIcons[2] = iconRegister->registerIcon(L"clockP2");
    mIcons[3] = iconRegister->registerIcon(L"clockP3");
}
