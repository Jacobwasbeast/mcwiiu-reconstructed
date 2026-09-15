#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MD5_CONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MD5_CONTEXT_HPP

namespace nn { namespace pia { namespace common {

class Md5Context
{
public:
    virtual ~Md5Context();
    void ProcessBlock();
    void Initialize();
    void Update(const void* data, unsigned int size);
    void GetHash(void* hash);
    unsigned int GetHashSize() const;
    unsigned int GetBlockSize() const;

private:
    unsigned int m_state[4];
    unsigned char m_blockSize;
    unsigned char m_padding[3];
    unsigned char m_block[64];
    unsigned int m_blockCount;
    unsigned int m_blockCountHigh;
};

}}}

#endif
