#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Property.hpp"
#include "game/core/Boxed.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/LevelSource.hpp"
#include <algorithm>
#include <sstream>

BlockStateDefinition::BlockStateDefinition(Block* block) : mBlock(block) {}
BlockStateDefinition::BlockStateDefinition(Block* block, const Property** properties, unsigned int count) : mBlock(block) { _init(block, properties, count); }
BlockStateDefinition::BlockStateImpl::BlockStateImpl(BlockStateDefinition* definition, unsigned short elementId) : mDefinition(definition), mBlock(definition->getBlock()) { mElementId=elementId; }

BlockStateDefinition::BlockStateImpl* BlockStateDefinition::any() { return mStates.empty() ? NULL : mStates[0]; }
const std::vector<const BlockState*>& BlockStateDefinition::getPossibleBlockStates() const { return mPossibleStates; }
Block* BlockStateDefinition::getBlock() const { return mBlock; }
const std::vector<const Property*>& BlockStateDefinition::getProperties() const { return mProperties; }
std::vector<const std::vector<Boxed*>*> BlockStateDefinition::getPropertyValues()
{
    std::vector<const std::vector<Boxed*>*> result;
    for (std::vector<const Property*>::iterator i = mProperties.begin(); i != mProperties.end(); ++i)
    {
        result.push_back(&(*i)->getPossibleValues());
    }
    return result;
}
std::wstring BlockStateDefinition::PROPERTY_TO_STRING_FUNCTION(const Property* property)
{
    return property ? property->getName() : std::wstring(L"_NULL_");
}
bool BlockStateDefinition::hasProperty(const Property* property) const
{
    return std::find(mProperties.begin(), mProperties.end(), property) != mProperties.end();
}
Boxed* BlockStateDefinition::getPropertyValue(const Property* property, unsigned short data) const
{
    for (unsigned int i = 0; i < mProperties.size(); ++i)
    {
        if (mProperties[i] == property)
        {
            unsigned int n = (data >> mBitOffsets[i]) & mBitMasks[i];
            return property->getValueAtIndex(n);
        }
    }
    return NULL;
}
BlockStateDefinition::BlockStateImpl* BlockStateDefinition::getNeighbour(const Property* property, Boxed* value, unsigned short data) const
{
    for (unsigned int i = 0; i < mProperties.size(); ++i)
    {
        if (mProperties[i] == property)
        {
            unsigned int n = property->getIndexForValue(value);
            unsigned short d = (unsigned short)((data & ~(mBitMasks[i] << mBitOffsets[i])) | ((n & mBitMasks[i]) << mBitOffsets[i]));
            return d < mStates.size() ? mStates[d] : NULL;
        }
    }
    return data < mStates.size() ? mStates[data] : NULL;
}
const boost::unordered_map<const Property*, Boxed*>* BlockStateDefinition::getValues(unsigned short data) const { return data<mStates.size()?&mStates[data]->mValues:NULL; }
std::wstring BlockStateDefinition::toString() { return mBlock ? mBlock->toString() : std::wstring(); }

void BlockStateDefinition::_init(Block* block, const Property** properties, unsigned int count)
{
    mBlock=block; mProperties.clear(); mBitOffsets.clear(); mBitMasks.clear();
    for (unsigned int i=0;i<count;++i) mProperties.push_back(properties[i]);
    unsigned int bits=0;
    for (unsigned int i=0;i<mProperties.size();++i) { unsigned int n=mProperties[i]->getValueCount(); unsigned short mask=1; while (mask<n) mask=(unsigned short)((mask<<1)|1); mBitOffsets.push_back((unsigned char)bits); mBitMasks.push_back(mask); while (mask) { ++bits; mask=(unsigned short)(mask>>1); } }
    unsigned int size=1u<<bits; mStates.reserve(size); mPossibleStates.reserve(size);
    for (unsigned int i=0;i<size;++i) { BlockStateImpl* state=new BlockStateImpl(this,(unsigned short)i); mStates.push_back(state); mPossibleStates.push_back(state); for (unsigned int j=0;j<mProperties.size();++j) state->mValues[mProperties[j]]=getPropertyValue(mProperties[j],(unsigned short)i); }
}

bool BlockStateDefinition::BlockStateImpl::valuesContainsKey(const Property* p) const { return mValues.find(p)!=mValues.end(); }
const std::vector<const Property*>& BlockStateDefinition::BlockStateImpl::getProperties() const { return mDefinition->getProperties(); }
bool BlockStateDefinition::BlockStateImpl::hasProperty(const Property* p) const { return mDefinition->hasProperty(p); }
Boxed* BlockStateDefinition::BlockStateImpl::getBoxedValue(const Property* p) const { boost::unordered_map<const Property*,Boxed*>::const_iterator i=mValues.find(p); return i==mValues.end()?NULL:i->second; }
const BlockState* BlockStateDefinition::BlockStateImpl::setBoxedValue(const Property* p, Boxed* v) const { if (!p->getValue(p->getName(v))) return this; return mDefinition->getNeighbour(p,v,mElementId); }
boost::unordered_map<const Property*, Boxed*>* BlockStateDefinition::BlockStateImpl::getValues() const { return const_cast<boost::unordered_map<const Property*,Boxed*>*>(&mValues); }
Block* BlockStateDefinition::BlockStateImpl::getBlock() const { return mBlock; }
bool BlockStateDefinition::BlockStateImpl::equals(const BlockState* other) { return this==other; }
int BlockStateDefinition::BlockStateImpl::hashCode() const { return 0; }
std::wstring BlockStateDefinition::BlockStateImpl::toString() const { return mBlock ? mBlock->getName() : std::wstring(); }
Material* BlockStateDefinition::BlockStateImpl::getMaterial() const { return mBlock->getCraftingMaterial(); }
bool BlockStateDefinition::BlockStateImpl::isSolid() const { return mBlock->mFullBlock; }
bool BlockStateDefinition::BlockStateImpl::isValidSpawn(boost::shared_ptr<Entity> e) const { return mBlock->isValidSpawn(this,e); }
int BlockStateDefinition::BlockStateImpl::getLightBlock() const { return mBlock->mLightBlock; }
int BlockStateDefinition::BlockStateImpl::getLightEmission() const { return mBlock->getLightEmission(this); }
bool BlockStateDefinition::BlockStateImpl::isTranslucent() const { return mBlock->mTranslucent; }
bool BlockStateDefinition::BlockStateImpl::isTransparent() const { return mBlock->isTransparent(this); }
bool BlockStateDefinition::BlockStateImpl::doesPropagate() const { return mBlock->mUseNeighbourBrightness; }
const MaterialColor* BlockStateDefinition::BlockStateImpl::getMapColor(LevelSource* l,const BlockPos& p) const { return mBlock->getMapColor(this,l,p); }
const BlockState* BlockStateDefinition::BlockStateImpl::rotate(Rotation* r) const { return mBlock->rotate(this,r); }
const BlockState* BlockStateDefinition::BlockStateImpl::mirror(Mirror* r) const { return mBlock->mirror(this,r); }
bool BlockStateDefinition::BlockStateImpl::isCubeShaped() const { return mBlock->isCubeShaped(this); }
bool BlockStateDefinition::BlockStateImpl::hasCustomBreakingProgress() const { return mBlock->hasCustomBreakingProgress(this); }
int BlockStateDefinition::BlockStateImpl::getRenderShape() const { return mBlock->getRenderShape(this); }
int BlockStateDefinition::BlockStateImpl::getLightColor(LevelSource* l,const BlockPos& p) const { return mBlock->getLightColor(this,l,p,0); }
float BlockStateDefinition::BlockStateImpl::getShadeBrightness() const { return mBlock->getShadeBrightness(this); }
bool BlockStateDefinition::BlockStateImpl::isSolidBlockingCube() const { return mBlock->isSolidBlockingCube(this); }
bool BlockStateDefinition::BlockStateImpl::isSolidBlockingCubeAndNotSignalSource() const { return mBlock->isSolidBlockingCubeAndNotSignalSource(this); }
bool BlockStateDefinition::BlockStateImpl::isSolidBlockingBlock() const { return mBlock->isSolidBlockingCube(this)||mBlock->isSolidBlockingCubeAndNotSignalSource(this); }
bool BlockStateDefinition::BlockStateImpl::canProvideSupport(const Direction* d,EBlockSupportType t) const { return mBlock->canProvideSupport(this,d,t); }
bool BlockStateDefinition::BlockStateImpl::isSignalSource() const { return mBlock->isSignalSource(this); }
int BlockStateDefinition::BlockStateImpl::getSignal(LevelSource* l,const BlockPos& p,const Direction* d) const { return mBlock->getSignal(this,l,p,d); }
bool BlockStateDefinition::BlockStateImpl::hasAnalogOutputSignal() const { return mBlock->hasAnalogOutputSignal(this); }
int BlockStateDefinition::BlockStateImpl::getAnalogOutputSignal(Level* l,const BlockPos& p) const { return mBlock->getAnalogOutputSignal(this,l,p); }
float BlockStateDefinition::BlockStateImpl::getDestroySpeed(Level* l,const BlockPos& p) const { return mBlock->getDestroySpeed(this,l,p); }
float BlockStateDefinition::BlockStateImpl::getDestroyProgress(boost::shared_ptr<Player> p,Level* l,const BlockPos& pos) const { return mBlock->getDestroyProgress(this,p,l,pos); }
int BlockStateDefinition::BlockStateImpl::getDirectSignal(LevelSource* l,const BlockPos& p,const Direction* d) const { return mBlock->getDirectSignal(this,l,p,d); }
EPistonPushReaction BlockStateDefinition::BlockStateImpl::getPistonPushReaction() const { return mBlock->getPistonPushReaction(this); }
const BlockState* BlockStateDefinition::BlockStateImpl::fillVirtualBlockStateProperties(LevelSource* l,const BlockPos& p) const { return mBlock->fillVirtualBlockStateProperties(this,l,p); }
const AABB* BlockStateDefinition::BlockStateImpl::getOutlineAABB(Level* l,const BlockPos& p) const { return mBlock->getOutlineAABB(this,l,p); }
bool BlockStateDefinition::BlockStateImpl::shouldRenderFace(LevelSource* l,const BlockPos& p,const Direction* d) const { return mBlock->shouldRenderFace(this,l,p,d); }
bool BlockStateDefinition::BlockStateImpl::isSolidRender() const { return mBlock->isSolidRender(this); }
const AABB* BlockStateDefinition::BlockStateImpl::getClipAABB(LevelSource* l,const BlockPos& p) const { return mBlock->getClipAABB(this,l,p); }
void BlockStateDefinition::BlockStateImpl::addCollissionAABBs(Level* l,const BlockPos& p,const AABB* b,std::vector<AABB*>* v,boost::shared_ptr<Entity> e,bool a) const { mBlock->addCollisionAABBs(this,l,p,b,v,e,a); }
const AABB* BlockStateDefinition::BlockStateImpl::getShape(LevelSource* l,const BlockPos& p) const { return mBlock->getShape(this,l,p); }
HitResult* BlockStateDefinition::BlockStateImpl::clip(Level* l,const BlockPos& p,Vec3* a,Vec3* b) const { return mBlock->clip(this,l,p,a,b); }
bool BlockStateDefinition::BlockStateImpl::isTopSolidBlocking() const { return mBlock->isTopSolidBlocking(this); }
Vec3* BlockStateDefinition::BlockStateImpl::getOffset(LevelSource* l,const BlockPos& p) const { return mBlock->getOffset(this,l,p); }
bool BlockStateDefinition::BlockStateImpl::isViewBlocking() const { return mBlock->isViewBlocking(this); }
EBlockFaceShape BlockStateDefinition::BlockStateImpl::getBlockFaceShape(LevelSource* l,const BlockPos& p,const Direction* d) const { return mBlock->getBlockFaceShape(l,this,p,d); }
const BlockState* BlockStateDefinition::BlockStateImpl::cycle(const Property* p) const
{
    Boxed* v = getBoxedValue(p);
    if (!v)
    {
        return this;
    }
    unsigned int i = p->getIndexForValue(v);
    return mDefinition->getNeighbour(p, p->getValueAtIndex((i + 1) % p->getValueCount()), mElementId);
}
bool BlockStateDefinition::BlockStateImpl::triggerEvent(Level* l,const BlockPos& p,int e,int q) const { return mBlock->triggerEvent(this,l,p,e,q); }
void BlockStateDefinition::BlockStateImpl::neighborChanged(Level* l,const BlockPos& p,Block* b,const BlockPos& f) const { mBlock->neighborChanged(this,l,p,b,f); }
void BlockStateDefinition::BlockStateImpl::populateNeighbours(boost::unordered_map<std::map<const Property*,Boxed*>,BlockStateImpl*>*) {}
void BlockStateDefinition::BlockStateImpl::makeNeighbourValues(const Property*,Boxed*,boost::unordered_map<const Property*,Boxed*>&) {}
