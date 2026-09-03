#include "vendor/__N_16_FFLiFsClient_cpp_c2849364.hpp"

extern "C" int FSGetLastError();
extern "C" void OSReport(const char *message, ...);

void __N_16_FFLiFsClient_cpp_c2849364::StateChangeCallback(FSClient *client, FSVolumeState state, void *context)
{
    OSReport("client = %p state = %u context = %p lastError = %d\n", client, state, context, FSGetLastError());
}
