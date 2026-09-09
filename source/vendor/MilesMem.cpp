#include "vendor/MilesMem.hpp"

#include <stddef.h>

#include "vendor/MilesApi.hpp"
#include "vendor/MilesAsync.hpp"

extern "C" void AIL_debug_log(const char* pFormat, ...);

extern "C" unsigned long long rrGetTime();
extern "C" unsigned long long rrTimeToMillis(unsigned long long uTime);

static const char MILES_DEBUG_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\mssdbg.c";

int AIL_debug_enabled = 0;
int AIL_debug_nested = 0;
int AIL_debug_depth = 0;

MilesProfileSite MSS_alloc_site;
MilesProfileSite MSS_free_site;

static int ShouldLogAllocation()
{
    if (AIL_debug_enabled == 0)
    {
        return 0;
    }

    if (AIL_debug_depth != 1 && AIL_debug_nested == 0)
    {
        return 0;
    }

    return AIL_API_background() == 0;
}

extern "C" void* AIL_mem_alloc_lock_info(int iSize, const char* pOwnerName, int iOwnerLine)
{
    void* pBlock;
    unsigned int uCallStack;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (ShouldLogAllocation() != 0)
    {
        AIL_debug_log("@AIL_mem_alloc_lock(%ld) from: %s, line: %d ", iSize, pOwnerName,
                      iOwnerLine);
    }

    pBlock = AIL_API_mem_alloc_lock((unsigned int)iSize);

    if (MSS_profile != NULL)
    {
        uCallStack = MSS_profile->pCallStack(MSS_profile, 0, 0);
        MSS_profile->pAlloc(MSS_profile, pBlock, 0, iSize, pOwnerName, iOwnerLine, &MSS_alloc_site,
                            "Miles alloc: %t", uCallStack);
    }

    if (ShouldLogAllocation() != 0)
    {
        outresuhex((unsigned int)pBlock);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return pBlock;
}

extern "C" void AIL_mem_free_lock(void* pBlock)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (ShouldLogAllocation() != 0)
    {
        AIL_debug_log("@AIL_mem_free_lock(0x%lX)", pBlock);
    }

    if (MSS_profile != NULL)
    {
        MSS_profile->pFree(MSS_profile, pBlock, MILES_DEBUG_FILE, 440, &MSS_free_site);
    }

    AIL_API_mem_free_lock(pBlock);

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void outresuhex(unsigned int uResult)
{
    AIL_debug_log("!-Result = 0x%lX", uResult);
}

extern "C" int AIL_ms_count()
{
    return (int)rrTimeToMillis(rrGetTime());
}
