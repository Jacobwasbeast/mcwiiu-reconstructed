#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MD5_CONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MD5_CONTEXT_HPP

namespace nn { namespace pia { namespace common {

class Md5Context
{
public:
    virtual void Initialize();
    virtual void Update(const void* data, unsigned int size);
    virtual unsigned int GetHashSize() const;
    virtual unsigned int GetBlockSize() const;
    virtual void GetHash(void* hash);
    virtual void ProcessBlock();

private:
    unsigned int m_state[4];
    unsigned int m_totalBytes;
    unsigned char m_block[64];
};

}}}

#endif
