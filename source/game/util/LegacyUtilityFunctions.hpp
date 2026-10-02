#ifndef GAME_UTIL_LEGACYUTILITYFUNCTIONS_HPP
#define GAME_UTIL_LEGACYUTILITYFUNCTIONS_HPP

#include <stdint.h>

void outresfloat();
void outresint64(uint32_t low, uint32_t high);
void outresuint(uint32_t value);
int32_t ourtoupper(char value);
int OnlyNum(const char* value);
extern "C" unsigned int find_ASI_provider(const char* attributeName, const char* fileName);
extern "C" int AIL_stricmp(const char* first, const char* second);
extern "C" int AIL_strnicmp(const char* first, const char* second, int count);

#endif
