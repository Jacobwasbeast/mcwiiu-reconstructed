#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_LOCATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_LOCATION_HPP

#include "nn/pia/common/StationAddress.hpp"

namespace nn { namespace pia { namespace common { class String; }} }

namespace nn { namespace pia { namespace transport {

class StationLocation : public common::StationAddress
{
public:
    StationLocation();
    StationLocation(const StationLocation& other);
    virtual ~StationLocation();
    StationLocation& operator=(const StationLocation& other);
    void SetStationLocation(const StationLocation& other);
    void SetStationAddress(const common::StationAddress& address);
    bool operator==(const StationLocation& other) const;
    void GetLocationString(common::String* string) const;
    virtual unsigned int GetSerializedSize() const;
    virtual common::Result Serialize(unsigned char* buffer, unsigned int* size,
                                     unsigned int maxSize) const;
    virtual common::Result Deserialize(const unsigned char* buffer);
    virtual void Trace(unsigned long long value) const;

    int m_field10;
    int m_field14;
    int m_field18;
    int m_field1c;
    unsigned char m_field20;
    unsigned char m_field21;
    unsigned char m_field22;
    unsigned char m_field23;
    unsigned char m_field24;
    unsigned char m_field25;
    unsigned char m_field26;
    common::InetAddress m_inetAddress;
};

}}}

#endif
