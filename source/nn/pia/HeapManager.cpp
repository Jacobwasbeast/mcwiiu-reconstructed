#include "nn/pia/HeapManager.hpp"

namespace nn { namespace pia { namespace common {


namespace
{
pead::Arena g_arena;
unsigned int g_currentHeap = 20;
bool g_initialized = false;
pead::Heap* g_heaps[21] = {};
}

unsigned int HeapManager::convert(ModuleType moduleType)
{
    static const unsigned char moduleHeaps[20] =
        { 0, 1, 3, 5, 2, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 0, 0 };
    if (moduleType < 2)
        return 20;
    if (moduleType > 19)
        return moduleType == 22 ? 19 : 20;
    return moduleHeaps[moduleType];
}

pead::Heap* HeapManager::GetHeap(ModuleType moduleType)
{
    return g_heaps[convert(moduleType)];
}

pead::Heap* HeapManager::GetHeap()
{
    return g_heaps[g_currentHeap];
}

void HeapManager::SetCurrentHeap(ModuleType moduleType)
{
    g_currentHeap = convert(moduleType);
}

void HeapManager::ClearCurrentHeap()
{
    g_currentHeap = 20;
}

void HeapManager::Setup(ModuleType moduleType, unsigned int size,
                        const pead::SafeStringBase<char>& name)
{
    const unsigned int heapIndex = convert(moduleType);
    pead::Heap* parent = pead::HeapMgr::getRootHeap(0);
    g_heaps[heapIndex] = pead::ExpHeap::create(size, name, parent, pead::Heap::cHeapDirection_Forward, false);
}

void HeapManager::Cleanup(ModuleType moduleType)
{
    const unsigned int heapIndex = convert(moduleType);
    pead::Heap* heap = g_heaps[heapIndex];
    if (!heap)
        return;
    heap->destroy();
    g_heaps[heapIndex] = 0;
}

bool HeapManager::Initialize(void* memory, unsigned int size)
{
    if (g_initialized)
        return false;
    g_arena.mStart = static_cast<unsigned char*>(memory);
    g_arena.mSize = size;
    pead::HeapMgr::initialize(&g_arena);
    pead::Heap* rootHeap = pead::HeapMgr::getRootHeap(0);
    pead::ThreadMgr::createInstance(rootHeap);
    pead::ThreadMgr::instance()->initialize(rootHeap);
    g_currentHeap = 20;
    g_initialized = true;
    return true;
}

void HeapManager::Finalize()
{
    if (!g_initialized)
        return;
    g_initialized = false;
    pead::ThreadMgr::instance()->destroy();
    pead::ThreadMgr::deleteInstance();
    pead::HeapMgr::destroy();
}

}}}
