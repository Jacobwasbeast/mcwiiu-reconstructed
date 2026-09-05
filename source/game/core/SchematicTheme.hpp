#ifndef GAME_CORE_SCHEMATICTHEME_HPP
#define GAME_CORE_SCHEMATICTHEME_HPP

#include "game/core/AssetTheme.hpp"
#include "game/world/generation/LayerTheme.hpp"

class ConsoleSchematicFile;

class SchematicTheme : public LayerTheme
{
public:
    SchematicTheme();
    ~SchematicTheme();

    virtual void Apply(Random&, BlockTransaction*, int&, int&) const;

private:
    AssetTheme<ConsoleSchematicFile*> mAssets;
};

#endif
