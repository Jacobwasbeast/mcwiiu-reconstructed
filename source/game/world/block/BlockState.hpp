#ifndef GAME_WORLD_BLOCK_BLOCKSTATE_HPP
#define GAME_WORLD_BLOCK_BLOCKSTATE_HPP

#include <cstddef>
#include <string>
#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

class AABB;
class Block;
class BlockPos;
class Boxed;
class Direction;
class Entity;
class HitResult;
class Level;
class LevelSource;
class Material;
class MaterialColor;
class Mirror;
class Player;
class Property;
class Rotation;
class Vec3;

template <typename T>
class AbstractProperty;

template <typename T>
class TypedBoxed;

enum EBlockSupportType
{
    BLOCK_SUPPORT_CENTER = 0,
    BLOCK_SUPPORT_EDGE = 1,
    BLOCK_SUPPORT_ANY = 2
};

enum EPistonPushReaction
{
    PISTON_PUSH_NORMAL = 0,
    PISTON_PUSH_DESTROY = 1,
    PISTON_PUSH_BLOCK = 2,
    PISTON_PUSH_IGNORE = 3,
    PISTON_PUSH_PUSH_ONLY = 4
};

enum EBlockFaceShape
{
    BLOCK_FACE_SOLID = 0,
    BLOCK_FACE_BOWL = 1,
    BLOCK_FACE_CENTER_SMALL = 2,
    BLOCK_FACE_MIDDLE_POLE_THIN = 3,
    BLOCK_FACE_CENTER = 4,
    BLOCK_FACE_MIDDLE_POLE_THICK = 5,
    BLOCK_FACE_UNDEFINED = 8
};

class BlockState
{
public:
    ~BlockState();

    virtual Material* getMaterial() const = 0;
    virtual bool isSolid() const = 0;
    virtual bool isValidSpawn(boost::shared_ptr<Entity> entity) const = 0;
    virtual int getLightBlock() const = 0;
    virtual int getLightEmission() const = 0;
    virtual bool isTranslucent() const = 0;
    virtual bool isTransparent() const = 0;
    virtual bool doesPropagate() const = 0;
    virtual const MaterialColor* getMapColor(LevelSource* levelSource, const BlockPos& pos) = 0;
    virtual const BlockState* rotate(Rotation* rotation) const = 0;
    virtual const BlockState* mirror(Mirror* mirror) const = 0;
    virtual bool isCubeShaped() const = 0;
    virtual bool hasCustomBreakingProgress() const = 0;
    virtual int getRenderShape() const = 0;
    virtual int getLightColor(LevelSource* levelSource, const BlockPos& pos) const = 0;
    virtual float getShadeBrightness() const = 0;
    virtual bool isSolidBlockingCube() const = 0;
    virtual bool isSolidBlockingCubeAndNotSignalSource() const = 0;
    virtual bool isSolidBlockingBlock() const = 0;
    virtual bool canProvideSupport(const Direction* direction, EBlockSupportType supportType) const = 0;
    virtual bool isSignalSource() const = 0;
    virtual int getSignal(LevelSource* levelSource, const BlockPos& pos, const Direction* direction) const = 0;
    virtual bool hasAnalogOutputSignal() const = 0;
    virtual int getAnalogOutputSignal(Level* level, const BlockPos& pos) const = 0;
    virtual float getDestroySpeed(Level* level, const BlockPos& pos) const = 0;
    virtual float getDestroyProgress(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos) const = 0;
    virtual int getDirectSignal(LevelSource* levelSource, const BlockPos& pos, const Direction* direction) const = 0;
    virtual EPistonPushReaction getPistonPushReaction() const = 0;
    virtual const BlockState* fillVirtualBlockStateProperties(LevelSource* levelSource, const BlockPos& pos) const = 0;
    virtual const AABB* getOutlineAABB(Level* level, const BlockPos& pos) const = 0;
    virtual bool shouldRenderFace(LevelSource* levelSource, const BlockPos& pos, const Direction* direction) const = 0;
    virtual bool isSolidRender() const = 0;
    virtual const AABB* getClipAABB(LevelSource* levelSource, const BlockPos& pos) const = 0;
    virtual void addCollissionAABBs(Level* level, const BlockPos& pos, const AABB* entityBox, std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity, bool isActualState) const = 0;
    virtual const AABB* getShape(LevelSource* levelSource, const BlockPos& pos) const = 0;
    virtual HitResult* clip(Level* level, const BlockPos& pos, Vec3* from, Vec3* to) const = 0;
    virtual bool isTopSolidBlocking() const = 0;
    virtual Vec3* getOffset(LevelSource* levelSource, const BlockPos& pos) const = 0;
    virtual bool isViewBlocking() const = 0;
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockPos& pos, const Direction* direction) const = 0;
    virtual const std::vector<const Property*>& getProperties() const = 0;
    virtual bool hasProperty(const Property* property) const = 0;
    virtual Boxed* getBoxedValue(const Property* property) const = 0;
    virtual const BlockState* setBoxedValue(const Property* property, Boxed* value) const = 0;
    virtual const BlockState* cycle(const Property* property) const = 0;
    virtual boost::unordered_map<const Property*, Boxed*>* getValues() const = 0;
    virtual Block* getBlock() const = 0;
    virtual std::wstring toString() const = 0;
    virtual bool equals(const BlockState* other) = 0;
    virtual int hashCode() const = 0;

    template <typename T>
    T getValue(const AbstractProperty<T>* property) const;

    int mElementId;
};

template <typename T>
T BlockState::getValue(const AbstractProperty<T>* property) const
{
    TypedBoxed<T>* boxed = static_cast<TypedBoxed<T>*>(getBoxedValue(property));
    TypedBoxed<T>* value = boxed->isA(typeid(T)) ? boxed : NULL;
    return value->getValue();
}

#endif
