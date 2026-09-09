#include "game/world/generation/GhostPlayer.hpp"

#include "game/core/GhostGameMode.hpp"

GhostPlayer::GhostPlayer(Level* level, const std::wstring& name)
    : RemotePlayer(level, name),
      mLastCheckpointId(0),
      mLapsCompleted(0),
      mProgress(0.0),
      mGameMode(new GhostGameMode(this))
{
}

GhostPlayer::~GhostPlayer()
{
    delete mGameMode;
}

int GhostPlayer::getLastCheckpointId()
{
    return mLastCheckpointId;
}

void GhostPlayer::setLastCheckpointId(int checkpointId)
{
    mLastCheckpointId = checkpointId;
}

int GhostPlayer::getLapsCompleted()
{
    return mLapsCompleted;
}

void GhostPlayer::setLapsCompleted(int lapsCompleted)
{
    mLapsCompleted = lapsCompleted;
}

double GhostPlayer::getProgress()
{
    return mProgress;
}

void GhostPlayer::setProgress(double progress)
{
    mProgress = progress;
}

GhostGameMode* GhostPlayer::GetGameMode()
{
    return mGameMode;
}

unsigned long long GhostPlayer::GetType()
{
    return 0x000000000C800004ULL;
}
