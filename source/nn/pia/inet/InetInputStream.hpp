#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETINPUTSTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETINPUTSTREAM_HPP
namespace nn { namespace pia { namespace common { class Packet; class InetAddress; } namespace inet { class InetInputStream { public: InetInputStream(); virtual ~InetInputStream(); int readFromSocket(common::Packet*); int Read(common::Packet*); int CalculateCryptoParameter(unsigned char*, unsigned int, common::Packet*); static void AddIgnoreAddress(const common::InetAddress&, const common::InetAddress&); }; }}}
#endif
