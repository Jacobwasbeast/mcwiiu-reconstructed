#include "game/ui/FJ_SlotList.hpp"

#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_SlotList_Slot.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

typedef std::vector<FJ_SlotList_Slot*> FJSlotListVector;
typedef FJSlotListVector::iterator FJSlotListIterator;

void __CPR235___Insert_n__Q2_3std80vector__tm__66_P16FJJ_SlotList_SlotQ2_3std37allocator__tm__20_PJ36JFQ2_3std68_Vector_iterator__tm__44_Z1ZQ3_3std25_Vector_val__tm__7_Z1ZZ2Z5_AltyQ4_3stdJ145JJ172J9size_typeRCZ1Z_v(
    FJSlotListVector& vector, FJSlotListIterator position, FJSlotListVector::size_type count,
    FJ_SlotList_Slot* const& value)
{
    vector.insert(position, count, value);
}

FJSlotListIterator __CPR258__insert__Q2_3std80vector__tm__66_P16FJJ_SlotList_SlotQ2_3std37allocator__tm__20_PJ33JFQ2_3std68_Vector_iterator__tm__44_Z1ZQ3_3std25_Vector_val__tm__7_Z1ZZ2Z5_AltyRCZ1Z_Q2_3stdJ105J(
    FJSlotListVector& vector, FJSlotListIterator position, FJ_SlotList_Slot* const& value)
{
    return vector.insert(position, value);
}

FJ_SlotList::FJ_SlotList(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode), m_slotSize(50), m_slotVisible(false),
      m_rows(0), m_columns(0), m_loaded(false)
{
}

FJ_SlotList::~FJ_SlotList()
{
}

FJ_FuiNode* FJ_SlotList::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_SlotList(pRenderNode);
}

unsigned int FJ_SlotList::getType()
{
    return 0x5e;
}

float FJ_SlotList::getWidth()
{
    return FJ_FuiNode::getWidth();
}

float FJ_SlotList::getHeight()
{
    return FJ_FuiNode::getHeight();
}

void FJ_SlotList::Init()
{
    if (m_loaded)
        return;
    setAlpha(0.5f);
    m_rows = Math::round(getHeight() / (float)m_slotSize);
    m_columns = Math::round(getWidth() / (float)m_slotSize);
    if (m_slotSize > 0)
        m_loaded = true;
    FJ_FuiNode* background = getChildByName("SlotListBackground")->asMovieClip();
    removeChild(background, true);
}

int FJ_SlotList::getRows()
{
    return m_rows;
}

int FJ_SlotList::getColumns()
{
    return m_columns;
}

void FJ_SlotList::Clear()
{
    if (m_slots.empty())
        return;
    for (unsigned int i = 0; i < m_slots.size(); ++i)
    {
        if (m_slots[i]->getParent() != NULL)
        {
            FJ_FuiNode* parent = FJ_FuiNode::asMovieClip(m_slots[i])->getParent();
            parent->getParent()->removeChild(parent, true);
        }
    }
    m_slots.clear();
}

void FJ_SlotList::addSlot(int slot, bool highlighted, bool visible)
{
    if (!m_loaded)
        Init();
    if (!m_loaded)
        return;
    if (m_slots.size() >= (unsigned int)(m_rows * m_columns))
        return;

    FJ_FuiNode* container = FJ_Factory::createObject(
        "fourj.listClasses.FJ_SlotList", m_pRenderNode, true, NULL);
    FJ_FuiNode* parent = getParent();
    int index = parent->getChildIndex(this);
    parent->addChildAt(container, index);

    FJ_SlotList_Slot* slotNode = static_cast<FJ_SlotList_Slot*>(FJ_Factory::createObject(
        "fourj.listClasses.FJ_SlotList_Slot", m_pRenderNode, true, NULL));
    slotNode->SetParams(m_slotSize, m_slotVisible);
    unsigned int count = (unsigned int)m_slots.size();
    int column = m_columns != 0 ? (int)(count % (unsigned int)m_columns) : 0;
    int row = m_columns != 0 ? (int)(count / (unsigned int)m_columns) : 0;
    container->setX(getX() + (float)(column * m_slotSize));
    container->setY(getY() + (float)(row * m_slotSize));
    container->addChild(slotNode);
    slotNode->SetSlotID(slot);
    m_slots.push_back(slotNode);
    slotNode->SetHighlightActive(highlighted);
    slotNode->SetRedBoxActive(visible);
}

bool FJ_SlotList::SetSlotRedBox(int slot, bool show)
{
    if (m_slots.empty() || slot < 0 || slot >= (int)m_slots.size() || m_slots[slot] == NULL)
        return false;
    m_slots[slot]->SetRedBoxActive(show);
    return true;
}

bool FJ_SlotList::SetSlotHighlight(int slot, bool highlighted)
{
    if (m_slots.empty() || slot < 0 || slot >= (int)m_slots.size() || m_slots[slot] == NULL)
        return false;
    m_slots[slot]->SetHighlightActive(highlighted);
    return true;
}

void FJ_SlotList::setVisible(bool visible)
{
    if (!m_slots.empty())
    {
        for (unsigned int i = 0; i < m_slots.size(); ++i)
        {
            if (m_slots[i]->getParent() != NULL)
                m_slots[i]->getParent()->setVisible(visible);
        }
        return;
    }
    FJ_FuiNode::setVisible(visible);
}
