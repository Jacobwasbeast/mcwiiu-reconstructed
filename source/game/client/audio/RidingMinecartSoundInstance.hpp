#ifndef GAME_CLIENT_AUDIO_RIDINGMINECARTSOUNDINSTANCE_HPP
#define GAME_CLIENT_AUDIO_RIDINGMINECARTSOUNDINSTANCE_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/audio/AbstractTickableSoundInstance.hpp"

class Minecart;
class Player;

class RidingMinecartSoundInstance : public AbstractTickableSoundInstance
{
public:
    RidingMinecartSoundInstance(boost::shared_ptr<Player> player, boost::shared_ptr<Minecart> minecart);
    RidingMinecartSoundInstance(const RidingMinecartSoundInstance& other);
    virtual ~RidingMinecartSoundInstance();

    virtual SoundInstance* getDRCClone() const;
    virtual void tick();

private:
    boost::shared_ptr<Player> player;
    boost::shared_ptr<Minecart> minecart;
};

#endif
