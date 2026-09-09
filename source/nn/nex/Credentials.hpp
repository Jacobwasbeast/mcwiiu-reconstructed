#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CREDENTIALS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CREDENTIALS_HPP

#include <cstdint>

#include "nn/nex/RefCountedObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class EndPoint;
class StreamManager;

class Credentials : public RefCountedObject
{
public:
    Credentials();
    Credentials(std::uint32_t value, StreamManager* streamManager);
    ~Credentials() override;

    void Logout();
    StreamManager* MarkInvalid();
    void SetStreamManager(StreamManager* streamManager);
    EndPoint* GetConnection(unsigned short connectionType) const;
    void SetAuthenticationConnection(EndPoint* connection);
    void SetSecureConnection(EndPoint* connection);

private:
    std::uint32_t m_value;
    std::uint32_t m_state;
    String m_name;
    String m_description;
    StreamManager* m_streamManager;
    EndPoint* m_authenticationConnection;
    EndPoint* m_secureConnection;
};

}
}

#endif
