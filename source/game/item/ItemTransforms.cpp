#include <cstddef>
#include "game/item/ItemTransforms.hpp"
#include "game/item/ItemTransform.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Item.hpp"
#include "game/core/GlStateManager.hpp"

static ItemTransforms* gDefaultTransforms = NULL;

namespace
{
enum ArmourStandTransformKind
{
    AS_NONE,
    AS_JOINED,
    AS_Y278,
    AS_Y374,
    AS_Y12C,
    AS_Y468,
    AS_Y173,
    AS_Y17A,
    AS_Y186,
    AS_Y1C4
};

ArmourStandTransformKind classifyArmourStandItem(int itemId)
{
    switch (itemId)
    {
    case 111: return AS_JOINED;
    case 263: case 264: return AS_JOINED;
    case 287: return AS_JOINED;
    case 318: case 321: return AS_JOINED;
    case 325: case 328: return AS_JOINED;
    case 334: case 335: return AS_JOINED;
    case 342: case 344: return AS_JOINED;
    case 354: return AS_JOINED;
    case 358: return AS_JOINED;
    case 363: case 364: return AS_JOINED;
    case 373: return AS_JOINED;
    case 375: case 377: return AS_JOINED;
    case 382: return AS_JOINED;
    case 388: case 389: return AS_JOINED;
    case 392: case 395: return AS_JOINED;
    case 400: return AS_JOINED;
    case 407: case 409: return AS_JOINED;
    case 415: return AS_JOINED;
    case 420: return AS_JOINED;
    case 426: return AS_JOINED;
    case 432: return AS_JOINED;
    case 434: return AS_JOINED;
    case 438: return AS_JOINED;
    case 441: return AS_JOINED;
    case 459: case 461: return AS_JOINED;
    case 288: case 289: return AS_Y278;
    case 296: return AS_Y278;
    case 329: return AS_Y278;
    case 331: case 333: return AS_Y278;
    case 337: return AS_Y278;
    case 341: return AS_Y278;
    case 345: return AS_Y278;
    case 348: case 350: return AS_Y278;
    case 353: return AS_Y278;
    case 357: return AS_Y278;
    case 367: return AS_Y278;
    case 385: return AS_Y278;
    case 406: return AS_Y278;
    case 410: case 412: return AS_Y278;
    case 433: return AS_Y278;
    case 444: case 447: return AS_Y278;
    case 265: case 266: return AS_Y374;
    case 295: return AS_Y374;
    case 336: return AS_Y374;
    case 338: return AS_Y374;
    case 351: return AS_Y374;
    case 359: return AS_Y374;
    case 361: return AS_Y374;
    case 391: return AS_Y374;
    case 396: return AS_Y374;
    case 405: return AS_Y374;
    case 281: case 282: return AS_Y12C;
    case 413: return AS_Y12C;
    case 355: return AS_Y468;
    case 362: return AS_Y468;
    case 370: return AS_Y468;
    case 372: return AS_Y468;
    case 414: return AS_Y468;
    case 417: case 419: return AS_Y468;
    case 371: return AS_Y173;
    case 378: return AS_Y17A;
    case 390: return AS_Y186;
    case 452: return AS_Y1C4;
    default: return AS_NONE;
    }
}

ItemTransforms* lazyArmourStandHands(float translationY, ItemTransforms** cache, bool* initialized)
{
    if (!*initialized)
    {
        *initialized = true;
        *cache = new ItemTransforms(*ItemTransforms::GetDefaultTransforms());
        (*cache)->set_thirdperson_hands(0.f, 0.f, 0.f, 0.f, translationY, 1.f, 0.55f, 0.55f, 0.55f);
    }
    return *cache;
}

ItemTransforms* armourStandHandsForKind(ArmourStandTransformKind kind)
{
    static ItemTransforms* joinedCache = NULL;
    static ItemTransforms* y278Cache = NULL;
    static ItemTransforms* y374Cache = NULL;
    static ItemTransforms* y12cCache = NULL;
    static ItemTransforms* y468Cache = NULL;
    static ItemTransforms* y173Cache = NULL;
    static ItemTransforms* y17aCache = NULL;
    static ItemTransforms* y186Cache = NULL;
    static ItemTransforms* y1c4Cache = NULL;
    static bool joinedInit = false;
    static bool y278Init = false;
    static bool y374Init = false;
    static bool y12cInit = false;
    static bool y468Init = false;
    static bool y173Init = false;
    static bool y17aInit = false;
    static bool y186Init = false;
    static bool y1c4Init = false;

    switch (kind)
    {
    case AS_JOINED: return lazyArmourStandHands(2.3f, &joinedCache, &joinedInit);
    case AS_Y278: return lazyArmourStandHands(1.5f, &y278Cache, &y278Init);
    case AS_Y374: return lazyArmourStandHands(-80.f, &y374Cache, &y374Init);
    case AS_Y12C: return lazyArmourStandHands(1.7f, &y12cCache, &y12cInit);
    case AS_Y468: return lazyArmourStandHands(1.75f, &y468Cache, &y468Init);
    case AS_Y173: return lazyArmourStandHands(1.75f, &y173Cache, &y173Init);
    case AS_Y17A: return lazyArmourStandHands(2.32f, &y17aCache, &y17aInit);
    case AS_Y186: return lazyArmourStandHands(1.75f, &y186Cache, &y186Init);
    case AS_Y1C4: return lazyArmourStandHands(0.65f, &y1c4Cache, &y1c4Init);
    default: return NULL;
    }
}
}

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
            createTransform(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 3.0f, 0.55f, 0.55f, 0.55f),
            createTransform(0.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 0.55f, 0.55f, 0.55f),
            createTransform(0.0f, -90.0f, 25.0f, 1.13f, 3.2f, 1.13f, 0.68f, 0.68f, 0.68f),
            createTransform(0.0f, -90.0f, 25.0f, 1.13f, 3.2f, 1.13f, 0.68f, 0.68f, 0.68f),
            createTransform(0.0f, 180.0f, 0.0f, 0.0f, 13.0f, 7.0f, 1.0f, 1.0f, 1.0f),
            createTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.9f, 0.9f, 0.9f, 0.9f),
            createTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f),
            createTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f));
    }
    return gDefaultTransforms;
}

ItemTransforms* ItemTransforms::getArmourStandTransforms(const ItemInstance* item, LivingEntity* entity)
{
    if (item == NULL)
        return NULL;
    Item* itemObj = item->getItem();
    if (itemObj == NULL)
        return NULL;
    ArmourStandTransformKind kind = classifyArmourStandItem(Item::getId(itemObj));
    if (kind == AS_NONE)
        return NULL;
    return armourStandHandsForKind(kind);
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
