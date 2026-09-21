#include "nn/pia/transport/StationProtocol.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace transport {

unsigned int StationProtocol::GetConnectionRequestDataSize() const
{
    return g_stationConnectionInfoTable->GetSerializedSize() + 17;
}

void StationProtocol::MakeConnectionRequestData(unsigned char* buffer, unsigned char nonce,
                                                 unsigned long long principalId,
                                                 unsigned int infoField,
                                                 unsigned char field55, bool relay,
                                                 bool secure) const
{
    buffer[0] = relay ? 6 : 1;
    buffer[1] = nonce;
    buffer[2] = 8;
    buffer[3] = secure;
    common::serializeU32(buffer + 4, static_cast<unsigned int>(principalId >> 32));
    common::serializeU32(buffer + 8, static_cast<unsigned int>(principalId));
    common::serializeU32(buffer + 12, infoField);
    buffer[16] = field55;
    unsigned int serializedSize;
    g_stationConnectionInfoTable->Serialize(buffer + 17, &serializedSize,
                                            GetConnectionRequestDataSize() - 17);
}

}}}
