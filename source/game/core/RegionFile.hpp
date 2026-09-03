#ifndef GAME_CORE_REGIONFILE_HPP
#define GAME_CORE_REGIONFILE_HPP
#include <vector>
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/File.hpp"
#include "game/data/ConsoleSaveFile.hpp"
class RegionFile {
public:
 class ChunkBuffer : public ByteArrayOutputStream {
 public:
  ChunkBuffer(int x, RegionFile* region, int z, bool compressed);
  virtual ~ChunkBuffer();
  virtual void close();
 private:
  int mX; RegionFile* mRegion; int mZ; bool mCompressed;
 };
 RegionFile(ConsoleSaveFile* saveFile, File* file); ~RegionFile();
 unsigned int getOffset(int x,int z); unsigned int outOfBounds(int x,int z);
 DataInputStream* getChunkDataInputStream(int x,int z,bool* compressed);
 DataOutputStream* getChunkDataOutputStream(int x,int z,bool compressed);
 void GetThreadCompressedDataStorage();
 void write(int sector,unsigned char* data,int length,unsigned int offset,bool compressed,bool rle,bool deleteData);
 void zero(int sector,int length); void insertInitialSectors();
 void setOffset(int x,int z,int offset); void setTimestamp(int x,int z,int timestamp);
 void write(int x,int z,unsigned char* data,int length,bool compressed);
 int fjCountSavedChunks() const; void writeAllOffsets(); void close();
 static void CreateNewThreadStorage(); long long lastModified(); int getSizeDelta();
 bool hasChunk(int x,int z); static void ReleaseThreadStorage();
private:
 FileEntry* mFileEntry; File* mFile; ConsoleSaveFile* mSaveFile;
 unsigned int* mOffsets; unsigned int* mTimestamps; std::vector<unsigned int>* mUsedSectors;
 int mSizeDelta; int mUnknown28; int mUnknown32; int mUnknown36; bool mNewFile; int mUnknown44;
};
#endif
