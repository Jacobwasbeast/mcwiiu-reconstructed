#include "nn/nex/Stream.hpp"

namespace nn
{
namespace nex
{

Stream::~Stream()
{
}

bool Stream::IsDuplicateReorderingPacket(Packet*)
{
    return false;
}

}
}
