#ifndef GAME_WORLD_BLOCK_QUARTZBLOCK_HPP
#define GAME_WORLD_BLOCK_QUARTZBLOCK_HPP

#include "game/world/block/Block.hpp"
#include "game/core/Direction.hpp"
#include <string>

class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class MaterialColor;
class Rotation;
class TextureAtlasSprite;
class LivingEntity;
template <class T> class EnumProperty;

class QuartzBlock : public Block
{
public:
    class Variant : public StringRepresentable
    {
    public:
        Variant(int data, const std::wstring& name, const std::wstring& serializedName);
        int getData();
        static Variant* byData(int data);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring getName();
        static Variant* DEFAULT;
        static Variant* CHISELED;
        static Variant* LINES;
        static Variant* LINES_X;
        static Variant* LINES_Y;
    private:
        int mData;
        std::wstring mName;
        std::wstring mSerializedName;
    };
    QuartzBlock();
    virtual ~QuartzBlock();
    static void blockStaticCtor();
    virtual const BlockState* getStateForPlacement(Level*, const BlockPos&, const Direction*, float, float, float, int, boost::shared_ptr<LivingEntity>);
    virtual int getSpawnResourcesAuxValue(const BlockState*);
    virtual not_null_ptr<ItemInstance> getSilkTouchItemInstance(const BlockState*);
    virtual const MaterialColor* getMapColor(const BlockState*, LevelSource*, const BlockPos&);
    virtual const BlockState* getBlockState(int);
    virtual int convertBlockStateToLegacyData(const BlockState*);
    virtual const BlockState* rotate(const BlockState*, Rotation*);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction*, const BlockState*);
    virtual void registerIcons(IconRegister*);
    virtual EBlockRenderShape getRenderShape(const BlockState*);
    static EnumProperty<Variant*>* VARIANT;
private:
    TextureAtlasSprite* mTextures[9];
};

#endif
