#include "game/client/render/Particle.hpp"

#include <vector>

#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Math.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

double Particle::xOff = 0.0;
double Particle::yOff = 0.0;
double Particle::zOff = 0.0;

static Vec3* cameraViewDirection = Vec3::newPermanent(0.0, 0.0, 0.0);
static AABB* INITIAL_AABB = AABB::newPermanent(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

Particle::Particle(Level* level, double x, double y, double z, double xa, double ya, double za)
{
    _init(level, x, y, z);

    xd = xa + (Math::random() * 2.0 - 1.0) * 0.4000000059604645;
    yd = ya + (Math::random() * 2.0 - 1.0) * 0.4000000059604645;
    zd = za + (Math::random() * 2.0 - 1.0) * 0.4000000059604645;

    float power = (float)(Math::random() + Math::random() + 1.0) * 0.15f;
    double length = Mth::sqrt(xd * xd + yd * yd + zd * zd);

    xd = xd / length * power * 0.4000000059604645;
    yd = yd / length * power * 0.4000000059604645 + 0.10000000149011612;
    zd = zd / length * power * 0.4000000059604645;
}

Particle::Particle(Level* level, double x, double y, double z)
{
    _init(level, x, y, z);
}

Particle::~Particle()
{
    delete bb;
}

void Particle::_init(Level* level, double x, double y, double z)
{
    this->level = level;
    bb = AABB::newPermanent(INITIAL_AABB);

    textureIndex = 0;
    onGround = false;
    hasPhysics = true;
    removed = false;

    alpha = 1.0f;
    bbWidth = 0.6f;
    bbHeight = 1.8f;

    gravity = 0.0f;
    sprite = NULL;
    roll = 0.0f;
    oRoll = 0.0f;

    rCol = 1.0f;
    gCol = 1.0f;
    bCol = 1.0f;

    age = 0;

    uo = random.nextFloat() * 3.0f;
    vo = random.nextFloat() * 3.0f;
    size = (random.nextFloat() * 0.5f + 0.5f) * 2.0f;
    lifetime = (int)(4.0f / (random.nextFloat() * 0.9f + 0.1f));

    uvs = TextureUVCoordinateSet(0.0f, 0.0f, 1.0f, 1.0f, 16, 16);

    setSize(0.2f, 0.2f);
    setPos(x, y, z);

    xo = x;
    yo = y;
    zo = z;

    cachedLightPos = -1;

    xd = 0.0;
    yd = 0.0;
    zd = 0.0;
}

void Particle::setPower(float power)
{
    xd *= power;
    yd = (yd - 0.10000000149011612) * power + 0.10000000149011612;
    zd *= power;
}

Particle* Particle::scale(float factor)
{
    setSize(0.2f * factor, 0.2f * factor);
    size *= factor;

    return this;
}

void Particle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime)
    {
        remove();
    }

    yd -= 0.04 * (double)gravity;

    move(xd, yd, zd);

    xd *= 0.9800000190734863;
    yd *= 0.9800000190734863;
    zd *= 0.9800000190734863;

    if (onGround)
    {
        xd *= 0.699999988079071;
        zd *= 0.699999988079071;
    }

}

void Particle::render(BufferBuilder* buffer,
                      const boost::shared_ptr<Entity>& camera,
                      float partialTicks,
                      float xa,
                      float ya,
                      float za,
                      float xa2,
                      float za2)
{
    float halfSize = 0.1f * size;

    if (sprite != NULL)
    {
        uvs.u0 = sprite->getU0(false);
        uvs.u1 = sprite->getU1(false);
        uvs.v0 = sprite->getV0(false);
        uvs.v1 = sprite->getV1(false);
    }

    float px = (float)(xo + (x - xo) * partialTicks - xOff);
    float py = (float)(yo + (y - yo) * partialTicks - yOff);
    float pz = (float)(zo + (z - zo) * partialTicks - zOff);

    int light = getLightColor(partialTicks);

    Vec3* corner0 = Vec3::newTemp(-xa * halfSize - xa2 * halfSize,
                                  -ya * halfSize,
                                  -za * halfSize - za2 * halfSize);
    Vec3* corner1 = Vec3::newTemp(-xa * halfSize + xa2 * halfSize,
                                  ya * halfSize,
                                  -za * halfSize + za2 * halfSize);
    Vec3* corner2 = Vec3::newTemp(xa * halfSize + xa2 * halfSize,
                                  ya * halfSize,
                                  za * halfSize + za2 * halfSize);
    Vec3* corner3 = Vec3::newTemp(xa * halfSize - xa2 * halfSize,
                                  -ya * halfSize,
                                  za * halfSize - za2 * halfSize);

    if (roll != 0.0f)
    {
        float angle = (roll + (roll - oRoll) * partialTicks) * 0.5f;
        float cosAngle = Mth::cos(angle);

        float sinAngle = Mth::sin(angle);
        Vec3* axis = Vec3::newTemp(sinAngle * (float)cameraViewDirection->x,
                                   sinAngle * (float)cameraViewDirection->y,
                                   sinAngle * (float)cameraViewDirection->z);

        corner0 = axis->scale(2.0 * corner0->dot(axis))
                      ->add(corner0->scale((double)(cosAngle * cosAngle) - axis->dot(axis)))
                      ->add(axis->cross(corner0)->scale(2.0f * cosAngle));
        corner1 = axis->scale(2.0 * corner1->dot(axis))
                      ->add(corner1->scale((double)(cosAngle * cosAngle) - axis->dot(axis)))
                      ->add(axis->cross(corner1)->scale(2.0f * cosAngle));
        corner2 = axis->scale(2.0 * corner2->dot(axis))
                      ->add(corner2->scale((double)(cosAngle * cosAngle) - axis->dot(axis)))
                      ->add(axis->cross(corner2)->scale(2.0f * cosAngle));
        corner3 = axis->scale(2.0 * corner3->dot(axis))
                      ->add(corner3->scale((double)(cosAngle * cosAngle) - axis->dot(axis)))
                      ->add(axis->cross(corner3)->scale(2.0f * cosAngle));
    }

    buffer->color(rCol, gCol, bCol, alpha);
    buffer->tex2(light);

    buffer->vertexUV(px + (float)corner0->x,
                     py + (float)corner0->y,
                     pz + (float)corner0->z,
                     uvs.u1,
                     uvs.v1);
    buffer->vertexUV(px + (float)corner1->x,
                     py + (float)corner1->y,
                     pz + (float)corner1->z,
                     uvs.u1,
                     uvs.v0);
    buffer->vertexUV(px + (float)corner2->x,
                     py + (float)corner2->y,
                     pz + (float)corner2->z,
                     uvs.u0,
                     uvs.v0);
    buffer->vertexUV(px + (float)corner3->x,
                     py + (float)corner3->y,
                     pz + (float)corner3->z,
                     uvs.u0,
                     uvs.v1);
}

int Particle::getParticleTexture()
{
    return 0;
}

void Particle::setSprite(TextureAtlasSprite* sprite)
{
    int texture = getParticleTexture();

    if (texture == 1 || texture == 2)
    {
        this->sprite = sprite;
    }
}

void Particle::setMiscTex(int texture)
{
    if (getParticleTexture() == 0 || getParticleTexture() == 4)
    {
        uvs = TextureUVCoordinateSet::fromOldSystem(texture);
    }
}

std::wstring Particle::toString()
{
    return std::wstring(L"A particle");
}

bool Particle::isSemiTransparent()
{
    return false;
}

bool Particle::isTransparent()
{
    return false;
}

void Particle::setBoundingBox(const AABB* box)
{
    bb->set(box);
}

AABB* Particle::getBoundingBox()
{
    return bb;
}

void Particle::setPos(double x, double y, double z)
{
    this->x = x;
    this->y = y;
    this->z = z;

    float halfWidth = bbWidth / 2.0f;

    setBoundingBox(AABB::newTemp(x - halfWidth,
                                 y,
                                 z - halfWidth,
                                 x + halfWidth,
                                 y + bbHeight,
                                 z + halfWidth));
}

void Particle::move(double xa, double ya, double za)
{
    double xMoved = xa;
    double yMoved = ya;
    double zMoved = za;

    if (!hasPhysics)
    {
        setBoundingBox(getBoundingBox()->move(xa, ya, za));
        setLocationFromBoundingbox();

        onGround = false;
    }
    else
    {
        boost::shared_ptr<Entity> entity;
        std::vector<AABB*>& boxes = *level->getCollisionAABBs(entity, getBoundingBox()->expand(xa, ya, za), false, false, true);

        std::vector<AABB*>::iterator box;

        for (box = boxes.begin(); box != boxes.end(); ++box)
        {
            yMoved = (*box)->clipYCollide(getBoundingBox(), yMoved);
        }

        setBoundingBox(getBoundingBox()->move(0.0, yMoved, 0.0));

        for (box = boxes.begin(); box != boxes.end(); ++box)
        {
            xMoved = (*box)->clipXCollide(getBoundingBox(), xMoved);
        }

        setBoundingBox(getBoundingBox()->move(xMoved, 0.0, 0.0));

        for (box = boxes.begin(); box != boxes.end(); ++box)
        {
            zMoved = (*box)->clipZCollide(getBoundingBox(), zMoved);
        }

        setBoundingBox(getBoundingBox()->move(0.0, 0.0, zMoved));
        setLocationFromBoundingbox();

        onGround = ya != yMoved && ya < 0.0;
    }

    if (xa != xMoved)
    {
        xd = 0.0;
    }

    if (za != zMoved)
    {
        zd = 0.0;
    }

}

int Particle::getLightColor(float partialTicks)
{
    BlockPos pos(x, y, z);
    uint32_t key = (uint32_t)((pos.z & 0xFF) | ((pos.y & 0xFF) << 8) | ((pos.x & 0xFF) << 16));

    if (key == cachedLightPos)
    {
        return (uint32_t)cachedLightColor;
    }
    if (level->hasChunkAt(pos))
    {
        cachedLightPos = (int)key;
        cachedLightColor = level->getLightColor(pos, 0, -1);
        return (uint32_t)cachedLightColor;
    }
    return 0;
}

float Particle::getBrightness(float partialTicks)
{
    BlockPos pos(x, y, z);
    if (level->hasChunkAt(pos))
        return level->getBrightness(pos);
    return 0.0;
}

void Particle::setColor(float red, float green, float blue)
{
    rCol = red;
    gCol = green;
    bCol = blue;
}

void Particle::setAlpha(float alpha)
{
    this->alpha = alpha;
}

void Particle::setLifetime(int lifetime)
{
    this->lifetime = lifetime;
}

double Particle::getRedCol()
{
    return rCol;
}

double Particle::getGreenCol()
{
    return gCol;
}

double Particle::getBlueCol()
{
    return bCol;
}

void Particle::setSize(float width, float height)
{
    if (width != bbWidth || height != bbHeight)
    {
        bbWidth = width;
        bbHeight = height;

        AABB* box = getBoundingBox();

        setBoundingBox(AABB::newTemp(box->mMinX,
                                     box->mMinY,
                                     box->mMinZ,
                                     box->mMinX + width,
                                     box->mMinY + height,
                                     box->mMinZ + width));
    }
}

void Particle::setLocationFromBoundingbox()
{
    AABB* box = getBoundingBox();

    x = (box->mMinX + box->mMaxX) * 0.5;
    y = box->mMinY;
    z = (box->mMinZ + box->mMaxZ) * 0.5;
}

Level* Particle::getLevel()
{
    return level;
}

double Particle::getAlpha()
{
    return alpha;
}

uint8_t Particle::isAlive()
{
    return !removed;
}

void Particle::remove()
{
    removed = true;
}

void Particle::setXOffset(double offset)
{
    xOff = offset;
}

void Particle::setYOffset(double offset)
{
    yOff = offset;
}

void Particle::setZOffset(double offset)
{
    zOff = offset;
}

void Particle::setCamerViewDirection(Vec3* direction)
{
    cameraViewDirection->x = direction->x;
    cameraViewDirection->y = direction->y;
    cameraViewDirection->z = direction->z;
}
