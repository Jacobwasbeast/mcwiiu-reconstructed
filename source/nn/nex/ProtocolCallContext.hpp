#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLCALLCONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLCALLCONTEXT_HPP

#include <cstddef>
#include <vector>

#include "nn/nex/CallContext.hpp"

namespace nn
{
namespace nex
{

class Credentials;

class ProtocolCallContext : public CallContext
{
public:
    explicit ProtocolCallContext(Credentials* credentials);
    ~ProtocolCallContext() override;

    void SetCredentials(Credentials* credentials);
    void AddReturnValuePtr(void* value);
    void* GetReturnValuePtr(std::size_t index) const;

protected:
    void BeginTransition(State state, const qResult& result, bool publishResult) override;
    std::vector<void*> m_returnValues;
    Credentials* m_credentials;
    bool* m_completionFlag;
    qResult* m_completionResult;
};

}
}

#endif
