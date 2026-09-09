#include "vendor/__N_29_transport_ProtocolManager_cpp_70a76ebe.hpp"

uint32_t g_piaProtocolTypeMask = 0;

uint32_t __N_29_transport_ProtocolManager_cpp_70a76ebe::getProtocolTypeBit(unsigned short protocolType)
{
    switch (protocolType)
    {
    case 0x14:
        return 2;
    case 0x18:
        return 4;
    case 0x1c:
        return 8;
    case 0x24:
        return 0x80000;
    case 0x28:
        return 0x100000;
    case 0x2c:
        return 0x200000;
    case 0x34:
        return 0x10;
    case 0x44:
        return 0x20000;
    case 0x54:
        return 0x40000;
    case 0x58:
        return 0x20;
    case 0x64:
        return 0x40;
    case 0x68:
        return 0x80;
    case 0x6c:
        return 0x1000;
    case 0x74:
        return 0x4000;
    case 0x78:
        return 0x8000;
    case 0x7c:
        return 0x100;
    case 0x84:
        return 0x200;
    case 0x94:
        return 0x10000;
    case 0xa4:
        return 0x400;
    default:
        return 0;
    }
}
