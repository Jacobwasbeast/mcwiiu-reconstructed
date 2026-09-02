#include "game/core/Uniform.hpp"

#include "game/core/Buffer.hpp"
#include "game/core/FloatBuffer.hpp"
#include "game/core/IntBuffer.hpp"
#include "game/core/Matrix4f.hpp"
#include "game/item/Effect.hpp"

Uniform::Uniform(const std::wstring& name, int type, int count, Effect* effect)
    : mName(name)
{
    mEffect = effect;
    mType = type;
    mCount = count;

    if (type < 4)
    {
        mIntBuffer = new IntBuffer(count);
        mFloatBuffer = NULL;
    }
    else
    {
        mIntBuffer = NULL;
        mFloatBuffer = new FloatBuffer(count);
    }

    mLocation = -1;
    markDirty();
}

Uniform::~Uniform()
{
}

void Uniform::markDirty()
{
    mDirty = true;

    if (mEffect != NULL)
    {
        mEffect->markDirty();
    }
}

void Uniform::set(float x)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, x);
    markDirty();
}

void Uniform::set(float x, float y)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, x);
    mFloatBuffer->put(1, y);
    markDirty();
}

void Uniform::set(float x, float y, float z)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, x);
    mFloatBuffer->put(1, y);
    mFloatBuffer->put(2, z);
    markDirty();
}

void Uniform::set(const Vector3f& value)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, value.x);
    mFloatBuffer->put(1, value.y);
    mFloatBuffer->put(2, value.z);
    markDirty();
}

void Uniform::set(float x, float y, float z, float w)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(x);
    mFloatBuffer->put(y);
    mFloatBuffer->put(z);
    mFloatBuffer->put(w);
    mFloatBuffer->flip();
    markDirty();
}

void Uniform::setSafe(float x, float y, float z, float w)
{
    mFloatBuffer->position(0);

    if (mType >= 4)
    {
        mFloatBuffer->put(0, x);

        if (mType >= 5)
        {
            mFloatBuffer->put(1, y);

            if (mType >= 6)
            {
                mFloatBuffer->put(2, z);

                if (mType >= 7)
                {
                    mFloatBuffer->put(3, w);
                }
            }
        }
    }

    markDirty();
}

void Uniform::setSafe(int x, int y, int z, int w)
{
    mIntBuffer->position(0);

    if (mType >= 0)
    {
        mIntBuffer->put(0, x);

        if (mType >= 1)
        {
            mIntBuffer->put(1, y);

            if (mType >= 2)
            {
                mIntBuffer->put(2, z);

                if (mType >= 3)
                {
                    mIntBuffer->put(3, w);
                }
            }
        }
    }

    markDirty();
}

void Uniform::set(int x)
{
    mIntBuffer->position(0);
    mIntBuffer->put(0, x);
    markDirty();
}

void Uniform::set(int x, int y)
{
    mIntBuffer->position(0);
    mIntBuffer->put(0, x);
    mIntBuffer->put(1, y);
    markDirty();
}

void Uniform::set(int x, int y, int z)
{
    mIntBuffer->position(0);
    mIntBuffer->put(0, x);
    mIntBuffer->put(1, y);
    mIntBuffer->put(2, z);
    markDirty();
}

void Uniform::set(int x, int y, int z, int w)
{
    mIntBuffer->position(0);
    mIntBuffer->put(0, x);
    mIntBuffer->put(1, y);
    mIntBuffer->put(2, z);
    mIntBuffer->put(3, w);
    markDirty();
}

void Uniform::set(arrayWithLength<float> values)
{
    if (mCount <= values.length)
    {
        mFloatBuffer->position(0);
        mFloatBuffer->put(values);
        mFloatBuffer->position(0);
        markDirty();
    }
}

void Uniform::setMat2x2(float m00, float m01, float m10, float m11)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, m00);
    mFloatBuffer->put(1, m01);
    mFloatBuffer->put(2, m10);
    mFloatBuffer->put(3, m11);
    markDirty();
}

void Uniform::setMat2x3(float m00, float m01, float m02, float m10, float m11, float m12)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, m00);
    mFloatBuffer->put(1, m01);
    mFloatBuffer->put(2, m02);
    mFloatBuffer->put(3, m10);
    mFloatBuffer->put(4, m11);
    mFloatBuffer->put(5, m12);
    markDirty();
}

void Uniform::setMat2x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, m00);
    mFloatBuffer->put(1, m01);
    mFloatBuffer->put(2, m02);
    mFloatBuffer->put(3, m03);
    mFloatBuffer->put(4, m10);
    mFloatBuffer->put(5, m11);
    mFloatBuffer->put(6, m12);
    mFloatBuffer->put(7, m13);
    markDirty();
}

void Uniform::setMat3x2(float m00, float m01, float m10, float m11, float m20, float m21)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, m00);
    mFloatBuffer->put(1, m01);
    mFloatBuffer->put(2, m10);
    mFloatBuffer->put(3, m11);
    mFloatBuffer->put(4, m20);
    mFloatBuffer->put(5, m21);
    markDirty();
}

void Uniform::setMat3x3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, m00);
    mFloatBuffer->put(1, m01);
    mFloatBuffer->put(2, m02);
    mFloatBuffer->put(3, m10);
    mFloatBuffer->put(4, m11);
    mFloatBuffer->put(5, m12);
    mFloatBuffer->put(6, m20);
    mFloatBuffer->put(7, m21);
    mFloatBuffer->put(8, m22);
    markDirty();
}

void Uniform::setMat3x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, m00);
    mFloatBuffer->put(1, m01);
    mFloatBuffer->put(2, m02);
    mFloatBuffer->put(3, m03);
    mFloatBuffer->put(4, m10);
    mFloatBuffer->put(5, m11);
    mFloatBuffer->put(6, m12);
    mFloatBuffer->put(7, m13);
    mFloatBuffer->put(8, m20);
    mFloatBuffer->put(9, m21);
    mFloatBuffer->put(10, m22);
    mFloatBuffer->put(11, m23);
    markDirty();
}

void Uniform::setMat4x2(float m00, float m01, float m10, float m11, float m20, float m21, float m30, float m31)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, m00);
    mFloatBuffer->put(1, m01);
    mFloatBuffer->put(2, m10);
    mFloatBuffer->put(3, m11);
    mFloatBuffer->put(4, m20);
    mFloatBuffer->put(5, m21);
    mFloatBuffer->put(6, m30);
    mFloatBuffer->put(7, m31);
    markDirty();
}

void Uniform::setMat4x3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22, float m30, float m31, float m32)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, m00);
    mFloatBuffer->put(1, m01);
    mFloatBuffer->put(2, m02);
    mFloatBuffer->put(3, m10);
    mFloatBuffer->put(4, m11);
    mFloatBuffer->put(5, m12);
    mFloatBuffer->put(6, m20);
    mFloatBuffer->put(7, m21);
    mFloatBuffer->put(8, m22);
    mFloatBuffer->put(9, m30);
    mFloatBuffer->put(10, m31);
    mFloatBuffer->put(11, m32);
    markDirty();
}

void Uniform::setMat4x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
    mFloatBuffer->position(0);
    mFloatBuffer->put(0, m00);
    mFloatBuffer->put(1, m01);
    mFloatBuffer->put(2, m02);
    mFloatBuffer->put(3, m03);
    mFloatBuffer->put(4, m10);
    mFloatBuffer->put(5, m11);
    mFloatBuffer->put(6, m12);
    mFloatBuffer->put(7, m13);
    mFloatBuffer->put(8, m20);
    mFloatBuffer->put(9, m21);
    mFloatBuffer->put(10, m22);
    mFloatBuffer->put(11, m23);
    mFloatBuffer->put(12, m30);
    mFloatBuffer->put(13, m31);
    mFloatBuffer->put(14, m32);
    mFloatBuffer->put(15, m33);
    markDirty();
}

void Uniform::set(Matrix4f* matrix)
{
    setMat4x4(matrix->m[0], matrix->m[1], matrix->m[2], matrix->m[3],
              matrix->m[4], matrix->m[5], matrix->m[6], matrix->m[7],
              matrix->m[8], matrix->m[9], matrix->m[10], matrix->m[11],
              matrix->m[12], matrix->m[13], matrix->m[14], matrix->m[15]);
}
