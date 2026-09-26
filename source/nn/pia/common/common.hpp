#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_COMMON_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_COMMON_HPP
#include <stdint.h>
#include "nn/pia/common/AesImpl.hpp"
namespace nn
{
namespace pia
{
namespace common
{
struct GcmModeGenericHelper
{
};
struct XCUiL_2_16
{
};
template<typename Helper> class GcmModeImpl;
template<> class GcmModeImpl<GcmModeGenericHelper>
{
public:
    struct Block
    {
        uint8_t bytes[16];
    };
    typedef void (*BlockFunction)(void*, const void*, const void*);
    static void BlockCallback(void*, const void*, const void*);
    void Initialize(const AesImpl<XCUiL_2_16>&);
    ~GcmModeImpl();
    void GfMultH(Block*);
    void Reset(const void*, unsigned int);
    unsigned int EncryptUpdate(void*, unsigned int, const void*, unsigned int);
    unsigned int DecryptUpdate(void*, unsigned int, const void*, unsigned int);
    void GenerateMac();
    void GetMac(void*, unsigned int);
    void InitializeHashKey();
    void UpdateAad(const void*, unsigned int);
    void ComputeMac(bool);

private:
    uint32_t m_state;
    const void* m_cipher;
    BlockFunction m_encryptBlock;
    uint32_t m_layoutPadding;
    Block m_hash;
    Block m_counter;
    Block m_keystream;
    Block m_mac;
    uint32_t m_hashKeyWords[4];
    uint32_t m_aadSizeLow;
    uint32_t m_aadSizeHigh;
    uint32_t m_textSizeLow;
    uint32_t m_textSizeHigh;
    uint32_t m_partialSize;
    uint32_t m_keystreamOffset;
    uint32_t m_blockCounter;
    uint32_t m_statePadding;
    uint32_t m_reductionTable[16][4];
};
typedef GcmModeImpl<GcmModeGenericHelper> GcmMode;
static_assert(sizeof(GcmMode) == 0x180, "Wii U GCM state layout");
}
}
}
#endif
