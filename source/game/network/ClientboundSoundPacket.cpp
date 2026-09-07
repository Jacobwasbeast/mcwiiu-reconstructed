#include "game/network/ClientboundSoundPacket.hpp"

#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSoundPacket::ClientboundSoundPacket()
    : sound(NULL), source(SoundSource::eSoundSource_Master), x(0), y(0), z(0), volume(0.0f), pitch(0.0f), playUI(false), looping(false), soundInstance(-1), entityId(-1)
{
}

ClientboundSoundPacket::ClientboundSoundPacket(const SoundEvent* sound, SoundSource::ESoundSource source, double x, double y, double z, float volume, float pitch)
    : sound(sound), source(source), x((int)(x * 8.0)), y((int)(y * 8.0)), z((int)(z * 8.0)), volume(volume), pitch(pitch), playUI(false), looping(false), soundInstance(-1), entityId(-1)
{
}

ClientboundSoundPacket::ClientboundSoundPacket(const SoundEvent* sound, float volume, float pitch)
    : sound(sound), source(SoundSource::eSoundSource_Master), x(0), y(0), z(0), volume(volume), pitch(pitch), playUI(true), looping(false), soundInstance(-1), entityId(-1)
{
}

ClientboundSoundPacket::ClientboundSoundPacket(ESoundInstances soundInstance, const SoundEvent* sound, float volume, float pitch, bool looping, int entityId)
    : sound(sound), source(SoundSource::eSoundSource_Master), x(0), y(0), z(0), volume(volume), pitch(pitch), playUI(false), looping(looping), soundInstance((int)soundInstance), entityId(entityId)
{
}

ClientboundSoundPacket::~ClientboundSoundPacket()
{
}

void ClientboundSoundPacket::read(DataInputStream* input)
{
    int id = input->readVarInt();
    sound = SoundEvents::soundEventRegistry.byId(id);
    source = (SoundSource::ESoundSource)input->readVarInt();
    x = input->readInt();
    y = input->readInt();
    z = input->readInt();
    volume = input->readFloat();
    pitch = input->readFloat();
    playUI = input->readBoolean();
    soundInstance = input->readInt();
    looping = input->readBoolean();
    entityId = input->readInt();
}

void ClientboundSoundPacket::write(DataOutputStream* output)
{
    output->writeVarInt(sound == NULL ? -1 : sound->getIndex());
    output->writeVarInt((int)source);
    output->writeInt(x);
    output->writeInt(y);
    output->writeInt(z);
    output->writeFloat(volume);
    output->writeFloat(pitch);
    output->writeBoolean(playUI);
    output->writeInt(soundInstance);
    output->writeBoolean(looping);
    output->writeInt(entityId);
}

const SoundEvent* ClientboundSoundPacket::getSound() { return sound; }
SoundSource::ESoundSource ClientboundSoundPacket::getSource() { return source; }
double ClientboundSoundPacket::getX() { return (double)x * 0.125; }
double ClientboundSoundPacket::getY() { return (double)y * 0.125; }
double ClientboundSoundPacket::getZ() { return (double)z * 0.125; }
double ClientboundSoundPacket::getVolume() { return (double)volume; }
double ClientboundSoundPacket::getPitch() { return (double)pitch; }
bool ClientboundSoundPacket::getPlayUI() { return playUI; }
int ClientboundSoundPacket::getEntityId() { return entityId; }
bool ClientboundSoundPacket::getLooping() const { return looping; }
int ClientboundSoundPacket::getSoundInstance() { return soundInstance; }

void ClientboundSoundPacket::handle(PacketListener* listener)
{
    listener->handleSoundEvent(shared_from_this());
}

int ClientboundSoundPacket::getEstimatedSize() { return 24; }
int ClientboundSoundPacket::getPacketId() { return 62; }

boost::shared_ptr<Packet> ClientboundSoundPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundSoundPacket());
}
