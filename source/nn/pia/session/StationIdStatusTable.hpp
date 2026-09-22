#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_STATION_ID_STATUS_TABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_STATION_ID_STATUS_TABLE_HPP

#include <vector>
#include "nn/pia.h"

namespace nn { namespace pia { namespace session {

class StationIdStatusTable
{
public:
    enum PrepareStatus { cPrepareStatusNone = 0, cPrepareStatusPrepared = 1 };
    StationIdStatusTable();
    virtual ~StationIdStatusTable();
    void Finalize();
    Result Initialize(unsigned short stationNum);
    void ClearAllElements();
    Result AddElement(unsigned long long stationId, unsigned int sessionId, PrepareStatus status, bool host);
    Result RemoveElement(unsigned long long stationId);
    bool FindElement(unsigned long long stationId) const;
    void ClearAllPrepareStatus();
    void SetPrepareStatus(unsigned long long stationId, bool value);
    bool CheckAllPrepareStatus(unsigned long long* stationIds, unsigned short count) const;
    bool CheckPrepareStatus(unsigned long long stationId) const;
    void ClearAllConnectStatus();
    void SetConnect(unsigned long long stationId, bool value);
    bool CheckAllConnectStatus(unsigned long long* stationIds, unsigned short count) const;
    bool CheckConnectStatus(unsigned long long stationId) const;
    void SetAnswer(unsigned long long stationId, bool value);
    bool CheckAnswerStatus(unsigned long long stationId);
    void ClearAllAnswerStatus();
    void SetSendInvitationStatus(unsigned long long stationId, bool value);
    bool CheckSendInvitationStatus(unsigned long long stationId) const;
    void ClearAllSendInvitationStatus();
    void SetMigrationCompleteStatus(unsigned long long stationId, bool value);
    bool CheckMigrationCompleteStatus(unsigned long long stationId) const;
    void ClearAllMigrationCompleteStatus();
    void SetEventNotifiedStatus(unsigned long long stationId, bool value);
    bool CheckEventNotifiedStatus(unsigned long long stationId) const;
    void CallJoinEventNotYetNotified();
    void ClearDisconnectedStationId();
    bool DisconnectionEventListenerForJointSession(void* event) const;
    bool GetSessionId(unsigned long long stationId, unsigned int* sessionId) const;
    void SetSessionHost(unsigned long long stationId, bool value);
    bool GetSessionHost(unsigned long long stationId, bool* value) const;
    unsigned int GetStationIdNum(unsigned int sessionId) const;
    unsigned int GetSessionIdRank(unsigned int sessionId) const;
    unsigned int GetSessionIdArray(unsigned int* sessionIds) const;
    void SetStationIndex(unsigned long long stationId, StationIndex index);
    bool GetStationIndex(unsigned long long stationId, StationIndex* index) const;
    virtual void Trace(unsigned long long value) const;
    unsigned int GetValidStationIdNum() const;
    void SetSessionId(unsigned long long stationId, unsigned int sessionId);
    bool CheckSessionHost(unsigned int sessionId) const;

private:
    struct Element {
        unsigned long long stationId;
        unsigned int sessionId;
        PrepareStatus prepare;
        bool connect;
        bool answer;
        bool invitation;
        bool migration;
        bool eventNotified;
        bool host;
        StationIndex stationIndex;
        Element() : stationId(0), sessionId(0), prepare(cPrepareStatusNone), connect(false), answer(false), invitation(false), migration(false), eventNotified(false), host(false), stationIndex(static_cast<StationIndex>(0xfd)) {}
    };
    std::vector<Element> m_elements;
    unsigned short m_capacity;
    Element* find(unsigned long long stationId);
    const Element* find(unsigned long long stationId) const;
};

}}}

#endif
