#include "vendor/__N_17_FFLiCharModel_cpp_08c41fd9.hpp"

const uint32_t *__N_17_FFLiCharModel_cpp_08c41fd9::GetShapeTypeInfo(FFLModelType modelType)
{
    static const uint32_t shapeTypeInfo[2][4] =
    {
        { 2u, 3u, 7u, 9u },
        { 9u, 10u, 11u, 0x00070B01u }
    };
    if (modelType != FFLModelTypeSpecial)
    {
        return shapeTypeInfo[0];
    }
    return shapeTypeInfo[1];
}
