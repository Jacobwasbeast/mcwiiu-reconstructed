#ifndef GAME_NETWORK_FLEXIBLEPACKET_HPP
#define GAME_NETWORK_FLEXIBLEPACKET_HPP

#include <string>
#include <vector>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;

class FlexiblePacket : public Packet
{
public:
    FlexiblePacket();
    virtual ~FlexiblePacket();

    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();

protected:
    std::vector<std::wstring> strings;
    std::vector<int> ints;
    std::vector<bool> bools;
    std::vector<float> floats;
};

#endif
