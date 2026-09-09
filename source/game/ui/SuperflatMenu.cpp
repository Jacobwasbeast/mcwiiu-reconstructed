#include "game/ui/SuperflatMenu.hpp"

#include "game/ui/UIControl_MultiList.hpp"
#include "game/core/L10N.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/EnumProperty.hpp"
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
    if (*labels < 0) return length;
    do
    {
        ++labels;
        ++length;
    } while (*labels >= 0);
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

int SuperflatMenu::Param::getName() const
{
    return mName;
}

bool SuperflatMenu::Param::isTerminal() const
{
    return mTerminal;
}

bool SuperflatMenu::Param::isParent() const
{
    return mParent;
}

bool SuperflatMenu::Param::isChild() const
{
    return !mTerminal && !mParent;
}

int SuperflatMenu::Param::getDesc() const
{
    return mDesc;
}

int SuperflatMenu::Param::getMinValue() const
{
    return mMinValue;
}

int SuperflatMenu::Param::getMaxValue() const
{
    return mMaxValue;
}

SuperflatMenu::Param::MappingFactory SuperflatMenu::Param::getFunc() const
{
    return mFactory;
}

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

bool SuperflatMenu::Mapping::isChecked()
{
    return mList->GetCheckboxValue(mControl);
}
void SuperflatMenu::Mapping::updateChildren()
{
    bool enabled = isChecked();
    for (std::vector<Sub*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        (*it)->setEnabled(enabled);
        (*it)->update();
    }
}
void SuperflatMenu::Mapping::setChecked(bool checked)
{
    mList->SetCheckboxValue(mControl, checked, true);
}

void SuperflatMenu::Mapping::writeConfig(FlatGeneratorInfo& info)
{
    write(info);
    updateChildren();
}

void SuperflatMenu::Mapping::readConfig(FlatGeneratorInfo& info)
{
    read(info);
    updateChildren();
}

SuperflatMenu::Mapping::Sub::Sub(Mapping* mapping)
    : mEnabled(false), mMapping(mapping), mPosition(0), mActive(false), mSlider(0)
{
    if (mapping != NULL)
    {
        mapping->mChildren.push_back(this);
    }
}

SuperflatMenu::Mapping::Sub::~Sub()
{
}

void SuperflatMenu::Mapping::Sub::setEnabled(bool enabled)
{
    mEnabled = enabled;
}

void SuperflatMenu::Mapping::Sub::update()
{
    if (mMapping != NULL)
        mActive = mMapping->isChecked();
}

void SuperflatMenu::Mapping::Sub::setPosition(int position)
{
    mPosition = position;
}

int SuperflatMenu::Mapping::Sub::getPosition()
{
    return mPosition;
}

SuperflatMenu::MapCanyons::MapCanyons(int control, UIControl_MultiList* list)
    : Mapping(control, list)
{
}
SuperflatMenu::MapCanyons::~MapCanyons()
{
}
void SuperflatMenu::MapCanyons::write(FlatGeneratorInfo& info)
{
    info.setCanyons(isChecked());
}
void SuperflatMenu::MapCanyons::read(FlatGeneratorInfo& info)
{
    setChecked(info.hasCanyons());
}

SuperflatMenu::MapCaves::MapCaves(int control, UIControl_MultiList* list)
    : Mapping(control, list)
{
}
SuperflatMenu::MapCaves::~MapCaves()
{
}
void SuperflatMenu::MapCaves::write(FlatGeneratorInfo& info)
{
    info.setCaves(isChecked());
}
void SuperflatMenu::MapCaves::read(FlatGeneratorInfo& info)
{
    setChecked(info.hasCaves());
}

SuperflatMenu::MapDecorations::MapDecorations(int control, UIControl_MultiList* list)
    : Mapping(control, list)
{
}
SuperflatMenu::MapDecorations::~MapDecorations()
{
}
void SuperflatMenu::MapDecorations::write(FlatGeneratorInfo& info)
{
    info.setDecorations(isChecked());
}
void SuperflatMenu::MapDecorations::read(FlatGeneratorInfo& info)
{
    setChecked(info.hasDecorations());
}

SuperflatMenu::MapDungeon::MapDungeon(int control, UIControl_MultiList* list)
    : Mapping(control, list)
{
}
SuperflatMenu::MapDungeon::~MapDungeon()
{
}
void SuperflatMenu::MapDungeon::write(FlatGeneratorInfo& info)
{
    info.setDungeons(isChecked());
}
void SuperflatMenu::MapDungeon::read(FlatGeneratorInfo& info)
{
    setChecked(info.hasDungeons());
}

SuperflatMenu::MapLake::MapLake(int control, UIControl_MultiList* list)
    : Mapping(control, list)
{
}
SuperflatMenu::MapLake::~MapLake()
{
}
void SuperflatMenu::MapLake::write(FlatGeneratorInfo& info)
{
    info.setLakes(isChecked());
}
void SuperflatMenu::MapLake::read(FlatGeneratorInfo& info)
{
    setChecked(info.hasLakes());
}

SuperflatMenu::MapLavaLake::MapLavaLake(int control, UIControl_MultiList* list)
    : Mapping(control, list)
{
}
SuperflatMenu::MapLavaLake::~MapLavaLake()
{
}
void SuperflatMenu::MapLavaLake::write(FlatGeneratorInfo& info)
{
    info.setLavaLakes(isChecked());
}
void SuperflatMenu::MapLavaLake::read(FlatGeneratorInfo& info)
{
    setChecked(info.hasLavaLakes());
}

SuperflatMenu::MapBiomeSpecific::MapBiomeSpecific(int control, UIControl_MultiList* list)
    : Mapping(control, list), mSub(this)
{
}
SuperflatMenu::MapBiomeSpecific::~MapBiomeSpecific()
{
}
void SuperflatMenu::MapBiomeSpecific::write(FlatGeneratorInfo& info)
{
    if (isChecked())
        info.setBiomeSpecific(mSub.getPosition());
    else
        info.setBiomeSpecific(false);
}
void SuperflatMenu::MapBiomeSpecific::read(FlatGeneratorInfo& info)
{
    mSub.setPosition(info.getBiomeSpecificSpacing());
    setChecked(info.hasBiomeSpecific());
}

SuperflatMenu::MapMineshaft::MapMineshaft(int control, UIControl_MultiList* list)
    : Mapping(control, list), mSub(this)
{
}
SuperflatMenu::MapMineshaft::~MapMineshaft()
{
}
void SuperflatMenu::MapMineshaft::write(FlatGeneratorInfo& info)
{
    if (isChecked())
        info.setMineshaft(mSub.getPosition());
    else
        info.setMineshaft(false);
}
void SuperflatMenu::MapMineshaft::read(FlatGeneratorInfo& info)
{
    mSub.setPosition(info.getMineshaftChance());
    setChecked(info.hasMineshaft());
}

SuperflatMenu::MapVillage::MapVillage(int control, UIControl_MultiList* list)
    : Mapping(control, list), mSub(this), mSub2(this)
{
}
SuperflatMenu::MapVillage::~MapVillage()
{
}
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
    : Mapping(control, list), mSub(this), mSub2(this)
{
}
SuperflatMenu::MapStronghold::~MapStronghold()
{
}
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
    : Mapping(control, list), mSub(this), mSub2(this)
{
}
SuperflatMenu::MapOceanMonument::~MapOceanMonument()
{
}
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
    : Mapping(control, list), mSub(this), mSub2(this)
{
}
SuperflatMenu::MapShipwrecks::~MapShipwrecks()
{
}
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
    : Mapping(control, list), mSub(this), mSub2(this)
{
}
SuperflatMenu::MapWoodlandMansion::~MapWoodlandMansion()
{
}
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

std::wstring SuperflatMenu::BiomeInfo::getName() const
{
    return std::wstring((const wchar_t*)mName);
}

SuperflatMenu::BiomeInfo::~BiomeInfo()
{
}

SuperflatMenu::CustomSuperflatBlocks::CustomSuperflatBlocks()
    : boost::unordered_map<int, not_null_ptr<ItemInstance> >()
{
}

bool SuperflatMenu::CustomSuperflatBlocks::inlist(Block** list, Block* block)
{
    if (list == NULL)
        return false;
    while (*list != NULL)
    {
        if (*list++ == block)
            return true;
    }
    return false;
}

int SuperflatMenu::CustomSuperflatBlocks::addItem(int id, not_null_ptr<ItemInstance> itemValue)
{
    (*this)[id] = itemValue;
    return id;
}

int SuperflatMenu::CustomSuperflatBlocks::addItem(int id, ItemInstance* itemValue)
{
    return addItem(id, not_null_ptr<ItemInstance>(itemValue));
}

not_null_ptr<ItemInstance> SuperflatMenu::CustomSuperflatBlocks::item(int id) const
{
    const_iterator it = find(id);
    return it == end() ? not_null_ptr<ItemInstance>() : it->second;
}

bool SuperflatMenu::CustomSuperflatBlocks::hasRestrictedPlacement(Block* block)
{
    std::vector<Block*>* list = getPlacementRestrictedList();
    return list != NULL && std::find(list->begin(), list->end(), block) != list->end();
}

bool SuperflatMenu::CustomSuperflatBlocks::hasRestrictedPlacement(int id)
{
    return hasRestrictedPlacement(Block::byId(id));
}

bool SuperflatMenu::CustomSuperflatBlocks::sanitiseLayer(int*, int*, int layer)
{
    return layer >= 0 && layer < 256;
}
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
uint32_t SuperflatMenu::CustomSuperflatBlocks::verify(LaunchMoreOptionsMenuInitData*, SuperflatConfig* config)
{
    if (config == NULL)
        return 0x6e694430;
    FlatGeneratorInfo info(config, true);
    info.reduceLayers();
    const int biome = info.getBiome();
    if (info.hasBiomeSpecific() && biome != 2 && biome != 0x15 && biome != 6 && biome != 0x1e &&
        biome != 0xc && biome != 0x18 && biome != 0x2e && biome != 0x2f && biome != 0x2c &&
        biome != 0x2d && biome != 0x2a && biome != 0x2b && biome != 0x28 && biome != 0x29)
        return 0xd862b5d5;
    if (info.hasOceanMonument() && biome != 0 && biome != 0x18 && biome != 0x2e && biome != 0x2f &&
        biome != 0x2c && biome != 0x2d && biome != 0x2a && biome != 0x2b && biome != 0x28 && biome != 0x29)
        return 0xc2e51385;
    if (info.hasShipwrecks() && biome != 0 && biome != 0x18 && biome != 0x2e && biome != 0x2f &&
        biome != 0x2c && biome != 0x2d && biome != 0x2a && biome != 0x2b && biome != 0x28 && biome != 0x29 &&
        biome != 0x10 && biome != 0x1a && biome != 0xf)
        return 0x37917102;
    if (info.hasWoodlandMansion() && biome != 0x1d)
        return 0xdae45b4d;
    if ((info.hasCaves() && info.getHeight() <= 1) || (info.hasCanyons() && info.getHeight() <= 3))
        return 0x258655f1;
    std::vector<FlatLayerInfo> layers;
    info.getLayers(layers);
    for (std::vector<FlatLayerInfo>::const_iterator it = layers.begin(); it != layers.end(); ++it)
    {
        Block* block = Block::byId(it->getId());
        if (block == NULL)
            return 0x6e694430;
        if (it->getHeight() > 1 && hasRestrictedPlacement(block))
            return 0x16c16d5e;
    }
    return 0;
}

void SuperflatMenu::PresetCache::GetCachedPreset(int index, SuperflatConfig& config)
{
    static SuperflatConfig cache[8];
    config = cache[index];
}

void SuperflatMenu::PresetCache::SetCachedPreset(int index, const SuperflatConfig& config)
{
    static SuperflatConfig cache[8];
    cache[index] = config;
}

SuperflatMenu::BiomeItem* SuperflatMenu::getBiomeInfoA()
{
    // The Wii U image initializes this descriptor table lazily.  Keep the
    // table resident because callers retain the item handles between menu
    // updates.
    static bool initialized = false;
    static BiomeItem items[21] = {};
    if (!initialized)
    {
        initialized = true;
        static const int blockIds[17] =
        {
            0x23, 0x25, 0x04, 0x1d, 0x06, 0x03, 0x00, 0x28,
            0x2a, 0x2c, 0x2e, 0x0c, 0x1e, 0x05, 0x0e, 0x08, 0x09
        };
        static const unsigned int itemIndices[17] =
        { 0, 1, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 17, 18, 19 };
        for (unsigned int i = 0; i < 17; ++i)
        {
            Block* block = Block::byId(blockIds[i]);
            if (block != NULL)
                items[itemIndices[i]].id = blockIds[i];
                createItem__FP5Block(&items[itemIndices[i]].item, block);
        }
        Block* sand = Block::byId(0x02);
        if (sand != NULL && SandBlock::TYPE != NULL && SandBlock::Variant::RED_SAND != NULL)
        {
            TypedBoxed<SandBlock::Variant*> value(SandBlock::Variant::RED_SAND);
            const BlockState* state = sand->defaultBlockState()->setBoxedValue(
                static_cast<const Property*>(SandBlock::TYPE), &value);
            items[2].id = 0x02;
            items[2].item = not_null_ptr<ItemInstance>(createItem(state));
        }
        Block* planks = Block::byId(0x15);
        if (planks != NULL && PlanksBlock::VARIANT != NULL && PlanksBlock::Variant::DARK_OAK != NULL)
        {
            TypedBoxed<PlanksBlock::Variant*> value(PlanksBlock::Variant::DARK_OAK);
            const BlockState* state = planks->defaultBlockState()->setBoxedValue(
                static_cast<const Property*>(PlanksBlock::VARIANT), &value);
            items[5].id = 0x15;
            items[5].item = not_null_ptr<ItemInstance>(createItem(state));
        }
        Block* planksVariant = Block::byId(0x20);
        if (planksVariant != NULL && PlanksBlock::VARIANT != NULL && PlanksBlock::Variant::SPRUCE != NULL)
        {
            TypedBoxed<PlanksBlock::Variant*> value(PlanksBlock::Variant::SPRUCE);
            const BlockState* state = planksVariant->defaultBlockState()->setBoxedValue(
                static_cast<const Property*>(PlanksBlock::VARIANT), &value);
            items[16].id = 0x20;
            items[16].item = not_null_ptr<ItemInstance>(createItem(state));
        }
        // Entry 20 is the image's raw FFFFFFFF sentinel and intentionally
        // remains an empty descriptor.
    }
    return items;
}

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
