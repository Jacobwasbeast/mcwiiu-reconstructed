#include "game/client/renderer/entity/LivingEntityRenderer.hpp"

double hermiteBlend__Ff(float amount)
{
    float scaled = static_cast<float>(32.00000193591933 * amount);
    float quadratic = static_cast<float>(static_cast<double>(scaled) * amount);
    float doubled = amount + amount;
    float product = static_cast<float>(static_cast<double>(doubled) * amount);
    float cubic = static_cast<float>(static_cast<double>(product) * amount);
    return static_cast<double>(static_cast<float>(static_cast<double>(quadratic) - static_cast<double>(cubic)));
}
