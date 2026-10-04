#ifndef SOURCE_VENDOR_N_20_SHARED_NEX_SETUP_CPP_392F2B82_HPP
#define SOURCE_VENDOR_N_20_SHARED_NEX_SETUP_CPP_392F2B82_HPP

namespace __N_20_Shared_Nex_Setup_cpp_392f2b82
{
    void* InnerAlloc(unsigned int size);
    void InnerFree(void* block);
    void FreeFunction(void* block);
    void ReleaseReservedMemory();
    void* AllocFunction(unsigned int size);
    void AcquireReservedMemory();
    void InitializeRuntimeMemory(unsigned int runtimeSize, unsigned int reservedSize);
    void FinalizeRuntimeMemory();
}

#endif
