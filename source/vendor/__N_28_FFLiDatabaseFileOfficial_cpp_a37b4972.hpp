#ifndef SOURCE_VENDOR_N_28_FFLIDATABASEFILEOFFICIAL_CPP_A37B4972_HPP
#define SOURCE_VENDOR_N_28_FFLIDATABASEFILEOFFICIAL_CPP_A37B4972_HPP

struct FFLCreateID
{
    unsigned char data[10];
};

class FFLiDatabaseFileOfficial
{
public:
    struct AdjustRegularBuffer
    {
        unsigned int num;
        FFLCreateID createIds[3000];
    };
};

class __N_28_FFLiDatabaseFileOfficial_cpp_a37b4972
{
public:
    static void SetNullCreateID(FFLCreateID& createId);
    static bool IsExistCreateID(const FFLCreateID& createId, const FFLiDatabaseFileOfficial::AdjustRegularBuffer& buffer);
};

#endif
