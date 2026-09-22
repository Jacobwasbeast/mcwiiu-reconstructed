#ifndef NN_PIA_COMMON_LISTBASE_HPP
#define NN_PIA_COMMON_LISTBASE_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace common {


struct ListNode
{
    ListNode* prev;
    ListNode* next;
};

class ListBase
{
public:
    ListBase();
    void Init();
    void InsertBeforeNode(ListNode* node, ListNode* newNode);
    void InsertAfterNode(ListNode* node, ListNode* newNode);
    void EraseNode(ListNode* node);
    bool IsIncludeNode(const ListNode* node) const;
    ListNode* PopFrontNode();
    ListNode* PopBackNode();
    ListNode* Sentinel();
    const ListNode* Sentinel() const;
    void ClearNode();
    void RotateNode(ListNode* node);

private:
    ListNode mSentinel;
    uint32_t mSize;
};

}}}

#endif
