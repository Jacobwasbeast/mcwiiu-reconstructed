#ifndef SOURCE_VENDOR_N_22_FFLIDATABASERANDOM_CPP_41291E5D_HPP
#define SOURCE_VENDOR_N_22_FFLIDATABASERANDOM_CPP_41291E5D_HPP

#include "platform/wiiu/FFLiRandomContext.hpp"

typedef unsigned char FFLAge;

class __N_22_FFLiDatabaseRandom_cpp_41291e5d
{
public:
    struct RandomPartsArray;

    static RandomPartsArray GetRandomParts(const RandomPartsArray& parts,
                                           FFLiRandomContext* randomContext);
    static unsigned int GetRandomGlassType(FFLAge age,
                                            FFLiRandomContext* randomContext);
};

#endif
