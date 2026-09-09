#include "vendor/__N_20_Shared_Nex_Setup_cpp_392f2b82.hpp"
#include "nn/nex/MemoryManager.hpp"

extern "C" void checkPiaCoreToUse();
extern "C" void* MEMAllocFromDefaultHeapEx(unsigned int size, int alignment);
extern "C" void MEMFreeToDefaultHeap(void* block);
extern "C" void* MEMCreateExpHeapEx(void* address, unsigned int size, unsigned short flags);
extern "C" void MEMDestroyExpHeap(void* heap);
extern "C" void* MEMAllocFromExpHeapEx(void* heap, unsigned int size, int alignment);
extern "C" unsigned int MEMGetSizeForMBlockExpHeap(void* block);
extern "C" void MEMFreeToExpHeap(void* heap, void* block);
extern "C" void MEMDumpHeap(void* heap);

static unsigned int sRuntimeMemorySize = 0;
static unsigned int sReservedMemorySize = 0;
static void* sRuntimeMemory = 0;
static void* sRuntimeHeap = 0;
static void* sReservedMemory = 0;
static unsigned int sMemoryUsed = 0;
static unsigned int sPeakMemoryUsed = 0;
typedef void* (*NexAllocator)(unsigned int);
typedef void (*NexFreer)(void*);
static NexAllocator sNexAllocator = 0;
static NexFreer sNexFreer = 0;

void* __N_20_Shared_Nex_Setup_cpp_392f2b82::InnerAlloc(unsigned int size)
{
    unsigned int blockSize;
    checkPiaCoreToUse();
    void* block = MEMAllocFromExpHeapEx(sRuntimeHeap, size, 8);
    if (block != 0)
    {
        blockSize = MEMGetSizeForMBlockExpHeap(block);
        sMemoryUsed += blockSize;
        if (sPeakMemoryUsed < sMemoryUsed)
        {
            sPeakMemoryUsed = sMemoryUsed;
        }
    }
    return block;
}

void __N_20_Shared_Nex_Setup_cpp_392f2b82::InnerFree(void* block)
{
    checkPiaCoreToUse();
    sMemoryUsed -= MEMGetSizeForMBlockExpHeap(block);
    MEMFreeToExpHeap(sRuntimeHeap, block);
}

void __N_20_Shared_Nex_Setup_cpp_392f2b82::FreeFunction(void* block)
{
    InnerFree(block);
}

void __N_20_Shared_Nex_Setup_cpp_392f2b82::ReleaseReservedMemory()
{
    unsigned int memoryUsed;
    unsigned int peakMemoryUsed;
    checkPiaCoreToUse();
    memoryUsed = sMemoryUsed;
    peakMemoryUsed = sPeakMemoryUsed;
    if (sReservedMemory != 0)
    {
        FreeFunction(sReservedMemory);
        sReservedMemory = 0;
    }
    sMemoryUsed = memoryUsed;
    sPeakMemoryUsed = peakMemoryUsed;
}

void* __N_20_Shared_Nex_Setup_cpp_392f2b82::AllocFunction(unsigned int size)
{
    checkPiaCoreToUse();
    void* block = InnerAlloc(size);
    if (block == 0)
    {
        ReleaseReservedMemory();
        block = InnerAlloc(size);
    }
    return block;
}

void __N_20_Shared_Nex_Setup_cpp_392f2b82::AcquireReservedMemory()
{
    unsigned int memoryUsed;
    unsigned int peakMemoryUsed;
    checkPiaCoreToUse();
    memoryUsed = sMemoryUsed;
    peakMemoryUsed = sPeakMemoryUsed;
    if (sReservedMemory == 0)
    {
        sReservedMemory = AllocFunction(sReservedMemorySize);
    }
    sMemoryUsed = memoryUsed;
    sPeakMemoryUsed = peakMemoryUsed;
}

void __N_20_Shared_Nex_Setup_cpp_392f2b82::InitializeRuntimeMemory(unsigned int runtimeSize, unsigned int reservedSize)
{
    checkPiaCoreToUse();
    sRuntimeMemorySize = runtimeSize;
    sReservedMemorySize = reservedSize;
    sRuntimeMemory = MEMAllocFromDefaultHeapEx(runtimeSize + reservedSize, 8);
    sRuntimeHeap = MEMCreateExpHeapEx(sRuntimeMemory, runtimeSize + reservedSize, 5);
    sNexAllocator = &__N_20_Shared_Nex_Setup_cpp_392f2b82::AllocFunction;
    sNexFreer = &__N_20_Shared_Nex_Setup_cpp_392f2b82::FreeFunction;
    nn::nex::MemoryManager::SetCallbacks(sNexAllocator, sNexFreer);
    if (sReservedMemorySize != 0)
    {
        AcquireReservedMemory();
    }
}

void __N_20_Shared_Nex_Setup_cpp_392f2b82::FinalizeRuntimeMemory()
{
    checkPiaCoreToUse();
    if (sReservedMemorySize != 0)
    {
        ReleaseReservedMemory();
    }
    if (sMemoryUsed != 0)
    {
        MEMDumpHeap(sRuntimeHeap);
    }
    MEMDestroyExpHeap(sRuntimeHeap);
    MEMFreeToDefaultHeap(sRuntimeMemory);
    sMemoryUsed = 0;
    sPeakMemoryUsed = 0;
}
