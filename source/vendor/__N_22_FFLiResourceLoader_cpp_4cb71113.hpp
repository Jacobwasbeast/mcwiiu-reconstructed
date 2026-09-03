#ifndef SOURCE_VENDOR_N_22_FFLIRESOURCELOADER_CPP_4CB71113_HPP
#define SOURCE_VENDOR_N_22_FFLIRESOURCELOADER_CPP_4CB71113_HPP

#include "platform/wiiu/FFLiResourcePartsInfo.hpp"
#include "platform/wiiu/FFLiResourceUncompressBuffer.hpp"

class __N_22_FFLiResourceLoader_cpp_4cb71113
{
public:
    static bool Uncompress(void* destination, const void* source,
                           FFLiResourceUncompressBuffer* buffer,
                           const FFLiResourcePartsInfo& parts);
};

#endif
