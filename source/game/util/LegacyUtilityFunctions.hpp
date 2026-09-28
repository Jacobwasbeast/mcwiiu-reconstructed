#ifndef GAME_UTIL_LEGACYUTILITYFUNCTIONS_HPP
#define GAME_UTIL_LEGACYUTILITYFUNCTIONS_HPP

#include <stdint.h>

void outresfloat();
void outresint(uint32_t value);
void outresint64(uint32_t low, uint32_t high);
void outresuint(uint32_t value);
void outresuhex(uint32_t value);
int32_t ourtoupper(char value);
int OnlyNum(const char* value);

#endif
