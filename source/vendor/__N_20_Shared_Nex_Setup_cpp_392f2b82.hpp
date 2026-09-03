#ifndef SOURCE_VENDOR_N_20_SHARED_NEX_SETUP_CPP_392F2B82_HPP
#define SOURCE_VENDOR_N_20_SHARED_NEX_SETUP_CPP_392F2B82_HPP

class __N_20_Shared_Nex_Setup_cpp_392f2b82
{
public:
    static void* InnerAlloc(unsigned int size);
    static void InnerFree(void* block);
    static void FreeFunction(void* block);
    static void ReleaseReservedMemory();
    static void* AllocFunction(unsigned int size);
    static void AcquireReservedMemory();
    static void InitializeRuntimeMemory(unsigned int runtimeSize, unsigned int reservedSize);
    static void FinalizeRuntimeMemory();
};

#endif
