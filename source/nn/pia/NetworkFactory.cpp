#include "nn/pia/NetworkFactory.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

NetworkFactory::NetworkFactory()
{
}

NetworkFactory::~NetworkFactory()
{
}

unsigned int NetworkFactory::IsSupportedProtocol(unsigned char protocolType) const
{
    if (protocolType < 100)
    {
        if (protocolType < 0x24)
        {
            return protocolType == 8 || protocolType == 0x14 ||
                   protocolType == 0x18 || protocolType == 0x1c;
        }
        if (protocolType < 0x44)
        {
            return protocolType == 0x24 || protocolType == 0x28 ||
                   protocolType == 0x2c || protocolType == 0x34;
        }
        return protocolType == 0x44 || protocolType == 0x54 ||
               protocolType == 0x58;
    }
    if (protocolType < 0x74)
    {
        return protocolType == 100 || protocolType == 0x68 ||
               protocolType == 0x6c;
    }
    if (protocolType < 0x84)
    {
        return protocolType == 0x74 || protocolType == 0x78 ||
               protocolType == 0x7c;
    }
    return protocolType == 0x84 || protocolType == 0x94 ||
           protocolType == 0xa4 || protocolType == 0xa8;
}

}
}
}
