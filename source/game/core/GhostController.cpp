#include "game/core/GhostController.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/System.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/generation/GhostPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/util/phys__Vec2.hpp"
#include <cmath>
#include <sstream>
#include <cstdlib>
#include <cstring>

GhostController::GhostRecordSample::GhostRecordSample(const GhostRecordSample& sample) { _init(sample.x, sample.y, sample.z, sample.rotX, sample.rotY, sample.rotZ, sample.timestamp); }
GhostController::GhostRecordSample::GhostRecordSample(short a, short b, short c, short d, short e, short f, long long t) { _init(a,b,c,d,e,f,t); }
GhostController::GhostRecordSample::GhostRecordSample(DataInputStream* stream) { read(stream); }
void GhostController::GhostRecordSample::_init(short a, short b, short c, short d, short e, short f, long long t) { x=a; y=b; z=c; rotX=d; rotY=e; rotZ=f; timestamp=t; }
void GhostController::GhostRecordSample::read(DataInputStream* stream) {
    timestamp = stream->readLong();
    x = stream->readShort();
    y = stream->readShort();
    z = stream->readShort();
    rotX = stream->readShort();
    rotY = stream->readShort();
    rotZ = stream->readShort();
}
unsigned int GhostController::GhostRecordSample::getSampleSize() { return 20; }
void GhostController::GhostRecordSample::write(DataOutputStream* stream) {
    stream->writeLong(timestamp);
    stream->writeShort(x);
    stream->writeShort(y);
    stream->writeShort(z);
    stream->writeShort(rotX);
    stream->writeShort(rotY);
    stream->writeShort(rotZ);
}

GhostController::GhostRecord::GhostRecord(Vec3* pos) : hasRecording(false), startPos(), mElapsedHigh(0), mElapsedLow(0) { startPos = Vec3::newPermanent(pos); samples.reserve(1200); }
GhostController::GhostRecord::~GhostRecord() {}
unsigned int GhostController::GhostRecord::GetPosAtIndex(unsigned int index)
{
    if (index >= samples.size())
        return 0;
    const GhostRecordSample& sample = samples[index];
    Vec3* position = Vec3::newTemp(startPos.x + (double)sample.x / 10.0,
                                    startPos.y + (double)sample.y / 10.0,
                                    startPos.z + (double)sample.z / 10.0);
    return (unsigned int)(uintptr_t)position;
}
bool GhostController::GhostRecord::HasRecording() const { return hasRecording; }
void GhostController::GhostRecord::ResetHasRecording() { hasRecording=false; }
void GhostController::GhostRecord::AddSample(double a, double b, double c, float d, float e, float f) {
    const long long timestamp = (static_cast<long long>(mElapsedHigh) << 32) | mElapsedLow;
    GhostRecordSample sample((short)GhostController::Double1DPAsInt16(a - startPos.x),
                             (short)GhostController::Double1DPAsInt16(b - startPos.y),
                             (short)GhostController::Double1DPAsInt16(c - startPos.z),
                             (short)GhostController::Double1DPAsInt16(d),
                             (short)GhostController::Double1DPAsInt16(e),
                             (short)GhostController::Double1DPAsInt16(f), timestamp);
    samples.push_back(sample);
    hasRecording = true;
}
int GhostController::GhostRecord::NumSamples() const { return (int)samples.size(); }
long long GhostController::GhostRecord::GetTimestampAtIndex(unsigned int index)
{
    return index < samples.size() ? samples[index].timestamp : -1;
}
void GhostController::GhostRecord::AddSample(const GhostRecordSample& sample) { samples.push_back(sample); hasRecording=true; }
void GhostController::GhostRecord::WriteStartPos(DataOutputStream* stream) { stream->writeFloat((float)startPos.x); stream->writeFloat((float)startPos.y); stream->writeFloat((float)startPos.z); }
void GhostController::GhostRecord::WriteSampleAtIndex(unsigned int index, DataOutputStream* stream) { if (index < samples.size()) samples[index].write(stream); }
unsigned int GhostController::GhostRecord::GetRotAtIndex(unsigned int index)
{
    if (index >= samples.size())
        return 0;
    const GhostRecordSample& sample = samples[index];
    Vec3* rotation = Vec3::newTemp((double)sample.rotX / 10.0,
                                    (double)sample.rotY / 10.0,
                                    (double)sample.rotZ / 10.0);
    return (unsigned int)(uintptr_t)rotation;
}

double GhostController::GhostRecord::GetVelocityAtIndex(unsigned int index)
{
    if (index == 0 || index >= samples.size())
        return 0.0;
    const GhostRecordSample& previous = samples[index - 1];
    const GhostRecordSample& current = samples[index];
    const float dx = (float)(current.x - previous.x) / 10.0f;
    const float dy = (float)(current.y - previous.y) / 10.0f;
    const float dz = (float)(current.z - previous.z) / 10.0f;
    return (double)std::sqrt(dx * dx + dy * dy + dz * dz);
}

GhostController::TrackHeader::TrackHeader() : time(0), score(0), completed(false), recorded(false), rank(0) {}
GhostController::GhostLoadSaveOperation::GhostLoadSaveOperation(EOpType t, int i, GhostRecord* r) : type(t), track(i), record(r) {}

int32_t GhostController::Double1DPAsInt16(double value) { return (int32_t)(short)std::floor(value*10.0+0.5); }
GhostController::GhostController(int mode, const std::wstring& name, bool saving) : mGameMode(7), mLevelName(), mSaving(true), mDone(false), mPlayback(false), mLastTickHigh(0), mLastTickLow(0), mPlaybackTimeHigh(0), mPlaybackTimeLow(0), mPlaybackIndex(0), mLastRecordingChangeTimeHigh(0), mLastRecordingChangeTimeLow(0), mAnyRecordingActive(false), mSaveHeaderBuffer(NULL), mSaveHeaderLength(0), mSaveHeaderStatus(6) {
    int i;
    InitializeCriticalSection(&mQueueLock);
    for (i = 0; i < 4; ++i) {
        mRecording[i] = NULL;
        mLoaded[i] = NULL;
        mRecordingActive[i] = false;
        mRecordingSlotFlags1[i] = false;
        mRecordingSlotFlags2[i] = false;
        mTrackHeaderEverSaved[i] = false;
    }
    LoadGhostHeader(0); LoadGhostHeader(1); LoadGhostHeader(2); LoadGhostHeader(3);
    mTrackHeaderEverSaved[0] = true; mTrackHeaderEverSaved[1] = true;
    mTrackHeaderEverSaved[2] = true; mTrackHeaderEverSaved[3] = true;
    Initialise(mode, name, saving);
}
GhostController::~GhostController() { int i; for(i=0;i<4;i++){delete mRecording[i];delete mLoaded[i];} DeleteCriticalSection(&mQueueLock); }
void GhostController::addLoadSaveOp(GhostLoadSaveOperation::EOpType t,int i,GhostRecord* r){EnterCriticalSection(&mQueueLock);mQueue.push_back(GhostLoadSaveOperation(t,i,r));LeaveCriticalSection(&mQueueLock);}
void GhostController::LoadGhostRecord(int i){if(gStorage!=NULL)addLoadSaveOp(GhostLoadSaveOperation::LOAD_RECORD,i,0);}
void GhostController::Initialise(int mode,const std::wstring& name,bool saving)
{
    mGameMode = mode;
    mLevelName = name;
    mSaving = saving;
    mDone = false;
    mPlayback = false;
    mAnyRecordingActive = false;
    for (int i = 0; i < 4; ++i)
    {
        mRecordingActive[i] = false;
        mRecordingSlotFlags1[i] = false;
        mRecordingSlotFlags2[i] = false;
        delete mLoaded[i];
        mLoaded[i] = NULL;
        delete mRecording[i];
        mRecording[i] = NULL;
        LoadGhostRecord(i);
    }
}
void GhostController::LoadGhostHeader(int i){if(gStorage!=NULL)addLoadSaveOp(GhostLoadSaveOperation::LOAD_HEADER,i,0);}
void GhostController::reset(int mode, const std::wstring& name, bool saving)
{
    mPlaybackTimeHigh = 0;
    mPlaybackTimeLow = 0;
    mPlayback = false;
    if (mGameMode != mode) {
        Initialise(mode, name, saving);
        return;
    }
    for (int i = 0; i < 4; ++i) {
        mRecordingActive[i] = false;
        mRecordingSlotFlags1[i] = false;
        mRecordingSlotFlags2[i] = false;
        if (mRecording[i] != NULL) {
            delete mRecording[i];
            mRecording[i] = NULL;
        }
    }
}
std::wstring GhostController::getGhostPlayerName(int i){std::wstringstream stream;stream<<L"GHOST_CONTROLLER_PLAYER_"<<i;return stream.str();}
void GhostController::createGhostPlayers()
{
    Level* level = static_cast<Level*>(Minecraft::GetInstance()->getLevel(0));
    for (int i = 0; i < 4; ++i)
        mGhostPlayers[i] = boost::shared_ptr<GhostPlayer>(new GhostPlayer(level, getGhostPlayerName(i)));
}
void GhostController::startPlayback()
{
    mRecordingSlotFlags2[0] = mLoaded[0] != NULL;
    mRecordingSlotFlags2[1] = mLoaded[1] != NULL;
    mRecordingSlotFlags2[2] = mLoaded[2] != NULL;
    mRecordingSlotFlags2[3] = mLoaded[3] != NULL;
    mPlaybackIndex = 0;
    const uint64_t now = System::processTimeInMilliSecs();
    mLastTickHigh = static_cast<int>(now >> 32);
    mLastTickLow = static_cast<unsigned int>(now);
    createGhostPlayers();
}
boost::shared_ptr<GhostPlayer> GhostController::getGhostPlayer(int i){return i>=0 && i<4 && mLoaded[i]!=NULL?mGhostPlayers[i]:boost::shared_ptr<GhostPlayer>();}
std::vector<boost::shared_ptr<GhostPlayer> > GhostController::getGhostPlayers()
{
    std::vector<boost::shared_ptr<GhostPlayer> > players;
    for (int i = 0; i < 4; ++i)
    {
        if (mGhostPlayers[i] && mLoaded[i] != NULL)
            players.push_back(mGhostPlayers[i]);
    }
    return players;
}
std::vector<boost::shared_ptr<Player> > GhostController::getPlayers()
{
    std::vector<boost::shared_ptr<Player> > players;
    for (int i = 0; i < 4; ++i)
    {
        if (mGhostPlayers[i] && mLoaded[i] != NULL)
            players.push_back(mGhostPlayers[i]);
    }
    return players;
}
void GhostController::stopPlayback(){mRecordingSlotFlags2[0]=false;mRecordingSlotFlags2[1]=false;mRecordingSlotFlags2[2]=false;mRecordingSlotFlags2[3]=false;}
void GhostController::stopRecording(int i){if(i>=0&&i<4){if(mRecording[i])mRecording[i]->ResetHasRecording();setRecordingActive(i,false);delete mRecording[i];mRecording[i]=NULL;}}
void GhostController::UpdateGhostRecord(int i, long long t)
{
    if (i < 0 || i >= 4)
        return;

    GhostRecord* record = mRecording[i];
    if (record == NULL)
        return;

    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft == NULL)
        return;

    boost::shared_ptr<Player> player = minecraft->GetPlayerByPlayerIndex(i);
    if (!player)
        return;

    const unsigned int deltaLow = (unsigned int)t;
    const int deltaHigh = (int)(t >> 32);
    const unsigned int carryAdd = (record->mElapsedLow + deltaLow < record->mElapsedLow) ? 1u : 0u;
    const unsigned int newLow = record->mElapsedLow + deltaLow;
    const int newHigh = record->mElapsedHigh + deltaHigh + (int)carryAdd;

    record->mElapsedLow = newLow;
    record->mElapsedHigh = newHigh;

    if (newHigh >= 0 && (newHigh > 0 || newLow > 300000u))
    {
        stopRecording(i);
        return;
    }

    phys::Vec2* rotation = player->getRotationVector();
    record->AddSample(player->mX, player->mY, player->mZ,
                      rotation->x, rotation->y, player->mYBodyRot);
}
void GhostController::setRecordingActive(int i,bool active)
{
    if(i<0||i>=4)return;
    const uint64_t now = System::processTimeInMilliSecs();
    mLastRecordingChangeTimeHigh = static_cast<int>(now >> 32);
    mLastRecordingChangeTimeLow = static_cast<unsigned int>(now);
    mRecordingActive[i] = active;
    mAnyRecordingActive = mRecordingActive[0] || mRecordingActive[1] || mRecordingActive[2] || mRecordingActive[3];
    if (active)
    {
        mRecordingSlotFlags1[i] = true;
        UpdateGhostRecord(i, 0);
    }
}
void GhostController::startRecording(int i,Vec3* pos){if(i>=0&&i<4){delete mRecording[i];mRecording[i]=new GhostRecord(pos);setRecordingActive(i,true);}}
unsigned short GhostController::GetBestScore(int i) const {if(i<0||i>=4)return 0;boost::unordered_map<int,TrackHeader>::const_iterator it=mTrackHeaders[i].find(mGameMode);return it==mTrackHeaders[i].end()?0:it->second.score;}
bool GhostController::HasFinished(GhostRecord* r){return r && r->NumSamples()-2U<=mPlaybackIndex;}
unsigned int GhostController::getPlaybackIndex(GhostRecord* record, float& part)
{
    part = 1.0f;
    if (mGhostPlayers[0] && record != NULL && record->HasRecording())
    {
        const uint64_t now = System::processTimeInMilliSecs();
        const unsigned int deltaLow = static_cast<unsigned int>(now) - mLastTickLow;
        const int deltaHigh = static_cast<int>(now >> 32) - (mLastTickHigh + static_cast<int>(static_cast<unsigned int>(now) < mLastTickLow));
        const long long elapsed = (static_cast<long long>(deltaHigh) << 32) | deltaLow;

        unsigned int index = mPlaybackIndex;
        unsigned int count = static_cast<unsigned int>(record->NumSamples());

        if (!(deltaHigh < 0 || (deltaHigh < 1 && deltaLow == 0)))
        {
            if (count > index)
            {
                unsigned int candidate = index == 0 ? 0 : index - 1;
                unsigned int i = index;
                for (;;)
                {
                    const long long timestamp = record->GetTimestampAtIndex(i);
                    if (elapsed < timestamp)
                    {
                        mPlaybackIndex = candidate;
                        index = candidate;
                        count = static_cast<unsigned int>(record->NumSamples());
                        break;
                    }
                    ++i;
                    ++candidate;
                    const unsigned int n = static_cast<unsigned int>(record->NumSamples());
                    if (i >= n)
                        break;
                }
            }
        }

        if (index + 1 < count)
        {
            const long long t0 = record->GetTimestampAtIndex(index);
            const long long t1 = record->GetTimestampAtIndex(index + 1);
            const double d0 = static_cast<double>(elapsed - t0);
            const double d1 = static_cast<double>(t1 - t0);
            float frac = static_cast<float>(d0 / d1);
            if (frac > 1.0f)
                frac = 1.0f;
            part = frac;
        }
        else
        {
            part = 1.0f;
        }
    }
    return mPlaybackIndex;
}
std::wstring GhostController::BuildDatakey(bool header)
{
    if (header)
        return L"ghosts";
    return std::wstring(L"buckets_") + mLevelName;
}
void GhostController::SaveGhostHeader(int i){if(gStorage!=NULL)addLoadSaveOp(GhostLoadSaveOperation::SAVE_HEADER,i,0);}
void GhostController::SetTrackHeader(int track,int key,unsigned int time,unsigned short score,bool completed,bool recorded,bool save)
{
    bool changed = false;
    boost::unordered_map<int, TrackHeader>::iterator it = mTrackHeaders[track].find(key);
    if (it == mTrackHeaders[track].end())
    {
        TrackHeader& header = mTrackHeaders[track][key];
        header.time = time;
        header.score = score;
        header.completed = completed;
        header.recorded = recorded;
        changed = true;
    }
    else
    {
        TrackHeader& header = mTrackHeaders[track][key];
        if (header.time == 0 || time < header.time)
        {
            header.time = time;
            changed = true;
        }
        if (header.score < score)
        {
            header.score = score;
            changed = true;
        }
        if (completed && !header.completed)
        {
            header.completed = true;
            changed = true;
        }
        if (recorded && !header.recorded)
        {
            header.recorded = true;
            changed = true;
        }
    }
    if (save && (changed || !mTrackHeaderEverSaved[track]))
        SaveGhostHeader(track);
}
void GhostController::LoadGhostHeader(int track, void* data, unsigned int length)
{
    if (data == NULL || track < 0 || track >= 4) return;
    arrayWithLength<unsigned char> bytes(length, false);
    std::memcpy(bytes.data, data, length);
    ByteArrayInputStream input(bytes);
    DataInputStream stream(&input);
    if (stream.readUnsignedShort() != 3) return;
    const unsigned int count = (unsigned int)stream.readUnsignedShort();
    for (unsigned int i = 0; i < count; ++i)
        SetTrackHeader(track, stream.readUnsignedShort(), (unsigned int)stream.readInt(),
                       (unsigned short)stream.readUnsignedShort(), stream.readBoolean(),
                       stream.readBoolean(), false);
    input.mBuffer.data = NULL;
    input.mBuffer.length = 0;
    input.mCount = 0;
    input.mMark = 0;
    input.mPosition = 0;
}
void GhostController::LoadGhostHeader_Callback(void* context,int track,C4JStorage::eGeneralData_Callback_Status status,int)
{
    GhostController* controller = static_cast<GhostController*>(context);
    if (controller == NULL)
        return;
    if (controller->mSaveHeaderStatus == 0 && status == C4JStorage::eGeneralData_Callback_Status(0) && controller->mSaveHeaderBuffer != NULL)
        controller->LoadGhostHeader(track, controller->mSaveHeaderBuffer, controller->mSaveHeaderLength);
    if (controller->mSaveHeaderBuffer != NULL) {
        std::free(controller->mSaveHeaderBuffer);
        controller->mSaveHeaderBuffer = NULL;
    }
    controller->mSaveHeaderStatus = 6;
}
unsigned int GhostController::LoadGhostHeaderOp(int track)
{
    if (track < 0 || track >= 4)
        return 0;
    if (gStorage == NULL) return 0;
    return (unsigned int)gStorage->ReadFromGenericData(track, BuildDatakey(true), &mSaveHeaderBuffer, &mSaveHeaderLength, LoadGhostHeader_Callback, this);
}
unsigned int GhostController::GetTrackHeader(int t,int key) const {return t>=0&&t<4&&mTrackHeaders[t].find(key)!=mTrackHeaders[t].end()?mTrackHeaders[t].find(key)->second.time:0;}
void GhostController::SaveGhostRecord(int i,GhostRecord* r){if(gStorage!=NULL)addLoadSaveOp(GhostLoadSaveOperation::SAVE_RECORD,i,r);}
void GhostController::AddGhostRecord(int i,GhostRecord* r,bool loaded)
{
    if(i<0||i>=4)return;
    if(mLoaded[i]==NULL){mLoaded[i]=r;if(!loaded){SaveGhostRecord(i,r);}}
}
void GhostController::AddLoadedGhostRecord(int track, void* data, unsigned int length)
{
    if (data == NULL || track < 0 || track >= 4) return;
    arrayWithLength<unsigned char> bytes(length, false);
    std::memcpy(bytes.data, data, length);
    ByteArrayInputStream input(bytes);
    DataInputStream stream(&input);
    if (stream.readUnsignedShort() != 4) return;
    Vec3 start(stream.readFloat(), stream.readFloat(), stream.readFloat());
    GhostRecord* record = new GhostRecord(&start);
    const long long elapsed = stream.readLong();
    record->mElapsedHigh = (int)(elapsed >> 32);
    record->mElapsedLow = (unsigned int)elapsed;
    const int count = stream.readInt();
    for (int i = 0; i < count; ++i)
        record->AddSample(GhostRecordSample(&stream));
    if (record->HasRecording())
    {
        AddGhostRecord(track, record, true);
        mRecordingSlotFlags2[track] = true;
    }
    else
        delete record;
    input.mBuffer.data = NULL;
    input.mBuffer.length = 0;
    input.mCount = 0;
    input.mMark = 0;
    input.mPosition = 0;
}
void GhostController::LoadGhostRecord_Callback(void* context,int track,C4JStorage::eGeneralData_Callback_Status status,int)
{
    GhostController* controller = static_cast<GhostController*>(context);
    if (controller != NULL && !controller->mDone && status == C4JStorage::eGeneralData_Callback_Status(0) && controller->mSaveHeaderBuffer != NULL) {
        controller->AddLoadedGhostRecord(track, controller->mSaveHeaderBuffer, controller->mSaveHeaderLength);
        std::free(controller->mSaveHeaderBuffer);
        controller->mSaveHeaderBuffer = NULL;
    }
    if (controller != NULL)
        controller->mSaveHeaderStatus = 6;
}
unsigned int GhostController::LoadGhostRecordOp(int track)
{
    if (track < 0 || track >= 4)
        return 0;
    if (gStorage == NULL) return 0;
    return (unsigned int)gStorage->ReadFromGenericData(track, BuildDatakey(false), &mSaveHeaderBuffer, &mSaveHeaderLength, LoadGhostRecord_Callback, this);
}
int GhostController::SaveGhostHeader_Callback(void* context,int,C4JStorage::eGeneralData_Callback_Status,int)
{
    GhostController* controller = static_cast<GhostController*>(context);
    void* buffer = controller->mSaveHeaderBuffer;
    if (buffer != NULL) {
        free(buffer);
        controller->mSaveHeaderBuffer = NULL;
    }
    controller->mSaveHeaderStatus = 6;
    return 0;
}
unsigned int GhostController::SaveGhostHeaderOp(int track)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);
    stream.writeUnsignedShort(3);
    const boost::unordered_map<int, TrackHeader>& headers = mTrackHeaders[track];
    stream.writeUnsignedShort((unsigned short)headers.size());
    for (boost::unordered_map<int, TrackHeader>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        stream.writeUnsignedShort((unsigned short)it->first);
        stream.writeInt((int)it->second.time);
        stream.writeUnsignedShort(it->second.score);
        stream.writeBoolean(it->second.completed);
        stream.writeBoolean(it->second.recorded);
    }
    arrayWithLength<unsigned char> bytes = byteStream.toByteArray();
    mSaveHeaderBuffer = bytes.data;
    mSaveHeaderLength = bytes.length;
    const std::wstring key = BuildDatakey(true);
    return (unsigned int)gStorage->WriteToGenericData(track, key, bytes.data, bytes.length,
                                                      SaveGhostHeader_Callback, this);
}
int GhostController::SaveGhostRecord_Callback(void* context,int,C4JStorage::eGeneralData_Callback_Status,int)
{
    GhostController* controller = static_cast<GhostController*>(context);
    if (controller != NULL)
        controller->mSaveHeaderStatus = 6;
    return 0;
}
unsigned int GhostController::SaveGhostRecordOp(int track,GhostRecord* record)
{
    const unsigned int sampleSize = GhostRecordSample::getSampleSize();
    ByteArrayOutputStream byteStream(sampleSize * 1200 + 12);
    DataOutputStream stream(&byteStream);
    stream.writeUnsignedShort(4);
    record->WriteStartPos(&stream);
    const long long elapsed = (static_cast<long long>(record->mElapsedHigh) << 32) | static_cast<unsigned int>(record->mElapsedLow);
    stream.writeLong(elapsed);
    const unsigned int count = static_cast<unsigned int>(record->NumSamples());
    stream.writeInt(count);
    for (unsigned int i = 0; i < count; ++i)
        record->WriteSampleAtIndex(i, &stream);
    arrayWithLength<unsigned char> bytes = byteStream.toByteArray();
    mSaveHeaderBuffer = bytes.data;
    mSaveHeaderLength = bytes.length;
    const std::wstring key = BuildDatakey(false);
    return (unsigned int)gStorage->WriteToGenericData(track, key, bytes.data, bytes.length,
                                                      SaveGhostRecord_Callback, this);
}
void GhostController::ProcessLoadSaveQueue()
{
    EnterCriticalSection(&mQueueLock);
    if (mSaveHeaderStatus == 6 && !mQueue.empty())
    {
        const GhostLoadSaveOperation op = mQueue.front();
        mQueue.pop_front();
        if (op.type == GhostLoadSaveOperation::LOAD_HEADER)
        {
            mSaveHeaderStatus = 0;
            if (LoadGhostHeaderOp(op.track) == 0)
                mSaveHeaderStatus = 6;
        }
        else if (op.type == GhostLoadSaveOperation::LOAD_RECORD)
        {
            mSaveHeaderStatus = 1;
            if (GetTrackHeader(op.track, mGameMode) == 0)
            {
                mRecordingSlotFlags2[op.track] = false;
                mSaveHeaderStatus = 6;
            }
            else if (LoadGhostRecordOp(op.track) == 0)
            {
                mSaveHeaderStatus = 6;
            }
        }
        else if (op.type == GhostLoadSaveOperation::SAVE_HEADER)
        {
            mSaveHeaderStatus = op.type;
            if (SaveGhostHeaderOp(op.track) == 0)
                mSaveHeaderStatus = 6;
        }
        else if (op.type == GhostLoadSaveOperation::SAVE_RECORD)
        {
            mSaveHeaderStatus = op.type;
            if (SaveGhostRecordOp(op.track, op.record) == 0)
                mSaveHeaderStatus = 6;
        }
        else
        {
            mSaveHeaderStatus = 6;
        }
    }
    LeaveCriticalSection(&mQueueLock);
}
void GhostController::tick()
{
    if (mAnyRecordingActive && g_pApp != NULL && g_pApp->IsAppPaused() == 0)
    {
        const uint64_t now = System::processTimeInMilliSecs();
        const unsigned int nowLow = static_cast<unsigned int>(now);
        const int nowHigh = static_cast<int>(now >> 32);
        const uint64_t lastTick = (static_cast<uint64_t>(static_cast<int64_t>(mLastTickHigh) << 32) | mLastTickLow);
        if (lastTick + 250ULL <= now)
        {
            const unsigned int borrow = (nowLow < mLastTickLow) ? 1u : 0u;
            const int deltaHigh = nowHigh - (mLastTickHigh + static_cast<int>(borrow));
            const unsigned int deltaLow = nowLow - mLastTickLow;
            const long long delta = (static_cast<long long>(deltaHigh) << 32) | static_cast<long long>(deltaLow);

            if (mRecordingActive[0])
                UpdateGhostRecord(0, delta);
            if (mRecordingActive[1])
                UpdateGhostRecord(1, delta);
            if (mRecordingActive[2])
                UpdateGhostRecord(2, delta);
            if (mRecordingActive[3])
                UpdateGhostRecord(3, delta);

            mLastTickHigh = nowHigh;
            mLastTickLow = nowLow;
            mPlaybackTimeHigh = 0;
            mPlaybackTimeLow = 0;
        }
    }
    ProcessLoadSaveQueue();
}
bool GhostController::canBeDeleted(){return mDone;}
void GhostController::setDone(){mDone=true;}
unsigned int GhostController::finaliseRecording(int i, bool promoteScore)
{
    if (i < 0 || i >= 4) return 0;
    setRecordingActive(i, false);
    GhostRecord* recording = mRecording[i];
    GhostRecord* loaded = mLoaded[i];
    unsigned int result = 0;
    if (!promoteScore || recording != NULL) {
        if (loaded != NULL) {
            if (recording != NULL &&
                (recording->mElapsedHigh < loaded->mElapsedHigh ||
                 (recording->mElapsedHigh == loaded->mElapsedHigh && recording->mElapsedLow <= loaded->mElapsedLow))) {
                delete loaded;
                mLoaded[i] = NULL;
                AddGhostRecord(i, recording, false);
                mRecording[i] = NULL;
                return 1;
            }
            if (!promoteScore) {
                recording = loaded;
                loaded = NULL;
            }
        }
        if (recording != NULL && promoteScore) {
            result = 1;
            if (mLoaded[i] != NULL) { delete mLoaded[i]; mLoaded[i] = NULL; }
            AddGhostRecord(i, recording, false);
            mRecording[i] = NULL;
            return 1;
        }
    } else if (loaded != NULL) {
        delete loaded;
        mLoaded[i] = NULL;
    }
    if (!mRecordingSlotFlags1[i]) {
        mLoaded[i] = NULL;
        AddGhostRecord(i, recording, false);
    }
    delete mRecording[i];
    mRecording[i] = NULL;
    return result;
}
int GhostController::RecordTime(int track, int key, unsigned int time)
{
    if (track < 0 || track >= 4)
        return 0;
    if (mSaving) { mRecordingSlotFlags1[track] = false; setRecordingActive(track, false); return 0; }
    const bool recorded = finaliseRecording(track, false) != 0;
    boost::unordered_map<int,TrackHeader>::iterator it = mTrackHeaders[track].find(key);
    bool changed = false;
    if (it == mTrackHeaders[track].end()) {
        TrackHeader& header = mTrackHeaders[track][key];
        header.time = time;
        header.score = 0;
        changed = true;
        if (recorded) header.completed = true;
        SaveGhostHeader(track);
    } else {
        TrackHeader& header = it->second;
        if (header.time == 0 || time < header.time) { header.time = time; changed = true; }
        if (recorded && !header.completed) { header.completed = true; SaveGhostHeader(track); }
        else if (changed || !mRecordingSlotFlags1[track]) SaveGhostHeader(track);
    }
    mRecordingSlotFlags1[track] = false;
    return changed ? 1 : 0;
}
int GhostController::RecordScore(int track, int key, unsigned short score, unsigned int time)
{
    if (track < 0 || track >= 4)
        return 0;
    if (mSaving) { mRecordingSlotFlags1[track] = false; setRecordingActive(track, false); return 0; }
    boost::unordered_map<int,TrackHeader>::iterator it = mTrackHeaders[track].find(key);
    bool changed = false;
    if (it == mTrackHeaders[track].end()) {
        if (score != 0) {
            TrackHeader& header = mTrackHeaders[track][key];
            header.score = score;
            header.time = 0;
            header.recorded = finaliseRecording(track, true) != 0;
            changed = true;
        }
    } else {
        TrackHeader& header = it->second;
        if (score != 0 && score > header.score) {
            header.score = score;
            header.recorded = finaliseRecording(track, true) != 0;
            changed = true;
        } else if (score != 0 && score == header.score) {
            GhostRecord* loaded = mLoaded[track];
            if (loaded == NULL || loaded->mElapsedHigh >= 0 || loaded->mElapsedLow > time) {
                header.recorded = header.recorded || (finaliseRecording(track, true) != 0);
                changed = header.recorded;
            } else {
                setRecordingActive(track, false);
            }
        } else {
            setRecordingActive(track, false);
        }
    }
    if (changed) SaveGhostHeader(track);
    mRecordingSlotFlags1[track] = false;
    return changed ? 1 : 0;
}
