#include "nn/pia/lan/LanHostMessage.hpp"
#include <cstring>

namespace nn { namespace pia { namespace lan {

LanHostMessage::LanHostMessage() : LanMessage(), messageType(4), sessionId(0), connectionInfo() {}
LanHostMessage::LanHostMessage(unsigned int value, const transport::StationConnectionInfo& info) : LanMessage(), messageType(4), sessionId(value), connectionInfo(info) {}
LanHostMessage::~LanHostMessage() {}
void LanHostMessage::Serialize(unsigned char* buffer, unsigned int* size) const { if (buffer != 0) { std::memset(buffer, 0, GetSerializedSize()); buffer[3] = static_cast<unsigned char>(messageType); buffer[12] = static_cast<unsigned char>(sessionId >> 24); buffer[13] = static_cast<unsigned char>(sessionId >> 16); buffer[14] = static_cast<unsigned char>(sessionId >> 8); buffer[15] = static_cast<unsigned char>(sessionId); unsigned int connectionSize = 0; connectionInfo.Serialize(buffer + 16, &connectionSize, static_cast<unsigned int>(GetSerializedSize() - 16)); } if (size != 0) *size = static_cast<unsigned int>(GetSerializedSize()); }
void LanHostMessage::Deserialize(const unsigned char* buffer) { if (buffer != 0) { sessionId = (static_cast<unsigned int>(buffer[12]) << 24) | (static_cast<unsigned int>(buffer[13]) << 16) | (static_cast<unsigned int>(buffer[14]) << 8) | static_cast<unsigned int>(buffer[15]); connectionInfo.Deserialize(buffer + 16); } }
unsigned int LanHostMessage::GetSerializedSize() const { return 0x90; }

}}}
