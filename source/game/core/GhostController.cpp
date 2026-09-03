#include "game/core/GhostController.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include <cmath>
#include <sstream>

GhostController::GhostRecordSample::GhostRecordSample() : x(0), y(0), z(0), rotX(0), rotY(0), rotZ(0), timestamp(0) {}
GhostController::GhostRecordSample::GhostRecordSample(const GhostRecordSample& sample) { _init(sample.x, sample.y, sample.z, sample.rotX, sample.rotY, sample.rotZ, sample.timestamp); }
GhostController::GhostRecordSample::GhostRecordSample(short a, short b, short c, short d, short e, short f, long long t) { _init(a,b,c,d,e,f,t); }
GhostController::GhostRecordSample::GhostRecordSample(DataInputStream* stream) { read(stream); }
void GhostController::GhostRecordSample::_init(short a, short b, short c, short d, short e, short f, long long t) { x=a; y=b; z=c; rotX=d; rotY=e; rotZ=f; timestamp=t; }
void GhostController::GhostRecordSample::read(DataInputStream* stream) {
    x = stream->readShort();
    y = stream->readShort();
    z = stream->readShort();
    rotX = stream->readShort();
    rotY = stream->readShort();
    rotZ = stream->readShort();
    timestamp = stream->readLong();
}
unsigned int GhostController::GhostRecordSample::getSampleSize() { return 20; }
void GhostController::GhostRecordSample::write(DataOutputStream* stream) {
    stream->writeShort(x);
    stream->writeShort(y);
    stream->writeShort(z);
    stream->writeShort(rotX);
    stream->writeShort(rotY);
    stream->writeShort(rotZ);
    stream->writeLong(timestamp);
}

GhostController::GhostRecord::GhostRecord(Vec3* pos) : hasRecording(false), startPos(), time(0), score(0) { if (pos) startPos = *pos; samples.reserve(1200); }
GhostController::GhostRecord::~GhostRecord() {}
unsigned int GhostController::GhostRecord::GetPosAtIndex(unsigned int index) { return index < samples.size() ? (unsigned int)samples[index].x : 0; }
bool GhostController::GhostRecord::HasRecording() const { return hasRecording; }
void GhostController::GhostRecord::ResetHasRecording() { hasRecording=false; }
void GhostController::GhostRecord::AddSample(double a, double b, double c, float d, float e, float f) {
    long long timestamp = samples.empty() ? 0 : samples.back().timestamp + 1;
    GhostRecordSample sample((short)GhostController::Double1DPAsInt16(a), (short)GhostController::Double1DPAsInt16(b), (short)GhostController::Double1DPAsInt16(c), (short)d, (short)e, (short)f, timestamp);
    samples.push_back(sample);
    hasRecording = true;
}
int GhostController::GhostRecord::NumSamples() const { return (int)samples.size(); }
long long GhostController::GhostRecord::GetTimestampAtIndex(unsigned int index) { return index < samples.size() ? samples[index].timestamp : 0; }
void GhostController::GhostRecord::AddSample(const GhostRecordSample& sample) { samples.push_back(sample); hasRecording=true; }
void GhostController::GhostRecord::WriteStartPos(DataOutputStream* stream) { stream->writeDouble(startPos.x); stream->writeDouble(startPos.y); stream->writeDouble(startPos.z); }
void GhostController::GhostRecord::WriteSampleAtIndex(unsigned int index, DataOutputStream* stream) { if (index < samples.size()) samples[index].write(stream); }
unsigned int GhostController::GhostRecord::GetRotAtIndex(unsigned int index) { return index < samples.size() ? (unsigned int)samples[index].rotY : 0; }
double GhostController::GhostRecord::GetVelocityAtIndex(unsigned int index) { if (index==0 || index>=samples.size()) return 0.0; return (double)(samples[index].timestamp-samples[index-1].timestamp); }

GhostController::TrackHeader::TrackHeader() : time(0), rank(0), score(0), completed(false), recorded(false) {}
GhostController::GhostLoadSaveOperation::GhostLoadSaveOperation(EOpType t, int i, GhostRecord* r) : type(t), track(i), record(r) {}

int32_t GhostController::Double1DPAsInt16(double value) { return (int32_t)(short)std::floor(value*10.0+0.5); }
GhostController::GhostController(int mode, const std::wstring& name, bool saving) : mGameMode(mode), mLevelName(name), mSaving(saving), mDone(false), mPlayback(false) {
    int i;
    for (i = 0; i < 4; ++i) {
        mRecording[i] = NULL;
        mLoaded[i] = NULL;
        mRecordingActive[i] = false;
    }
}
GhostController::~GhostController() { int i; for(i=0;i<4;i++){delete mRecording[i];delete mLoaded[i];} }
void GhostController::addLoadSaveOp(GhostLoadSaveOperation::EOpType t,int i,GhostRecord* r){mQueue.push_back(GhostLoadSaveOperation(t,i,r));}
void GhostController::LoadGhostRecord(int i){addLoadSaveOp(GhostLoadSaveOperation::LOAD_RECORD,i,0);}
void GhostController::Initialise(int mode,const std::wstring& name,bool saving){mGameMode=mode;mLevelName=name;mSaving=saving;}
void GhostController::LoadGhostHeader(int i){addLoadSaveOp(GhostLoadSaveOperation::LOAD_HEADER,i,0);}
void GhostController::reset(int mode,const std::wstring& name,bool saving){Initialise(mode,name,saving);}
std::wstring GhostController::getGhostPlayerName(int i){std::wstringstream stream;stream<<L"GHOST_CONTROLLER_PLAYER_"<<i;return stream.str();}
void GhostController::createGhostPlayers(){}
void GhostController::startPlayback(){mPlayback=true;createGhostPlayers();}
boost::shared_ptr<GhostPlayer> GhostController::getGhostPlayer(int i){return i>=0 && i<(int)mGhostPlayers.size()?mGhostPlayers[i]:boost::shared_ptr<GhostPlayer>();}
std::vector<boost::shared_ptr<GhostPlayer> > GhostController::getGhostPlayers(){return mGhostPlayers;}
std::vector<boost::shared_ptr<Player> > GhostController::getPlayers(){return std::vector<boost::shared_ptr<Player> >();}
void GhostController::stopPlayback(){mPlayback=false;mGhostPlayers.clear();}
void GhostController::stopRecording(int i){if(i>=0&&i<4){mRecordingActive[i]=false;if(mRecording[i])mRecording[i]->ResetHasRecording();}}
void GhostController::UpdateGhostRecord(int i,long long t){if(i>=0&&i<4&&mRecording[i])mRecording[i]->AddSample(0,0,0,0,0,0);}
void GhostController::setRecordingActive(int i,bool active){if(i>=0&&i<4)mRecordingActive[i]=active;}
void GhostController::startRecording(int i,Vec3* pos){if(i>=0&&i<4){delete mRecording[i];mRecording[i]=new GhostRecord(pos);mRecordingActive[i]=true;}}
unsigned short GhostController::GetBestScore(int i) const {std::map<int,TrackHeader>::const_iterator it=mTrackHeaders[i].find(i);return it==mTrackHeaders[i].end()?0:it->second.score;}
bool GhostController::HasFinished(GhostRecord* r){return r && r->NumSamples()>0;}
unsigned int GhostController::getPlaybackIndex(GhostRecord* r,float& part){part=0;return r&&r->NumSamples()?0:0;}
std::wstring GhostController::BuildDatakey(bool){return mLevelName;}
void GhostController::SaveGhostHeader(int i){addLoadSaveOp(GhostLoadSaveOperation::SAVE_HEADER,i,0);}
void GhostController::SetTrackHeader(int t,int key,unsigned int time,unsigned short score,bool a,bool b,bool c){if(t>=0&&t<4){TrackHeader& h=mTrackHeaders[t][key];h.time=time;h.score=score;h.completed=a;h.recorded=b;}}
void GhostController::LoadGhostHeader(int,void*,unsigned int){}
void GhostController::LoadGhostHeader_Callback(void*,int,C4JStorage::eGeneralData_Callback_Status,int){}
unsigned int GhostController::LoadGhostHeaderOp(int){return 0;}
unsigned int GhostController::GetTrackHeader(int t,int key) const {return t>=0&&t<4&&mTrackHeaders[t].find(key)!=mTrackHeaders[t].end()?mTrackHeaders[t].find(key)->second.time:0;}
void GhostController::SaveGhostRecord(int i,GhostRecord* r){addLoadSaveOp(GhostLoadSaveOperation::SAVE_RECORD,i,r);}
void GhostController::AddGhostRecord(int i,GhostRecord* r,bool){if(i>=0&&i<4){delete mLoaded[i];mLoaded[i]=r;}}
void GhostController::AddLoadedGhostRecord(int,void*,unsigned int){}
void GhostController::LoadGhostRecord_Callback(void*,int,C4JStorage::eGeneralData_Callback_Status,int){}
unsigned int GhostController::LoadGhostRecordOp(int){return 0;}
void GhostController::SaveGhostHeader_Callback(void*,int,C4JStorage::eGeneralData_Callback_Status,int){}
unsigned int GhostController::SaveGhostHeaderOp(int){return 0;}
void GhostController::SaveGhostRecord_Callback(void*,int,C4JStorage::eGeneralData_Callback_Status,int){}
unsigned int GhostController::SaveGhostRecordOp(int,GhostRecord*){return 0;}
void GhostController::ProcessLoadSaveQueue(){while(!mQueue.empty())mQueue.pop_front();}
void GhostController::tick(){ProcessLoadSaveQueue();}
bool GhostController::canBeDeleted(){return mDone;}
void GhostController::setDone(){mDone=true;}
unsigned int GhostController::finaliseRecording(int i,bool){if(i>=0&&i<4){setRecordingActive(i,false);return mRecording[i]?1:0;}return 0;}
int GhostController::RecordTime(int,int,unsigned int){return 0;}
int GhostController::RecordScore(int,int,unsigned short,unsigned int){return 0;}
void GhostController::DisableSaving(void*,int,C4JStorage::EMessageResult){}
