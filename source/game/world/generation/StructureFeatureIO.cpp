#include "game/world/generation/StructureFeatureIO.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/EndCityPieces.hpp"
#include "game/core/MineShaftPieces.hpp"
#include "game/core/MineShaftStart.hpp"
#include "game/core/NetherBridgePieces.hpp"
#include "game/core/OceanMonumentPieces.hpp"
#include "game/core/StrongholdPieces.hpp"
#include "game/data/ByteArrayTag.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/BuriedTreasureFeature.hpp"
#include "game/world/generation/NetherBridgeFeature.hpp"
#include "game/world/generation/OceanMonumentFeature.hpp"
#include "game/world/generation/RandomScatteredLargeFeature.hpp"
#include "game/world/generation/ScatteredFeaturePieces.hpp"
#include "game/world/generation/ShipwreckFeature.hpp"
#include "game/world/generation/StrongholdFeature.hpp"
#include "game/world/generation/StructureStart.hpp"
#include "game/world/generation/VillageFeature.hpp"
#include "game/world/generation/VillagePieces.hpp"
#include "game/world/generation/WoodlandMansionFeature.hpp"
#include "game/world/generation/WoodlandMansionPieces.hpp"

boost::unordered_map<std::wstring, StructureFeatureIO::StructureStartFactory> StructureFeatureIO::startFactories;
boost::unordered_map<unsigned int, std::wstring> StructureFeatureIO::startIds;
boost::unordered_map<std::wstring, StructureFeatureIO::StructurePieceFactory> StructureFeatureIO::pieceFactories;
boost::unordered_map<unsigned int, std::wstring> StructureFeatureIO::pieceIds;
int StructureFeatureIO::version = 0;

void StructureFeatureIO::staticCtor()
{
    setStartId(eStructureStart_MineShaft, MineShaftStart::Create, L"Mineshaft");
    setStartId(eStructureStart_Village, VillageFeature::VillageStart::Create, L"Village");
    setStartId(eStructureStart_NetherBridge, NetherBridgeFeature::NetherBridgeStart::Create, L"Fortress");
    setStartId(eStructureStart_Stronghold, StrongholdFeature::StrongholdStart::Create, L"StrongHold");
    setStartId(eStructureStart_ScatteredFeature, RandomScatteredLargeFeature::ScatteredFeatureStart::Create, L"Temple");
    setStartId(eStructureStart_OceanMonument, OceanMonumentFeature::OceanMonumentStart::Create, L"Monument");
    setStartId(eStructureStart_WoodlandMansion, WoodlandMansionFeature::WoodlandMansionStart::Create, L"Mansion");
    setStartId(eStructureStart_BuriedTreasure, BuriedTreasureFeature::BuriedTreasureStart::Create, L"BuriedTreasure");
    setStartId(eStructureStart_Shipwreck, ShipwreckFeature::ShipwreckStart::Create, L"Shipwreck");

    MineShaftPieces::loadStatic();
    VillagePieces::loadStatic();
    NetherBridgePieces::loadStatic();
    StrongholdPieces::loadStatic();
    ScatteredFeaturePieces::loadStatic();
    OceanMonumentPieces::loadStatic();
    EndCityPieces::loadStatic();
    WoodlandMansionPieces::loadStatic();
}

void StructureFeatureIO::setStartId(EStructureStart id, StructureStartFactory factory, const std::wstring& name)
{
    startFactories[name] = factory;
    startIds[id] = name;
}

void StructureFeatureIO::setPieceId(EStructurePiece id, StructurePieceFactory factory, const std::wstring& name)
{
    pieceFactories[name] = factory;
    pieceIds[id] = name;
}

int StructureFeatureIO::getVersion()
{
    return version;
}

std::wstring StructureFeatureIO::getEncodeId(StructureStart* start)
{
    boost::unordered_map<unsigned int, std::wstring>::iterator entry = startIds.find(start->GetType());
    if (entry == startIds.end())
    {
        return L"";
    }

    return entry->second;
}

std::wstring StructureFeatureIO::getEncodeId(StructurePiece* piece)
{
    boost::unordered_map<unsigned int, std::wstring>::iterator entry = pieceIds.find(piece->GetType());
    if (entry == pieceIds.end())
    {
        return L"";
    }

    return entry->second;
}

StructureStart* StructureFeatureIO::loadStaticStart(ByteArrayTag* tag, Level* level)
{
    StructureStart* start = NULL;

    ByteArrayInputStream buffer(tag->getAsByteArray());
    DataInputStream input(&buffer);

    version = input.readInt();
    std::wstring id = input.readUTF();

    boost::unordered_map<std::wstring, StructureStartFactory>::iterator entry = startFactories.find(id);
    if (entry != startFactories.end())
    {
        start = (entry->second)();
        if (start != NULL)
        {
            if (version == 0)
            {
                EStructureStart type = start->GetType();
                if (type == eStructureStart_Village || type == eStructureStart_ScatteredFeature)
                {
                    delete start;
                    return NULL;
                }
            }

            start->load(level, &input);
        }
    }

    return start;
}

StructurePiece* StructureFeatureIO::loadStaticPiece(DataInputStream* input, Level* level)
{
    StructurePiece* piece = NULL;

    std::wstring id = input->readUTF();

    boost::unordered_map<std::wstring, StructurePieceFactory>::iterator entry = pieceFactories.find(id);
    if (entry != pieceFactories.end())
    {
        piece = (entry->second)();
        if (piece != NULL)
        {
            piece->load(level, input);
        }
    }

    return piece;
}
