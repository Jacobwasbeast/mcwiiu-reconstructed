#include "game/ui/FJ_Event.hpp"
#include "game/ui/fui.hpp"

#include <vector>

typedef std::vector<FJ_EventListener> FJEventListenerVector;
typedef FJEventListenerVector::iterator FJEventListenerVectorIterator;

FJ_Event::FJ_Event(eFJEventType eType, bool bBubbles, bool bCancelable)
    : m_eType(eType)
    , m_bBubbles(bBubbles)
    , m_bCancelable(bCancelable)
{
}

void __CPR233___Insert_n__Q2_3std78vector__tm__64_16FJJ_EventListenerQ2_3std36allocator__tm__19_J35JFQ2_3std68_Vector_iterator__tm__44_Z1ZQ3_3std25_Vector_val__tm__7_Z1ZZ2Z5_AltyQ4_3stdJ143JJ170J9size_typeRCZ1Z_v(
    FJEventListenerVector& vector, FJEventListenerVectorIterator position, size_t count, const FJ_EventListener& value)
{
    vector.insert(position, count, value);
}

FJEventListenerVectorIterator __CPR256__insert__Q2_3std78vector__tm__64_16FJJ_EventListenerQ2_3std36allocator__tm__19_J32JFQ2_3std68_Vector_iterator__tm__44_Z1ZQ3_3std25_Vector_val__tm__7_Z1ZZ2Z5_AltyRCZ1Z_Q2_3stdJ103J(
    FJEventListenerVector& vector, FJEventListenerVectorIterator position, const FJ_EventListener& value)
{
    return vector.insert(position, value);
}

FJEventListenerVectorIterator __CPR250__erase__Q2_3std78vector__tm__64_16FJJ_EventListenerQ2_3std36allocator__tm__19_J31JFQ2_3std68_Vector_iterator__tm__44_Z1ZQ3_3std25_Vector_val__tm__7_Z1ZZ2Z5_Alty_Q2_3stdJ102J(
    FJEventListenerVector& vector, FJEventListenerVectorIterator position)
{
    return vector.erase(position);
}
