#include "nn/pia/inet/InetProcessHostMigrationJob.hpp"
#include "nn/pia/inet/NexFacade.hpp"
namespace nn { namespace pia { namespace inet {
InetProcessHostMigrationJob::InetProcessHostMigrationJob():m_protocolCallContext(0),m_waitTime(0),m_waitCount(0),m_waitLimit(0),m_reselectDeadlineHigh(0),m_reselectDeadlineLow(0),m_newHost(-1),m_stationUrls(0),m_migrationMode(0),m_hostChanged(0),m_notificationHandler(0),m_finishDeadlineHigh(0),m_finishDeadlineLow(0),m_updateDeadlineHigh(0),m_updateDeadlineLow(0),m_hostUpdateDeadlineHigh(0),m_hostUpdateDeadlineLow(0){}
InetProcessHostMigrationJob::~InetProcessHostMigrationJob(){CleanupImpl();}
unsigned int InetProcessHostMigrationJob::ManualHostMigration(){m_migrationMode=1;m_newHost=-1;return 1;}
unsigned int InetProcessHostMigrationJob::AutomaticHostMigration(){m_migrationMode=2;m_newHost=-1;return 1;}
unsigned int InetProcessHostMigrationJob::InetPrepareForBecomingHost(){m_newHost=0;return 1;}
unsigned int InetProcessHostMigrationJob::WaitAfterPrepareForBecomingHost(){return m_newHost>=0;}
int InetProcessHostMigrationJob::CalcTimeToWaitOtherClients(unsigned int rank,unsigned int count){if(!rank||!count)return 0;unsigned int half=count>>1;if(!half)return m_waitLimit;int step=m_waitLimit/static_cast<int>((half*(half-1))+half+(count&1));if(rank<half){int sum=0;for(unsigned int i=0;i<rank;++i)sum+=static_cast<int>((half<<1)-((i+1)<<1));return step*sum;}return m_waitLimit-step*static_cast<int>((count-rank)-1);}
unsigned int InetProcessHostMigrationJob::InetSendRankDecision(){m_migrationMode=3;m_waitCount=0;return m_migrationMode;}
unsigned int InetProcessHostMigrationJob::InetWaitRankDecision(){return m_waitCount<m_waitLimit?++m_waitCount:0;}
unsigned int InetProcessHostMigrationJob::InetGetMatchMakingClientHost(){m_migrationMode=4;return m_newHost>=0;}
unsigned int InetProcessHostMigrationJob::InetWaitMatchMakingClientHost(){if(m_newHost<0)return 0;m_migrationMode=5;return 1;}
unsigned int InetProcessHostMigrationJob::InetCheckOldHostDisconnection(){m_migrationMode=6;return m_hostChanged==0;}
unsigned int InetProcessHostMigrationJob::InetWaitCheckOldHostDisconnection(){return m_migrationMode==6&&m_hostChanged==0;}
void InetProcessHostMigrationJob::StopRankDecisionResending(){m_waitCount=0;}
unsigned int InetProcessHostMigrationJob::InetPrepareForBecomingHostMulti(){m_migrationMode=7;m_newHost=0;return m_newHost>=0;}
unsigned int InetProcessHostMigrationJob::UpdateSessionOpenStatus(){m_migrationMode=9;m_callContext.Reset();return 1;}
unsigned int InetProcessHostMigrationJob::WaitUpdateSessionOpenStatus(){return m_callContext.m_state==nn::pia::common::CallContext::cStateInProgress?0u:1u;}
unsigned int InetProcessHostMigrationJob::InetCheckMatchMakingClientHost(){m_migrationMode=8;return m_newHost>=0;}
void InetProcessHostMigrationJob::HostChangeEventNotice(unsigned int sessionId)
{
    if (NexFacade::s_instance->m_gatheringId == sessionId)
        m_hostChanged = 1;
}
unsigned int InetProcessHostMigrationJob::IsFatalErrorOccur(){return m_hostChanged&&m_newHost<0;}
unsigned long long InetProcessHostMigrationJob::CheckWhetherNewHostIsUnknown(unsigned int,int high,int low){if((high!=m_reselectDeadlineHigh||low!=m_reselectDeadlineLow)&&(high!=m_finishDeadlineHigh||low!=m_finishDeadlineLow)){m_newHost=-1;return 1;}return 0;}
unsigned int InetProcessHostMigrationJob::InetCheckMatchMakingClientHostIsUpdated(){return m_newHost>=0&&m_hostChanged==0;}
unsigned int InetProcessHostMigrationJob::InetWaitMatchMakingClientHostIsUpdated(){return m_newHost>=0&&m_hostChanged!=0;}
unsigned int InetProcessHostMigrationJob::IsValidHostMigrationSetting(){return m_migrationMode!=0;}
unsigned int InetProcessHostMigrationJob::PreprocessForSendingMigrationFinish(){m_finishDeadlineLow+=2000;return 1;}
void InetProcessHostMigrationJob::Trace(unsigned long long) const{}
unsigned int InetProcessHostMigrationJob::StartupImpl(bool multi,unsigned int station){m_migrationMode=multi?2:1;m_newHost=static_cast<int>(station);m_callContext.Reset();return 1;}
void InetProcessHostMigrationJob::CleanupImpl(){m_hostChanged=0;m_notificationHandler=0;}
unsigned int InetProcessHostMigrationJob::UpdateSessionHostAsync(unsigned int station){if(!station)return 0;m_callContext.Reset();m_newHost=static_cast<int>(station);return 1;}
unsigned int InetProcessHostMigrationJob::IsUpdateSessionHostCompleted(){if(m_protocolCallContext==0)return 1;if(m_protocolCallContext->GetState()!=nn::nex::CallContext::CallInProgress)return 1;if(m_hostUpdateDeadlineHigh<m_updateDeadlineHigh||(m_hostUpdateDeadlineHigh==m_updateDeadlineHigh&&m_hostUpdateDeadlineLow<=m_updateDeadlineLow)){m_protocolCallContext->Cancel(nn::nex::CallContext::CallCanceled);m_protocolCallContext->Reset();return 1;}return 0;}
unsigned int InetProcessHostMigrationJob::StartupMultiImpl(bool multi,unsigned short count){m_migrationMode=multi?2:1;m_waitLimit=m_waitTime-5000;if(m_waitLimit<5000)m_waitLimit=5000;m_reselectDeadlineLow=static_cast<int>(count);m_newHost=-1;return 1;}
void InetProcessHostMigrationJob::CleanupStatus(){StopRankDecisionResending();if(m_callContext.m_state==nn::pia::common::CallContext::cStateInProgress)m_callContext.SignalCancel();m_callContext.Reset();m_newHost=-1;}
unsigned int InetProcessHostMigrationJob::CheckWhetherReselectNewHost(){if(!m_migrationMode)return 0;if(m_newHost<0){m_hostChanged=0;StopRankDecisionResending();return 1;}return 0;}
}}}
