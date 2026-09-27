#include "game/item/ItemTransform.hpp"

#include "game/core/Mth.hpp"

ItemTransform* ItemTransform::NO_TRANSFORM =
    new ItemTransform(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 0.0f), Vector3f(1.0f, 1.0f, 1.0f), false);

ItemTransform::ItemTransform(Vector3f rotation, Vector3f translation, Vector3f scale, bool clampValues)
    : rotation(rotation), translation(translation), scale(scale)
{
    if (clampValues)
    {
        this->translation.x = Mth::clamp(translation.x * 0.0625f, -5.0f, 5.0f);
        this->translation.y = Mth::clamp(translation.y * 0.0625f, -5.0f, 5.0f);
        this->translation.z = Mth::clamp(translation.z * 0.0625f, -5.0f, 5.0f);
        this->scale.x = Mth::clamp(scale.x, -4.0f, 4.0f);
        this->scale.y = Mth::clamp(scale.y, -4.0f, 4.0f);
        this->scale.z = Mth::clamp(scale.z, -4.0f, 4.0f);
    }
}

ItemTransform* ItemTransform::deserialize(float rotationX, float rotationY, float rotationZ, float translationX,
                                          float translationY, float translationZ, float scaleX, float scaleY,
                                          float scaleZ)
{
    return new ItemTransform(Vector3f(rotationX, rotationY, rotationZ), Vector3f(translationX, translationY, translationZ),
                             Vector3f(scaleX, scaleY, scaleZ), true);
}
