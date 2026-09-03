#include "game/world/block/MushroomBlockThemeRuleDefinition.hpp"

#include "game/world/block/Blocks.hpp"
#include "game/world/block/HugeMushroomBlock.hpp"
#include "game/world/block/MushroomBlockLayerTheme.hpp"

static HugeMushroomBlock* sDefaultMushroomBlock = static_cast<HugeMushroomBlock*>(Blocks::red_mushroom_block);

MushroomBlockThemeRuleDefinition::MushroomBlockThemeRuleDefinition()
    : mBlock(sDefaultMushroomBlock)
{
    addBlockAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(12), &mBlock);
}

MushroomBlockThemeRuleDefinition::~MushroomBlockThemeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType MushroomBlockThemeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(69);
}

LayerTheme* MushroomBlockThemeRuleDefinition::Generate() const
{
    HugeMushroomBlock* mushroomBlock = dynamic_cast<HugeMushroomBlock*>(mBlock);

    if (mushroomBlock == NULL)
    {
        mushroomBlock = sDefaultMushroomBlock;
    }

    return new MushroomBlockLayerTheme(mushroomBlock);
}
