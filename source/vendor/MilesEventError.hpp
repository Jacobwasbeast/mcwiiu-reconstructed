#ifndef SOURCE_VENDOR_MILES_EVENT_ERROR_HPP
#define SOURCE_VENDOR_MILES_EVENT_ERROR_HPP

enum ErrorType : unsigned int
{
    ERROR_TYPE_OUT_OF_MEMORY = 0,
    ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY = 1,
    ERROR_TYPE_BAD_STRING_PASSED_TO_SET_LIMITS = 3,
    ERROR_TYPE_COMMAND_BUFFER_FULL = 4,
    ERROR_TYPE_MISSING_VARIABLE = 5
};

typedef void (*MilesEventErrorCallback)(long long context, const char* text);
extern MilesEventErrorCallback g_milesEventErrorCallback;
extern MilesEventErrorCallback g_milesAuditionErrorCallback;
extern "C" void MilesSetEventErrorCallback(MilesEventErrorCallback callback);
extern "C" void MilesSetEventErrorCallbackInternal(MilesEventErrorCallback callback);
extern "C" void Audition_Error(long long context, const char* text);
void ErrorHandler__F9ErrorTypeULPCc(ErrorType type, unsigned long long context, const char* text);

#endif
