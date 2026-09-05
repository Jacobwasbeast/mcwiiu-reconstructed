#ifndef GAME_CORE_COMMONMASTERGAMEMODE_HPP
#define GAME_CORE_COMMONMASTERGAMEMODE_HPP
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "game/core/CriticalSection.hpp"
#include "game/core/RoundInfo.hpp"
#include "game/data/StringTable.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/network/GameModePacket.hpp"
class AABB;
class CheckpointRuleDefinition;
class LevelGenerationOptions;
class MapGenerator;
class MiniGameDef;
class Player;
class PowerupRuleDefinition;
class TargetAreaRuleDefinition;
class Team;
class CommonMasterGameMode
{
public:
    CommonMasterGameMode();
    virtual ~CommonMasterGameMode();
    static CommonMasterGameMode* GetMasterOrClient();
    static const wchar_t* GetStateName(EGameModeState state);
    virtual void Setup(EMiniGameId miniGameId);
    virtual void Tick();
    static const MiniGameDef* GetMiniGame();
    void SetupMiniGameInstance(const MiniGameDef& definition, int variant);
    virtual void OnExitedGame();
    virtual unsigned int GetNextGameMode() const;
    virtual AABB* GetTeamArea(Team* team) const;
    virtual Team* GetTeamAtPosition(double x, double y, double z, double radius) const;
    bool HasFallenOffLayer(const double& x, const double& z);
    bool IsOnGeneratedLayer(const int& layer, const double& y);
    int GetLayer(const double& y);
    unsigned int GetLayerCount();
    AABB* GetLayerAABB(const int& layer);
    void UpdateGeneratorSeed(long long seed, float scale);
    virtual void RestartMapGenerator();
    void RemoveMapGenerator();
    void CleanUpMapGenerator();
    const std::wstring& getChosenThemeWord();
    void clearBuildOffVotes();
    std::wstring getThemeWordEnglish(int id);
    LevelGenerationOptions* getCurrentLevelGenOptions() const;
    LevelGenerationOptions* getRoundLevelGenOptions() const;
    bool isUsingTrialContent();
    unsigned int CheckpointIDtoSequenceID(int id) const;
    void setCheckpoints(const std::vector<CheckpointRuleDefinition*>& checkpoints);
    double getTotalDistance();
    std::vector<double>* getCheckpointDistances();
    virtual void ResetMapDistances();
    virtual void CacheMapDistance();
    void CheckProgress(boost::shared_ptr<Player> player, const std::vector<CheckpointRuleDefinition*>* rules, const std::vector<AABB*>* areas);
    void CheckProgress(const std::vector<boost::shared_ptr<Player> >& players);
    virtual void OnLapCompleted(const boost::shared_ptr<Player>& player);
    void CheckCheckpoints(const std::vector<boost::shared_ptr<Player> >& players);
    void CheckTargets(const std::vector<boost::shared_ptr<Player> >& players);
    void CheckPowerups(const std::vector<boost::shared_ptr<Player> >& players);
    virtual void OnCheckpointEnter(const boost::shared_ptr<Player>& player, unsigned int checkpoint, CheckpointRuleDefinition* rule);
    virtual void OnTargetEnter(const boost::shared_ptr<Player>& player, unsigned int target, TargetAreaRuleDefinition* rule);
    virtual void OnPrimaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* rule);
    virtual void OnSecondaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* rule);
    virtual void OnProgressMade(const boost::shared_ptr<Player>& player, double progress);
    virtual void setChosenThemeWordId(int id);
    template<class Rule> void CheckIntersectsNamedAreas(const std::vector<boost::shared_ptr<Player> >& players, const std::vector<Rule*>& rules, unsigned int callback);
private:
    LevelGenerationOptions* mCurrentLevelGenOptions;
    StringTable* mLocalizedStringTable;
    StringTable* mEnglishStringTable;
    RoundInfo mRoundInfo;
    unsigned int mState;
    unsigned int mNextState;
    unsigned int mChosenThemeWordId;
    unsigned int mLayerCount;
    float mGeneratorSeedScale;
    MapGenerator* mMapGenerator;
    OSFastMutex mGeneratorMutex;
    std::vector<AABB*> mTeamAreas;
    std::vector<int> mLayerIds;
    std::vector<double> mCheckpointDistances;
    std::vector<double> mLayerDistances;
    std::vector<AABB*> mLayerAreas;
    std::vector<CheckpointRuleDefinition*> mCheckpoints;
    unsigned int mCheckpointCount;
    unsigned int mCurrentCheckpoint;
    double mTotalDistance;
    bool mDistancesCached;
    bool mPrimaryTargetActive;
    bool mUsingTrialContent;
    bool mBuildOffVotes;
    unsigned char mFlag0;
    unsigned char mFlag1;
    unsigned char mFlag2;
};
#endif
