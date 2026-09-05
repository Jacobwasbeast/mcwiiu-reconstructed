#ifndef GAME_ITEM_MAPITEMSAVEDDATA_HPP
#define GAME_ITEM_MAPITEMSAVEDDATA_HPP
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "game/data/SavedData.hpp"
#include "game/core/MapDecoration.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"
class BlockPos;
class CompoundTag;
class ItemInstance;
class Level;
class Player;
class ClientboundMapItemDataPacket;
class MapItemSavedData : public SavedData
{
public:
    class HoldingPlayer
    {
    public:
        HoldingPlayer(boost::shared_ptr<Player> player, MapItemSavedData* data);
        ~HoldingPlayer();
        boost::shared_ptr<ClientboundMapItemDataPacket> nextUpdatePacket(not_null_ptr<ItemInstance> item);
        void markDirty(int x, int y);
    private:
        boost::shared_ptr<Player> mPlayer;
        bool mDirty;
        int mMinX;
        int mMinY;
        int mMaxX;
        int mMaxY;
        int mTick;
        MapItemSavedData* mMapData;
    };
private:
    struct PlayerKeyHash
    {
        std::size_t operator()(const boost::shared_ptr<Player>& player) const { return (std::size_t)player.get(); }
    };
    struct PlayerKeyEq
    {
        bool operator()(const boost::shared_ptr<Player>& left, const boost::shared_ptr<Player>& right) const { return left.get() == right.get(); }
    };
public:
    MapItemSavedData(const std::wstring& id);
    virtual ~MapItemSavedData();
    void setOrigin(double x, double z, int scale);
    boost::shared_ptr<HoldingPlayer> getHoldingPlayer(boost::shared_ptr<Player> player);
    void setDirty(int x, int y);
    int fjEnumerateAxisPoint(int point);
    int fjGetDecorationId(CompoundTag* tag);
    void addDecoration(const MapDecoration::Type* type, Level* level, int id, double x, double z, double rotation, double ignored, bool visible);
    void tickCarriedBy(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item);
    void mergeInMapData(boost::shared_ptr<MapItemSavedData> data);
    std::vector<MapDecoration> fjGetAllDecorations();
    boost::shared_ptr<ClientboundMapItemDataPacket> getUpdatePacket(not_null_ptr<ItemInstance> item, Level* level, boost::shared_ptr<Player> player);
    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);
    static void addTargetDecoration(const not_null_ptr<ItemInstance>& item, const BlockPos& pos, const std::wstring& id, const MapDecoration::Type* type);
    bool fjIsCompletelyExplored() const;
    void fjRemoveDecorationByEntityId(int id);
private:
    int mXCenter;
    int mZCenter;
    unsigned char mDimension;
    bool mTrackingPosition;
    unsigned char mUnlimitedTracking;
    bool mLocked;
    unsigned char mScale;
    arrayWithLength<unsigned char> mColors;
    std::vector<boost::shared_ptr<HoldingPlayer> > mHoldingPlayers;
    boost::unordered_map<boost::shared_ptr<Player>, boost::shared_ptr<HoldingPlayer>, PlayerKeyHash, PlayerKeyEq> mCarriedBy;
    boost::unordered_map<int, MapDecoration> mDecorations;
    boost::unordered_map<int, MapDecoration> mDecorationsById;
};
#endif
