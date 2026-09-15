#include "game/core/UnityClient6.hpp"

#include <cstring>
#include <new>

namespace
{
CTelemetryManager* InitializeTelemetryManager()
{
    void* storage = ::operator new(0x28, std::nothrow);
    if (storage == NULL)
        return NULL;

    std::memset(storage, 0, 0x28);
    return new (storage) CTelemetryManager();
}
}

CTelemetryManager* telemetryManagerInstance = InitializeTelemetryManager();
