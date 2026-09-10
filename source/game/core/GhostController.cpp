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

GhostController::GhostRecordSample::GhostRecordSample() : x(0), y(0), z(0), rotX(0), rotY(0), rotZ(0), timestamp(0) {}
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

GhostController::GhostRecord::GhostRecord(Vec3* pos) : hasRecording(false), startPos(), mElapsedHigh(0), mElapsedLow(0), time(0), score(0) { if (pos) startPos = *pos; samples.reserve(1200); }
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
    long long timestamp = samples.empty() ? 0 : samples.back().timestamp + 1;
    GhostRecordSample sample((short)GhostController::Double1DPAsInt16(a), (short)GhostController::Double1DPAsInt16(b), (short)GhostController::Double1DPAsInt16(c), (short)d, (short)e, (short)f, timestamp);
    samples.push_back(sample);
    hasRecording = true;
}
int GhostController::GhostRecord::NumSamples() const { return (int)samples.size(); }
long long GhostController::GhostRecord::GetTimestampAtIndex(unsigned int index)
{
    return index < samples.size() ? samples[index].timestamp : -1;
}
void GhostController::GhostRecord::AddSample(const GhostRecordSample& sample) { samples.push_back(sample); hasRecording=true; }
void GhostController::GhostRecord::WriteStartPos(DataOutputStream* stream) { stream->writeDouble(startPos.x); stream->writeDouble(startPos.y); stream->writeDouble(startPos.z); }
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
    const double dx = (double)(current.x - previous.x) / 10.0;
    const double dy = (double)(current.y - previous.y) / 10.0;
    const double dz = (double)(current.z - previous.z) / 10.0;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

GhostController::TrackHeader::TrackHeader() : time(0), rank(0), score(0), completed(false), recorded(false) {}
GhostController::GhostLoadSaveOperation::GhostLoadSaveOperation(EOpType t, int i, GhostRecord* r) : type(t), track(i), record(r) {}

int32_t GhostController::Double1DPAsInt16(double value) { return (int32_t)(short)std::floor(value*10.0+0.5); }
GhostController::GhostController(int mode, const std::wstring& name, bool saving) : mGameMode(mode), mLevelName(name), mSaving(saving), mDone(false), mPlayback(false), mLastTickHigh(0), mLastTickLow(0), mPlaybackTimeHigh(0), mPlaybackTimeLow(0), mSaveHeaderBuffer(NULL), mSaveHeaderStatus(0) {
    int i;
    for (i = 0; i < 4; ++i) {
        mRecording[i] = NULL;
        mLoaded[i] = NULL;
        mRecordingActive[i] = false;
        mRecordingSlotFlags1[i] = false;
        mRecordingSlotFlags2[i] = false;
    }
}
GhostController::~GhostController() { int i; for(i=0;i<4;i++){delete mRecording[i];delete mLoaded[i];} }
void GhostController::addLoadSaveOp(GhostLoadSaveOperation::EOpType t,int i,GhostRecord* r){mQueue.push_back(GhostLoadSaveOperation(t,i,r));}
void GhostController::LoadGhostRecord(int i){addLoadSaveOp(GhostLoadSaveOperation::LOAD_RECORD,i,0);}
void GhostController::Initialise(int mode,const std::wstring& name,bool saving){mGameMode=mode;mLevelName=name;mSaving=saving;}
void GhostController::LoadGhostHeader(int i){addLoadSaveOp(GhostLoadSaveOperation::LOAD_HEADER,i,0);}
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
    mGhostPlayers.clear();
    Minecraft* minecraft = Minecraft::GetInstance();
    Level* level = minecraft != NULL ? static_cast<Level*>(minecraft->getLevel(0)) : NULL;
    if (level == NULL)
        return;
    for (int i = 0; i < 4; ++i)
        mGhostPlayers.push_back(boost::shared_ptr<GhostPlayer>(new GhostPlayer(level, getGhostPlayerName(i))));
}
void GhostController::startPlayback(){mPlayback=true;createGhostPlayers();}
boost::shared_ptr<GhostPlayer> GhostController::getGhostPlayer(int i){return i>=0 && i<(int)mGhostPlayers.size()?mGhostPlayers[i]:boost::shared_ptr<GhostPlayer>();}
std::vector<boost::shared_ptr<GhostPlayer> > GhostController::getGhostPlayers(){return mGhostPlayers;}
std::vector<boost::shared_ptr<Player> > GhostController::getPlayers(){return std::vector<boost::shared_ptr<Player> >();}
void GhostController::stopPlayback(){mPlayback=false;mGhostPlayers.clear();}
void GhostController::stopRecording(int i){if(i>=0&&i<4){mRecordingActive[i]=false;if(mRecording[i])mRecording[i]->ResetHasRecording();}}
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

    const unsigned int deltaLow = (unsigned int)t - mPlaybackTimeLow;
    const unsigned int borrowLow = ((unsigned int)t < mPlaybackTimeLow) ? 1u : 0u;
    const int deltaHigh = (int)(t >> 32) - (mPlaybackTimeHigh + (int)borrowLow);
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
void GhostController::setRecordingActive(int i,bool active){if(i>=0&&i<4)mRecordingActive[i]=active;}
void GhostController::startRecording(int i,Vec3* pos){if(i>=0&&i<4){delete mRecording[i];mRecording[i]=new GhostRecord(pos);mRecordingActive[i]=true;}}
unsigned short GhostController::GetBestScore(int i) const {std::map<int,TrackHeader>::const_iterator it=mTrackHeaders[i].find(i);return it==mTrackHeaders[i].end()?0:it->second.score;}
bool GhostController::HasFinished(GhostRecord* r){return r && r->NumSamples()>0;}
unsigned int GhostController::getPlaybackIndex(GhostRecord* r,float& part){part=0;return r&&r->NumSamples()?0:0;}
std::wstring GhostController::BuildDatakey(bool){return mLevelName;}
void GhostController::SaveGhostHeader(int i){addLoadSaveOp(GhostLoadSaveOperation::SAVE_HEADER,i,0);}
void GhostController::SetTrackHeader(int t,int key,unsigned int time,unsigned short score,bool a,bool b,bool c){if(t>=0&&t<4){TrackHeader& h=mTrackHeaders[t][key];h.time=time;h.score=score;h.completed=a;h.recorded=b;}}
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
}
void GhostController::LoadGhostHeader_Callback(void* context,int track,C4JStorage::eGeneralData_Callback_Status status,int)
{
    GhostController* controller = static_cast<GhostController*>(context);
    if (controller == NULL)
        return;
    if (controller->mSaveHeaderStatus == 0 && status == C4JStorage::eGeneralData_Callback_Status(0) && controller->mSaveHeaderBuffer != NULL)
        controller->LoadGhostHeader(track, controller->mSaveHeaderBuffer, static_cast<unsigned int>(controller->mSaveHeaderStatus));
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
    mSaveHeaderStatus = 1;
    return 1;
}
unsigned int GhostController::GetTrackHeader(int t,int key) const {return t>=0&&t<4&&mTrackHeaders[t].find(key)!=mTrackHeaders[t].end()?mTrackHeaders[t].find(key)->second.time:0;}
void GhostController::SaveGhostRecord(int i,GhostRecord* r){addLoadSaveOp(GhostLoadSaveOperation::SAVE_RECORD,i,r);}
void GhostController::AddGhostRecord(int i,GhostRecord* r,bool){if(i>=0&&i<4){delete mLoaded[i];mLoaded[i]=r;}}
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
    record->time = (int)stream.readLong();
    const int count = stream.readInt();
    for (int i = 0; i < count; ++i)
        record->AddSample(GhostRecordSample(&stream));
    if (record->HasRecording())
    {
        AddGhostRecord(track, record, true);
        mRecordingActive[track] = true;
    }
    else
        delete record;
}
void GhostController::LoadGhostRecord_Callback(void* context,int,C4JStorage::eGeneralData_Callback_Status,int)
{
    GhostController* controller = static_cast<GhostController*>(context);
    if (controller != NULL)
        controller->mSaveHeaderStatus = 6;
}
unsigned int GhostController::LoadGhostRecordOp(int track)
{
    if (track < 0 || track >= 4)
        return 0;
    mSaveHeaderStatus = 1;
    return 1;
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
    const std::map<int, TrackHeader>& headers = mTrackHeaders[track];
    stream.writeUnsignedShort((unsigned short)headers.size());
    for (std::map<int, TrackHeader>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        stream.writeUnsignedShort((unsigned short)it->first);
        stream.writeInt((int)it->second.time);
        stream.writeUnsignedShort(it->second.score);
        stream.writeBoolean(it->second.completed);
        stream.writeBoolean(it->second.recorded);
    }
    arrayWithLength<unsigned char> bytes = byteStream.toByteArray();
    mSaveHeaderBuffer = bytes.data;
    const std::wstring key = BuildDatakey(true);
    return (unsigned int)gStorage->WriteToGenericData(track, key, bytes.data, bytes.length,
                                                      SaveGhostHeader_Callback, this);
}
void GhostController::SaveGhostRecord_Callback(void* context,int,C4JStorage::eGeneralData_Callback_Status,int)
{
    GhostController* controller = static_cast<GhostController*>(context);
    if (controller != NULL)
        controller->mSaveHeaderStatus = 6;
}
unsigned int GhostController::SaveGhostRecordOp(int track,GhostRecord* record)
{
    if (track < 0 || track >= 4 || record == NULL || !record->HasRecording())
        return 0;
    mSaveHeaderStatus = 1;
    return 1;
}
void GhostController::ProcessLoadSaveQueue(){while(!mQueue.empty())mQueue.pop_front();}
void GhostController::tick()
{
    if (mPlayback && g_pApp != NULL && g_pApp->IsAppPaused() == 0)
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
    if (i < 0 || i >= 4)
        return 0;

    setRecordingActive(i, false);

    GhostRecord* recording = mRecording[i];
    GhostRecord* loaded = mLoaded[i];
    unsigned int result = 0;
    GhostRecord* recordToAdd = NULL;
    bool skipPromote = false;

    if (!promoteScore || recording != NULL) {
        loaded = mLoaded[i];
        if (loaded != NULL) {
            if (recording != NULL) {
                const bool recordingIsBetter =
                    recording->mElapsedHigh <= loaded->mElapsedHigh &&
                    (recording->mElapsedHigh < loaded->mElapsedHigh ||
                     recording->mElapsedLow < loaded->mElapsedLow);
                if (recordingIsBetter) {
                    delete loaded;
                    mLoaded[i] = NULL;
                    recording = mRecording[i];
                    AddGhostRecord(i, recording, false);
                    mRecording[i] = NULL;
                    return 1;
                }
            }
            if (!promoteScore) {
                recordToAdd = loaded;
                skipPromote = true;
            }
        }
        if (!skipPromote) {
            result = 1;
            if (recording != NULL) {
                loaded = mLoaded[i];
                if (loaded != NULL) {
                    delete loaded;
                    mLoaded[i] = NULL;
                    recording = mRecording[i];
                }
                AddGhostRecord(i, recording, false);
                mRecording[i] = NULL;
                return 1;
            }
        }
    } else {
        loaded = mLoaded[i];
        recordToAdd = NULL;
        if (loaded != NULL) {
            delete loaded;
            mLoaded[i] = NULL;
        }
    }

    if (!mRecordingSlotFlags1[i]) {
        mLoaded[i] = NULL;
        AddGhostRecord(i, recordToAdd, false);
    }
    if (mRecording[i] != NULL) {
        delete mRecording[i];
        mRecording[i] = NULL;
    }
    return result;
}
int GhostController::RecordTime(int track, int key, unsigned int time)
{
    if (track < 0 || track >= 4)
        return 0;
    if (mSaving) {
        setRecordingActive(track, false);
        return 0;
    }
    const bool wasRecorded = finaliseRecording(track, false) != 0;
    TrackHeader& header = mTrackHeaders[track][key];
    bool changed = false;
    if (header.time == 0 || time < header.time) {
        header.time = time;
        changed = true;
    }
    if (wasRecorded && !header.completed) {
        header.completed = true;
        changed = true;
    }
    if (changed || header.recorded)
        SaveGhostHeader(track);
    setRecordingActive(track, false);
    return changed ? 1 : 0;
}
int GhostController::RecordScore(int track, int key, unsigned short score, unsigned int time)
{
    if (track < 0 || track >= 4)
        return 0;
    if (mSaving) {
        setRecordingActive(track, false);
        return 0;
    }
    TrackHeader& header = mTrackHeaders[track][key];
    bool changed = false;
    if (score != 0 && score > header.score) {
        header.score = score;
        changed = true;
    }
    if (score != 0 && score == header.score && (header.time == 0 || time <= header.time)) {
        const bool finished = finaliseRecording(track, true) != 0;
        header.recorded = header.recorded || finished;
        changed = changed || finished;
    } else {
        setRecordingActive(track, false);
    }
    if (changed)
        SaveGhostHeader(track);
    return changed ? 1 : 0;
}
void GhostController::DisableSaving(void*,int,C4JStorage::EMessageResult){}
