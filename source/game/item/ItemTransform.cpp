#include "game/item/ItemTransform.hpp"

ItemTransform::ItemTransform(Vector3f rotation, Vector3f translation, Vector3f scale, bool clampValues)
{
    this->rotation = rotation;
    this->translation = translation;
    this->scale = scale;

    if (clampValues)
    {
        float value;

        value = translation.x * 0.0625f;
        this->translation.x = value < -5.0f ? -5.0f : value > 5.0f ? 5.0f : value;
        value = translation.y * 0.0625f;
        this->translation.y = value < -5.0f ? -5.0f : value > 5.0f ? 5.0f : value;
        value = translation.z * 0.0625f;
        this->translation.z = value < -5.0f ? -5.0f : value > 5.0f ? 5.0f : value;

        value = scale.x;
        this->scale.x = value < -4.0f ? -4.0f : value > 4.0f ? 4.0f : value;
        value = scale.y;
        this->scale.y = value < -4.0f ? -4.0f : value > 4.0f ? 4.0f : value;
        value = scale.z;
        this->scale.z = value < -4.0f ? -4.0f : value > 4.0f ? 4.0f : value;
    }
}

ItemTransform* ItemTransform::deserialize(float rotationX, float rotationY, float rotationZ, float translationX, float translationY, float translationZ, float scaleX, float scaleY, float scaleZ)
{
    return new ItemTransform(Vector3f(rotationX, rotationY, rotationZ), Vector3f(translationX, translationY, translationZ), Vector3f(scaleX, scaleY, scaleZ), true);
}
