#include "game/core/SchematicTheme.hpp"

#include "game/core/ConsoleSchematicFile.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockTransaction.hpp"

SchematicTheme::SchematicTheme()
{
}

void SchematicTheme::Apply(Random&, BlockTransaction* transaction, int& minY, int& maxY) const
{
    unsigned int size = transaction->mSize.x > transaction->mSize.z ? transaction->mSize.x : transaction->mSize.z;
    ConsoleSchematicFile* schematic = getAssetForSize(size);
    schematic->applySchematicFile(transaction);
    minY = transaction->getMin().y;
    maxY = transaction->getMax().y;
}
