#include "nn/pia/transport/NetworkRttManager.hpp"

#include <algorithm>
#include <cstring>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Common.hpp"
#include "nn/pia/transport/TransportSupport.hpp"

namespace nn { namespace pia { namespace transport {

namespace
{
enum
{
    ResultAlreadyInitialized = 0x10c02,
    ResultAllocationFailed = 0x10c01
};

common::StationAddress& packetDestination(common::Packet& packet)
{ return packet.m_field5f4; }
const common::StationAddress& packetDestination(const common::Packet& packet)
{ return packet.m_field5f4; }
common::StationAddress& packetSource(common::Packet& packet)
{ return packet.m_field608; }
const common::StationAddress& packetSource(const common::Packet& packet)
{ return packet.m_field608; }
unsigned short& packetResponseFlag(common::Packet& packet)
{ return packet.m_packetData.m_header.m_field0c; }
unsigned short& packetMeasurement(common::Packet& packet)
{ return packet.m_packetData.m_header.m_field0e; }
unsigned short packetResponseFlag(const common::Packet& packet)
{ return packet.m_packetData.m_header.m_field0c; }
unsigned short packetMeasurement(const common::Packet& packet)
{ return packet.m_packetData.m_header.m_field0e; }
}

NetworkRttManager* NetworkRttManager::sInstance = 0;
static void* sStorage = 0;

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
        if (current - previous > 15)
        {
            Clear();
            time = 0;
            return current;
        }
        else
        {
            unsigned int slot = previous & 15;
            const unsigned int target = current & 15;
            while (slot != target)
            {
                slot = (slot + 1) & 15;
                values[slot].Clear();
            }
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
    for (unsigned int i = 0; i < 16; ++i) { total += values[i].total; count += values[i].count; }
    return count == 0 ? 0 : static_cast<unsigned int>(total / count);
}

int NetworkRttManager::MeasurementData::GetStdDev() const
{
    unsigned int total = 0;
    unsigned long long squares = 0;
    unsigned int count = 0;
    for (unsigned int i = 0; i < 16; ++i) { total += values[i].total; squares += values[i].squares; count += values[i].count; }
    if (count == 0) return 0;
    const double mean = static_cast<double>(total) / count;
    const double variance = std::max(0.0, static_cast<double>(squares) / count - mean * mean);
    return static_cast<int>(std::sqrt(variance));
}

int NetworkRttManager::MeasurementData::GetCount() const
{
    int count = 0; for (unsigned int i = 0; i < 16; ++i) count += values[i].count; return count;
}

unsigned int NetworkRttManager::MeasurementData::GetLatest() const
{
    const unsigned int current = (time >> 8) & 15;
    for (unsigned int n = 0; n < 16; ++n)
    {
        const StatUnit& unit = values[(current + 16 - n) & 15];
        if (unit.count != 0) return unit.latest;
    }
    return 0;
}

void NetworkRttManager::MeasurementData::GetValues(StatValues* output) const
{
    output->average = GetAverage();
    output->standardDeviation = static_cast<unsigned int>(GetStdDev());
    output->count = static_cast<unsigned int>(GetCount());
    output->latest = GetLatest();
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
    sStorage = pead::allocateFromSeadHeap(0x80, heap, 8);
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
    if (sStorage) pead::freeToSeadHeap(sStorage);
    sStorage = 0;
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

Result NetworkRttManager::WriteToPacket(common::Packet* packet)
{
    common::Time now; now.SetNow();
    const unsigned int elapsed = static_cast<unsigned int>((now.m_tick - mStartTime.m_tick) / common::TimeSpan::GetTicksPerMSec().m_tick);
    packetResponseFlag(*packet) = 0;
    packetMeasurement(*packet) = 0;
    if (packetDestination(*packet).IsValid())
    {
        mResponses.Lock();
        ResponseData* response = mResponses.Get(packetDestination(*packet));
        packetResponseFlag(*packet) = response ? 1 : 0;
        if (response && elapsed - response->time < 10000)
        {
            const unsigned int value = response->sequence + elapsed - response->time;
            packetMeasurement(*packet) = static_cast<unsigned short>(value == 0 ? 0xffff : value);
        }
        mResponses.Unlock();
    }
    return 0;
}

Result NetworkRttManager::ReadFromPacket(const common::Packet* packet)
{
    if (!packetSource(*packet).IsValid())
    {
        packetSource(*packet).Trace(0x40000);
        return 0;
    }
    common::Time now; now.SetNow();
    const unsigned int elapsed = static_cast<unsigned int>((now.m_tick - mStartTime.m_tick) / common::TimeSpan::GetTicksPerMSec().m_tick);
    const unsigned short response = packetResponseFlag(*packet);
    const unsigned short measurement = packetMeasurement(*packet);
    if (response)
    {
        mResponses.Lock();
        ResponseData* value = mResponses.Assign(packetSource(*packet), elapsed);
        if (value) value->sequence = response;
        else packetSource(*packet).Trace(0x8000);
        mResponses.Unlock();
    }
    if (measurement)
    {
        mMeasurements.Lock();
        MeasurementData* value = mMeasurements.Assign(packetSource(*packet), elapsed);
        if (value) value->AddValue(elapsed, (elapsed - measurement) & 0xffff);
        packetSource(*packet).Trace(0x40000);
        mMeasurements.Unlock();
    }
    return 0;
}

unsigned int NetworkRttManager::GetAverage(const common::StationAddress& address)
{
    mMeasurements.Lock();
    MeasurementData* value = mMeasurements.Get(address);
    common::Time now; now.SetNow();
    const unsigned int elapsed = static_cast<unsigned int>((now.m_tick - mStartTime.m_tick) / common::TimeSpan::GetTicksPerMSec().m_tick);
    if (value) value->UpdateTime(elapsed);
    unsigned int result = value ? value->GetAverage() : 0;
    mMeasurements.Unlock();
    return result;
}
int NetworkRttManager::GetStdDev(const common::StationAddress& address)
{
    mMeasurements.Lock();
    MeasurementData* value = mMeasurements.Get(address);
    common::Time now; now.SetNow();
    const unsigned int elapsed = static_cast<unsigned int>((now.m_tick - mStartTime.m_tick) / common::TimeSpan::GetTicksPerMSec().m_tick);
    if (value) value->UpdateTime(elapsed);
    int result = value ? value->GetStdDev() : 0;
    mMeasurements.Unlock();
    return result;
}
int NetworkRttManager::GetCount(const common::StationAddress& address)
{
    mMeasurements.Lock();
    MeasurementData* value = mMeasurements.Get(address);
    common::Time now; now.SetNow();
    const unsigned int elapsed = static_cast<unsigned int>((now.m_tick - mStartTime.m_tick) / common::TimeSpan::GetTicksPerMSec().m_tick);
    if (value) value->UpdateTime(elapsed);
    int result = value ? value->GetCount() : 0;
    mMeasurements.Unlock();
    return result;
}
unsigned int NetworkRttManager::GetLatest(const common::StationAddress& address)
{
    mMeasurements.Lock();
    MeasurementData* value = mMeasurements.Get(address);
    common::Time now; now.SetNow();
    const unsigned int elapsed = static_cast<unsigned int>((now.m_tick - mStartTime.m_tick) / common::TimeSpan::GetTicksPerMSec().m_tick);
    if (value) value->UpdateTime(elapsed);
    unsigned int result = value ? value->GetLatest() : 0;
    mMeasurements.Unlock();
    return result;
}
void NetworkRttManager::GetValues(StatValues* values, const common::StationAddress& address)
{
    mMeasurements.Lock();
    MeasurementData* value = mMeasurements.Get(address);
    common::Time now; now.SetNow();
    const unsigned int elapsed = static_cast<unsigned int>((now.m_tick - mStartTime.m_tick) / common::TimeSpan::GetTicksPerMSec().m_tick);
    if (value) value->UpdateTime(elapsed);
    if (value) value->GetValues(values);
    else
    {
        values->average = 0;
        values->standardDeviation = 0;
        values->count = 0;
        values->latest = 0;
    }
    mMeasurements.Unlock();
}

int NetworkRttManager::CompareFunc(const ResponseData* value, const common::StationAddress* address)
{
    if (!value || !address) return -1;
    if (value->address < *address) return -1;
    return *address < value->address ? 1 : 0;
}
int NetworkRttManager::CompareFunc(const MeasurementData* value, const common::StationAddress* address)
{
    if (!value || !address) return -1;
    if (value->address < *address) return -1;
    return *address < value->address ? 1 : 0;
}

}}}
