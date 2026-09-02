#include "game/util/phys__Vec2.hpp"

#include <stddef.h>

unsigned int TlsAlloc();
int TlsSetValue(unsigned int index, void* pValue);
void* TlsGetValue(unsigned int index);

namespace phys
{

static const int VEC2_POOL_SIZE = 1024;

static unsigned int g_vec2TlsIndex = 0;
static Vec2::ThreadStorage* g_pDefaultThreadStorage = NULL;

Vec2::ThreadStorage::ThreadStorage()
{
    m_pPool = new Vec2[VEC2_POOL_SIZE];
    m_nextIndex = 0;
}

Vec2::ThreadStorage::~ThreadStorage()
{
    delete[] m_pPool;
}

void Vec2::CreateNewThreadStorage()
{
    ThreadStorage* pStorage = new ThreadStorage();

    if (g_pDefaultThreadStorage == NULL)
    {
        g_vec2TlsIndex = TlsAlloc();
        g_pDefaultThreadStorage = pStorage;
    }

    TlsSetValue(g_vec2TlsIndex, pStorage);
}

void Vec2::UseDefaultThreadStorage()
{
    TlsSetValue(g_vec2TlsIndex, g_pDefaultThreadStorage);
}

void Vec2::ReleaseThreadStorage()
{
    ThreadStorage* pStorage = (ThreadStorage*)TlsGetValue(g_vec2TlsIndex);

    if (pStorage != g_pDefaultThreadStorage)
    {
        delete pStorage;
    }
}

Vec2::Vec2(float x, float y)
{
    this->x = x;
    this->y = y;
}

Vec2* Vec2::newTemp(float x, float y)
{
    ThreadStorage* pStorage = (ThreadStorage*)TlsGetValue(g_vec2TlsIndex);

    Vec2* pResult = &pStorage->m_pPool[pStorage->m_nextIndex];
    pStorage->m_nextIndex = (pStorage->m_nextIndex + 1) & (VEC2_POOL_SIZE - 1);

    pResult->x = x;
    pResult->y = y;

    return pResult;
}

float Vec2::distanceToSqr(Vec2* other)
{
    float dx = other->x - x;
    float dy = other->y - y;

    return dx * dx + dy * dy;
}

}
