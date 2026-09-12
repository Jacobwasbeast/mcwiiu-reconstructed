#include "game/core/UnityClient6.hpp"

#include <new>

CTelemetryManager* telemetryManagerInstance = new (std::nothrow) CTelemetryManager();
CTelemetryManager& gTelemetryManager = *telemetryManagerInstance;
