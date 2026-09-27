#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/L10N.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/CanyonFeature.hpp"
#include "game/world/generation/FlatLayerInfo.hpp"
#include "game/world/generation/LakeFeature.hpp"
#include "game/world/generation/LargeCaveFeature.hpp"
#include "game/world/generation/MineShaftFeature.hpp"
#include "game/world/generation/OceanMonumentFeature.hpp"
#include "game/world/generation/RandomScatteredLargeFeature.hpp"
#include "game/world/generation/ShipwreckFeature.hpp"
#include "game/world/generation/StrongholdFeature.hpp"
#include "game/world/generation/VillageFeature.hpp"
#include "game/core/StringFormat.hpp"

long long getSpacingAndSeparationFromPercentage(int percentage, int spacing, int maximum)
{
    int adjustedPercentage = percentage < 100 ? percentage : 100;
    if (adjustedPercentage < 16)
        adjustedPercentage = 16;

    int separation = (spacing * 5) / 100;
    if (separation < maximum)
        separation = maximum;

    return (static_cast<long long>(adjustedPercentage) << 32) |
           static_cast<unsigned int>(separation);
}

FlatGeneratorInfo::FlatGeneratorInfo(SuperflatConfig* config) : mOwnsConfig(false), mConfig(config), mOwnedConfig() {}

FlatGeneratorInfo::FlatGeneratorInfo(SuperflatConfig* config, bool copy) : mOwnsConfig(copy), mConfig(0), mOwnedConfig()
{
    if (copy)
        mOwnedConfig.reset(config);
    else
        mConfig = config;
}

FlatGeneratorInfo::~FlatGeneratorInfo() {}

SuperflatConfig* FlatGeneratorInfo::core() const
{
    return mOwnsConfig ? mOwnedConfig.get() : mConfig;
}

bool FlatGeneratorInfo::hasCaves() { return core()->hasParam(0x25); }
bool FlatGeneratorInfo::hasCanyons() { return core()->hasParam(0x26); }
bool FlatGeneratorInfo::hasVillage() { return core()->hasParam(0x20); }
bool FlatGeneratorInfo::hasBiomeSpecific() { return core()->hasParam(0x21); }
bool FlatGeneratorInfo::hasMineshaft() { return core()->hasParam(0x23); }
bool FlatGeneratorInfo::hasStronghold() { return core()->hasParam(0x22); }
bool FlatGeneratorInfo::hasOceanMonument() { return core()->hasParam(0x24); }
bool FlatGeneratorInfo::hasWoodlandMansion() { return core()->hasParam(0x27); }
bool FlatGeneratorInfo::hasShipwrecks() { return core()->hasParam(0x28); }
bool FlatGeneratorInfo::hasDecorations() { return core()->hasParam(0x30); }
bool FlatGeneratorInfo::hasDungeons() { return core()->hasParam(0x31); }
bool FlatGeneratorInfo::hasLakes() { return core()->hasParam(0x32); }
bool FlatGeneratorInfo::hasLavaLakes() { return core()->hasParam(0x33); }

LargeCaveFeature* FlatGeneratorInfo::getCaves() { return hasCaves() ? new LargeCaveFeature() : 0; }
CanyonFeature* FlatGeneratorInfo::getCanyons() { return hasCanyons() ? new CanyonFeature() : 0; }
VillageFeature* FlatGeneratorInfo::getVillage(int size) { return hasVillage() ? new VillageFeature(core(), size) : 0; }
RandomScatteredLargeFeature* FlatGeneratorInfo::getBiomeSpecific() { return hasBiomeSpecific() ? new RandomScatteredLargeFeature(core()) : 0; }
MineShaftFeature* FlatGeneratorInfo::getMineshaft() { return hasMineshaft() ? new MineShaftFeature(core()) : 0; }
StrongholdFeature* FlatGeneratorInfo::getStronghold() { return hasStronghold() ? new StrongholdFeature(core()) : 0; }
OceanMonumentFeature* FlatGeneratorInfo::getOceanMonument() { return hasOceanMonument() ? new OceanMonumentFeature(core()) : 0; }
ShipwreckFeature* FlatGeneratorInfo::getShipwreck(GenericOverworldLevelSource* source, OceanMonumentFeature* monument) { return hasShipwrecks() ? new ShipwreckFeature(source, monument, core()) : 0; }
LakeFeature* FlatGeneratorInfo::getLakes() { return hasLakes() ? new LakeFeature(Blocks::water) : 0; }
LakeFeature* FlatGeneratorInfo::getLavaLakes() { return hasLavaLakes() ? new LakeFeature(Blocks::lava) : 0; }

int FlatGeneratorInfo::getLayerIndexAtHeight(int height) const
{
    const std::vector<FlatLayerInfo>& layers = core()->layers;
    for (int i = 0; i < (int)layers.size(); ++i)
        if (layers[i].getStart() <= (unsigned int)height && height <= layers[i].getEnd())
            return i;
    return -1;
}

FlatLayerInfo FlatGeneratorInfo::getLayer(int index) const { return core()->layers[index]; }

void FlatGeneratorInfo::getLayers(const BlockState* (&states)[256]) const
{
    for (int height = 0; height < 256; height += 4)
    {
        for (int offset = 0; offset < 4 && height + offset < 256; ++offset)
        {
            int index = getLayerIndexAtHeight(height + offset);
            states[height + offset] = index < 0 ? Blocks::air->defaultBlockState() : getLayer(index).getBlockState();
        }
    }
}

void FlatGeneratorInfo::getLayers(std::vector<FlatLayerInfo>& layers) const { layers = core()->layers; }
void FlatGeneratorInfo::setDisplayname(int displayname) { core()->setDisplayname(std::wstring(L10N::GetString(displayname))); }
void FlatGeneratorInfo::setBiome(int biome) { core()->setOption(0x10, 0, biome); }

void FlatGeneratorInfo::setVillage(bool enabled) { core()->removeParam(0x20); if (enabled) core()->addParam(0x20); }
void FlatGeneratorInfo::setDungeons(bool enabled) { if (enabled) core()->addParam(0x31); else core()->removeParam(0x31); }
void FlatGeneratorInfo::setDecorations(bool enabled) { if (enabled) core()->addParam(0x30); else core()->removeParam(0x30); }
void FlatGeneratorInfo::setStronghold(bool enabled) { core()->removeParam(0x22); if (enabled) core()->addParam(0x22); }
void FlatGeneratorInfo::setMineshaft(bool enabled) { core()->removeParam(0x23); if (enabled) core()->addParam(0x23); }
void FlatGeneratorInfo::setLakes(bool enabled) { if (enabled) core()->addParam(0x32); else core()->removeParam(0x32); }
void FlatGeneratorInfo::setLavaLakes(bool enabled) { if (enabled) core()->addParam(0x33); else core()->removeParam(0x33); }
void FlatGeneratorInfo::setBiomeSpecific(bool enabled) { core()->removeParam(0x21); if (enabled) core()->addParam(0x21); }
void FlatGeneratorInfo::setWoodlandMansion(bool enabled) { core()->removeParam(0x27); if (enabled) core()->addParam(0x27); }
void FlatGeneratorInfo::setOceanMonument(bool enabled) { core()->removeParam(0x24); if (enabled) core()->addParam(0x24); }
void FlatGeneratorInfo::setShipwrecks(bool enabled) { core()->removeParam(0x28); if (enabled) core()->addParam(0x28); }
void FlatGeneratorInfo::setCaves(bool enabled) { if (enabled) core()->addParam(0x25); else core()->removeParam(0x25); }
void FlatGeneratorInfo::setCanyons(bool enabled) { if (enabled) core()->addParam(0x26); else core()->removeParam(0x26); }

void FlatGeneratorInfo::insertLayer(int index, const FlatLayerInfo& layer) { core()->layers.insert(core()->layers.begin() + index, layer); core()->updateLayers(); }
void FlatGeneratorInfo::addLayer(const FlatLayerInfo& layer) { core()->layers.insert(core()->layers.begin(), layer); core()->updateLayers(); }
void FlatGeneratorInfo::setDisplayname(const std::wstring& displayname) { core()->setDisplayname(displayname); }
void FlatGeneratorInfo::setWoodlandMansion(int spacing, int separation) { setWoodlandMansion(true); core()->setOption(0x27, 0, spacing); core()->setOption(0x27, 1, separation); }
void FlatGeneratorInfo::setOceanMonument(int spacing, int separation) { setOceanMonument(true); core()->setOption(0x24, 0, spacing); core()->setOption(0x24, 1, separation); }
void FlatGeneratorInfo::setShipwrecks(int spacing, int separation) { setShipwrecks(true); core()->setOption(0x28, 0, spacing); core()->setOption(0x28, 1, separation); }
void FlatGeneratorInfo::setBiome() { setBiome(Biome::DEFAULT->id); }
void FlatGeneratorInfo::setVillage(int size, int distance) { setVillage(true); core()->setOption(0x20, 0, size); core()->setOption(0x20, 1, distance); }
void FlatGeneratorInfo::setBiomeSpecific(int spacing) { setBiomeSpecific(true); core()->setOption(0x21, 0, spacing); }
void FlatGeneratorInfo::setMineshaft(int chance) { setMineshaft(true); core()->setOption(0x23, 0, chance); }
void FlatGeneratorInfo::setStronghold(int distance, int count) { setStronghold(true); core()->setOption(0x22, 0, distance); core()->setOption(0x22, 1, count); }

void FlatGeneratorInfo::setEqualTo(SuperflatConfig* config) { *core() = *config; }
int FlatGeneratorInfo::numLayers() const { return (int)core()->layers.size(); }
void FlatGeneratorInfo::setLayer(int index, const FlatLayerInfo& layer) { core()->layers[index] = layer; core()->updateLayers(); }
void FlatGeneratorInfo::setLayers(const std::vector<FlatLayerInfo>& layers) { core()->layers = layers; core()->updateLayers(); }
void FlatGeneratorInfo::remLayer(int index) { core()->layers.erase(core()->layers.begin() + index); core()->updateLayers(); }

void FlatGeneratorInfo::getLayers(int* ids, int* data) const
{
    for (int height = 0; height < 256; height += 4)
    {
        for (int offset = 0; offset < 4 && height + offset < 256; ++offset)
        {
            int index = getLayerIndexAtHeight(height + offset);
            if (index < 0)
            {
                ids[height + offset] = 0;
                data[height + offset] = 0;
            }
            else
            {
                FlatLayerInfo layer = getLayer(index);
                ids[height + offset] = layer.getId();
                data[height + offset] = layer.getData();
            }
        }
    }
}

int FlatGeneratorInfo::getHeight() const { int height = 0; for (std::vector<FlatLayerInfo>::const_iterator it = core()->layers.begin(); it != core()->layers.end(); ++it) height += it->getHeight(); return height; }
int FlatGeneratorInfo::getTopLayer() const { return numLayers() - 1; }

int FlatGeneratorInfo::findTopNonAirLayer() const
{
    int layer = getTopLayer();
    while (layer >= 0 && core()->layers[layer].getId() == 0) layer -= core()->layers[layer].getHeight();
    return layer;
}

int FlatGeneratorInfo::findTopSolidLayer() const
{
    int layer = findTopNonAirLayer();
    while (layer >= 0 && !getLayer(layer).getBlockState()->isSolid())
        layer -= getLayer(layer).getHeight();
    return layer;
}
int FlatGeneratorInfo::getBiome() { return core()->maybeOption(0x10, 0, Biome::DEFAULT->id); }
int FlatGeneratorInfo::getVillageSize() { return core()->maybeOption(0x20, 0, 1); }
int FlatGeneratorInfo::getVillageDistance() { return core()->maybeOption(0x20, 1, 0x20); }
int FlatGeneratorInfo::getMineshaftChance() { return core()->maybeOption(0x23, 0, 1); }
int FlatGeneratorInfo::getStrongholdCount() { return core()->maybeOption(0x22, 1, 1); }
int FlatGeneratorInfo::getStrongholdDistance() { return core()->maybeOption(0x22, 0, 0x20); }
int FlatGeneratorInfo::getBiomeSpecificSpacing() { return core()->maybeOption(0x21, 0, 0x20); }
int FlatGeneratorInfo::getOceanMonumentSpacing() { return core()->maybeOption(0x24, 0, 0x20); }
int FlatGeneratorInfo::getOceanMonuemntSeparation() { return core()->maybeOption(0x24, 1, 5); }
int FlatGeneratorInfo::getShipwreckSpacing() { return core()->maybeOption(0x28, 0, 0x20); }
int FlatGeneratorInfo::getShipwreckSeparation() { return core()->maybeOption(0x28, 1, 5); }
int FlatGeneratorInfo::getWoodlandMansionSpacing() { return core()->maybeOption(0x27, 0, 0x20); }
int FlatGeneratorInfo::getWoodlandMansionSeparation() { return core()->maybeOption(0x27, 1, 5); }

std::wstring FlatGeneratorInfo::generateDescription()
{
    std::wstring biomeName = Biome::getBiomes()[getBiome()]->getName(false);
    std::wstring description = formatwstr(L"%ls: %ls<br /><br />", L10N::GetString(IDS_BIOME), biomeName.c_str());
    description += formatwstr(L"%ls:<br />", L10N::GetString(IDS_LAYERS));
    for (int i = numLayers() - 1; i >= 0; --i)
    {
        FlatLayerInfo layer = getLayer(i);
        description += formatwstr(L"- %ls %ls<br />", layer.getHeightText().c_str(), layer.getNameText().c_str());
    }
    std::wstring features;
    if (hasVillage()) features += formatwstr(L"- %ls<br />", L10N::GetString(0x1a0175a8));
    if (hasCaves()) features += formatwstr(L"- %ls<br />", L10N::GetString(0x7d539616));
    if (hasCanyons()) features += formatwstr(L"- %ls<br />", L10N::GetString(0x6339594b));
    if (hasMineshaft()) features += formatwstr(L"- %ls<br />", L10N::GetString(0xb502a2bb));
    if (hasStronghold()) features += formatwstr(L"- %ls<br />", L10N::GetString(0x98925ce0));
    if (hasBiomeSpecific()) features += formatwstr(L"- %ls<br />", L10N::GetString(0x92176858));
    if (hasDungeons()) features += formatwstr(L"- %ls<br />", L10N::GetString(0x58eda210));
    if (hasDecorations()) features += formatwstr(L"- %ls<br />", L10N::GetString(0xf5ded1b6));
    if (hasLakes()) features += formatwstr(L"- %ls<br />", L10N::GetString(0xedaa5ce4));
    if (hasLavaLakes()) features += formatwstr(L"- %ls<br />", L10N::GetString(0xae1b1941));
    if (hasOceanMonument()) features += formatwstr(L"- %ls<br />", L10N::GetString(0xff686d36));
    if (hasWoodlandMansion()) features += formatwstr(L"- %ls<br />", L10N::GetString(0x9a333426));
    if (!features.empty())
    {
        description += formatwstr(L"<br />%ls:<br />", L10N::GetString(IDS_FEATURES));
        description += features;
    }
    return description;
}

void FlatGeneratorInfo::reduceLayers()
{
    for (int i = 1; i < numLayers();)
    {
        if (core()->layers[i - 1].getId() == core()->layers[i].getId() && core()->layers[i - 1].getData() == core()->layers[i].getData())
        {
            core()->layers[i - 1].setHeight(core()->layers[i - 1].getHeight() + core()->layers[i].getHeight());
            core()->layers.erase(core()->layers.begin() + i);
        }
        else
            ++i;
    }
    while (numLayers() > 1 && core()->layers.back().getId() == 0) core()->layers.pop_back();
    core()->updateLayers();
}
