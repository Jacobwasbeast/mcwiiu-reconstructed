#include "game/minigame/SchematicThemeRuleDefinition.hpp"

#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/ConsoleSchematicFile.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/core/SchematicTheme.hpp"
#include "game/world/LevelGenerationOptions.hpp"

SchematicThemeRuleDefinition::SchematicThemeRuleDefinition() = default;

SchematicThemeRuleDefinition::~SchematicThemeRuleDefinition()
{
    for (std::vector<AssetRuleDef*>::iterator asset = mAssets.begin(); asset != mAssets.end(); ++asset)
    {
        delete *asset;
    }
}

ConsoleGameRules::EGameRuleType SchematicThemeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(71);
}

LayerTheme* SchematicThemeRuleDefinition::Generate() const
{
    SchematicTheme* theme = new SchematicTheme();

    for (std::vector<AssetRuleDef*>::const_iterator asset = mAssets.begin(); asset != mAssets.end(); ++asset)
    {
        std::wstring filename = ConsoleSchematicFile::formatFilenameWithExt((*asset)->mFilename);
        if (!filename.empty())
        {
            CommonMasterGameMode* gameMode = CommonMasterGameMode::GetMasterOrClient();
            LevelGenerationOptions* options = gameMode->getRoundLevelGenOptions();
            ConsoleSchematicFile* schematic = options->getSchematicFile(filename);
            if (schematic != NULL)
            {
                static_cast<AssetTheme<ConsoleSchematicFile*>*>(theme)->addAsset(
                    static_cast<unsigned int>((*asset)->mMinimumWidth), schematic);
            }
        }
    }

    return theme;
}

GameRuleDefinition* SchematicThemeRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    if (type == static_cast<ConsoleGameRules::EGameRuleType>(39))
    {
        AssetRuleDef* asset = new AssetRuleDef();
        mAssets.push_back(asset);
        return asset;
    }

    return GameRuleDefinition::addChild(type);
}

SchematicThemeRuleDefinition::AssetRuleDef::AssetRuleDef()
    : mFilename()
    , mMinimumWidth(0)
{
    addStringAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(16), &mFilename);
    GameRules_IntAttribute* width = addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(64), &mMinimumWidth);
    width->setMin(0);
    width->setMax(128);
}

SchematicThemeRuleDefinition::AssetRuleDef::~AssetRuleDef() = default;

ConsoleGameRules::EGameRuleType SchematicThemeRuleDefinition::AssetRuleDef::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(39);
}
