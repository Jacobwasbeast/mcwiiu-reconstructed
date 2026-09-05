#ifndef SOURCE_VENDOR_N_22_FFLIRESOURCEHEADER_CPP_823CC586_HPP
#define SOURCE_VENDOR_N_22_FFLIRESOURCEHEADER_CPP_823CC586_HPP

struct FFLiResourcePartsInfo;
struct FFLiResourceHeader;
class FFLiBufferAllocator;
struct FFLExpandBuffer;
typedef int FFLResult;

class __N_22_FFLiResourceHeader_cpp_823cc586
{
public:
    static void SwapEndianResourcePartsInfo(FFLiResourcePartsInfo* parts,
                                            unsigned int count);
    static void Uncompress(void* destination, unsigned int destinationSize,
                           const void* source, unsigned int sourceSize,
                           FFLExpandBuffer* expandBuffer, int flags);
    static void ExpandPartsInfoAndData(FFLiResourcePartsInfo& destination,
                                       const FFLiResourcePartsInfo& source,
                                       const FFLiResourceHeader* sourceHeader,
                                       const FFLiResourceHeader* destinationHeader,
                                       FFLiBufferAllocator& allocator,
                                       unsigned int resourceType,
                                       FFLExpandBuffer* expandBuffer);

    template <typename T, typename U>
    static FFLResult ExpandHeaderAndData(T& destination, const T& source,
                                         const FFLiResourceHeader* sourceHeader,
                                         const FFLiResourceHeader* destinationHeader,
                                         FFLiBufferAllocator& allocator,
                                         FFLExpandBuffer* expandBuffer);
};

#endif
