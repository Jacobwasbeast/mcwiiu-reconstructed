#include "vendor/MilesMem.hpp"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "vendor/MilesApi.hpp"
#include "vendor/MilesAsync.hpp"
#include "vendor/MilesTime.hpp"

typedef void (*AIL_LOG_CALLBACK)(const char* pText, int iDepth);

static const char MILES_DEBUG_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\mssdbg.c";

static const char AIL_indent[] = "                                ";

static const int AIL_LOG_PREFIX_LENGTH = 15;
static const unsigned int AIL_LOG_MAXIMUM_HOURS = 99;

int AIL_debug_enabled = 0;
int AIL_debug_nested = 0;
int AIL_debug_depth = 0;

static int AIL_log_start_ms = 0;
static AIL_LOG_CALLBACK AIL_log_callback = NULL;
static FILE* AIL_log_file = NULL;

MilesProfileSite AIL_debug_log_site;
MilesProfileSite MSS_alloc_site;
MilesProfileSite MSS_free_site;

static int ShouldLog()
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

extern "C" void AIL_debug_log(const char* pFormat, ...)
{
    va_list arguments;
    char szTimestamp[16];
    char szBuffer[512];
    char* pText;
    char* pLine;
    const char* pMessageFormat;
    const char* pDescription;
    unsigned int uElapsed;
    unsigned int uHours;

    if (AIL_debug_enabled == 0)
    {
        return;
    }

    va_start(arguments, pFormat);

    pText = szBuffer + AIL_LOG_PREFIX_LENGTH;
    pMessageFormat = pFormat;

    if (*pFormat == '!' || *pFormat == '@')
    {
        if (AIL_debug_depth > 1)
        {
            strcpy(pText, &AIL_indent[sizeof(AIL_indent) - AIL_debug_depth * 2]);
            pText = pText + AIL_debug_depth * 2 - 1;
        }

        pMessageFormat = pFormat + 1;
    }

    vsnprintf(pText, (sizeof(szBuffer) - 1) - (size_t)(pText - szBuffer), pMessageFormat,
              arguments);
    va_end(arguments);

    szBuffer[sizeof(szBuffer) - 1] = '\0';

    if (MSS_profile != NULL)
    {
        pDescription = MSS_profile->pDescribe(MSS_profile, szBuffer + AIL_LOG_PREFIX_LENGTH);
        MSS_profile->pMessage(MSS_profile, 0, 1, &AIL_debug_log_site, "%s", pDescription);
    }

    if (AIL_log_file == NULL && AIL_log_callback == NULL && MSS_profile != NULL)
    {
        return;
    }

    uElapsed = (unsigned int)(AIL_ms_count() - AIL_log_start_ms);

    uHours = uElapsed / 3600000;
    if (uHours > AIL_LOG_MAXIMUM_HOURS)
    {
        uHours = AIL_LOG_MAXIMUM_HOURS;
    }

    pLine = szBuffer + AIL_LOG_PREFIX_LENGTH;

    if (*pFormat == '@' || *pFormat == '!')
    {
        if (*pFormat == '@' && AIL_debug_depth == 1)
        {
            sprintf(szTimestamp, "[%.02d:%.02d:%.02d.%.03d] ", (int)uHours,
                    (int)((uElapsed / 60000) % 60), (int)((uElapsed / 1000) % 60),
                    (int)(uElapsed % 1000));
            memcpy(szBuffer, szTimestamp, AIL_LOG_PREFIX_LENGTH);
        }
        else
        {
            memset(szBuffer, ' ', AIL_LOG_PREFIX_LENGTH);
        }

        pLine = szBuffer;
    }

    if (AIL_log_file != NULL)
    {
        fprintf(AIL_log_file, "%s\n", pLine);
        return;
    }

    if (AIL_log_callback != NULL)
    {
        AIL_log_callback(pLine, AIL_debug_depth);
    }
}

extern "C" void AIL_set_error(const char* pError)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (ShouldLog() != 0)
    {
        if (pError == NULL)
        {
            AIL_debug_log("@AIL_set_error(NULL)");
        }
        else
        {
            AIL_debug_log("@AIL_set_error(%s)", pError);
        }
    }

    AIL_API_set_error(pError);

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void* AIL_mem_alloc_lock_info(int iSize, const char* pOwnerName, int iOwnerLine)
{
    void* pBlock;
    unsigned int uCallStack;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (ShouldLog() != 0)
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

    if (ShouldLog() != 0)
    {
        outresuhex((unsigned int)pBlock);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return pBlock;
}

extern "C" void AIL_mem_free_lock(void* pBlock)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (ShouldLog() != 0)
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
