#include "game/ui/SuperflatMenu.hpp"
#include "game/core/L10N.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include <algorithm>
#include <cstdio>

static std::wstring formatwstr(const wchar_t* format, ...);

SuperflatMenu::SliderLabelBehaviour::SliderLabelBehaviour(int name,unsigned int minimum,unsigned int maximum):mName(name),mMinimum(minimum),mMaximum(maximum),mFormat(0) {}
SuperflatMenu::CategoricalLabelBehaviour::CategoricalLabelBehaviour(int name,int* labels):SliderLabelBehaviour(name,0,0),mLabels(labels) {}
int SuperflatMenu::CategoricalLabelBehaviour::getLength(int*) { int length=0; if(mLabels!=0) { while(mLabels[length]!=-1) ++length; } return length; }
std::wstring SuperflatMenu::CategoricalLabelBehaviour::GetLabel(int value) { if(mLabels==0) return std::wstring(); return std::wstring(L10N::GetString(mLabels[value])); }

SuperflatMenu::NumericalLabelBehaviour::NumericalLabelBehaviour(int name,unsigned int minimum,unsigned int maximum):SliderLabelBehaviour(name,minimum,maximum),mType(0),mScale(1.0),mPrecision(0),mFormatString(0),mLabel(0),mUnused(0) {}
SuperflatMenu::NumericalLabelBehaviour::NumericalLabelBehaviour(int name,const std::wstring& label,int type,unsigned int minimum,unsigned int maximum):SliderLabelBehaviour(name,minimum,maximum),mType(type),mScale(1.0),mPrecision(0),mFormatString(0),mLabel(0),mUnused(0) { mLabel=label.empty()?0:1; }
SuperflatMenu::NumericalLabelBehaviour::NumericalLabelBehaviour(int name,const std::wstring& label,double scale,unsigned int minimum,unsigned int maximum):SliderLabelBehaviour(name,minimum,maximum),mType(0),mScale(scale),mPrecision(0),mFormatString(0),mLabel(0),mUnused(0) { mLabel=label.empty()?0:1; }
SuperflatMenu::NumericalLabelBehaviour::NumericalLabelBehaviour(int name,const std::wstring& label,unsigned int minimum,unsigned int maximum):SliderLabelBehaviour(name,minimum,maximum),mType(0),mScale(1.0),mPrecision(0),mFormatString(0),mLabel(0),mUnused(0) { mLabel=label.empty()?0:1; }
std::wstring SuperflatMenu::NumericalLabelBehaviour::GetLabel(int value) { std::wstring result; if(mType==0) result=formatwstr(L"%i",(int)((value-mMinimum)*mScale)); else if(mType==1) result=formatwstr(L"%i",(int)((value-mMinimum)*mScale)); else result=std::wstring(L10N::GetString(mName)); return result; }

SuperflatMenu::Param::Param(bool terminal,bool parent,int name,int desc,int minimum,int maximum,int unused,MappingFactory factory,SliderLabelBehaviour* behaviour):mTerminal(terminal),mParent(parent),mName(name),mDesc(desc),mMinValue(minimum),mMaxValue(maximum),mBehaviour(behaviour),mFactory(factory) { (void)unused; }
std::wstring SuperflatMenu::Param::getLabel(int value) const { if(mBehaviour!=0) return mBehaviour->GetLabel(value); return std::wstring(L10N::GetString(mName)); }
std::wstring SuperflatMenu::Param::getName() const { return std::wstring(L10N::GetString(mName)); }
bool SuperflatMenu::Param::isTerminal() const { return mTerminal!=0; }
bool SuperflatMenu::Param::isParent() const { return mParent!=0; }
bool SuperflatMenu::Param::isChild() const { return !isTerminal()&&!isParent(); }
int SuperflatMenu::Param::getDesc() const { return mDesc; }
int SuperflatMenu::Param::getMinValue() const { return mMinValue; }
int SuperflatMenu::Param::getMaxValue() const { return mMaxValue; }
SuperflatMenu::SliderLabelBehaviour* SuperflatMenu::Param::getFunc() const { return mBehaviour; }
SuperflatMenu::Param* SuperflatMenu::Param::Parent(MappingFactory factory,int name,int desc) { return new Param(false,true,name,desc,0,0,0,factory,0); }
SuperflatMenu::Param* SuperflatMenu::Param::Child(SliderLabelBehaviour* behaviour,int name) { return new Param(false,false,name,0,0,0,0,0,behaviour); }
SuperflatMenu::Param* SuperflatMenu::Param::Terminal() { return new Param(true,false,0,0,0,0,0,0,0); }

SuperflatMenu::Mapping::Mapping(int control,UIControl_MultiList* list):mControl(control),mList(list),mName(0) {}
SuperflatMenu::Mapping::~Mapping() { for(std::vector<Sub*>::iterator i=mChildren.begin();i!=mChildren.end();++i) delete *i; }
bool SuperflatMenu::Mapping::isChecked() { return false; }
void SuperflatMenu::Mapping::updateChildren() { for(std::vector<Sub*>::iterator i=mChildren.begin();i!=mChildren.end();++i) (*i)->update(); }
void SuperflatMenu::Mapping::setChecked(bool) {}
void SuperflatMenu::Mapping::writeConfig(FlatGeneratorInfo&) {}
void SuperflatMenu::Mapping::readConfig(FlatGeneratorInfo&) {}

SuperflatMenu::Mapping::Sub::Sub(Mapping* mapping):mEnabled(0),mMapping(mapping),mPosition(0),mActive(0),mSlider(0) { if(mapping!=0) mapping->mChildren.push_back(this); }
SuperflatMenu::Mapping::Sub::~Sub() {}
void SuperflatMenu::Mapping::Sub::setEnabled(bool enabled) { mEnabled=enabled; }
void SuperflatMenu::Mapping::Sub::update() { if(mMapping!=0) mActive=mMapping->isChecked(); }
void SuperflatMenu::Mapping::Sub::setPosition(int position) { mPosition=position; }
int SuperflatMenu::Mapping::Sub::getPosition() { return mPosition; }

#define SF_SIMPLE(N,SET,HAS) SuperflatMenu::N::N(int control,UIControl_MultiList* list):Mapping(control,list) {} SuperflatMenu::N::~N() {} void SuperflatMenu::N::write(FlatGeneratorInfo& info) { info.SET(isChecked()); } void SuperflatMenu::N::read(FlatGeneratorInfo& info) { setChecked(info.HAS()); }
SF_SIMPLE(MapCanyons,setCanyons,hasCanyons)
SF_SIMPLE(MapCaves,setCaves,hasCaves)
SF_SIMPLE(MapDecorations,setDecorations,hasDecorations)
SF_SIMPLE(MapDungeon,setDungeons,hasDungeons)
SF_SIMPLE(MapLake,setLakes,hasLakes)
SF_SIMPLE(MapLavaLake,setLavaLakes,hasLavaLakes)
#undef SF_SIMPLE

#define SF_SUB(N) SuperflatMenu::N::N(int control,UIControl_MultiList* list):Mapping(control,list),mSub(this) {} SuperflatMenu::N::~N() {} void SuperflatMenu::N::write(FlatGeneratorInfo&) {} void SuperflatMenu::N::read(FlatGeneratorInfo&) {}
SF_SUB(MapBiomeSpecific)
SF_SUB(MapMineshaft)
#undef SF_SUB
SuperflatMenu::MapOceanMonument::MapOceanMonument(int control,UIControl_MultiList* list):Mapping(control,list),mSub(this),mSub2(this) {}
SuperflatMenu::MapOceanMonument::~MapOceanMonument() {}
void SuperflatMenu::MapOceanMonument::write(FlatGeneratorInfo& info) { info.setOceanMonument(isChecked()); }
void SuperflatMenu::MapOceanMonument::read(FlatGeneratorInfo& info) { setChecked(info.hasOceanMonument()); }
#define SF_TWO(N,SET,HAS) SuperflatMenu::N::N(int control,UIControl_MultiList* list):Mapping(control,list),mSub(this),mSub2(this) {} SuperflatMenu::N::~N() {} void SuperflatMenu::N::write(FlatGeneratorInfo& info) { info.SET(isChecked()); } void SuperflatMenu::N::read(FlatGeneratorInfo& info) { setChecked(info.HAS()); }
SF_TWO(MapShipwrecks,setShipwrecks,hasShipwrecks)
SF_TWO(MapStronghold,setStronghold,hasStronghold)
SF_TWO(MapVillage,setVillage,hasVillage)
SF_TWO(MapWoodlandMansion,setWoodlandMansion,hasWoodlandMansion)
#undef SF_TWO

std::wstring SuperflatMenu::BiomeInfo::getName() const { return std::wstring(); }
SuperflatMenu::BiomeInfo::~BiomeInfo() {}

SuperflatMenu::CustomSuperflatBlocks::CustomSuperflatBlocks():boost::unordered_map<int,not_null_ptr<ItemInstance> >() {}
SuperflatMenu::CustomSuperflatBlocks::~CustomSuperflatBlocks() {}
bool SuperflatMenu::CustomSuperflatBlocks::inlist(Block**,Block*) { return false; }
int SuperflatMenu::CustomSuperflatBlocks::addItem(int id,not_null_ptr<ItemInstance> item) { (*this)[id]=item; return id; }
int SuperflatMenu::CustomSuperflatBlocks::addItem(int id,ItemInstance* item) { return addItem(id,not_null_ptr<ItemInstance>(item)); }
not_null_ptr<ItemInstance> SuperflatMenu::CustomSuperflatBlocks::item(int id) const { const_iterator i=find(id); if(i==end()) return not_null_ptr<ItemInstance>(); return i->second; }
bool SuperflatMenu::CustomSuperflatBlocks::hasRestrictedPlacement(Block*) { return false; }
bool SuperflatMenu::CustomSuperflatBlocks::hasRestrictedPlacement(int) { return false; }
bool SuperflatMenu::CustomSuperflatBlocks::sanitiseLayer(int*,int*,int) { return true; }
void SuperflatMenu::CustomSuperflatBlocks::sanitise(FlatGeneratorInfo&) {}
bool SuperflatMenu::CustomSuperflatBlocks::verify(LaunchMoreOptionsMenuInitData*,SuperflatConfig*) { return true; }
bool SuperflatMenu::CustomSuperflatBlocks::inlist(const int*,int) { return false; }
int SuperflatMenu::CustomSuperflatBlocks::itemcount() const { return (int)size(); }

void SuperflatMenu::PresetCache::GetCachedPreset(int,SuperflatConfig&) {}
void SuperflatMenu::PresetCache::SetCachedPreset(int,const SuperflatConfig&) {}
int SuperflatMenu::getBiomeInfoA() { return 0; }

SuperflatMenu::Mapping* SuperflatMenu::Mapping::make__tm__31_Q2_13SuperflatMenu10MapVillage(int c,UIControl_MultiList* l) { return new MapVillage(c,l); }
SuperflatMenu::Mapping* SuperflatMenu::Mapping::make__tm__28_Q2_13SuperflatMenu8MapCaves(int c,UIControl_MultiList* l) { return new MapCaves(c,l); }
SuperflatMenu::Mapping* SuperflatMenu::Mapping::make__tm__31_Q2_13SuperflatMenu10MapCanyons(int c,UIControl_MultiList* l) { return new MapCanyons(c,l); }
SuperflatMenu::Mapping* SuperflatMenu::Mapping::make__tm__33_Q2_13SuperflatMenu12MapMineshaft(int c,UIControl_MultiList* l) { return new MapMineshaft(c,l); }
SuperflatMenu::Mapping* SuperflatMenu::Mapping::make__tm__34_Q2_13SuperflatMenu13MapStronghold(int c,UIControl_MultiList* l) { return new MapStronghold(c,l); }
SuperflatMenu::Mapping* SuperflatMenu::Mapping::make__tm__37_Q2_13SuperflatMenu16MapBiomeSpecific(int c,UIControl_MultiList* l) { return new MapBiomeSpecific(c,l); }
SuperflatMenu::Mapping* SuperflatMenu::Mapping::make__tm__31_Q2_13SuperflatMenu10MapDungeon(int c,UIControl_MultiList* l) { return new MapDungeon(c,l); }
SuperflatMenu::Mapping* SuperflatMenu::Mapping::make__tm__35_Q2_13SuperflatMenu14MapDecorations(int c,UIControl_MultiList* l) { return new MapDecorations(c,l); }
SuperflatMenu::Mapping* SuperflatMenu::Mapping::make__tm__27_Q2_13SuperflatMenu7MapLake(int c,UIControl_MultiList* l) { return new MapLake(c,l); }
SuperflatMenu::Mapping* SuperflatMenu::Mapping::make__tm__32_Q2_13SuperflatMenu11MapLavaLake(int c,UIControl_MultiList* l) { return new MapLavaLake(c,l); }
SuperflatMenu::Mapping* SuperflatMenu::Mapping::make__tm__37_Q2_13SuperflatMenu16MapOceanMonument(int c,UIControl_MultiList* l) { return new MapOceanMonument(c,l); }
SuperflatMenu::Mapping* SuperflatMenu::Mapping::make__tm__34_Q2_13SuperflatMenu13MapShipwrecks(int c,UIControl_MultiList* l) { return new MapShipwrecks(c,l); }
SuperflatMenu::Mapping* SuperflatMenu::Mapping::make__tm__39_Q2_13SuperflatMenu18MapWoodlandMansion(int c,UIControl_MultiList* l) { return new MapWoodlandMansion(c,l); }
