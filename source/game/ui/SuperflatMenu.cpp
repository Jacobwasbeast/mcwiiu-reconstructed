#include "game/ui/SuperflatMenu.hpp"
#include "game/ui/UIControl_MultiList.hpp"
#include "game/core/L10N.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/FireBlock.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/world/generation/FlatLayerInfo.hpp"
#include <algorithm>
#include <cstdio>
#include <cwchar>

std::wstring formatwstr(const wchar_t* format, ...);
std::vector<Block*>* getPlacementRestrictedList();
long long getSpacingAndSeparationFromPercentage(int percentage, int spacing, int maximum);

SuperflatMenu::SliderLabelBehaviour::SliderLabelBehaviour(int name, unsigned int minimum, unsigned int maximum)
    : mName(name), mMinimum(minimum), mMaximum(maximum)
{
}

SuperflatMenu::CategoricalLabelBehaviour::CategoricalLabelBehaviour(int name, int* labels)
    : SliderLabelBehaviour(name, 0, getLength(labels)), mLabels(labels)
{
}

int SuperflatMenu::CategoricalLabelBehaviour::getLength(int* labels)
{
    int length = 0;
    if (labels != NULL)
    {
        while (labels[length] != -1)
        {
            ++length;
        }
    }
    return length;
}

std::wstring SuperflatMenu::CategoricalLabelBehaviour::GetLabel(int value)
{
    std::wstring result = L10N::GetString(mName);
    result += L" ";
    result += L10N::GetString(mLabels[value]);
    return result;
}

SuperflatMenu::NumericalLabelBehaviour::NumericalLabelBehaviour(int name, unsigned int minimum, unsigned int maximum)
    : SliderLabelBehaviour(name, minimum, maximum), mType(0), mScale(1.0), mPrecision(0), mFormatString(0), mLabel(0), mUnused(0)
{
}

SuperflatMenu::NumericalLabelBehaviour::NumericalLabelBehaviour(int name, const std::wstring& label, int type, unsigned int minimum, unsigned int maximum)
    : SliderLabelBehaviour(name, minimum, maximum), mType(type), mScale(1.0), mPrecision(0), mFormatString(0), mLabel(label.empty() ? 0 : 1), mUnused(0)
{
}

SuperflatMenu::NumericalLabelBehaviour::NumericalLabelBehaviour(int name, const std::wstring& label, double scale, unsigned int minimum, unsigned int maximum)
    : SliderLabelBehaviour(name, minimum, maximum), mType(0), mScale(scale), mPrecision(0), mFormatString(0), mLabel(label.empty() ? 0 : 1), mUnused(0)
{
}

std::wstring SuperflatMenu::NumericalLabelBehaviour::GetLabel(int value)
{
    std::wstring result;
    if (mType == 0)
    {
        result = formatwstr(L"%i", (int)((value - (int)mMinimum) * mScale));
    }
    else if (mType == 1)
    {
        result = formatwstr(L"%i%%", (int)((value - (int)mMinimum) * mScale));
    }
    else
    {
        result = L10N::GetString(mName);
    }
    return result;
}

SuperflatMenu::Param::Param(bool terminal, bool parent, int name, int desc, int minimum, int maximum, MappingFactory factory, SliderLabelBehaviour* behaviour)
    : mTerminal(terminal), mParent(parent), mName(name), mDesc(desc), mMinValue(minimum), mMaxValue(maximum), mFactory(factory), mBehaviour(behaviour)
{
}

std::wstring SuperflatMenu::Param::getLabel(int value) const
{
    return mBehaviour->GetLabel(value);
}

int SuperflatMenu::Param::getName() const { return mName; }
bool SuperflatMenu::Param::isTerminal() const { return mTerminal; }
bool SuperflatMenu::Param::isParent() const { return mParent; }
bool SuperflatMenu::Param::isChild() const { return !mTerminal && !mParent; }
int SuperflatMenu::Param::getDesc() const { return mDesc; }
int SuperflatMenu::Param::getMinValue() const { return mMinValue; }
int SuperflatMenu::Param::getMaxValue() const { return mMaxValue; }
SuperflatMenu::Param::MappingFactory SuperflatMenu::Param::getFunc() const { return mFactory; }

SuperflatMenu::Param SuperflatMenu::Param::Parent(MappingFactory factory, int name, int desc)
{
    return Param(false, true, name, desc, 0, 0, factory, NULL);
}

SuperflatMenu::Param SuperflatMenu::Param::Child(SliderLabelBehaviour* behaviour, int name)
{
    return Param(false, false, behaviour->mName, name, behaviour->mMinimum,
                 behaviour->mMaximum, NULL, behaviour);
}

SuperflatMenu::Param SuperflatMenu::Param::Terminal()
{
    return Param(true, false, 0, 0, 0, 0, NULL, NULL);
}

SuperflatMenu::Mapping::Mapping(int control, UIControl_MultiList* list)
    : mControl(control), mList(list), mChildren()
{
}

SuperflatMenu::Mapping::~Mapping()
{
}

bool SuperflatMenu::Mapping::isChecked() { return mList->GetCheckboxValue(mControl); }
void SuperflatMenu::Mapping::updateChildren()
{
    bool enabled = isChecked();
    for (std::vector<Sub*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        (*it)->setEnabled(enabled);
        (*it)->update();
    }
}
void SuperflatMenu::Mapping::setChecked(bool checked) { mList->SetCheckboxValue(mControl, checked, true); }
void SuperflatMenu::Mapping::writeConfig(FlatGeneratorInfo& info) { write(info); updateChildren(); }
void SuperflatMenu::Mapping::readConfig(FlatGeneratorInfo& info) { read(info); updateChildren(); }

SuperflatMenu::Mapping::Sub::Sub(Mapping* mapping)
    : mEnabled(false), mMapping(mapping), mPosition(0), mActive(false), mSlider(0)
{
    if (mapping != NULL)
    {
        mapping->mChildren.push_back(this);
    }
}

SuperflatMenu::Mapping::Sub::~Sub() { }
void SuperflatMenu::Mapping::Sub::setEnabled(bool enabled) { mEnabled = enabled; }
void SuperflatMenu::Mapping::Sub::update() { if (mMapping != NULL) mActive = mMapping->isChecked(); }
void SuperflatMenu::Mapping::Sub::setPosition(int position) { mPosition = position; }
int SuperflatMenu::Mapping::Sub::getPosition() { return mPosition; }

SuperflatMenu::MapCanyons::MapCanyons(int control, UIControl_MultiList* list)
    : Mapping(control, list) { }
SuperflatMenu::MapCanyons::~MapCanyons() { }
void SuperflatMenu::MapCanyons::write(FlatGeneratorInfo& info) { info.setCanyons(isChecked()); }
void SuperflatMenu::MapCanyons::read(FlatGeneratorInfo& info) { setChecked(info.hasCanyons()); }

SuperflatMenu::MapCaves::MapCaves(int control, UIControl_MultiList* list)
    : Mapping(control, list) { }
SuperflatMenu::MapCaves::~MapCaves() { }
void SuperflatMenu::MapCaves::write(FlatGeneratorInfo& info) { info.setCaves(isChecked()); }
void SuperflatMenu::MapCaves::read(FlatGeneratorInfo& info) { setChecked(info.hasCaves()); }

SuperflatMenu::MapDecorations::MapDecorations(int control, UIControl_MultiList* list)
    : Mapping(control, list) { }
SuperflatMenu::MapDecorations::~MapDecorations() { }
void SuperflatMenu::MapDecorations::write(FlatGeneratorInfo& info) { info.setDecorations(isChecked()); }
void SuperflatMenu::MapDecorations::read(FlatGeneratorInfo& info) { setChecked(info.hasDecorations()); }

SuperflatMenu::MapDungeon::MapDungeon(int control, UIControl_MultiList* list)
    : Mapping(control, list) { }
SuperflatMenu::MapDungeon::~MapDungeon() { }
void SuperflatMenu::MapDungeon::write(FlatGeneratorInfo& info) { info.setDungeons(isChecked()); }
void SuperflatMenu::MapDungeon::read(FlatGeneratorInfo& info) { setChecked(info.hasDungeons()); }

SuperflatMenu::MapLake::MapLake(int control, UIControl_MultiList* list)
    : Mapping(control, list) { }
SuperflatMenu::MapLake::~MapLake() { }
void SuperflatMenu::MapLake::write(FlatGeneratorInfo& info) { info.setLakes(isChecked()); }
void SuperflatMenu::MapLake::read(FlatGeneratorInfo& info) { setChecked(info.hasLakes()); }

SuperflatMenu::MapLavaLake::MapLavaLake(int control, UIControl_MultiList* list)
    : Mapping(control, list) { }
SuperflatMenu::MapLavaLake::~MapLavaLake() { }
void SuperflatMenu::MapLavaLake::write(FlatGeneratorInfo& info) { info.setLavaLakes(isChecked()); }
void SuperflatMenu::MapLavaLake::read(FlatGeneratorInfo& info) { setChecked(info.hasLavaLakes()); }

SuperflatMenu::MapBiomeSpecific::MapBiomeSpecific(int control, UIControl_MultiList* list) : Mapping(control, list), mSub(this) { }
SuperflatMenu::MapBiomeSpecific::~MapBiomeSpecific() { }
void SuperflatMenu::MapBiomeSpecific::write(FlatGeneratorInfo& info) { if (isChecked()) info.setBiomeSpecific(mSub.getPosition()); else info.setBiomeSpecific(false); }
void SuperflatMenu::MapBiomeSpecific::read(FlatGeneratorInfo& info) { mSub.setPosition(info.getBiomeSpecificSpacing()); setChecked(info.hasBiomeSpecific()); }

SuperflatMenu::MapMineshaft::MapMineshaft(int control, UIControl_MultiList* list) : Mapping(control, list), mSub(this) { }
SuperflatMenu::MapMineshaft::~MapMineshaft() { }
void SuperflatMenu::MapMineshaft::write(FlatGeneratorInfo& info) { if (isChecked()) info.setMineshaft(mSub.getPosition()); else info.setMineshaft(false); }
void SuperflatMenu::MapMineshaft::read(FlatGeneratorInfo& info) { mSub.setPosition(info.getMineshaftChance()); setChecked(info.hasMineshaft()); }

SuperflatMenu::MapVillage::MapVillage(int control, UIControl_MultiList* list)
    : Mapping(control, list), mSub(this), mSub2(this) { }
SuperflatMenu::MapVillage::~MapVillage() { }
void SuperflatMenu::MapVillage::write(FlatGeneratorInfo& info)
{
    if (isChecked()) info.setVillage(mSub.getPosition(), mSub2.getPosition());
    else info.setVillage(false);
}
void SuperflatMenu::MapVillage::read(FlatGeneratorInfo& info)
{
    mSub.setPosition(info.getVillageSize());
    mSub2.setPosition(info.getVillageDistance());
    setChecked(info.hasVillage());
}

SuperflatMenu::MapStronghold::MapStronghold(int control, UIControl_MultiList* list)
    : Mapping(control, list), mSub(this), mSub2(this) { }
SuperflatMenu::MapStronghold::~MapStronghold() { }
void SuperflatMenu::MapStronghold::write(FlatGeneratorInfo& info)
{
    if (isChecked()) info.setStronghold(mSub.getPosition(), mSub2.getPosition());
    else info.setStronghold(false);
}
void SuperflatMenu::MapStronghold::read(FlatGeneratorInfo& info)
{
    mSub.setPosition(info.getStrongholdCount());
    mSub2.setPosition(info.getStrongholdDistance());
    setChecked(info.hasStronghold());
}

SuperflatMenu::MapOceanMonument::MapOceanMonument(int control, UIControl_MultiList* list)
    : Mapping(control, list), mSub(this), mSub2(this) { }
SuperflatMenu::MapOceanMonument::~MapOceanMonument() { }
void SuperflatMenu::MapOceanMonument::write(FlatGeneratorInfo& info)
{
    if (isChecked()) info.setOceanMonument(mSub.getPosition(), mSub2.getPosition());
    else info.setOceanMonument(false);
}
void SuperflatMenu::MapOceanMonument::read(FlatGeneratorInfo& info)
{
    mSub.setPosition(info.getOceanMonumentSpacing());
    mSub2.setPosition(info.getOceanMonuemntSeparation());
    setChecked(info.hasOceanMonument());
}

SuperflatMenu::MapShipwrecks::MapShipwrecks(int control, UIControl_MultiList* list)
    : Mapping(control, list), mSub(this), mSub2(this) { }
SuperflatMenu::MapShipwrecks::~MapShipwrecks() { }
void SuperflatMenu::MapShipwrecks::write(FlatGeneratorInfo& info)
{
    if (isChecked()) info.setShipwrecks(mSub.getPosition(), mSub2.getPosition());
    else info.setShipwrecks(false);
}
void SuperflatMenu::MapShipwrecks::read(FlatGeneratorInfo& info)
{
    mSub.setPosition(info.getShipwreckSpacing());
    mSub2.setPosition(info.getShipwreckSeparation());
    setChecked(info.hasShipwrecks());
}

SuperflatMenu::MapWoodlandMansion::MapWoodlandMansion(int control, UIControl_MultiList* list)
    : Mapping(control, list), mSub(this), mSub2(this) { }
SuperflatMenu::MapWoodlandMansion::~MapWoodlandMansion() { }
void SuperflatMenu::MapWoodlandMansion::write(FlatGeneratorInfo& info)
{
    if (isChecked()) info.setWoodlandMansion(mSub.getPosition(), mSub2.getPosition());
    else info.setWoodlandMansion(false);
}
void SuperflatMenu::MapWoodlandMansion::read(FlatGeneratorInfo& info)
{
    mSub.setPosition(info.getWoodlandMansionSpacing());
    mSub2.setPosition(info.getWoodlandMansionSeparation());
    setChecked(info.hasWoodlandMansion());
}

std::wstring SuperflatMenu::BiomeInfo::getName() const { return std::wstring((const wchar_t*)mName); }
SuperflatMenu::BiomeInfo::~BiomeInfo() { }

SuperflatMenu::CustomSuperflatBlocks::CustomSuperflatBlocks() : boost::unordered_map<int, not_null_ptr<ItemInstance> >() { }
bool SuperflatMenu::CustomSuperflatBlocks::inlist(Block** list, Block* block) { if (list == NULL) return false; while (*list != NULL) { if (*list++ == block) return true; } return false; }
int SuperflatMenu::CustomSuperflatBlocks::addItem(int id, not_null_ptr<ItemInstance> itemValue) { (*this)[id] = itemValue; return id; }
int SuperflatMenu::CustomSuperflatBlocks::addItem(int id, ItemInstance* itemValue) { return addItem(id, not_null_ptr<ItemInstance>(itemValue)); }
not_null_ptr<ItemInstance> SuperflatMenu::CustomSuperflatBlocks::item(int id) const { const_iterator it = find(id); return it == end() ? not_null_ptr<ItemInstance>() : it->second; }
bool SuperflatMenu::CustomSuperflatBlocks::hasRestrictedPlacement(Block* block) { std::vector<Block*>* list = getPlacementRestrictedList(); return list != NULL && std::find(list->begin(), list->end(), block) != list->end(); }
bool SuperflatMenu::CustomSuperflatBlocks::hasRestrictedPlacement(int id) { return hasRestrictedPlacement(Block::byId(id)); }
bool SuperflatMenu::CustomSuperflatBlocks::sanitiseLayer(int*, int*, int layer) { return layer >= 0 && layer < 256; }
void SuperflatMenu::CustomSuperflatBlocks::sanitise(FlatGeneratorInfo& info)
{
    int ids[256];
    int data[256];
    std::vector<FlatLayerInfo> layers;
    info.getLayers(ids, data);
    for (int i = 0; i < 256; ++i)
    {
        sanitiseLayer(ids, data, i);
        layers.push_back(FlatLayerInfo(1, 1, Block::byId(ids[i]), data[i]));
    }
    info.setLayers(layers);
}
bool SuperflatMenu::CustomSuperflatBlocks::verify(LaunchMoreOptionsMenuInitData*, SuperflatConfig*) { return true; }

void SuperflatMenu::PresetCache::GetCachedPreset(int index, SuperflatConfig& config) { static SuperflatConfig cache[8]; config = cache[index]; }
void SuperflatMenu::PresetCache::SetCachedPreset(int index, const SuperflatConfig& config) { static SuperflatConfig cache[8]; cache[index] = config; }

int SuperflatMenu::getBiomeInfoA() { return 0; }

template<class T>
SuperflatMenu::Mapping* SuperflatMenu::Mapping::make(int control, UIControl_MultiList* list)
{
    return new T(control, list);
}

template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapVillage>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapCaves>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapCanyons>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapMineshaft>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapStronghold>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapBiomeSpecific>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapDungeon>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapDecorations>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapLake>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapLavaLake>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapOceanMonument>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapShipwrecks>(int, UIControl_MultiList*);
template SuperflatMenu::Mapping* SuperflatMenu::Mapping::make<SuperflatMenu::MapWoodlandMansion>(int, UIControl_MultiList*);
