#ifndef SOURCE_VENDOR_LOADINGINPUTPARAMS_HPP
#define SOURCE_VENDOR_LOADINGINPUTPARAMS_HPP

#include <stdint.h>

class _LoadingInputParams
{
public:
    _LoadingInputParams();

    void* (*m_firstParameter)(void*);
    void* m_secondParameter;
    void* m_thirdParameter;
    int32_t m_selectedIndex;
    int32_t m_fifthParameter;
    int32_t m_sixthParameter;
    int32_t m_seventhParameter;
    int32_t m_eighthParameter;
    bool m_isEnabled;
    int32_t m_timeout;
};

#endif
