#include "vendor/__N_28_FFLiDatabaseFileOfficial_cpp_a37b4972.hpp"

bool FFLiIsSameMiiID(const FFLCreateID* first, const FFLCreateID* second);

void __N_28_FFLiDatabaseFileOfficial_cpp_a37b4972::SetNullCreateID(FFLCreateID& createId)
{
    unsigned int i;
    for (i = 0; i < sizeof(createId.data); ++i)
        createId.data[i] = 0;
}

bool __N_28_FFLiDatabaseFileOfficial_cpp_a37b4972::IsExistCreateID(const FFLCreateID& createId, const FFLiDatabaseFileOfficial::AdjustRegularBuffer& buffer)
{
    unsigned int i;
    for (i = 0; i < buffer.num; ++i)
        if (FFLiIsSameMiiID(&createId, &buffer.createIds[i]))
            return true;
    return false;
}
