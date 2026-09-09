#ifndef SOURCE_VENDOR___N_14_CHAT_ADPCM_CPP_F2B530AF_HPP
#define SOURCE_VENDOR___N_14_CHAT_ADPCM_CPP_F2B530AF_HPP

#include "nn/pia/chat/Adpcm.hpp"

class __N_14_chat_Adpcm_cpp_f2b530af
{
public:
    static void vct_decode_adpcm_16(const unsigned char*, short*, unsigned int, nn::pia::chat::Adpcm::State*);
    static void vct_decode_adpcm_24(const unsigned char*, short*, unsigned int, nn::pia::chat::Adpcm::State*);
    static void vct_decode_adpcm_32(const unsigned char*, short*, unsigned int, nn::pia::chat::Adpcm::State*);
    static void vct_encode_adpcm_16(unsigned char*, const short*, unsigned int, nn::pia::chat::Adpcm::State*);
    static void vct_encode_adpcm_24(unsigned char*, const short*, unsigned int, nn::pia::chat::Adpcm::State*);
    static void vct_encode_adpcm_32(unsigned char*, const short*, unsigned int, nn::pia::chat::Adpcm::State*);
};

#endif
