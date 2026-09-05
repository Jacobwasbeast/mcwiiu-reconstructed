#include "game/core/MineShaftStart.hpp"

#include "game/core/BoundingBox.hpp"
#include "game/core/MineShaftPieces.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

MineShaftStart::MineShaftStart(Level* level, Random& random, int chunkX, int chunkZ,
                               MineShaftFeature::MineShaftType type)
    : StructureStart(chunkX, chunkZ), mType(type)
{
    MineShaftPieces::MineShaftRoom* room =
        new MineShaftPieces::MineShaftRoom(0, random, chunkX * 16 + 2, chunkZ * 16 + 2, type);
    push_back(room);
    room->addChildren(room, this, random);
    calculateBoundingBox();

    if (type == MineShaftFeature::eMineShaftType_Mesa)
    {
        BoundingBox* box = getBoundingBox();
        int y = level->getSeaLevel() - box->y0 + box->getYSpan() / 2 + 5;
        box->move(0, y, 0);
        for (std::vector<StructurePiece*>::iterator it = begin(); it != end(); ++it)
        {
            (*it)->move(0, y, 0);
        }
    }
    else
    {
        moveBelowSeaLevel(level, random, 10);
    }
}

MineShaftStart::MineShaftStart()
    : StructureStart()
{
}

MineShaftStart::~MineShaftStart()
{
}

EStructureStart MineShaftStart::GetType()
{
    return eStructureStart_MineShaft;
}

StructureStart* MineShaftStart::Create()
{
    return new MineShaftStart();
}
