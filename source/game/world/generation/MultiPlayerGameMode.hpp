#ifndef GAME_WORLD_GENERATION_MULTIPLAYERGAMEMODE_HPP
#define GAME_WORLD_GENERATION_MULTIPLAYERGAMEMODE_HPP

#include <string>
#include <vector>
#include <utility>
#include <cstdint>

#include <boost/shared_ptr.hpp>

#include "game/core/GameMode.hpp"
#include "game/core/ClickType.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/PlayerUID.hpp"

class ClientPacketListener;
class Direction;
class HitResult;
class Level;
class Minecraft;
class Player;
class TeamInvite;
class Tutorial;
class Vec3;
class ItemInstance;
class Entity;
class MiniGameDef;

class MultiPlayerGameMode : public GameMode
{
public:
    MultiPlayerGameMode(int playerId, ClientPacketListener* connection, int userIndex,
                        bool primaryPlayer, bool splitScreen);
    virtual ~MultiPlayerGameMode();

    GameModeConstraints* getGameModeConstraints();
    boost::shared_ptr<Player> createPlayer(Level* level);
    void initPlayer(boost::shared_ptr<Player> player);
    void adjustPlayer(boost::shared_ptr<Player> player);
    void ensureHasSentCarriedItem();
    bool _useItemOn(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos,
                    const Direction* direction, Vec3* hit, InteractionHand::EInteractionHand hand,
                    bool simulate, bool* used);
    bool GetTutorialsEnabled();
    InteractionResult::EInteractionResult useItemOn(boost::shared_ptr<Player> player, Level* level,
                                                     const BlockPos& pos, const Direction* direction,
                                                     Vec3* hit, InteractionHand::EInteractionHand hand,
                                                     bool simulate, bool* used);
    bool hasMissTime();
    void attack(boost::shared_ptr<Player> player, boost::shared_ptr<Entity> target);
    InteractionResult::EInteractionResult interact(boost::shared_ptr<Player> player,
                                                    boost::shared_ptr<Entity> target,
                                                    not_null_ptr<ItemInstance> item,
                                                    InteractionHand::EInteractionHand hand);
    InteractionResult::EInteractionResult interactAt(boost::shared_ptr<Player> player,
                                                      boost::shared_ptr<Entity> target,
                                                      const HitResult* hit,
                                                      not_null_ptr<ItemInstance> item,
                                                      InteractionHand::EInteractionHand hand);
    bool hasInfiniteItems();
    void handlePickItem(const boost::shared_ptr<ItemInstance>& item);
    bool isInputAllowed(int mapping);
    bool destroyBlock(const BlockPos& pos);
    static void creativeDestroyBlock(Minecraft* minecraft, MultiPlayerGameMode* mode,
                                     const BlockPos& pos, const Direction* direction);
    bool sameDestroyTarget(const BlockPos& pos);
    bool continueDestroyBlock(const BlockPos& pos, const Direction* direction);
    void stopDestroyBlock();
    Tutorial* getTutorial();
    void releaseUsingItem(boost::shared_ptr<Player> player);
    bool isServerControlledInventory();
    boost::shared_ptr<Player> GetLocalPlayer();
    bool SpectateSoundEnabled();
    double CalcSpectateSoundAllowance();
    bool CanMakeSpectateSound();
    void MakeSpectateSound();
    bool isCutScene();
    void Follow(bool follow, bool immediate);
    void StartSpectating();

    virtual bool startDestroyBlock(const BlockPos& pos, const Direction* direction);
    virtual bool canHurtPlayer();
    virtual void tick();
    virtual InteractionResult::EInteractionResult useItem(boost::shared_ptr<Player> player,
                                                            Level* level,
                                                            InteractionHand::EInteractionHand hand,
                                                            bool simulate);
    virtual void SetGameType(const GameType* gameType);
    virtual const GameType* GetGameType();

    void setLocalMode(const GameType* gameType);
    bool _destroyBlock(const BlockPos& pos);
    double getPickRange();
    boost::shared_ptr<ItemInstance> handleTakeAllInvenetory(int containerId, int slot, int button,
                                                             int mode, boost::shared_ptr<Player> player);
    boost::shared_ptr<ItemInstance> handleInventoryMouseClick(int containerId, int slot, int button,
                                                               ClickType::eClickType clickType,
                                                               boost::shared_ptr<Player> player);
    void handleInventoryButtonClick(int containerId, int button);
    void handleCreativeModeItemAdd(not_null_ptr<ItemInstance> item, int slot);
    void handleCreativeModeItemDrop(not_null_ptr<ItemInstance> item);
    bool hasExperience();
    bool hasFarPickRange();
    virtual boost::shared_ptr<ItemInstance> handleCraftItem(int category, int recipe,
                                                             boost::shared_ptr<Player> player);
    virtual boost::shared_ptr<ItemInstance> handleStackedCraftItem(
        std::vector<std::pair<int, int> >& recipes, int category, boost::shared_ptr<Player> player);
    virtual boost::shared_ptr<ItemInstance> handleCustomCraftItem(
        std::vector<int>& recipes, boost::shared_ptr<Player> player);
    void handleDebugOptions(unsigned int option, boost::shared_ptr<Player> player);

    virtual const MiniGameDef* GetMiniGame();
    virtual Level* GetLevel();
    virtual boost::shared_ptr<Player> GetPlayer();
    virtual void OnMouseDown();

    std::vector<TeamInvite*>::iterator FindTeamInvite(unsigned int teamId);
    void SyncTeamInvite(TeamInvite* invite);
    void RemoveTeamInvite(unsigned int teamId);
    std::vector<TeamInvite*>::iterator FindTeamInviteFrom(std::wstring playerName);
    bool IsTeamInvite(std::wstring playerName);
    void SendTeamInvite(std::wstring playerName);
    std::vector<TeamInvite*>::iterator FindTeamInviteTo(std::wstring playerName);
    bool CanSendInvite(std::wstring playerName, PlayerUID playerUid);
    bool CanLeaveTeam(PlayerUID playerUid);
    void AcceptTeamInvite(std::wstring playerName);
    void LeaveTeam();
    virtual bool RecordLastCheckpoint(int checkpoint, int lap, bool first, bool second, bool third);

    bool isAlwaysFlying();
    bool isDestroying();
    bool isTutorial();
    void RejectTeamInvite(std::wstring playerName);

private:
    BlockPos mDestroyBlockPos;
    not_null_ptr<ItemInstance> mDestroyItem;
    float mDestroyProgress;
    short mDestroyTicks;
    ClientPacketListener* mConnection;
    int mPlayerId;

protected:
    GameModeConstraints* mGameModeConstraints;
    Tutorial* mTutorial;

private:
    std::vector<TeamInvite*> mTeamInvites;
    float mMissTime;
    bool mTutorialsEnabled;
    bool mDestroying;
    uint64_t mSpectateSoundTime;
    int mCarriedItemSlot;
};

#endif
