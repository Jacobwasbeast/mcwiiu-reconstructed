#include "nn/pia/session/StationIdStatusTable.hpp"

namespace nn { namespace pia { namespace session {

StationIdStatusTable::StationIdStatusTable() : m_elements(), m_capacity(0) {}
StationIdStatusTable::~StationIdStatusTable() { Finalize(); }
void StationIdStatusTable::Finalize() { m_elements.clear(); m_capacity = 0; }
Result StationIdStatusTable::Initialize(unsigned short stationNum) { Finalize(); m_capacity = stationNum; m_elements.reserve(stationNum); return 0; }
void StationIdStatusTable::ClearAllElements() { m_elements.clear(); }
StationIdStatusTable::Element* StationIdStatusTable::FindElement(unsigned long long id) { for (std::vector<Element>::iterator i=m_elements.begin(); i!=m_elements.end(); ++i) if (i->stationId==id) return &*i; return 0; }
const StationIdStatusTable::Element* StationIdStatusTable::find(unsigned long long id) const { for (std::vector<Element>::const_iterator i=m_elements.begin(); i!=m_elements.end(); ++i) if (i->stationId==id) return &*i; return 0; }
Result StationIdStatusTable::AddElement(unsigned long long id, unsigned int session, PrepareStatus status, bool host) { if (FindElement(id)!=0 || (m_capacity!=0 && m_elements.size()>=m_capacity)) return static_cast<Result>(0x10c08); Element e; e.stationId=id; e.sessionId=session; e.prepare=status; e.host=host; m_elements.push_back(e); return 0; }
Result StationIdStatusTable::RemoveElement(unsigned long long id) { for (std::vector<Element>::iterator i=m_elements.begin(); i!=m_elements.end(); ++i) if(i->stationId==id){m_elements.erase(i);return 0;} return static_cast<Result>(0x2c0a); }
void StationIdStatusTable::ClearAllPrepareStatus(){for(std::vector<Element>::iterator i=m_elements.begin();i!=m_elements.end();++i)i->prepare=cPrepareStatusNone;}
void StationIdStatusTable::SetPrepareStatus(unsigned long long id,bool v){Element* e=FindElement(id);if(e)e->prepare=v?cPrepareStatusPrepared:cPrepareStatusNone;}
bool StationIdStatusTable::CheckAllPrepareStatus(unsigned long long* ids,unsigned short n) const {for(unsigned short i=0;i<n;++i)if(!CheckPrepareStatus(ids[i]))return false;return true;}
bool StationIdStatusTable::CheckPrepareStatus(unsigned long long id) const {const Element*e=find(id);return e!=0&&e->prepare!=cPrepareStatusNone;}
void StationIdStatusTable::ClearAllConnectStatus(){for(std::vector<Element>::iterator i=m_elements.begin();i!=m_elements.end();++i)i->connect=false;}
void StationIdStatusTable::SetConnect(unsigned long long id,bool v){Element*e=FindElement(id);if(e)e->connect=v;}
bool StationIdStatusTable::CheckAllConnectStatus(unsigned long long* ids,unsigned short n) const {for(unsigned short i=0;i<n;++i)if(!CheckConnectStatus(ids[i]))return false;return true;}
bool StationIdStatusTable::CheckConnectStatus(unsigned long long id) const {const Element*e=find(id);return e!=0&&e->connect;}
void StationIdStatusTable::SetAnswer(unsigned long long id,bool v){Element*e=FindElement(id);if(e)e->answer=v;}
bool StationIdStatusTable::CheckAnswerStatus(unsigned long long id){const Element*e=FindElement(id);return e!=0&&e->answer;}
void StationIdStatusTable::ClearAllAnswerStatus(){for(std::vector<Element>::iterator i=m_elements.begin();i!=m_elements.end();++i)i->answer=false;}
void StationIdStatusTable::SetSendInvitationStatus(unsigned long long id,bool v){Element*e=FindElement(id);if(e)e->invitation=v;}
bool StationIdStatusTable::CheckSendInvitationStatus(unsigned long long id)const{const Element*e=find(id);return e!=0&&e->invitation;}
void StationIdStatusTable::ClearAllSendInvitationStatus(){for(std::vector<Element>::iterator i=m_elements.begin();i!=m_elements.end();++i)i->invitation=false;}
void StationIdStatusTable::SetMigrationCompleteStatus(unsigned long long id,bool v){Element*e=FindElement(id);if(e)e->migration=v;}
bool StationIdStatusTable::CheckMigrationCompleteStatus(unsigned long long id)const{const Element*e=find(id);return e!=0&&e->migration;}
void StationIdStatusTable::ClearAllMigrationCompleteStatus(){for(std::vector<Element>::iterator i=m_elements.begin();i!=m_elements.end();++i)i->migration=false;}
void StationIdStatusTable::SetEventNotifiedStatus(unsigned long long id,bool v){Element*e=FindElement(id);if(e)e->eventNotified=v;}
bool StationIdStatusTable::CheckEventNotifiedStatus(unsigned long long id)const{const Element*e=find(id);return e!=0&&e->eventNotified;}
void StationIdStatusTable::CallJoinEventNotYetNotified(){for(std::vector<Element>::iterator i=m_elements.begin();i!=m_elements.end();++i)i->eventNotified=true;}
void StationIdStatusTable::ClearDisconnectedStationId(){for(std::vector<Element>::iterator i=m_elements.begin();i!=m_elements.end();)if(i->stationIndex==static_cast<StationIndex>(0xfd))i=m_elements.erase(i);else ++i;}
bool StationIdStatusTable::DisconnectionEventListenerForJointSession(void*)const{return false;}
bool StationIdStatusTable::GetSessionId(unsigned long long id,unsigned int*out)const{const Element*e=find(id);if(!e)return false;if(out)*out=e->sessionId;return true;}
void StationIdStatusTable::SetSessionHost(unsigned long long id,bool v){Element*e=FindElement(id);if(e)e->host=v;}
bool StationIdStatusTable::GetSessionHost(unsigned long long id,bool*out)const{const Element*e=find(id);if(!e)return false;if(out)*out=e->host;return true;}
unsigned int StationIdStatusTable::GetStationIdNum(unsigned int session)const{unsigned int n=0;for(std::vector<Element>::const_iterator i=m_elements.begin();i!=m_elements.end();++i)if(i->sessionId==session)++n;return n;}
unsigned int StationIdStatusTable::GetSessionIdRank(unsigned int session)const{unsigned int n=0;for(std::vector<Element>::const_iterator i=m_elements.begin();i!=m_elements.end();++i)if(i->sessionId<session)++n;return n;}
unsigned int StationIdStatusTable::GetSessionIdArray(unsigned int*out)const{if(out)for(unsigned int i=0;i<m_elements.size();++i)out[i]=m_elements[i].sessionId;return static_cast<unsigned int>(m_elements.size());}
void StationIdStatusTable::SetStationIndex(unsigned long long id,StationIndex index){Element*e=FindElement(id);if(e)e->stationIndex=index;}
bool StationIdStatusTable::GetStationIndex(unsigned long long id,StationIndex*out)const{const Element*e=find(id);if(!e)return false;if(out)*out=e->stationIndex;return true;}
void StationIdStatusTable::Trace(unsigned long long)const{}
unsigned int StationIdStatusTable::GetValidStationIdNum()const{return static_cast<unsigned int>(m_elements.size());}
void StationIdStatusTable::SetSessionId(unsigned long long id,unsigned int session){Element*e=FindElement(id);if(e)e->sessionId=session;}
bool StationIdStatusTable::CheckSessionHost(unsigned int session)const{for(std::vector<Element>::const_iterator i=m_elements.begin();i!=m_elements.end();++i)if(i->sessionId==session&&i->host)return true;return false;}

}}}
