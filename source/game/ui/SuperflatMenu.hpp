#ifndef GAME_UI_SUPERFLATMENU_HPP
#define GAME_UI_SUPERFLATMENU_HPP
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include "game/item/not_null_ptr.hpp"
class Block; class FlatGeneratorInfo; class ItemInstance; class LaunchMoreOptionsMenuInitData; class SuperflatConfig; class UIControl_MultiList;
class SuperflatMenu {
public:
 class SliderLabelBehaviour { public: SliderLabelBehaviour(int,unsigned int,unsigned int); virtual std::wstring GetLabel(int)=0; unsigned int mName; int mMinimum; int mMaximum; unsigned int mFormat; };
 class CategoricalLabelBehaviour:public SliderLabelBehaviour { public: CategoricalLabelBehaviour(int,int*); int getLength(int*); std::wstring GetLabel(int); int* mLabels; };
 class NumericalLabelBehaviour:public SliderLabelBehaviour { public: NumericalLabelBehaviour(int,unsigned int,unsigned int); NumericalLabelBehaviour(int,const std::wstring&,int,unsigned int,unsigned int); NumericalLabelBehaviour(int,const std::wstring&,double,unsigned int,unsigned int); NumericalLabelBehaviour(int,const std::wstring&,unsigned int,unsigned int); std::wstring GetLabel(int); int mType; double mScale; short mPrecision; int mFormatString; int mLabel; int mUnused; };
 class Mapping;
 class Param { public: typedef Mapping* (*MappingFactory)(int,UIControl_MultiList*); Param(bool,bool,int,int,int,int,int,MappingFactory,SliderLabelBehaviour*); std::wstring getLabel(int) const; std::wstring getName() const; bool isTerminal() const; bool isParent() const; bool isChild() const; int getDesc() const; int getMinValue() const; int getMaxValue() const; SliderLabelBehaviour* getFunc() const; static Param* Parent(MappingFactory,int,int); static Param* Child(SliderLabelBehaviour*,int); static Param* Terminal(); unsigned char mTerminal; unsigned char mParent; int mName; int mDesc; int mMinValue; int mMaxValue; SliderLabelBehaviour* mBehaviour; MappingFactory mFactory; };
 class Mapping { public: class Sub { public: Sub(Mapping*); ~Sub(); void setEnabled(bool); void update(); void setPosition(int); int getPosition(); unsigned char mEnabled; Mapping* mMapping; int mPosition; unsigned char mActive; int mSlider; }; Mapping(int,UIControl_MultiList*); ~Mapping(); bool isChecked(); void updateChildren(); void setChecked(bool); void writeConfig(FlatGeneratorInfo&); void readConfig(FlatGeneratorInfo&); static Mapping* make__tm__31_Q2_13SuperflatMenu10MapVillage(int,UIControl_MultiList*); static Mapping* make__tm__28_Q2_13SuperflatMenu8MapCaves(int,UIControl_MultiList*); static Mapping* make__tm__31_Q2_13SuperflatMenu10MapCanyons(int,UIControl_MultiList*); static Mapping* make__tm__33_Q2_13SuperflatMenu12MapMineshaft(int,UIControl_MultiList*); static Mapping* make__tm__34_Q2_13SuperflatMenu13MapStronghold(int,UIControl_MultiList*); static Mapping* make__tm__37_Q2_13SuperflatMenu16MapBiomeSpecific(int,UIControl_MultiList*); static Mapping* make__tm__31_Q2_13SuperflatMenu10MapDungeon(int,UIControl_MultiList*); static Mapping* make__tm__35_Q2_13SuperflatMenu14MapDecorations(int,UIControl_MultiList*); static Mapping* make__tm__27_Q2_13SuperflatMenu7MapLake(int,UIControl_MultiList*); static Mapping* make__tm__32_Q2_13SuperflatMenu11MapLavaLake(int,UIControl_MultiList*); static Mapping* make__tm__37_Q2_13SuperflatMenu16MapOceanMonument(int,UIControl_MultiList*); static Mapping* make__tm__34_Q2_13SuperflatMenu13MapShipwrecks(int,UIControl_MultiList*); static Mapping* make__tm__39_Q2_13SuperflatMenu18MapWoodlandMansion(int,UIControl_MultiList*); int mControl; UIControl_MultiList* mList; std::vector<Sub*> mChildren; int mName; };
#define SF_MAP(N) class N:public Mapping { public: N(int,UIControl_MultiList*); void write(FlatGeneratorInfo&); void read(FlatGeneratorInfo&); ~N(); };
 SF_MAP(MapCanyons) SF_MAP(MapCaves) SF_MAP(MapDecorations) SF_MAP(MapDungeon) SF_MAP(MapLake) SF_MAP(MapLavaLake)
 class MapBiomeSpecific:public Mapping { public: MapBiomeSpecific(int,UIControl_MultiList*); void write(FlatGeneratorInfo&); void read(FlatGeneratorInfo&); ~MapBiomeSpecific(); Sub mSub; };
 class MapMineshaft:public Mapping { public: MapMineshaft(int,UIControl_MultiList*); void write(FlatGeneratorInfo&); void read(FlatGeneratorInfo&); ~MapMineshaft(); Sub mSub; };
 class MapOceanMonument:public Mapping { public: MapOceanMonument(int,UIControl_MultiList*); void write(FlatGeneratorInfo&); void read(FlatGeneratorInfo&); ~MapOceanMonument(); Sub mSub; Sub mSub2; };
 class MapShipwrecks:public Mapping { public: MapShipwrecks(int,UIControl_MultiList*); void write(FlatGeneratorInfo&); void read(FlatGeneratorInfo&); ~MapShipwrecks(); Sub mSub; Sub mSub2; };
 class MapStronghold:public Mapping { public: MapStronghold(int,UIControl_MultiList*); void write(FlatGeneratorInfo&); void read(FlatGeneratorInfo&); ~MapStronghold(); Sub mSub; Sub mSub2; };
 class MapVillage:public Mapping { public: MapVillage(int,UIControl_MultiList*); void write(FlatGeneratorInfo&); void read(FlatGeneratorInfo&); ~MapVillage(); Sub mSub; Sub mSub2; };
 class MapWoodlandMansion:public Mapping { public: MapWoodlandMansion(int,UIControl_MultiList*); void write(FlatGeneratorInfo&); void read(FlatGeneratorInfo&); ~MapWoodlandMansion(); Sub mSub; Sub mSub2; };
 class BiomeInfo { public: std::wstring getName() const; ~BiomeInfo(); int mBiome; void* mName; };
 class PresetCache { public: static void GetCachedPreset(int,SuperflatConfig&); static void SetCachedPreset(int,const SuperflatConfig&); };
 class CustomSuperflatBlocks:public boost::unordered_map<int,not_null_ptr<ItemInstance> > { public: CustomSuperflatBlocks(); static bool inlist(Block**,Block*); int addItem(int,not_null_ptr<ItemInstance>); int addItem(int,ItemInstance*); not_null_ptr<ItemInstance> item(int) const; static bool hasRestrictedPlacement(Block*); static bool hasRestrictedPlacement(int); static bool sanitiseLayer(int*,int*,int); static void sanitise(FlatGeneratorInfo&); static bool verify(LaunchMoreOptionsMenuInitData*,SuperflatConfig*); static bool inlist(const int*,int); ~CustomSuperflatBlocks(); int itemcount() const; };
 static int getBiomeInfoA();
};
#undef SF_MAP
#endif
