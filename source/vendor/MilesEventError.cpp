#include "vendor/MilesEventError.hpp"
#include "vendor/MilesApi.hpp"
#include "vendor/MilesMem.hpp"

MilesEventErrorCallback g_milesEventErrorCallback = 0;
MilesEventErrorCallback g_milesAuditionErrorCallback = 0;

namespace {
const char* const errorMessages[] = {
    "MilesEvent: Out Of Memory (AIL_mem_alloc_lock failed).",
    "MilesEvent: Out of event system memory (pool passed to event system startup exhausted).",
    "MilesEvent: Bad string passed to SetLimits.",
    "MilesEvent: The command buffer was full.",
    "MilesEvent: Missing Variable.",
    "MilesEvent: An asset requested by an event was not found in memory.",
    "MilesEvent: An asynchronous load returned failure (file doesn't exist?).",
    "MilesEvent: Too many sounds were start in a blended sound.",
    "MilesEvent: The requested blend has not been set as it doesn't exist.",
    "MilesEvent: The enqueue buffer is full.",
    "MilesEvent: The event expected a selection that did not exist."
};
}

extern "C" void MilesSetEventErrorCallbackInternal(MilesEventErrorCallback callback)
{
    g_milesEventErrorCallback = callback;
}

extern "C" void MilesSetEventErrorCallback(MilesEventErrorCallback callback)
{
    ++AIL_debug_depth;
    if (AIL_debug_enabled && (AIL_debug_depth == 1 || AIL_debug_nested) &&
        AIL_API_background() == 0)
        AIL_debug_log("AIL_set_event_error_callback(0x%lX)", callback);
    InMilesMutex();
    MilesSetEventErrorCallbackInternal(callback);
    OutMilesMutex();
    --AIL_debug_depth;
}

extern "C" void Audition_Error(long long context, const char* text)
{
    if (g_milesAuditionErrorCallback)
        g_milesAuditionErrorCallback(context, text);
}

void ErrorHandler(ErrorType type, unsigned long long context, const char* text)
{
    const unsigned int value = static_cast<unsigned int>(type);
    if (value < 2)
        AIL_set_error(errorMessages[value]);
    else if (value >= 3 && value <= 11)
        AIL_set_error(errorMessages[value - 1]);
    if (g_milesEventErrorCallback)
        g_milesEventErrorCallback(static_cast<long long>(context), text);
    Audition_Error(static_cast<long long>(context), text);
}
