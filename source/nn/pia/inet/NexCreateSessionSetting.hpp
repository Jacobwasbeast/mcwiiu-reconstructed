#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_CREATE_SESSION_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_CREATE_SESSION_SETTING_HPP

namespace nn
{
namespace pia
{
namespace inet
{

class NexCreateSessionSetting
{
public:
    NexCreateSessionSetting();
    NexCreateSessionSetting(const NexCreateSessionSetting& other);
    virtual ~NexCreateSessionSetting();
    NexCreateSessionSetting& operator=(const NexCreateSessionSetting& other);
    void Copy(const NexCreateSessionSetting& other);
    void SetGameMode(unsigned int value);
    unsigned int GetGameMode() const;
    bool IsSetGameMode() const;
    void SetSessionType(int value);
    int GetSessionType() const;
    bool IsSetSessionType() const;
    bool IsAutoCloseSession() const;
    bool IsSetSessionMatchmakeKeyword() const;
    void SetOpenSession(bool value);
    bool IsOpenSession() const;
    bool IsSetOpenSession() const;
    bool SetAttribute(unsigned int index, unsigned int value);
    unsigned int GetAttribute(unsigned int index) const;
    bool IsSetAttribute(unsigned int index) const;
    bool SetApplicationData(const void* data, unsigned int size);
    bool GetApplicationData(void* data, unsigned int size) const;
    unsigned int GetApplicationDataSize() const;
    bool IsSetApplicationData() const;
    unsigned int GetMatchmakeSessionOption() const;
    bool IsSetMatchmakeSessionOption() const;
    unsigned char GetSelectionPriority() const;
    bool IsSetSelectionPriority() const;
    unsigned int GetRatingValue() const;
    bool IsSetRatingValue() const;
    unsigned int GetDisconnectionRate() const;
    bool IsSetDisconnectionRate() const;
    unsigned int GetViolationRate() const;
    bool IsSetViolationRate() const;
    bool GetUseGeoIp() const;
    bool IsSetUseGeoIp() const;
    unsigned int GetCountryCode() const;
    bool IsSetCountryCode() const;
    unsigned int GetExtraParticipantNum() const;
    bool IsSetExtraParticipantNum() const;
    unsigned int GetCommunityId() const;
    bool IsSetCommunityId() const;
    bool IsSetScoreBasedParameter() const;
    void Reset();

private:
    unsigned int m_gameMode;
    int m_sessionType;
    bool m_autoCloseSession;
    unsigned int m_attributes[6];
    unsigned char m_applicationData[512];
    unsigned int m_applicationDataSize;
    unsigned int m_matchmakeSessionOption;
    unsigned char m_selectionPriority;
    unsigned int m_ratingValue;
    unsigned int m_disconnectionRate;
    unsigned int m_violationRate;
    bool m_useGeoIp;
    unsigned int m_countryCode;
    unsigned int m_extraParticipantNum;
    unsigned int m_communityId;
    unsigned int m_stateFlags;
};

}
}
}

#endif
