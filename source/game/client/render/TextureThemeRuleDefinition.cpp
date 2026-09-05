#include "game/client/render/TextureThemeRuleDefinition.hpp"

#include "game/core/BufferedImage.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/DLCFile.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/LevelGenerationOptions.hpp"

std::vector<std::wstring> stringSplit(const std::wstring& text, wchar_t separator);

static const std::wstring gColourReplacements[4][8] =
{
    { L"white", L"0xFFFFFF", L"silver", L"0xC0C0C0", L"gray", L"0x808080", L"black", L"0x000000" },
    { L"red", L"0xFF0000", L"maroon", L"0x800000", L"yellow", L"0xFFFF00", L"olive", L"0x808000" },
    { L"lime", L"0x00FF00", L"green", L"0x008000", L"cyan", L"0x00FFFF", L"teal", L"0x008080" },
    { L"blue", L"0x0000FF", L"navy", L"0x000080", L"magenta", L"0xFF00FF", L"purple", L"0x800080" }
};

TextureThemeRuleDefinition::TextureThemeRuleDefinition()
    : mLayerTextures()
    , mBlockStackGetters()
{
}

TextureThemeRuleDefinition::~TextureThemeRuleDefinition()
{
    for (std::vector<LayerTexture*>::iterator it = mLayerTextures.begin(); it != mLayerTextures.end(); ++it)
    {
        delete *it;
    }
    for (std::vector<BlockStackGetter*>::iterator it = mBlockStackGetters.begin(); it != mBlockStackGetters.end(); ++it)
    {
        delete *it;
    }
}

ConsoleGameRules::EGameRuleType TextureThemeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(70);
}

TextureTheme* TextureThemeRuleDefinition::Generate() const
{
    TextureTheme* theme = new TextureTheme();
    for (std::vector<LayerTexture*>::const_iterator it = mLayerTextures.begin(); it != mLayerTextures.end(); ++it)
    {
        CommonMasterGameMode* gameMode = CommonMasterGameMode::GetMasterOrClient();
        LevelGenerationOptions* options = gameMode->getRoundLevelGenOptions();
        DLCPack* pack = options->mPack;
        DLCFile* file = pack->getFile(static_cast<DLCManager::EDLCType>(2), (*it)->mFilename, true, true);
        unsigned int length = 0;
        BufferedImage* image = new BufferedImage(file->getData(length), length);
        theme->addAsset(static_cast<unsigned int>((*it)->mMinimumWidth), image);
    }
    for (std::vector<BlockStackGetter*>::const_iterator it = mBlockStackGetters.begin(); it != mBlockStackGetters.end(); ++it)
    {
        TextureTheme::BlockStack stack;
        (*it)->getStack(stack);
        std::vector<std::wstring> colours = (*it)->getColours();
        for (std::vector<std::wstring>::const_iterator colour = colours.begin(); colour != colours.end(); ++colour)
        {
            theme->addStack(*colour, stack);
        }
    }
    return theme;
}

GameRuleDefinition* TextureThemeRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    if (type == static_cast<ConsoleGameRules::EGameRuleType>(39))
    {
        LayerTexture* texture = new LayerTexture();
        mLayerTextures.push_back(texture);
        return texture;
    }
    if (type == static_cast<ConsoleGameRules::EGameRuleType>(43))
    {
        IndexedBlockDef* block = new IndexedBlockDef();
        mBlockStackGetters.push_back(block);
        return block;
    }
    if (type == static_cast<ConsoleGameRules::EGameRuleType>(61))
    {
        MaskedShaftsThemeRuleDefinition* shafts = new MaskedShaftsThemeRuleDefinition();
        mBlockStackGetters.push_back(shafts);
        return shafts;
    }
    return GameRuleDefinition::addChild(type);
}

TextureThemeRuleDefinition::BlockStackGetter::~BlockStackGetter()
{
}

std::vector<std::wstring> TextureThemeRuleDefinition::BlockStackGetter::getColours() const
{
    std::vector<std::wstring> colours = stringSplit(mColours, L',');
    for (std::vector<std::wstring>::iterator colour = colours.begin(); colour != colours.end(); ++colour)
    {
        for (int group = 0; group < 4; ++group)
        {
            for (int replacement = 0; replacement < 4; ++replacement)
            {
                if (*colour == gColourReplacements[group][replacement * 2])
                {
                    *colour = gColourReplacements[group][replacement * 2 + 1];
                }
            }
        }
    }
    return colours;
}

TextureThemeRuleDefinition::IndexedBlockDef::~IndexedBlockDef()
{
}

void TextureThemeRuleDefinition::IndexedBlockDef::getStack(TextureTheme::BlockStack& stack)
{
    TextureTheme::BlockStack blockStack(getBlockState(), 1, 0);
    stack = blockStack;
}

TextureThemeRuleDefinition::LayerTexture::LayerTexture()
    : mFilename()
    , mMinimumWidth(0)
{
    addStringAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(9), &mFilename);
    GameRules_IntAttribute* width = addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(64), &mMinimumWidth);
    width->setMin(0);
    width->setMax(128);
}

TextureThemeRuleDefinition::LayerTexture::~LayerTexture()
{
}

ConsoleGameRules::EGameRuleType TextureThemeRuleDefinition::LayerTexture::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(39);
}

TextureThemeRuleDefinition::MaskedShaftsThemeRuleDefinition::~MaskedShaftsThemeRuleDefinition()
{
}

void TextureThemeRuleDefinition::MaskedShaftsThemeRuleDefinition::getStack(TextureTheme::BlockStack& stack)
{
    std::vector<const BlockState*> blocks;
    fillStack(blocks);
    TextureTheme::BlockStack blockStack(blocks, mHeightOffset);
    stack = blockStack;
}
