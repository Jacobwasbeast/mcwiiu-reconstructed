#include "game/core/DefaultPresets.hpp"

#include "game/core/SuperflatConfig.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/world/generation/FlatLayerInfo.hpp"

void DefaultPresets::makePreset(SuperflatConfig* config, Preset preset)
{
    if (config == NULL)
    {
        return;
    }
    FlatGeneratorInfo info(config);
    switch (preset)
    {
    case CLASSIC:
        info.setDisplayname(0xB9E6DF02);
        info.setBiome(1);
        info.setVillage(true);
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock));
        info.addLayer(FlatLayerInfo(2, Blocks::dirt));
        info.addLayer(FlatLayerInfo(1, Blocks::grass));
        break;
    case TUNNELERS_DREAM:
        info.setDisplayname(0x1D3AAD7A);
        info.setBiome(1);
        info.setDungeons(true); info.setDecorations(true); info.setStronghold(true); info.setMineshaft(true);
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock));
        info.addLayer(FlatLayerInfo(5, Blocks::dirt));
        info.addLayer(FlatLayerInfo(100, Blocks::stone));
        info.addLayer(FlatLayerInfo(1, Blocks::grass));
        break;
    case WATER_WORLD:
        info.setDisplayname(0x081BD646);
        info.setBiome(0);
        info.addLayer(FlatLayerInfo(90, Blocks::sand));
        info.addLayer(FlatLayerInfo(5, Blocks::sandstone));
        info.addLayer(FlatLayerInfo(5, Blocks::dirt));
        info.addLayer(FlatLayerInfo(5, Blocks::stone));
        info.addLayer(FlatLayerInfo(1, Blocks::grass));
        break;
    case OVERWORLD:
        info.setDisplayname(0x50DF66FA); info.setBiome(1);
        info.setVillage(true); info.setDecorations(true); info.setStronghold(true); info.setMineshaft(true);
        info.setDungeons(true); info.setLakes(true); info.setLavaLakes(true);
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock)); info.addLayer(FlatLayerInfo(3, Blocks::dirt));
        info.addLayer(FlatLayerInfo(59, Blocks::stone)); info.addLayer(FlatLayerInfo(1, Blocks::grass));
        break;
    case SNOWY_KINGDOM:
        info.setDisplayname(0xF1825B98); info.setBiome(12); info.setVillage(true); info.setBiomeSpecific(true);
        info.addLayer(FlatLayerInfo(1, Blocks::snow)); info.addLayer(FlatLayerInfo(1, Blocks::bedrock));
        info.addLayer(FlatLayerInfo(3, Blocks::dirt)); info.addLayer(FlatLayerInfo(59, Blocks::stone));
        info.addLayer(FlatLayerInfo(1, Blocks::grass));
        break;
    case BOTTOMLESS_PIT:
        info.setDisplayname(0x6FD9641A); info.setBiome(1);
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock)); info.addLayer(FlatLayerInfo(3, Blocks::dirt));
        info.addLayer(FlatLayerInfo(2, Blocks::gravel));
        break;
    case DESERT:
        info.setDisplayname(0xC671897F); info.setBiome(2); info.setVillage(true); info.setBiomeSpecific(true);
        info.setDecorations(true); info.setStronghold(true); info.setMineshaft(true);
        info.addLayer(FlatLayerInfo(8, Blocks::sandstone)); info.addLayer(FlatLayerInfo(52, Blocks::sand));
        info.addLayer(FlatLayerInfo(3, Blocks::stone)); info.addLayer(FlatLayerInfo(1, Blocks::grass));
        break;
    case REDSTONE_READY:
        info.setDisplayname(0xFDFA242A); info.setBiome(2);
        info.addLayer(FlatLayerInfo(52, Blocks::sand)); info.addLayer(FlatLayerInfo(3, Blocks::stone));
        info.addLayer(FlatLayerInfo(1, Blocks::grass));
        break;
    case THE_VOID:
        break;
    case DEBUG_JUNGLE:
        info.setDisplayname(L"Debug Preset: Jungle"); info.setBiome(21); info.setBiomeSpecific(true); info.setDecorations(true);
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock)); info.addLayer(FlatLayerInfo(4, Blocks::dirt));
        info.addLayer(FlatLayerInfo(50, Blocks::stone)); info.addLayer(FlatLayerInfo(1, Blocks::grass)); break;
    case DEBUG_WOODLAND_MANSION:
        info.setDisplayname(L"Debug Preset: Woodland Mansion"); info.setBiome(4); info.setDecorations(true); info.setWoodlandMansion(32, 15);
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock)); info.addLayer(FlatLayerInfo(4, Blocks::dirt));
        info.addLayer(FlatLayerInfo(50, Blocks::stone)); info.addLayer(FlatLayerInfo(1, Blocks::grass)); break;
    case DEBUG_OCEAN_MONUMENT:
        info.setDisplayname(L"Debug Preset: Ocean Monument"); info.setBiome(0); info.setOceanMonument(32, 15);
        info.addLayer(FlatLayerInfo(90, Blocks::sand)); info.addLayer(FlatLayerInfo(5, Blocks::sandstone));
        info.addLayer(FlatLayerInfo(5, Blocks::dirt)); info.addLayer(FlatLayerInfo(5, Blocks::stone)); info.addLayer(FlatLayerInfo(1, Blocks::grass)); break;
    case DEBUG_WARM_OCEAN:
        info.setDisplayname(L"Debug Preset: Warm Ocean"); info.setBiome(44); info.setDecorations(true); info.setShipwrecks(true);
        info.addLayer(FlatLayerInfo(20, Blocks::sand)); info.addLayer(FlatLayerInfo(5, Blocks::sandstone));
        info.addLayer(FlatLayerInfo(5, Blocks::dirt)); info.addLayer(FlatLayerInfo(40, Blocks::stone)); info.addLayer(FlatLayerInfo(1, Blocks::grass)); break;
    }
}
