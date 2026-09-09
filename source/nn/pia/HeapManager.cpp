#include "nn/pia/HeapManager.hpp"

namespace nn { namespace pia { namespace common {

namespace
{
pead::Heap* g_heaps[21] = {};
unsigned int g_currentHeap = 20;
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
    pead::Heap* parent = pead::HeapMgr::instance()
        ? pead::HeapMgr::instance()->getCurrentHeap() : 0;
    g_heaps[heapIndex] = pead::ExpHeap::create(size, name, parent,
                                                pead::Heap::cHeapDirection_Forward, false);
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

}}}
