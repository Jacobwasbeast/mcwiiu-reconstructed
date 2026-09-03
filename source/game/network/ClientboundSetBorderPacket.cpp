#include "game/network/ClientboundSetBorderPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/WorldBorder.hpp"

ClientboundSetBorderPacket::ClientboundSetBorderPacket()
    : type(SET_SIZE), absoluteMaxSize(0), centerX(0.0), centerZ(0.0),
      oldSize(0.0), newSize(0.0), lerpTime(0), warningTime(0), warningBlocks(0) {
}

ClientboundSetBorderPacket::ClientboundSetBorderPacket(WorldBorder* border, Type packetType)
    : type(packetType), absoluteMaxSize(0), centerX(0.0), centerZ(0.0),
      oldSize(0.0), newSize(0.0), lerpTime(0), warningTime(0), warningBlocks(0) {
    if (border == 0) return;
    absoluteMaxSize = border->getAbsoluteMaxSize();
    centerX = border->getCenterX();
    centerZ = border->getCenterZ();
    oldSize = border->getSize();
    newSize = border->getLerpTarget();
    lerpTime = border->getLerpRemainingTime();
    warningTime = border->getWarningTime();
    warningBlocks = border->getWarningBlocks();
}

ClientboundSetBorderPacket::~ClientboundSetBorderPacket() {
}

void ClientboundSetBorderPacket::handle(PacketListener* listener) {
    listener->handleSetBorder(shared_from_this());
}

void ClientboundSetBorderPacket::read(DataInputStream* input) {
    type = static_cast<Type>(input->readVarInt());
    if (type == SET_SIZE) { newSize = input->readDouble(); }
    else if (type == LERP_SIZE) { oldSize = input->readDouble(); newSize = input->readDouble(); lerpTime = input->readVarLong(); }
    else if (type == SET_CENTER) { centerX = input->readDouble(); centerZ = input->readDouble(); }
    else if (type == INITIALIZE) { centerX = input->readDouble(); centerZ = input->readDouble(); oldSize = input->readDouble(); newSize = input->readDouble(); lerpTime = input->readVarLong(); absoluteMaxSize = input->readVarInt(); warningBlocks = input->readVarInt(); warningTime = input->readVarInt(); }
    else if (type == SET_WARNING_TIME) { warningTime = input->readVarInt(); }
    else if (type == SET_WARNING_BLOCKS) { warningBlocks = input->readVarInt(); }
}

void ClientboundSetBorderPacket::write(DataOutputStream* output) {
    output->writeVarInt(static_cast<int>(type));
    if (type == SET_SIZE) { output->writeDouble(newSize); }
    else if (type == LERP_SIZE) { output->writeDouble(oldSize); output->writeDouble(newSize); output->writeVarLong(lerpTime); }
    else if (type == SET_CENTER) { output->writeDouble(centerX); output->writeDouble(centerZ); }
    else if (type == INITIALIZE) { output->writeDouble(centerX); output->writeDouble(centerZ); output->writeDouble(oldSize); output->writeDouble(newSize); output->writeVarLong(lerpTime); output->writeVarInt(absoluteMaxSize); output->writeVarInt(warningBlocks); output->writeVarInt(warningTime); }
    else if (type == SET_WARNING_TIME) { output->writeVarInt(warningTime); }
    else if (type == SET_WARNING_BLOCKS) { output->writeVarInt(warningBlocks); }
}

void ClientboundSetBorderPacket::applyChanges(WorldBorder* border) {
    if (type == SET_SIZE) border->setSize(newSize);
    else if (type == LERP_SIZE) border->lerpSizeBetween(oldSize, newSize, lerpTime);
    else if (type == SET_CENTER) border->setCenter(centerX, centerZ);
    else if (type == INITIALIZE) { border->setCenter(centerX, centerZ); border->setSize(newSize); border->setAbsoluteMaxSize(absoluteMaxSize); border->setWarningBlocks(warningBlocks); border->setWarningTime(warningTime); }
    else if (type == SET_WARNING_TIME) border->setWarningTime(warningTime);
    else if (type == SET_WARNING_BLOCKS) border->setWarningBlocks(warningBlocks);
}

int ClientboundSetBorderPacket::getPacketId() { return 217; }

boost::shared_ptr<Packet> ClientboundSetBorderPacket::create() {
    return boost::shared_ptr<Packet>(new ClientboundSetBorderPacket());
}
