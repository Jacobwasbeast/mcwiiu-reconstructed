#include "game/network/FlexiblePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"

FlexiblePacket::FlexiblePacket()
{
    strings = std::vector<std::wstring>();
    ints = std::vector<int>();
    bools = std::vector<bool>();
    floats = std::vector<float>();
}

void FlexiblePacket::read(DataInputStream* input)
{
    unsigned int header = input->readInt();
    int stringCount = header & 0xFF;
    int intCount = (header >> 8) & 0xFFF;
    int boolCount = (header >> 20) & 0xFF;
    int floatCount = header >> 28;

    for (int i = 0; i < stringCount; ++i)
    {
        strings.push_back(Packet::readUtf(input, 123));
    }

    for (int i = 0; i < intCount; ++i)
    {
        ints.push_back(input->readVarInt());
    }

    if (boolCount > 0)
    {
        unsigned int bits = input->readByte();

        for (int i = 0; i < boolCount; ++i)
        {
            bools.push_back(((bits >> i) & 1) != 0);
        }
    }

    for (int i = 0; i < floatCount; ++i)
    {
        floats.push_back(input->readFloat());
    }
}

void FlexiblePacket::write(DataOutputStream* output)
{
    unsigned int header = strings.size() & 0xFF;
    header |= (ints.size() & 0xFFF) << 8;
    int floatCount = floats.size();
    header |= static_cast<unsigned char>(bools.size()) << 20;
    header |= floatCount << 28;
    output->writeInt(header);

    for (unsigned int i = 0; i < strings.size(); ++i)
    {
        Packet::writeUtf(strings[i], output);
    }

    for (unsigned int i = 0; i < ints.size(); ++i)
    {
        output->writeVarInt(ints[i]);
    }

    if (!bools.empty())
    {
        unsigned char bits = 0;

        for (unsigned int i = 0; i < bools.size(); ++i)
        {
            std::vector<bool>::const_iterator bit = bools.begin();
            bit += i;

            if (*bit)
            {
                bits |= (1 << i);
            }
        }

        output->writeByte(bits);
    }

    for (unsigned int i = 0; i < floats.size(); ++i)
    {
        output->writeFloat(floats[i]);
    }
}

int FlexiblePacket::getEstimatedSize()
{
    int size = 0;

    for (unsigned int i = 0; i < strings.size(); ++i)
    {
        size += strings[i].size();
    }

    size += ints.size() * 4;
    size += !bools.empty();
    size += floats.size() * 4;

    return size + 4;
}
