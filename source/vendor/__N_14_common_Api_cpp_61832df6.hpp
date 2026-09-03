#ifndef SOURCE_VENDOR___N_14_COMMON_API_CPP_61832DF6_HPP
#define SOURCE_VENDOR___N_14_COMMON_API_CPP_61832DF6_HPP

#include <stdint.h>
#include "platform/wiiu/pead.hpp"

class __N_14_common_Api_cpp_61832df6
{
public:
    class DelegatePrint : public pead::IDelegate1<const pead::PrintConfig::PrintEventArg&>
    {
    public:
        virtual void invoke(const pead::PrintConfig::PrintEventArg& event);
    };

    class MyAllocCallback : public pead::IDelegate1<const pead::HeapMgr::AllocCallbackArg*>
    {
    public:
        virtual void invoke(const pead::HeapMgr::AllocCallbackArg* event);
    };

    class MyFreeCallback : public pead::IDelegate1<const pead::HeapMgr::FreeCallbackArg*>
    {
    public:
        virtual void invoke(const pead::HeapMgr::FreeCallbackArg* event);
    };

    void registerInternalPrintCallback();
    void unregisterInternalPrintCallback();
    void destroySystemSingletons();
    void createSystemSingletons();
};

#endif
