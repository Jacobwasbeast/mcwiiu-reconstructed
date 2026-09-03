#include "vendor/__N_23_FFLiDatabaseDefault_cpp_862bbafa.hpp"

#include <string.h>

void __N_23_FFLiDatabaseDefault_cpp_862bbafa::GetDefaultCreateID(FFLCreateID* createId, unsigned int index)
{
    static const unsigned char defaultCreateID[10] =
    {
        0xD0, 0x00, 0x00, 0x00, 0xEC, 0xFF, 0x82, 0xD2, 0x00, 0x00
    };
    memcpy(createId->data, defaultCreateID, sizeof(defaultCreateID));
    createId->data[3] = (unsigned char)index;
}
