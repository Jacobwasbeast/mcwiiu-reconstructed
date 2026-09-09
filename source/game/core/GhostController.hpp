#ifndef GAME_CORE_GHOSTCONTROLLER_HPP
#define GAME_CORE_GHOSTCONTROLLER_HPP
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <stdint.h>
#include <boost/shared_ptr.hpp>
#include "game/util/Vec3.hpp"
#include "platform/wiiu/C4JStorage.hpp"
class DataInputStream; class DataOutputStream; class Player; class GhostPlayer;
class GhostController {
 friend class ClientMasterGameMode;
public:
 struct GhostRecordSample {
  short x;
  short y;
  short z;
  short rotX;
  short rotY;
  short rotZ;
  long long timestamp;
  GhostRecordSample();
  GhostRecordSample(const GhostRecordSample&);
  GhostRecordSample(short, short, short, short, short, short, long long);
  GhostRecordSample(DataInputStream*);
  void _init(short, short, short, short, short, short, long long);
  void read(DataInputStream*);
  static unsigned int getSampleSize();
  void write(DataOutputStream*);
 };
 struct GhostRecord {
  std::vector<GhostRecordSample> samples;
  bool hasRecording;
  Vec3 startPos;
  int time;
  int score;
  GhostRecord(Vec3*);
  ~GhostRecord();
  unsigned int GetPosAtIndex(unsigned int);
  bool HasRecording() const;
  void ResetHasRecording();
  void AddSample(double, double, double, float, float, float);
  int NumSamples() const;
  long long GetTimestampAtIndex(unsigned int);
  void AddSample(const GhostRecordSample&);
  void WriteStartPos(DataOutputStream*);
  void WriteSampleAtIndex(unsigned int, DataOutputStream*);
  unsigned int GetRotAtIndex(unsigned int);
  double GetVelocityAtIndex(unsigned int);
 };
  struct TrackHeader { unsigned int time; unsigned int rank; unsigned short score; bool completed; bool recorded; TrackHeader(); };
  struct GhostLoadSaveOperation { enum EOpType { LOAD_HEADER, LOAD_RECORD, SAVE_HEADER, SAVE_RECORD }; EOpType type; int track; GhostRecord* record; GhostLoadSaveOperation(EOpType, int, GhostRecord*); };
  GhostController(int, const std::wstring&, bool);
  ~GhostController();
  static int32_t Double1DPAsInt16(double);
  void addLoadSaveOp(GhostLoadSaveOperation::EOpType, int, GhostRecord*);
  void LoadGhostRecord(int);
  void Initialise(int, const std::wstring&, bool);
  void LoadGhostHeader(int);
  void reset(int, const std::wstring&, bool);
  std::wstring getGhostPlayerName(int);
  void createGhostPlayers();
  void startPlayback();
  boost::shared_ptr<GhostPlayer> getGhostPlayer(int);
  std::vector<boost::shared_ptr<GhostPlayer> > getGhostPlayers();
  std::vector<boost::shared_ptr<Player> > getPlayers();
  void stopPlayback();
  void stopRecording(int);
  void UpdateGhostRecord(int, long long);
  void setRecordingActive(int, bool);
  void startRecording(int, Vec3*);
  unsigned short GetBestScore(int) const;
  bool HasFinished(GhostRecord*);
  unsigned int getPlaybackIndex(GhostRecord*, float&);
  std::wstring BuildDatakey(bool);
  void SaveGhostHeader(int);
  void SetTrackHeader(int, int, unsigned int, unsigned short, bool, bool, bool);
  void LoadGhostHeader(int, void*, unsigned int);
  static void LoadGhostHeader_Callback(void*, int, C4JStorage::eGeneralData_Callback_Status, int);
  unsigned int LoadGhostHeaderOp(int);
  unsigned int GetTrackHeader(int, int) const;
  void SaveGhostRecord(int, GhostRecord*);
  void AddGhostRecord(int, GhostRecord*, bool);
  void AddLoadedGhostRecord(int, void*, unsigned int);
  static void LoadGhostRecord_Callback(void*, int, C4JStorage::eGeneralData_Callback_Status, int);
  unsigned int LoadGhostRecordOp(int);
  static void SaveGhostHeader_Callback(void*, int, C4JStorage::eGeneralData_Callback_Status, int);
  unsigned int SaveGhostHeaderOp(int);
  static void SaveGhostRecord_Callback(void*, int, C4JStorage::eGeneralData_Callback_Status, int);
  unsigned int SaveGhostRecordOp(int, GhostRecord*);
  void ProcessLoadSaveQueue();
  void tick();
  bool canBeDeleted();
  void setDone();
  unsigned int finaliseRecording(int, bool);
  int RecordTime(int, int, unsigned int);
  int RecordScore(int, int, unsigned short, unsigned int);
  static void DisableSaving(void*, int, C4JStorage::EMessageResult);
 private:
  int mGameMode;
  std::wstring mLevelName;
  bool mSaving;
  bool mDone;
  bool mPlayback;
  GhostRecord* mRecording[4];
  GhostRecord* mLoaded[4];
 bool mRecordingActive[4];
  void* mSaveHeaderBuffer;
  int mSaveHeaderStatus;
  std::map<int, TrackHeader> mTrackHeaders[4];
  std::deque<GhostLoadSaveOperation> mQueue;
  std::vector<boost::shared_ptr<GhostPlayer> > mGhostPlayers;
};
#endif
