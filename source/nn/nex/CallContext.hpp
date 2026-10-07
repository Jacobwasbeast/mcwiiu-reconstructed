#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CALLCONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CALLCONTEXT_HPP

#include <list>
#include "nn/nex/CallbackRoot.hpp"

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/RefCountedObject.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/UserContext.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class CallContextRegister;
class ClientProtocol;
class ConnectionManager;
class JobConnectEndPoint;
class CallContext : public RefCountedObject
{
public:
    enum State
    {
        Ready = 0,
        CallInProgress = 1,
        CallSucceeded = 2,
        CallFailed = 3,
        CallCanceled = 4
    };

    typedef void (*CompletionCallback)(CallContext* pCallContext, const UserContext* pUserContext);

    CallContext();
    virtual ~CallContext();

    bool Wait(unsigned int uiTimeout);

    State GetState() const { return m_state; }

    const qResult& GetResult() const { return m_result; }

    static const wchar_t* GetStateString(State state);

    bool IsRegistered() const;
    void Register();
    void Unregister();

    bool Reset();
    bool InitiateCall();
    bool InitiateSpecialCall();

    void SignalSuccess(qResult result)
    {
        SetStateImpl(CallSucceeded, result, true);
    }
    void SignalFailure(qResult result);
    void SignalCompletion(qResult result);

    bool Cancel(State state);

    void SetDependentConnection(void* pConnection, unsigned int uiConnectionId);

    void SetFlag(unsigned int uiFlag);
    void ClearFlag(unsigned int uiFlag);
    bool FlagIsSet(unsigned int uiFlag) const;

    void SetUserContext(unsigned int uiIndex, const UserContext& rUserContext);
    UserContext GetUserContext(unsigned int uiIndex) const;

    void RegisterCompletionCallback(CompletionCallback cbCallback, const UserContext& rUserContext,
                                    bool bAppend);
    void RegisterCompletionCallback(CallbackRoot* pCallback, bool bOnSuccessOnly, bool bAppend);
    void RegisterCancellationCallback(CallbackRoot* pCallback);

protected:
    virtual bool FlagsAreValid() const;
    virtual void BeginTransition(State state, qResult result, bool bPublishResult);
    virtual void ProcessCallCompletion();

    void SetStateImpl(State state, qResult result, bool bPublishResult);
    bool CancelImpl(State state);
    bool SetCallInProgressState();

    static bool TransitionIsValid(State currentState, State newState);

private:
    friend class CallContextRegister;
    friend class ClientProtocol;
    friend class JobCallContextCallback;
    friend class JobConnectEndPoint;
    friend class JobPerformNATCheck;
    friend class ConnectionManager;
    friend class BackEndServices;
    friend class JobBackEndServicesTerminate;
    friend class JobBackEndServicesLogin;
    friend class JobBackEndServicesLoginWithData;
    friend class JobBackEndServicesLogout;
    friend class TicketManager;
    friend class AuthenticationClient;
    friend class JobTicketManagerLogin;
    friend class JobDeriveKey;
    friend class NgsFacade;
    friend class ChecksumAlgorithm;
    friend class JobTicketManagerLoginWithData;
    friend class JobTicketManagerAcquireTicket;
    class InternalData
    {
    public:
        CompletionCallback m_cbCallback;
        UserContext m_userContext;
    };

    typedef std::list<InternalData, MemAllocator<InternalData> > CompletionCallbackList;

    unsigned int m_uiFlags;
    State m_state;
    CompletionCallbackList m_completionCallbacks;
    qVector<UserContext> m_userContexts;
    qResult m_result;
    unsigned int m_uiCallId;
    void* m_pDependentConnection;
    unsigned int m_uiDependentConnectionId;
    CallbackRoot* m_pCancellationCallback;
    Time m_expirationTime;
};

void InvokeCallbackOnSuccess(CallContext* pCallContext, const UserContext* pUserContext);
void InvokeCallbackOnCompletion(CallContext* pCallContext, const UserContext* pUserContext);

}
}

#endif
