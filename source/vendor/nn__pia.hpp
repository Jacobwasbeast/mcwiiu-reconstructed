#ifndef NN_PIA_VENDOR_HPP
#define NN_PIA_VENDOR_HPP

namespace nn {
namespace pia {

void Assert(const char* expression, const char* file, unsigned int line,
            const char* message, ...);
void EmbedMiddlewareInfo();

class Result {
public:
    static void SetNexNetworkErrorCode(unsigned int code);
    static void ClearNexNetworkErrorCode();
    int GetErrorCode() const;
    const char* ToString() const;

    int m_code;
};

}
}

#endif
