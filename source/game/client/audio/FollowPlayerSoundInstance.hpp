#ifndef GAME_CLIENT_AUDIO_FOLLOWPLAYERSOUNDINSTANCE_HPP
#define GAME_CLIENT_AUDIO_FOLLOWPLAYERSOUNDINSTANCE_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/audio/AbstractTickableSoundInstance.hpp"
#include "game/entity/mob/Entity.hpp"

class FollowPlayerSoundInstance : public AbstractTickableSoundInstance
{
public:
    FollowPlayerSoundInstance(boost::shared_ptr<Player> player, const SoundEvent* sound, float volume, float pitch, bool looping);
    FollowPlayerSoundInstance(const FollowPlayerSoundInstance& other);
    virtual ~FollowPlayerSoundInstance();

    virtual SoundInstance* getDRCClone() const;
    virtual void tick();

private:
    boost::shared_ptr<Player> player;
};

#endif
