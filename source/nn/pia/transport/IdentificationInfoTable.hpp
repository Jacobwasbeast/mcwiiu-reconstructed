#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_IDENTIFICATION_INFO_TABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_IDENTIFICATION_INFO_TABLE_HPP

#include <new>

#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>

#include "nn/pia.h"
#include "nn/pia/Common.hpp"
#include "nn/pia/ListBase.hpp"
#include "nn/pia/String.hpp"
#include "nn/pia/transport/Station.hpp"

namespace nn { namespace pia { namespace transport {

class IdentificationInfo
{
public:
    struct Field20
    {
        unsigned char m_bytes[64];
    };

    Station::IdentificationToken m_field00;
    Field20 m_field20;
    common::FixedString<20> m_field60[1];
    common::FixedString<10> m_fieldC8[1];
    unsigned char m_field108;
    unsigned char m_field109;
    unsigned char m_playerCount;
    unsigned char m_participantCount;
    unsigned int m_field10c;
    unsigned long long m_principalId;
};

class IdentificationInfoTable : public IdentificationInfo
{
public:
    IdentificationInfoTable();
    ~IdentificationInfoTable();
    static Result CreateInstance();
    static void DestroyInstance();
    static IdentificationInfoTable* GetInstance();
    void Finalize();
    Result Initialize(unsigned short count);
    void ClearIdentificationInfo(IdentificationInfo* info);
    void SetLocalIdentificationInfo(const IdentificationInfo* info);
    Result GetLocalIdentificationInfo(IdentificationInfo* info);
    void ClearLocalIdentificationInfo();
    Result AddToTable(const Station* station, const IdentificationInfo* info, unsigned char kind);
    IdentificationInfo* GetIdentificationInfoPtr(const Station* station);
    Result GetIdentificationInfo(const Station* station, IdentificationInfo* info);
    IdentificationInfo* GetIdentificationInfoPtr(unsigned long long principalId);
    Result GetIdentificationInfo(unsigned long long principalId, IdentificationInfo* info);
    Result GetPlayerCount(const Station* station, unsigned char* count);
    Result GetParticipantCount(const Station* station, unsigned char* count);
    void EraseFromTable(const Station* station);
    void ClearTable();
    void Trace(unsigned long long value) const;

private:
    struct Record
    {
        IdentificationInfo m_info;
        unsigned char m_state;
        const Station* m_station;
    };

    struct Slot
    {
        Slot() { m_node.prev = 0; m_node.next = 0; }

        Record* GetRecord() { return reinterpret_cast<Record*>(&m_recordStorage); }

        common::ListNode m_node;
        boost::aligned_storage<sizeof(Record), boost::alignment_of<Record>::value>::type
            m_recordStorage;
    };

    struct OffsetList : public common::ListBase
    {
        OffsetList() : m_offset(-1) {}

        int m_offset;
    };

    struct SlotPoolBase
    {
        SlotPoolBase() : m_slots(0), m_slotCount(0) { m_freeSlots.m_offset = 0; }

        common::ListBase m_usedSlots;
        OffsetList m_freeSlots;
        Slot* m_slots;
        unsigned int m_slotCount;
    };

    struct SlotPool : public SlotPoolBase
    {
        SlotPool()
        {
            m_freeSlots.m_offset = 0;
            m_slots = reinterpret_cast<Slot*>(m_storage);
            m_slotCount = 32;
            for (unsigned int i = 0; i < m_slotCount; ++i)
            {
                Slot* slot = new (&m_slots[i]) Slot();
                m_freeSlots.InsertBeforeNode(
                    m_freeSlots.Sentinel(),
                    reinterpret_cast<common::ListNode*>(
                        reinterpret_cast<char*>(slot) + m_freeSlots.m_offset));
            }
        }

        unsigned char m_storage[32 * sizeof(Slot)];
    };

    SlotPool m_pool;
    unsigned short m_count;
    common::CriticalSection m_criticalSection;
};

}}}

#endif
