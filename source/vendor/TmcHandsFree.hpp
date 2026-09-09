#ifndef SOURCE_VENDOR_TMCHANDSFREE_HPP
#define SOURCE_VENDOR_TMCHANDSFREE_HPP

extern "C" void* TMC_HF_Init(void* config);
extern "C" unsigned int TMC_HF_Proc(void* handle, const short* inputLeft,
                                    const short* inputRight,
                                    const short* inputReference, short* output);
extern "C" unsigned int TMC_HF_Close(void* handle);

#endif
