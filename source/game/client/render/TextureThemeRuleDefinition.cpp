#include "game/client/render/TextureThemeRuleDefinition.hpp"

#include "game/core/BufferedImage.hpp"
#include "game/core/DLCFile.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/LevelGenerationOptions.hpp"

std::vector<std::wstring> stringSplit(const std::wstring& text, wchar_t separator);

class CommonMasterGameMode
{
public:
    static CommonMasterGameMode* GetMasterOrClient();
    LevelGenerationOptions* getRoundLevelGenOptions() const;
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
    return stringSplit(mColours, L',');
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
    TextureTheme::BlockStack blockStack(blocks, mMask);
    stack = blockStack;
}
