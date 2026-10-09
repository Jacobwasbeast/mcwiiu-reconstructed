#include "nn/pia/transport/NetworkRttManager.hpp"

#include <algorithm>
#include <cstring>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/common.hpp"
#include "nn/pia/transport/TransportSupport.hpp"

namespace nn { namespace pia { namespace transport {

static const unsigned long long cTraceCategoryRtt = 0x8000;
static const unsigned long long cTraceCategoryRttDetail = 0x40000;

namespace
{
enum
{
    ResultAlreadyInitialized = 0x10c02,
    ResultAllocationFailed = 0x10c01
};
}

NetworkRttManager* NetworkRttManager::sInstance = 0;
static unsigned char* sStorage = 0;

template <class T>
NetworkRttManager::DataBuffer<T>::DataBuffer()
    : mValues(), mData(0), mCritical(-1)
{
}

template <class T>
void NetworkRttManager::DataBuffer<T>::Initialize(unsigned int capacity, pead::Heap* heap)
{
    Finalize();
    if (capacity == 0) return;
    if (!heap) heap = common::HeapManager::GetHeap();
    mData = static_cast<T*>(pead::allocateFromSeadHeap(sizeof(T) * capacity, heap));
    if (!mData)
    {
        mData = 0;
        return;
    }
    for (unsigned int i = 0; i < capacity; ++i)
    {
        new (&mData[i]) T();
    }
    mValues.allocBuffer(static_cast<int>(capacity), heap, 4);
}

template <class T>
void NetworkRttManager::DataBuffer<T>::Finalize()
{
    mValues.freeBuffer();
    if (mData)
    {
        const unsigned int recordCount = pead::getArraySize(mData) / sizeof(T);
        for (unsigned int i = 0; i < recordCount; ++i) mData[i].~T();
        pead::freeToSeadHeap(mData);
    }
    mData = 0;
}

template <class T>
T* NetworkRttManager::DataBuffer<T>::Get(const common::StationAddress& address)
{
    const int result = mValues.binarySearch(&address, &Compare);
    if (result >= 0)
    {
        const unsigned int recordCount = pead::getArraySize(mData) / sizeof(T);
        for (unsigned int i = 0; i < recordCount; ++i)
            if (AddressOf(mData[i]) == address) return &mData[i];
    }
    return 0;
}

template <class T>
T* NetworkRttManager::DataBuffer<T>::Assign(const common::StationAddress& address, unsigned int now)
{
    T* existing = Get(address);
    if (existing)
    {
        Refresh(*existing, now);
        return existing;
    }
    if (!mData)
        return 0;
    const unsigned int recordCount = pead::getArraySize(mData) / sizeof(T);
    unsigned int activeCount = 0;
    for (unsigned int i = 0; i < recordCount; ++i)
        if (mValues.binarySearch(&AddressOf(mData[i]), &Compare) >= 0) ++activeCount;
    if (activeCount >= recordCount)
    {
        unsigned int oldest = 0;
        while (mValues.binarySearch(&AddressOf(mData[oldest]), &Compare) < 0) ++oldest;
        unsigned int oldestAge = now - EntryTime(mData[oldest]);
        for (unsigned int i = oldest + 1; i < recordCount; ++i)
        {
            if (mValues.binarySearch(&AddressOf(mData[i]), &Compare) < 0) continue;
            const unsigned int age = now - EntryTime(mData[i]);
            if (age > oldestAge) { oldest = i; oldestAge = age; }
        }
        T* value = &mData[oldest];
        const int oldIndex = mValues.binarySearch(&AddressOf(*value), &Compare);
        if (oldIndex >= 0) mValues.erase(oldIndex, 1);
        Reset(*value);
        InitializeValue(*value, address, now);
        const int insertion = mValues.binarySearch(&address, &Compare);
        const int position = insertion < 0 ? -insertion - 1 : insertion;
        mValues.insert(position, value);
        return value;
    }
    unsigned int freeIndex = 0;
    while (freeIndex < recordCount && mValues.binarySearch(&AddressOf(mData[freeIndex]), &Compare) >= 0) ++freeIndex;
    T* value = &mData[freeIndex];
    Reset(*value);
    InitializeValue(*value, address, now);
    const int insertion = mValues.binarySearch(&address, &Compare);
    const int position = insertion < 0 ? -insertion - 1 : insertion;
    mValues.insert(position, value);
    return value;
}

template class NetworkRttManager::DataBuffer<NetworkRttManager::MeasurementData>;
template class NetworkRttManager::DataBuffer<NetworkRttManager::ResponseData>;

NetworkRttManager::StatUnit::StatUnit() { Clear(); }

void NetworkRttManager::StatUnit::Clear()
{
    latest = 0;
    total = 0;
    squares = 0;
    count = 0;
}

void NetworkRttManager::StatUnit::AddValue(unsigned int value)
{
    const unsigned int square = value * value;
    if (squares + square < squares)
        return;
    latest = value;
    total += value;
    squares += square;
    ++count;
}

NetworkRttManager::MeasurementData::MeasurementData() : reserved(0)
{
    Clear();
}

void NetworkRttManager::MeasurementData::Clear()
{
    for (unsigned int i = 0; i < 16; ++i)
        values[i].Clear();
    time = 0;
}

unsigned int NetworkRttManager::MeasurementData::UpdateTime(unsigned int now)
{
    const unsigned int current = now >> 8;
    const unsigned int previous = time >> 8;
    if (current != previous)
    {
        if (current - previous >= 16)
        {
            Clear();
            time = now;
            return current;
        }
        const unsigned int slot = current & 15;
        const unsigned int previousSlot = previous & 15;
        if (slot < previousSlot)
        {
            for (unsigned int i = 0; i <= slot; ++i)
                values[i].Clear();
            for (unsigned int i = previousSlot + 1; i < 16; ++i)
                values[i].Clear();
        }
        else
        {
            for (unsigned int i = previousSlot + 1; i <= slot; ++i)
                values[i].Clear();
        }
    }
    time = now;
    return current;
}

void NetworkRttManager::MeasurementData::AddValue(unsigned int now, unsigned int value)
{
    const unsigned int slot = UpdateTime(now) & 15;
    values[slot].AddValue(value);
}

unsigned int NetworkRttManager::MeasurementData::GetAverage() const
{
    unsigned int total = 0;
    unsigned int count = 0;
    const StatUnit* unit = values;
    for (unsigned int i = 0; i < 16; ++i, ++unit)
    {
        total += unit->total;
        count += unit->count;
    }
    unsigned int average = 0;
    if (count != 0) average = total / count;
    return average;
}

int NetworkRttManager::MeasurementData::GetStdDev() const
{
    unsigned int total = 0;
    unsigned long long squares = 0;
    unsigned int count = 0;
    const StatUnit* unit = values;
    for (unsigned int i = 0; i < 16; ++i, ++unit)
    {
        squares += unit->squares;
        total += unit->total;
        count += unit->count;
    }
    if (count != 0)
    {
        const float n = static_cast<float>(count);
        const float mean = static_cast<float>(total) / n;
        const float variance = static_cast<float>(squares) / n - mean * mean;
        if (variance > 0.0f) return static_cast<unsigned int>(std::sqrt(variance));
    }
    return 0;
}

int NetworkRttManager::MeasurementData::GetCount() const
{
    int count = 0; for (unsigned int i = 0; i < 16; ++i) count += values[i].count; return count;
}

unsigned int NetworkRttManager::MeasurementData::GetLatest() const
{
    const unsigned int current = (time >> 8) & 15;
    for (int i = current; i >= 0; --i)
    {
        if (values[i].count != 0) return values[i].latest;
    }
    for (int i = 15; i > static_cast<int>(current); --i)
    {
        if (values[i].count != 0) return values[i].latest;
    }
    return 0;
}

void NetworkRttManager::MeasurementData::GetValues(StatValues* output) const
{
    unsigned int total = 0;
    unsigned long long squares = 0;
    unsigned int count = 0;
    const StatUnit* unit = values;
    for (unsigned int i = 0; i < 16; ++i, ++unit)
    {
        total += unit->total;
        squares += unit->squares;
        count += unit->count;
    }
    if (count == 0)
    {
        output->standardDeviation = 0;
        output->latest = 0;
        output->average = 0;
    }
    else
    {
        const float n = static_cast<float>(count);
        const float mean = static_cast<float>(total) / n;
        output->average = total / count;
        const float variance = static_cast<float>(squares) / n - mean * mean;
        if (variance <= 0.0f)
            output->standardDeviation = 0;
        else
            output->standardDeviation = static_cast<unsigned int>(std::sqrt(variance));
        output->count = count;
        output->latest = GetLatest();
    }
}

NetworkRttManager::NetworkRttManager()
{
}

NetworkRttManager::~NetworkRttManager() { Finalize(); }

Result NetworkRttManager::CreateInstance()
{
    if (IsInitialized() == 0) return 0x10c0c;
    if (IsInSetupMode() == 0) return 0x10c08;
    if (sInstance) return 0x10c0f;
    pead::Heap* heap = common::HeapManager::GetHeap();
    sStorage = static_cast<unsigned char*>(pead::allocateFromSeadHeap(0x80, heap, 8));
    if (!sStorage) return ResultAllocationFailed;
    sInstance = new (sStorage) NetworkRttManager();
    return sInstance ? 0 : ResultAllocationFailed;
}

void NetworkRttManager::Finalize()
{
    mResponses.Finalize();
    mMeasurements.Finalize();
}

void NetworkRttManager::DestroyInstance()
{
    if (sInstance)
    {
        sInstance->~NetworkRttManager();
        sInstance = 0;
    }
    if (sStorage)
    {
        common::DeleteArray(sStorage);
        sStorage = 0;
    }
}

Result NetworkRttManager::InitializeImpl(unsigned int measurementCapacity, unsigned int responseCapacity)
{
    if (mMeasurements.IsInitialized() || mResponses.IsInitialized()) return ResultAlreadyInitialized;
    pead::Heap* heap = common::HeapManager::GetHeap();
    mMeasurements.Initialize(measurementCapacity, heap);
    mResponses.Initialize(responseCapacity, heap);
    mStartTime.SetNow();
    return 0;
}

void NetworkRttManager::WriteToPacket(common::Packet* packet)
{
    const unsigned int now = GetElapsedMilliseconds();
    packet->m_packetData.m_header.m_rttResponseFlag = mMeasurements.IsInitialized() ? static_cast<unsigned short>(now) : 0;
    const common::StationAddress& destination = packet->m_field5f4;
    unsigned short measurement = 0;
    if (destination.IsValid())
    {
        common::CriticalSection& lock = mResponses.GetCriticalSection();
        lock.Lock();
        ResponseData* response = mResponses.Get(destination);
        if (response)
        {
            const unsigned int delay = now - response->time;
            if (delay < 10000)
            {
                measurement = static_cast<unsigned short>(response->sequence + delay);
                if (measurement == 0) measurement = 0xffff;
            }
        }
        lock.Unlock();
    }
    packet->m_packetData.m_header.m_rttMeasurement = measurement;
}

void NetworkRttManager::ReadFromPacket(const common::Packet* packet)
{
    const common::StationAddress& source = packet->m_field608;
    if (!source.IsValid())
    {
        source.Trace(cTraceCategoryRttDetail);
        return;
    }
    const unsigned int now = GetElapsedMilliseconds();
    if (packet->m_packetData.m_header.m_rttResponseFlag != 0)
    {
        common::CriticalSection& lock = mResponses.GetCriticalSection();
        lock.Lock();
        ResponseData* response = mResponses.Assign(source, now);
        if (response)
        {
            response->sequence = packet->m_packetData.m_header.m_rttResponseFlag;
        }
        else
        {
            source.Trace(cTraceCategoryRtt);
        }
        lock.Unlock();
    }
    const unsigned short measurement = packet->m_packetData.m_header.m_rttMeasurement;
    if (measurement != 0)
    {
        const unsigned int rtt = (now - measurement) & 0xffff;
        common::CriticalSection& lock = mMeasurements.GetCriticalSection();
        lock.Lock();
        MeasurementData* value = mMeasurements.Assign(source, now);
        source.Trace(cTraceCategoryRttDetail);
        if (value) value->AddValue(now, rtt);
        lock.Unlock();
    }
}

unsigned int NetworkRttManager::GetAverage(const common::StationAddress& address)
{
    mMeasurements.Lock();
    MeasurementData* value = mMeasurements.Get(address);
    if (!value)
    {
        address.Trace(cTraceCategoryRtt);
        mMeasurements.Unlock();
        return 0;
    }
    value->UpdateTime(GetElapsedMilliseconds());
    unsigned int result = value->GetAverage();
    mMeasurements.Unlock();
    return result;
}
int NetworkRttManager::GetStdDev(const common::StationAddress& address)
{
    mMeasurements.Lock();
    MeasurementData* value = mMeasurements.Get(address);
    if (!value)
    {
        address.Trace(cTraceCategoryRtt);
        mMeasurements.Unlock();
        return 0;
    }
    value->UpdateTime(GetElapsedMilliseconds());
    int result = value->GetStdDev();
    mMeasurements.Unlock();
    return result;
}
int NetworkRttManager::GetCount(const common::StationAddress& address)
{
    mMeasurements.Lock();
    MeasurementData* value = mMeasurements.Get(address);
    if (!value)
    {
        address.Trace(cTraceCategoryRtt);
        mMeasurements.Unlock();
        return 0;
    }
    value->UpdateTime(GetElapsedMilliseconds());
    int result = value->GetCount();
    mMeasurements.Unlock();
    return result;
}
unsigned int NetworkRttManager::GetLatest(const common::StationAddress& address)
{
    mMeasurements.Lock();
    MeasurementData* value = mMeasurements.Get(address);
    if (!value)
    {
        address.Trace(cTraceCategoryRtt);
        mMeasurements.Unlock();
        return 0;
    }
    value->UpdateTime(GetElapsedMilliseconds());
    unsigned int result = value->GetLatest();
    mMeasurements.Unlock();
    return result;
}
void NetworkRttManager::GetValues(StatValues* values, const common::StationAddress& address)
{
    MeasurementData* value;
    common::CriticalSection& lock = mMeasurements.GetCriticalSection();
    lock.Lock();
    value = mMeasurements.Get(address);
    if (!value)
    {
        address.Trace(cTraceCategoryRtt);
        values->average = 0;
        values->standardDeviation = 0;
        values->count = 0;
        values->latest = 0;
        lock.Unlock();
        return;
    }
    value->UpdateTime(GetElapsedMilliseconds());
    value->GetValues(values);
    lock.Unlock();
}

int NetworkRttManager::CompareFunc(const ResponseData* value, const common::StationAddress* address)
{
    return common::StationAddress::Compare(value->address, *address);
}
int NetworkRttManager::CompareFunc(const MeasurementData* value, const common::StationAddress* address)
{
    return common::StationAddress::Compare(value->address, *address);
}

}}}
