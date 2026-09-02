#ifndef GAME_WORLD_GENERATION_MINIGAMEPLAYERSETTINGS_HPP
#define GAME_WORLD_GENERATION_MINIGAMEPLAYERSETTINGS_HPP

#include "game/core/Abilities.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/maybe.hpp"

class MiniGamePlayerSettings : public GameModeOptions
{
public:
    MiniGamePlayerSettings();
    ~MiniGamePlayerSettings();

    bool operator==(const MiniGamePlayerSettings& other) const;

    Abilities mPlayerAbilities;
    unsigned int mLivesPerRound;
    unsigned int mLootSetId;
    bool mRandomLootSet;
    unsigned int mTeamCount;
    unsigned int mTeamMaxSize;
    bool mTeamBasedSpawn;
    bool mCheckpointBasedSpawn;
    bool mShowNametags;
    bool mTakeEverythingEnabled;
    bool mChestRefillEnabled;
    maybe<bool> mCanTntDestroyBlocks;
    maybe<int> mMapSizeOverride;
    bool mRandomMapSize;
    maybe<float> mArenaSizeMultiplier;
    maybe<unsigned int> mLayerCount;
    int mRoundCount;
    bool mSmallInventoryEnabled;
    bool mNaturalRegeneration;
    int mFoodBalance;
    bool mSmallSkinsEnabled;
    int mRoundLength;
    int mMaxPlayers;
    bool mFriendlyFire;
    bool mTeamGame;
    bool mSpectatorParticipation;
};

#endif
