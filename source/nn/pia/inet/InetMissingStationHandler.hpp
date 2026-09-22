#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETMISSINGSTATIONHANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETMISSINGSTATIONHANDLER_HPP
namespace nn { namespace pia { namespace transport { class StationConnectionInfo; } namespace inet { class InetMissingStationHandler { public: virtual ~InetMissingStationHandler(); unsigned int Execute(transport::StationConnectionInfo*); }; }}}
#endif
