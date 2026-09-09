#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NETWORKFACTORY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NETWORKFACTORY_HPP

namespace nn
{
namespace pia
{
namespace transport
{

class NetworkFactory
{
public:
    NetworkFactory();
    virtual ~NetworkFactory();
    virtual unsigned int IsSupportedProtocol(unsigned char protocolType) const;
};

}
}
}

#endif
