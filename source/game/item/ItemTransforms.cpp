#include <cstddef>
#include "game/item/ItemTransforms.hpp"
#include "game/item/ItemTransform.hpp"
#include "game/core/GlStateManager.hpp"

static ItemTransforms* gDefaultTransforms = NULL;
static ItemTransforms* gArmourStandTransforms = NULL;

static ItemTransform* createTransform(float rotationX, float rotationY, float rotationZ,
                                      float translationX, float translationY, float translationZ,
                                      float scaleX, float scaleY, float scaleZ)
{
    return ItemTransform::deserialize(rotationX, rotationY, rotationZ, translationX, translationY,
                                      translationZ, scaleX, scaleY, scaleZ);
}

ItemTransforms::ItemTransforms()
    : mThirdPersonLeftHand(NULL), mThirdPersonRightHand(NULL), mFirstPersonLeftHand(NULL),
      mFirstPersonRightHand(NULL), mHead(NULL), mGui(NULL), mGround(NULL), mFixed(NULL)
{
}

ItemTransforms::ItemTransforms(const ItemTransforms& other)
    : mThirdPersonLeftHand(other.mThirdPersonLeftHand),
      mThirdPersonRightHand(other.mThirdPersonRightHand),
      mFirstPersonLeftHand(other.mFirstPersonLeftHand),
      mFirstPersonRightHand(other.mFirstPersonRightHand),
      mHead(other.mHead), mGui(other.mGui), mGround(other.mGround), mFixed(other.mFixed)
{
}

ItemTransforms::ItemTransforms(ItemTransform* thirdPersonLeftHand, ItemTransform* thirdPersonRightHand,
                               ItemTransform* firstPersonLeftHand, ItemTransform* firstPersonRightHand,
                               ItemTransform* head, ItemTransform* gui, ItemTransform* ground,
                               ItemTransform* fixed)
    : mThirdPersonLeftHand(thirdPersonLeftHand), mThirdPersonRightHand(thirdPersonRightHand),
      mFirstPersonLeftHand(firstPersonLeftHand), mFirstPersonRightHand(firstPersonRightHand),
      mHead(head), mGui(gui), mGround(ground), mFixed(fixed)
{
}

ItemTransform* ItemTransforms::getTransform(TransformType type) const
{
    switch (type)
    {
    case THIRD_PERSON_RIGHT_HAND: return mThirdPersonRightHand;
    case THIRD_PERSON_LEFT_HAND: return mThirdPersonLeftHand;
    case FIRST_PERSON_RIGHT_HAND: return mFirstPersonRightHand;
    case FIRST_PERSON_LEFT_HAND: return mFirstPersonLeftHand;
    case HEAD: return mHead;
    case GUI: return mGui;
    case GROUND: return mGround;
    case FIXED: return mFixed;
    default: return NULL;
    }
}

ItemTransforms* ItemTransforms::set_thirdperson_righthand(float rotationX, float rotationY, float rotationZ,
                                                           float translationX, float translationY, float translationZ,
                                                           float scaleX, float scaleY, float scaleZ)
{
    mThirdPersonRightHand = createTransform(rotationX, rotationY, rotationZ, translationX, translationY,
                                            translationZ, scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::set_thirdperson_lefthand(float rotationX, float rotationY, float rotationZ,
                                                          float translationX, float translationY, float translationZ,
                                                          float scaleX, float scaleY, float scaleZ)
{
    mThirdPersonLeftHand = createTransform(rotationX, rotationY, rotationZ, translationX, translationY,
                                           translationZ, scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::set_thirdperson_hands(float rotationX, float rotationY, float rotationZ,
                                                       float translationX, float translationY, float translationZ,
                                                       float scaleX, float scaleY, float scaleZ)
{
    set_thirdperson_righthand(rotationX, rotationY, rotationZ, translationX, translationY, translationZ,
                              scaleX, scaleY, scaleZ);
    set_thirdperson_lefthand(rotationX, rotationY, rotationZ, translationX, translationY, translationZ,
                             scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::GetDefaultTransforms()
{
    if (gDefaultTransforms == NULL)
    {
        gDefaultTransforms = new ItemTransforms(
            createTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
            createTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
            createTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
            createTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
            createTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
            createTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
            createTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
            createTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));
    }
    return gDefaultTransforms;
}

ItemTransforms* ItemTransforms::getArmourStandTransforms(const ItemInstance* item, LivingEntity* entity)
{
    if (item == NULL || entity == NULL)
        return NULL;
    if (gArmourStandTransforms == NULL)
        gArmourStandTransforms = new ItemTransforms(*GetDefaultTransforms());
    return gArmourStandTransforms;
}

ItemTransforms* ItemTransforms::set_firstperson_lefthand(float rotationX, float rotationY, float rotationZ,
                                                          float translationX, float translationY, float translationZ,
                                                          float scaleX, float scaleY, float scaleZ)
{
    mFirstPersonLeftHand = createTransform(rotationX, rotationY, rotationZ, translationX, translationY,
                                           translationZ, scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::set_firstperson_righthand(float rotationX, float rotationY, float rotationZ,
                                                           float translationX, float translationY, float translationZ,
                                                           float scaleX, float scaleY, float scaleZ)
{
    mFirstPersonRightHand = createTransform(rotationX, rotationY, rotationZ, translationX, translationY,
                                            translationZ, scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::set_ground(float rotationX, float rotationY, float rotationZ,
                                           float translationX, float translationY, float translationZ,
                                           float scaleX, float scaleY, float scaleZ)
{
    mGround = createTransform(rotationX, rotationY, rotationZ, translationX, translationY, translationZ,
                              scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::set_fixed(float rotationX, float rotationY, float rotationZ,
                                          float translationX, float translationY, float translationZ,
                                          float scaleX, float scaleY, float scaleZ)
{
    mFixed = createTransform(rotationX, rotationY, rotationZ, translationX, translationY, translationZ,
                             scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::set_gui(float rotationX, float rotationY, float rotationZ,
                                        float translationX, float translationY, float translationZ,
                                        float scaleX, float scaleY, float scaleZ)
{
    mGui = createTransform(rotationX, rotationY, rotationZ, translationX, translationY, translationZ,
                           scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::set_head(float rotationX, float rotationY, float rotationZ,
                                         float translationX, float translationY, float translationZ,
                                         float scaleX, float scaleY, float scaleZ)
{
    mHead = createTransform(rotationX, rotationY, rotationZ, translationX, translationY, translationZ,
                            scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::set_firstperson_hands(float rotationX, float rotationY, float rotationZ,
                                                       float translationX, float translationY, float translationZ,
                                                       float scaleX, float scaleY, float scaleZ)
{
    set_firstperson_righthand(rotationX, rotationY, rotationZ, translationX, translationY, translationZ,
                              scaleX, scaleY, scaleZ);
    set_firstperson_lefthand(rotationX, rotationY, rotationZ, translationX, translationY, translationZ,
                             scaleX, scaleY, scaleZ);
    return this;
}

ItemTransforms* ItemTransforms::getTransforms(const ItemInstance* item, LivingEntity* entity)
{
    if (item == NULL)
        return NULL;
    ItemTransforms* armourStand = getArmourStandTransforms(item, entity);
    if (armourStand != NULL)
        return armourStand;
    return GetDefaultTransforms();
}

void ItemTransforms::apply(ItemTransform* transform, bool leftHand)
{
    if (transform == NULL)
        return;
    float direction = leftHand ? -1.0f : 1.0f;
    GlStateManager::translatef(direction * transform->translation.x, transform->translation.y,
                                transform->translation.z);
    float rotationY = leftHand ? -transform->rotation.y : transform->rotation.y;
    float rotationZ = leftHand ? -transform->rotation.z : transform->rotation.z;
    GlStateManager::rotatef(transform->rotation.x, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(rotationY, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(rotationZ, 0.0f, 0.0f, 1.0f);
    GlStateManager::scalef(transform->scale.x, transform->scale.y, transform->scale.z);
}
