#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETCREATEMESHJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETCREATEMESHJOB_HPP
namespace nn { namespace pia { namespace inet { class InetCreateMeshJob { public: InetCreateMeshJob(); virtual ~InetCreateMeshJob(); unsigned int StartupImpl(); void SetHostInfoToMonitoringDataImpl(); void SetNextStepAtSetupLocalPlayerInfoImpl(); unsigned int StartMonitoringServerAddressResolution(); void CleanupImpl(); }; }}}
#endif
