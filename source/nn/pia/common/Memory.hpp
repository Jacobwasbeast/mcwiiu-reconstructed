#ifndef SOURCE_NN_PIA_COMMON_MEMORY_HPP
#define SOURCE_NN_PIA_COMMON_MEMORY_HPP

namespace nn { namespace pia { namespace common {

template <typename T> T* NewArray(unsigned int count);
template <typename T> void DeleteArray(T* array);

template <> unsigned char* NewArray<unsigned char>(unsigned int count);
template <> void DeleteArray<unsigned char>(unsigned char* array);

}}}

#endif
