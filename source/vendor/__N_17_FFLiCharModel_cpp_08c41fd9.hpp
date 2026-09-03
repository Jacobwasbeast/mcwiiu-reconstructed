#ifndef SOURCE_VENDOR___N_17_FFLICHARMODEL_CPP_08C41FD9_HPP
#define SOURCE_VENDOR___N_17_FFLICHARMODEL_CPP_08C41FD9_HPP

#include <stdint.h>

enum FFLModelType
{
    FFLModelTypeNormal = 0,
    FFLModelTypeSpecial = 1
};

class __N_17_FFLiCharModel_cpp_08c41fd9
{
public:
    static const uint32_t *GetShapeTypeInfo(FFLModelType modelType);
};

#endif
