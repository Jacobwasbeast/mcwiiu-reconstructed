#ifndef GAME_UTIL_LEGACYUTILITYFUNCTIONS_HPP
#define GAME_UTIL_LEGACYUTILITYFUNCTIONS_HPP

#include <cstdint>

void outresfloat();
void outresint(std::uint32_t value);
void outresint64(std::uint32_t low, std::uint32_t high);
void outresuint(std::uint32_t value);
void outresuhex(std::uint32_t value);
std::int32_t ourtoupper(char value);
int OnlyNum(const char* value);

#endif
