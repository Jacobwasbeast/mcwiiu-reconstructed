#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/core/L10N.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/FlatLayerInfo.hpp"

FlatGeneratorInfo::FlatGeneratorInfo() : mOwnsConfig(false), mConfig(0), mOwnedConfig() {}

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

CaveFeature* FlatGeneratorInfo::getCaves() { if (!hasCaves()) return 0; return 0; }
CanyonFeature* FlatGeneratorInfo::getCanyons() { if (!hasCanyons()) return 0; return 0; }
VillageFeature* FlatGeneratorInfo::getVillage(int) { if (!hasVillage()) return 0; return 0; }
RandomScatteredLargeFeature* FlatGeneratorInfo::getBiomeSpecific() { if (!hasBiomeSpecific()) return 0; return 0; }
MineShaftFeature* FlatGeneratorInfo::getMineshaft() { if (!hasMineshaft()) return 0; return 0; }
StrongholdFeature* FlatGeneratorInfo::getStronghold() { if (!hasStronghold()) return 0; return 0; }
OceanMonumentFeature* FlatGeneratorInfo::getOceanMonument() { if (!hasOceanMonument()) return 0; return 0; }
WoodlandMansionFeature* FlatGeneratorInfo::getWoodlandMansion(OverworldLevelSource*) { if (!hasWoodlandMansion()) return 0; return 0; }
ShipwreckFeature* FlatGeneratorInfo::getShipwreck(GenericOverworldLevelSource*, OceanMonumentFeature*) { if (!hasShipwrecks()) return 0; return 0; }
LakeFeature* FlatGeneratorInfo::getLakes() { if (!hasLakes()) return 0; return 0; }
LakeFeature* FlatGeneratorInfo::getLavaLakes() { if (!hasLavaLakes()) return 0; return 0; }

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
    int height = 0;
    for (std::vector<FlatLayerInfo>::const_iterator it = core()->layers.begin(); it != core()->layers.end(); ++it)
    {
        for (int i = 0; i < (int)it->getHeight() && height + i < 256; ++i)
            states[height + i] = it->getBlockState();
        height += it->getHeight();
    }
}

void FlatGeneratorInfo::getLayers(std::vector<FlatLayerInfo>& layers) const { layers = core()->layers; }
void FlatGeneratorInfo::setDisplayname(int displayname) { core()->setOption(0x10, 0, displayname); }
void FlatGeneratorInfo::setBiome(int biome) { core()->setOption(0x10, 0, biome); }

void FlatGeneratorInfo::setVillage(bool enabled) { core()->removeParam(0x20); if (enabled) core()->addParam(0x20); }
void FlatGeneratorInfo::setDungeons(bool enabled) { core()->removeParam(0x31); if (enabled) core()->addParam(0x31); }
void FlatGeneratorInfo::setDecorations(bool enabled) { core()->removeParam(0x30); if (enabled) core()->addParam(0x30); }
void FlatGeneratorInfo::setStronghold(bool enabled) { core()->removeParam(0x22); if (enabled) core()->addParam(0x22); }
void FlatGeneratorInfo::setMineshaft(bool enabled) { core()->removeParam(0x23); if (enabled) core()->addParam(0x23); }
void FlatGeneratorInfo::setLakes(bool enabled) { core()->removeParam(0x32); if (enabled) core()->addParam(0x32); }
void FlatGeneratorInfo::setLavaLakes(bool enabled) { core()->removeParam(0x33); if (enabled) core()->addParam(0x33); }
void FlatGeneratorInfo::setBiomeSpecific(bool enabled) { core()->removeParam(0x21); if (enabled) core()->addParam(0x21); }
void FlatGeneratorInfo::setWoodlandMansion(bool enabled) { core()->removeParam(0x27); if (enabled) core()->addParam(0x27); }
void FlatGeneratorInfo::setOceanMonument(bool enabled) { core()->removeParam(0x24); if (enabled) core()->addParam(0x24); }
void FlatGeneratorInfo::setShipwrecks(bool enabled) { core()->removeParam(0x28); if (enabled) core()->addParam(0x28); }
void FlatGeneratorInfo::setCaves(bool enabled) { core()->removeParam(0x25); if (enabled) core()->addParam(0x25); }
void FlatGeneratorInfo::setCanyons(bool enabled) { core()->removeParam(0x26); if (enabled) core()->addParam(0x26); }

void FlatGeneratorInfo::insertLayer(int index, const FlatLayerInfo& layer) { core()->layers.insert(core()->layers.begin() + index, layer); core()->updateLayers(); }
void FlatGeneratorInfo::addLayer(const FlatLayerInfo& layer) { core()->layers.push_back(layer); core()->updateLayers(); }
void FlatGeneratorInfo::setDisplayname(const std::wstring& displayname) { core()->setDisplayname(displayname); }
void FlatGeneratorInfo::setWoodlandMansion(int spacing, int separation) { setWoodlandMansion(true); core()->setOption(0x27, 0, spacing); core()->setOption(0x27, 1, separation); }
void FlatGeneratorInfo::setOceanMonument(int spacing, int separation) { setOceanMonument(true); core()->setOption(0x24, 0, spacing); core()->setOption(0x24, 1, separation); }
void FlatGeneratorInfo::setShipwrecks(int spacing, int separation) { setShipwrecks(true); core()->setOption(0x28, 0, spacing); core()->setOption(0x28, 1, separation); }
void FlatGeneratorInfo::setBiome() { core()->setOption(0x10, 0, getBiome()); }
void FlatGeneratorInfo::setVillage(int size, int distance) { setVillage(true); core()->setOption(0x20, 0, size); core()->setOption(0x20, 1, distance); }
void FlatGeneratorInfo::setBiomeSpecific(int spacing) { setBiomeSpecific(true); core()->setOption(0x21, 0, spacing); }
void FlatGeneratorInfo::setMineshaft(int chance) { setMineshaft(true); core()->setOption(0x23, 0, chance); }
void FlatGeneratorInfo::setStronghold(int count, int distance) { setStronghold(true); core()->setOption(0x22, 0, distance); core()->setOption(0x22, 1, count); }

void FlatGeneratorInfo::setEqualTo(SuperflatConfig* config) { *core() = *config; }
int FlatGeneratorInfo::numLayers() const { return (int)core()->layers.size(); }
void FlatGeneratorInfo::setLayer(int index, const FlatLayerInfo& layer) { core()->layers[index] = layer; core()->updateLayers(); }
void FlatGeneratorInfo::setLayers(const std::vector<FlatLayerInfo>& layers) { core()->layers = layers; core()->updateLayers(); }
void FlatGeneratorInfo::remLayer(int index) { core()->layers.erase(core()->layers.begin() + index); core()->updateLayers(); }

void FlatGeneratorInfo::getLayers(int* ids, int* data) const
{
    for (int i = 0; i < numLayers(); ++i) { ids[i] = core()->layers[i].getId(); data[i] = core()->layers[i].getData(); }
}

int FlatGeneratorInfo::getHeight() const { int height = 0; for (std::vector<FlatLayerInfo>::const_iterator it = core()->layers.begin(); it != core()->layers.end(); ++it) height += it->getHeight(); return height; }
int FlatGeneratorInfo::getTopLayer() const { return numLayers() - 1; }

int FlatGeneratorInfo::findTopNonAirLayer() const
{
    int layer = getTopLayer();
    while (layer >= 0 && core()->layers[layer].getId() == 0) layer -= core()->layers[layer].getHeight();
    return layer;
}

int FlatGeneratorInfo::findTopSolidLayer() const { return findTopNonAirLayer(); }
int FlatGeneratorInfo::getBiome() { return core()->maybeOption(0x10, 0, 0); }
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
    std::wstring description = getDisplayname();
    description += L"\n";
    for (std::vector<FlatLayerInfo>::const_iterator it = core()->layers.begin(); it != core()->layers.end(); ++it)
    {
        description += it->getHeightText();
        description += L" ";
        description += it->getNameText();
        description += L"\n";
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

FlatGeneratorInfo FlatGeneratorInfo::getDefault() { return FlatGeneratorInfo((new SuperflatConfig())->makeDefaults(), true); }
void FlatGeneratorInfo::setEqualTo(const FlatGeneratorInfo& info) { *core() = *info.core(); }
std::wstring FlatGeneratorInfo::getDisplayname() { return core()->getDisplayname(); }
bool FlatGeneratorInfo::hasLayer(int index) const { return index >= 0 && index < numLayers(); }
FlatLayerInfo FlatGeneratorInfo::getLayerAtHeight(int height) const { return getLayer(getLayerIndexAtHeight(height)); }
void FlatGeneratorInfo::getSingleLayers(std::vector<FlatLayerInfo>& layers) const { layers = core()->layers; }
bool FlatGeneratorInfo::operator==(const FlatGeneratorInfo& info) const
{
    if (mOwnsConfig != info.mOwnsConfig || mConfig != info.mConfig || mOwnedConfig.get() != info.mOwnedConfig.get())
        return false;
    if (core()->layers.size() != info.core()->layers.size())
        return false;
    for (int i = 0; i < (int)core()->layers.size(); ++i)
        if (core()->layers[i].getHeight() != info.core()->layers[i].getHeight() || core()->layers[i].getId() != info.core()->layers[i].getId() || core()->layers[i].getData() != info.core()->layers[i].getData())
            return false;
    return true;
}
