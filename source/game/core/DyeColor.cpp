#include "game/core/DyeColor.hpp"

const DyeColor* DyeColor::WHITE;
const DyeColor* DyeColor::ORANGE;
const DyeColor* DyeColor::MAGENTA;
const DyeColor* DyeColor::LIGHT_BLUE;
const DyeColor* DyeColor::YELLOW;
const DyeColor* DyeColor::LIME;
const DyeColor* DyeColor::PINK;
const DyeColor* DyeColor::GRAY;
const DyeColor* DyeColor::SILVER;
const DyeColor* DyeColor::CYAN;
const DyeColor* DyeColor::PURPLE;
const DyeColor* DyeColor::BLUE;
const DyeColor* DyeColor::BROWN;
const DyeColor* DyeColor::GREEN;
const DyeColor* DyeColor::RED;
const DyeColor* DyeColor::BLACK;

std::vector<const DyeColor*> DyeColor::COLORS;
const DyeColor* DyeColor::BY_BLOCK_DATA[16];
const DyeColor* DyeColor::BY_ITEM_DATA[16];

DyeColor::DyeColor(int blockData, int itemData, const std::wstring& serializedName,
                   int name, int useName, int textureDiffuseColor,
                   eMinecraftColour fontColorFormat, eMinecraftColour shulkerColor,
                   eMinecraftColour bedColor)
{
    COLORS.push_back(this);

    this->blockData = blockData;
    this->itemData = itemData;
    this->serializedName = serializedName;
    this->name = name;
    this->useName = useName;
    this->textureDiffuseColor = textureDiffuseColor;
    this->fontColorFormat = fontColorFormat;
    this->shulkerColor = shulkerColor;
    this->bedColor = bedColor;

    float* colours = new float[3];

    colours[0] = ((textureDiffuseColor >> 16) & 0xFF) / 255.0f;
    colours[1] = ((textureDiffuseColor >> 8) & 0xFF) / 255.0f;
    colours[2] = (textureDiffuseColor & 0xFF) / 255.0f;

    this->textureDiffuseColors.data = colours;
    this->textureDiffuseColors.length = 3;
}

void DyeColor::staticCtor()
{
    WHITE = new DyeColor(0, 15, L"white", 0xB6554A0A, 0xF31D46E2, 0xF9FFFE,
                         eMinecraftColour_HTMLColor_White, eMinecraftColour_Shulker_Box_White, eMinecraftColour_Bed_White);
    ORANGE = new DyeColor(1, 14, L"orange", 0xC409EE05, 0xC5B9A09D, 0xF9801D,
                          eMinecraftColour_HTMLColor_6, eMinecraftColour_Shulker_Box_Orange, eMinecraftColour_Bed_Orange);
    MAGENTA = new DyeColor(2, 13, L"magenta", 0x8BFBBD58, 0x98ECFA70, 0xC74EBD,
                           eMinecraftColour_HTMLColor_b, eMinecraftColour_Shulker_Box_Magenta, eMinecraftColour_Bed_Magenta);
    LIGHT_BLUE = new DyeColor(3, 12, L"light_blue", 0x42FCCD5A, 0x422CC98D, 0x3AB3DA,
                              eMinecraftColour_HTMLColor_9, eMinecraftColour_Shulker_Box_Light_Blue, eMinecraftColour_Bed_Light_Blue);
    YELLOW = new DyeColor(4, 11, L"yellow", 0xB868CB59, 0x5A619CF1, 0xFED83D,
                          eMinecraftColour_HTMLColor_e, eMinecraftColour_Shulker_Box_Yellow, eMinecraftColour_Bed_Yellow);
    LIME = new DyeColor(5, 10, L"lime", 0x3E31AF30, 0x7A34CFB8, 0x80C71F,
                        eMinecraftColour_HTMLColor_a, eMinecraftColour_Shulker_Box_Light_Green, eMinecraftColour_Bed_Lime);
    PINK = new DyeColor(6, 9, L"pink", 0xCE5BA8C1, 0x65099C29, 0xF38BAA,
                        eMinecraftColour_HTMLColor_5, eMinecraftColour_Shulker_Box_Pink, eMinecraftColour_Bed_Pink);
    GRAY = new DyeColor(7, 8, L"gray", 0x23BCA928, 0x7AB37FB0, 0x474F52,
                        eMinecraftColour_HTMLColor_dark_7, eMinecraftColour_Shulker_Box_Grey, eMinecraftColour_Bed_Grey);
    SILVER = new DyeColor(8, 7, L"silver", 0x2E44B902, 0x8940715E, 0x9D9D97,
                          eMinecraftColour_HTMLColor_8, eMinecraftColour_Shulker_Box_Silver, eMinecraftColour_Bed_Silver);
    CYAN = new DyeColor(9, 6, L"cyan", 0x4BF23720, 0x80E27EB8, 0x169C9C,
                        eMinecraftColour_HTMLColor_b, eMinecraftColour_Shulker_Box_Cyan, eMinecraftColour_Bed_Cyan);
    PURPLE = new DyeColor(10, 5, L"purple", 0x99071123, 0xC0FC96AB, 0x8932B8,
                          eMinecraftColour_HTMLColor_dark_5, eMinecraftColour_Shulker_Box_Purple, eMinecraftColour_Bed_Purple);
    BLUE = new DyeColor(11, 4, L"blue", 0x421475DB, 0x75FE8F43, 0x3C44AA,
                        eMinecraftColour_HTMLColor_dark_1, eMinecraftColour_Shulker_Box_Blue, eMinecraftColour_Bed_Blue);
    BROWN = new DyeColor(12, 3, L"brown", 0x44529A8B, 0xFB9441C3, 0x835432,
                         eMinecraftColour_HTMLColor_dark_c, eMinecraftColour_Shulker_Box_Brown, eMinecraftColour_Bed_Brown);
    GREEN = new DyeColor(13, 2, L"green", 0x83F9110E, 0x77D75836, 0x5E7C16,
                         eMinecraftColour_HTMLColor_dark_2, eMinecraftColour_Shulker_Box_Green, eMinecraftColour_Bed_Green);
    RED = new DyeColor(14, 1, L"red", 0x54A5D9B6, 0xFBA2A6EE, 0xB02E26,
                       eMinecraftColour_HTMLColor_dark_4, eMinecraftColour_Shulker_Box_Red, eMinecraftColour_Bed_Red);
    BLACK = new DyeColor(15, 0, L"black", 0xBD5568E2, 0x6F2DFA1A, 0x1D1D21,
                         eMinecraftColour_HTMLColor_Black, eMinecraftColour_Shulker_Box_Black, eMinecraftColour_Bed_Black);

    for (std::vector<const DyeColor*>::iterator it = COLORS.begin(); it != COLORS.end(); ++it)
    {
        const DyeColor* color = *it;

        BY_BLOCK_DATA[color->getBlockData()] = color;
        BY_ITEM_DATA[color->getItemData()] = color;
    }
}

const DyeColor* DyeColor::byBlockData(int blockData)
{
    if (blockData < 0 || blockData >= 16)
    {
        blockData = 0;
    }

    return BY_BLOCK_DATA[blockData];
}

const DyeColor* DyeColor::byItemData(int itemData)
{
    if (itemData < 0 || itemData >= 16)
    {
        itemData = 0;
    }

    return BY_ITEM_DATA[itemData];
}

int DyeColor::getBlockData() const
{
    return blockData;
}

int DyeColor::getItemData() const
{
    return itemData;
}

int DyeColor::getName() const
{
    return name;
}

int DyeColor::getUseName() const
{
    return useName;
}

int DyeColor::getTextureDiffuseColor() const
{
    return textureDiffuseColor;
}

arrayWithLength<const float> DyeColor::getTextureDiffuseColors() const
{
    return textureDiffuseColors;
}

eMinecraftColour DyeColor::getShulkerColor() const
{
    return shulkerColor;
}

eMinecraftColour DyeColor::getBedColor() const
{
    return bedColor;
}

std::wstring DyeColor::toString() const
{
    return L"DyeColorString";
}

std::wstring DyeColor::getSerializedName() const
{
    return serializedName;
}
