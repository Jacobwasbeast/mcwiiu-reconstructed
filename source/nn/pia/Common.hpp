#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_HPP

#include <stdint.h>

namespace nn
{
namespace pia
{
namespace common
{

typedef int Result;

Result Initialize(void* workMemory, unsigned int workMemorySize);
Result BeginSetup();
Result EndSetup();
void Finalize();
unsigned char IsInitialized();
unsigned char IsInSetupMode();
void EmbedModuleInfo();

void serializeU16(unsigned char* buffer, unsigned short value);
void serializeU32(unsigned char* buffer, unsigned int value);
unsigned short deserializeU16(const unsigned char* buffer);
unsigned int deserializeU32(const unsigned char* buffer);

}
}
}

#endif
