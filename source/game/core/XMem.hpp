#ifndef SOURCE_GAME_CORE_XMEM_HPP
#define SOURCE_GAME_CORE_XMEM_HPP

enum _XMEMCODEC_TYPE
{
    XMEMCODEC_DEFAULT = 0,
    XMEMCODEC_LZX = 1
};

int XMemCreateCompressionContext(_XMEMCODEC_TYPE, const void*, unsigned int, void**);
int XMemCreateDecompressionContext(_XMEMCODEC_TYPE, const void*, unsigned int, void**);
void XMemDestroyCompressionContext(void*);
void XMemDestroyDecompressionContext(void*);

#endif
