#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/world/generation/FlatLayerInfo.hpp"

FlatGeneratorInfo::FlatGeneratorInfo() : ownsConfig(false), config(0), ownedConfig(), layers() {}
FlatGeneratorInfo::FlatGeneratorInfo(SuperflatConfig* value) : ownsConfig(false), config(value), ownedConfig(), layers() {}
FlatGeneratorInfo::FlatGeneratorInfo(SuperflatConfig* value, bool copy) : ownsConfig(copy), config(value), ownedConfig(), layers() {}
FlatGeneratorInfo::~FlatGeneratorInfo() {}
SuperflatConfig* FlatGeneratorInfo::core() const { return ownsConfig ? ownedConfig.get() : config; }

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
bool FlatGeneratorInfo::hasDungeons() { return core()->hasParam(0x1e); }
bool FlatGeneratorInfo::hasLakes() { return core()->hasParam(0x1c); }
bool FlatGeneratorInfo::hasLavaLakes() { return core()->hasParam(0x1d); }

CaveFeature* FlatGeneratorInfo::getCaves() { return hasCaves() ? new CaveFeature() : 0; }
CanyonFeature* FlatGeneratorInfo::getCanyons() { return hasCanyons() ? new CanyonFeature() : 0; }
VillageFeature* FlatGeneratorInfo::getVillage(int size) { return hasVillage() ? new VillageFeature(core(),size) : 0; }
RandomScatteredLargeFeature* FlatGeneratorInfo::getBiomeSpecific() { return hasBiomeSpecific() ? new RandomScatteredLargeFeature(core()) : 0; }
MineShaftFeature* FlatGeneratorInfo::getMineshaft() { return hasMineshaft() ? new MineShaftFeature(core()) : 0; }
StrongholdFeature* FlatGeneratorInfo::getStronghold() { return hasStronghold() ? new StrongholdFeature(core()) : 0; }
OceanMonumentFeature* FlatGeneratorInfo::getOceanMonument() { return hasOceanMonument() ? new OceanMonumentFeature(core()) : 0; }
ShipwreckFeature* FlatGeneratorInfo::getShipwreck(GenericOverworldLevelSource* source, OceanMonumentFeature* monument) { return hasShipwrecks() ? new ShipwreckFeature(source,monument,core()) : 0; }
LakeFeature* FlatGeneratorInfo::getLakes() { return hasLakes() ? new LakeFeature() : 0; }
LakeFeature* FlatGeneratorInfo::getLavaLakes() { return hasLavaLakes() ? new LakeFeature() : 0; }
WoodlandMansionFeature* FlatGeneratorInfo::getWoodlandMansion(OverworldLevelSource*) { return hasWoodlandMansion() ? new WoodlandMansionFeature() : 0; }

int FlatGeneratorInfo::getLayerIndexAtHeight(int height) const { int n=0; for (std::vector<FlatLayerInfo>::const_iterator i=layers.begin();i!=layers.end();++i,++n) if (height < i->getEnd()) return n; return -1; }
FlatLayerInfo FlatGeneratorInfo::getLayer(int index) const { return layers[index]; }
void FlatGeneratorInfo::getLayers(const BlockState* (&states)[256]) const
{
    int h=0;
    for (std::vector<FlatLayerInfo>::const_iterator i=layers.begin(); i!=layers.end(); ++i)
    {
        for (int j=0; j<i->getHeight() && h+j<256; ++j) states[h+j]=i->getBlockState();
        h+=i->getHeight();
    }
}
void FlatGeneratorInfo::getLayers(std::vector<FlatLayerInfo>& result) const { result=layers; }

void FlatGeneratorInfo::setDisplayname(int value) { core()->setOption(0x10,0,value); }
void FlatGeneratorInfo::setBiome(int value) { core()->setOption(0x10,0,value); }
void FlatGeneratorInfo::setVillage(bool v)
{
    core()->removeParam(0x20);
    if (v)
        core()->addParam(0x20);
}

void FlatGeneratorInfo::setDungeons(bool v)
{
    core()->removeParam(0x1e);
    if (v)
        core()->addParam(0x1e);
}

void FlatGeneratorInfo::setDecorations(bool v)
{
    core()->removeParam(0x30);
    if (v)
        core()->addParam(0x30);
}

void FlatGeneratorInfo::setStronghold(bool v)
{
    core()->removeParam(0x22);
    if (v)
        core()->addParam(0x22);
}

void FlatGeneratorInfo::setMineshaft(bool v)
{
    core()->removeParam(0x23);
    if (v)
        core()->addParam(0x23);
}

void FlatGeneratorInfo::setLakes(bool v)
{
    core()->removeParam(0x1c);
    if (v)
        core()->addParam(0x1c);
}

void FlatGeneratorInfo::setLavaLakes(bool v)
{
    core()->removeParam(0x1d);
    if (v)
        core()->addParam(0x1d);
}

void FlatGeneratorInfo::setBiomeSpecific(bool v)
{
    core()->removeParam(0x21);
    if (v)
        core()->addParam(0x21);
}

void FlatGeneratorInfo::setWoodlandMansion(bool v)
{
    core()->removeParam(0x27);
    if (v)
        core()->addParam(0x27);
}

void FlatGeneratorInfo::setOceanMonument(bool v)
{
    core()->removeParam(0x24);
    if (v)
        core()->addParam(0x24);
}

void FlatGeneratorInfo::setShipwrecks(bool v)
{
    core()->removeParam(0x28);
    if (v)
        core()->addParam(0x28);
}

void FlatGeneratorInfo::setCaves(bool v)
{
    core()->removeParam(0x25);
    if (v)
        core()->addParam(0x25);
}

void FlatGeneratorInfo::setCanyons(bool v)
{
    core()->removeParam(0x26);
    if (v)
        core()->addParam(0x26);
}
void FlatGeneratorInfo::insertLayer(int index,const FlatLayerInfo& layer) { layers.insert(layers.begin()+index,layer); core()->updateLayers(); }
void FlatGeneratorInfo::addLayer(const FlatLayerInfo& layer) { layers.push_back(layer); core()->updateLayers(); }
void FlatGeneratorInfo::setDisplayname(const std::wstring& value) { core()->setDisplayname(value); }
void FlatGeneratorInfo::setWoodlandMansion(int a,int b) { setWoodlandMansion(true); core()->setOption(0x27,0,a); core()->setOption(0x27,1,b); }
void FlatGeneratorInfo::setOceanMonument(int a,int b) { setOceanMonument(true); core()->setOption(0x24,0,a); core()->setOption(0x24,1,b); }
void FlatGeneratorInfo::setBiome() { core()->setOption(0x10,0,0); }
void FlatGeneratorInfo::setVillage(int a,int b) { setVillage(true); core()->setOption(0x20,0,a); core()->setOption(0x20,1,b); }
void FlatGeneratorInfo::setBiomeSpecific(int a) { setBiomeSpecific(true); core()->setOption(0x21,0,a); }
void FlatGeneratorInfo::setMineshaft(int a) { setMineshaft(true); core()->setOption(0x23,0,a); }
void FlatGeneratorInfo::setStronghold(int a,int b) { setStronghold(true); core()->setOption(0x22,0,a); core()->setOption(0x22,1,b); }
void FlatGeneratorInfo::setShipwrecks(int a,int b) { setShipwrecks(true); core()->setOption(0x28,0,a); core()->setOption(0x28,1,b); }
void FlatGeneratorInfo::setEqualTo(SuperflatConfig* value) { config=value; ownsConfig=false; ownedConfig.reset(); }
int FlatGeneratorInfo::numLayers() const { return (int)layers.size(); }
void FlatGeneratorInfo::setLayer(int index,const FlatLayerInfo& layer) { if(index>=0 && index<(int)layers.size()) layers[index]=layer; core()->updateLayers(); }
void FlatGeneratorInfo::setLayers(const std::vector<FlatLayerInfo>& value) { layers=value; core()->updateLayers(); }
void FlatGeneratorInfo::remLayer(int index) { if(index>=0 && index<(int)layers.size()) layers.erase(layers.begin()+index); core()->updateLayers(); }
void FlatGeneratorInfo::getLayers(int* ids,int* data) const { int n=0; for(std::vector<FlatLayerInfo>::const_iterator i=layers.begin();i!=layers.end();++i) { ids[n]=i->getId(); data[n++]=i->getData(); } }
int FlatGeneratorInfo::getHeight() const { int h=0; for(std::vector<FlatLayerInfo>::const_iterator i=layers.begin();i!=layers.end();++i) h+=i->getHeight(); return h; }
int FlatGeneratorInfo::getTopLayer() const { return numLayers()-1; }
int FlatGeneratorInfo::findTopNonAirLayer() const { for(int i=getTopLayer();i>=0;--i) if(layers[i].getId()!=0) return i; return -1; }
int FlatGeneratorInfo::findTopSolidLayer() const { return findTopNonAirLayer(); }
int FlatGeneratorInfo::getBiome()
{
    return core()->maybeOption(0x10, 0, 0);
}

int FlatGeneratorInfo::getVillageSize()
{
    return core()->maybeOption(0x20, 0, 1);
}

int FlatGeneratorInfo::getVillageDistance()
{
    return core()->maybeOption(0x20, 1, 0x20);
}

int FlatGeneratorInfo::getMineshaftChance()
{
    return core()->maybeOption(0x23, 0, 1);
}

int FlatGeneratorInfo::getStrongholdCount()
{
    return core()->maybeOption(0x22, 1, 1);
}

int FlatGeneratorInfo::getStrongholdDistance()
{
    return core()->maybeOption(0x22, 0, 0x20);
}

int FlatGeneratorInfo::getBiomeSpecificSpacing()
{
    return core()->maybeOption(0x21, 0, 0x20);
}

int FlatGeneratorInfo::getOceanMonumentSpacing()
{
    return core()->maybeOption(0x24, 0, 0x20);
}

int FlatGeneratorInfo::getOceanMonuemntSeparation()
{
    return core()->maybeOption(0x24, 1, 5);
}

int FlatGeneratorInfo::getShipwreckSpacing()
{
    return core()->maybeOption(0x28, 0, 0x20);
}

int FlatGeneratorInfo::getShipwreckSeparation()
{
    return core()->maybeOption(0x28, 1, 5);
}

int FlatGeneratorInfo::getWoodlandMansionSpacing()
{
    return core()->maybeOption(0x27, 0, 0x20);
}

int FlatGeneratorInfo::getWoodlandMansionSeparation()
{
    return core()->maybeOption(0x27, 1, 5);
}
std::wstring FlatGeneratorInfo::generateDescription() { return std::wstring(); }
void FlatGeneratorInfo::reduceLayers()
{
    for (std::size_t i=1; i<layers.size();)
    {
        if (layers[i-1].getId()==layers[i].getId() && layers[i-1].getData()==layers[i].getData())
        {
            layers[i-1].setHeight(layers[i-1].getHeight()+layers[i].getHeight());
            layers.erase(layers.begin()+i);
        }
        else ++i;
    }
    core()->updateLayers();
}
FlatGeneratorInfo FlatGeneratorInfo::getDefault()
{
    SuperflatConfig* defaults = new SuperflatConfig();
    defaults->makeDefaults();
    return FlatGeneratorInfo(defaults, true);
}
void FlatGeneratorInfo::setEqualTo(const FlatGeneratorInfo& value) { ownsConfig=value.ownsConfig; config=value.config; ownedConfig=value.ownedConfig; layers=value.layers; }
std::wstring FlatGeneratorInfo::getDisplayname() { return core()->getDisplayname(); }
bool FlatGeneratorInfo::hasLayer(int index) const { return index>=0 && index<(int)layers.size(); }
FlatLayerInfo FlatGeneratorInfo::getLayerAtHeight(int height) const { return getLayer(getLayerIndexAtHeight(height)); }
void FlatGeneratorInfo::getSingleLayers(std::vector<FlatLayerInfo>& result) const { result=layers; }
bool FlatGeneratorInfo::operator==(const FlatGeneratorInfo& value) const
{
    if (layers.size() != value.layers.size())
        return false;
    std::vector<FlatLayerInfo>::const_iterator left = layers.begin();
    std::vector<FlatLayerInfo>::const_iterator right = value.layers.begin();
    while (left != layers.end())
    {
        if (left->getHeight() != right->getHeight() || left->getId() != right->getId() || left->getData() != right->getData())
            return false;
        ++left;
        ++right;
    }
    return ownsConfig == value.ownsConfig && config == value.config;
}
