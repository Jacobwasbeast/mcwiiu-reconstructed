#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_H
#define LEGACY_CONSOLE_WIIU_NN_PIA_H

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/inet/InetProcessHostMigrationJob.hpp"
#include <coreinit/fastmutex.h>


namespace nn
{
namespace pia
{
typedef int Result;
typedef short StationIndex;

namespace common
{
class CriticalSection
{
public:
    CriticalSection(int value);
    ~CriticalSection();
    void Lock();
    bool TryLock();
    void Unlock();
    void Trace(unsigned long long value) const;
private:
    OSFastMutex m_mutex;
};
}



namespace chat
{
Result Initialize();
Result BeginSetup();
Result EndSetup();
Result Finalize();
}
void Assert(const char* expression, const char* file, unsigned int line,
            const char* function, ...);
}
}

#endif
