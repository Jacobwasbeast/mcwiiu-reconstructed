#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETDISCONNECTSTATIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETDISCONNECTSTATIONJOB_HPP
namespace nn { namespace pia { namespace transport { class Station; } namespace inet { class InetDisconnectStationJob { public: InetDisconnectStationJob(); virtual ~InetDisconnectStationJob(); void Trace(unsigned long long) const; void ProcessDisconnection(transport::Station*); }; }}}
#endif
