#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_INET_PROCESS_HOST_MIGRATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_INET_PROCESS_HOST_MIGRATION_JOB_HPP

#include "nn/pia/common/CallContext.hpp"
#include "nn/nex/CallContext.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/NotificationEventHandler.hpp"
#include "nn/nex/StationURL.hpp"
#include <list>

namespace nn
{
namespace pia
{
namespace inet
{
class InetProcessHostMigrationJob
{
public:
    InetProcessHostMigrationJob();
    virtual ~InetProcessHostMigrationJob();
    unsigned int ManualHostMigration();
    unsigned int AutomaticHostMigration();
    unsigned int InetPrepareForBecomingHost();
    unsigned int WaitAfterPrepareForBecomingHost();
    int CalcTimeToWaitOtherClients(unsigned int, unsigned int);
    unsigned int InetSendRankDecision();
    unsigned int InetWaitRankDecision();
    unsigned int InetGetMatchMakingClientHost();
    unsigned int InetWaitMatchMakingClientHost();
    unsigned int InetCheckOldHostDisconnection();
    unsigned int InetWaitCheckOldHostDisconnection();
    void StopRankDecisionResending();
    unsigned int InetPrepareForBecomingHostMulti();
    unsigned int UpdateSessionOpenStatus();
    unsigned int WaitUpdateSessionOpenStatus();
    unsigned int InetCheckMatchMakingClientHost();
    void HostChangeEventNotice(unsigned int);
    unsigned int IsFatalErrorOccur();
    void CheckWhetherNewHostIsUnknown(unsigned long long deadline);
    unsigned int InetCheckMatchMakingClientHostIsUpdated();
    unsigned int InetWaitMatchMakingClientHostIsUpdated();
    unsigned int IsValidHostMigrationSetting();
    unsigned int PreprocessForSendingMigrationFinish();
    void Trace(unsigned long long) const;
    unsigned int StartupImpl(bool, StationIndex);
    void CleanupImpl();
    unsigned int UpdateSessionHostAsync(unsigned int);
    unsigned int IsUpdateSessionHostCompleted();
    unsigned int StartupMultiImpl(bool, unsigned short);
    void CleanupStatus();
    unsigned int CheckWhetherReselectNewHost();

private:
    unsigned char m_base[0x110];
    nn::nex::CallContext* m_protocolCallContext;
    nn::pia::common::CallContext m_callContext;
    int m_waitTime;
    int m_waitCount;
    int m_waitLimit;
    unsigned char m_reserved134[4];
    int m_reselectDeadlineHigh;
    int m_reselectDeadlineLow;
    int m_newHost;
    std::list<nn::nex::StationURL, nn::nex::MemAllocator<nn::nex::StationURL> >* m_stationUrls;
    unsigned char m_stationUrlStorage[0x80];
    unsigned char m_migrationMode;
    unsigned char m_hostChanged;
    unsigned char m_reserved1ca[2];
    nn::nex::NotificationEventHandler* m_notificationHandler;
    int m_finishDeadlineHigh;
    int m_finishDeadlineLow;
    int m_updateDeadlineHigh;
    int m_updateDeadlineLow;
    int m_hostUpdateDeadlineHigh;
    int m_hostUpdateDeadlineLow;
};

}
}
}

#endif
