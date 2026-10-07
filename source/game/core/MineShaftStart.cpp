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
    mPieces.push_back(room);
    room->addChildren(room, &mPieces, random);
    calculateBoundingBox();

    if (type == MineShaftFeature::eMineShaftType_Mesa)
    {
        int seaLevel = level->getSeaLevel();
        BoundingBox* box = mBoundingBox;
        int y = seaLevel - box->y0 + box->getYSpan() / 2 + 5;
        box->move(0, y, 0);
        for (std::vector<StructurePiece*>::iterator it = mPieces.begin(); it != mPieces.end(); ++it)
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

EStructureStart MineShaftStart::GetType()
{
    return eStructureStart_MineShaft;
}

StructureStart* MineShaftStart::Create()
{
    return new MineShaftStart();
}
