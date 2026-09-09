#include "vendor/MilesFilters.hpp"

#include <stddef.h>

FLT_INSTANCE* FLT_list = NULL;

extern "C" void FLT_init_list()
{
    FLT_list = NULL;
}
