#include "game/core/RegionFile.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/Compression.hpp"
#include "game/core/System.hpp"
#include <cstring>
static unsigned int gRegionFileThreadStorageIndex=0;
void* TlsGetValue(unsigned int index); void TlsSetValue(unsigned int index,void* value);
RegionFile::RegionFile(ConsoleSaveFile* saveFile,File* file)
 :mFileEntry(NULL),mFile(file),mSaveFile(saveFile),mOffsets(new unsigned int[1024]),
  mTimestamps(new unsigned int[1024]),mUsedSectors(new std::vector<unsigned int>()),
  mSizeDelta(0),mUnknown28(0),mUnknown32(0),mUnknown36(0),mNewFile(false),mUnknown44(0)
{
 std::memset(mOffsets,0,4096);
 std::memset(mTimestamps,0,4096);
 if(file&&saveFile){ConsoleSavePath path(file->getPath());mFileEntry=saveFile->createFile(path);}
 if(mFileEntry!=NULL && mFileEntry->m_fileSize <= 0xfff){mSizeDelta += 0x2000; mNewFile=true;}
 mUsedSectors->resize(2,1);
}
RegionFile::~RegionFile(){delete[]mOffsets;delete[]mTimestamps;delete mUsedSectors;if(mSaveFile&&mFileEntry)mSaveFile->closeHandle(mFileEntry);}
unsigned int RegionFile::getOffset(int x,int z){return outOfBounds(x,z)?0:mOffsets[x+z*32];}
unsigned int RegionFile::outOfBounds(int x,int z){return(x<0||x>=32||z<0||z>=32)?1:0;}
DataInputStream* RegionFile::getChunkDataInputStream(int x,int z,bool* compressed){
 unsigned int encoded=getOffset(x,z); if(!encoded||!mSaveFile||!mFileEntry)return NULL;
 unsigned int sectors=encoded&255,length=0,read=0;
 mSaveFile->setFilePointer(mFileEntry,(long)((encoded>>8)<<12),NULL,0);
 if(!mSaveFile->readFile(mFileEntry,&length,4,&read)||read!=4)return NULL;
 if(mSaveFile->isSaveEndianDifferent())System::ReverseULONG(&length);
 if(length>(sectors<<12)-4)return NULL;
 unsigned int size=0;
 if(!mSaveFile->readFile(mFileEntry,&size,4,&read)||read!=4)return NULL;
 if(mSaveFile->isSaveEndianDifferent())System::ReverseULONG(&size);
 unsigned char* source=new unsigned char[length];
 if(!mSaveFile->readFile(mFileEntry,source,length,&read)){delete[]source;return NULL;}
 if(compressed)*compressed=(length&0x80000000U)!=0;
 length&=0x3fffffffU; unsigned char* result=new unsigned char[size];
 Compression* c=Compression::getCompression(); c->SetDecompressionType(mSaveFile->getSavePlatform());
 if(compressed&&*compressed)c->DecompressLZXRLE(result,&size,source,length);else c->Decompress(result,&size,source,length);
 delete[]source; arrayWithLength<unsigned char> bytes; bytes.data=result; bytes.length=size;
 return new DataInputStream((InputStream*)new ByteArrayInputStream(bytes));
}
DataOutputStream* RegionFile::getChunkDataOutputStream(int x,int z,bool compressed){return new DataOutputStream(new ChunkBuffer(x,this,z,compressed));}
void RegionFile::GetThreadCompressedDataStorage(){TlsGetValue(gRegionFileThreadStorageIndex);}
void RegionFile::write(int sector,unsigned char* data,int length,unsigned int offset,bool compressed,bool rle,bool deleteData)
{
 if(!mSaveFile||!mFileEntry){if(deleteData)delete[]data;return;}
 unsigned int header=offset|(compressed?0x80000000U:0)|(rle?0x40000000U:0),written=0;
 mSaveFile->setFilePointer(mFileEntry,(long)(sector<<12),NULL,0);
 mSaveFile->writeFile(mFileEntry,&header,4,&written); mSaveFile->writeFile(mFileEntry,&length,4,&written);
 mSaveFile->writeFile(mFileEntry,data,offset,&written); if(deleteData)delete[]data;
}
void RegionFile::zero(int sector,int length)
{if(!mSaveFile||!mFileEntry)return; mSaveFile->setFilePointer(mFileEntry,(long)(sector<<12),NULL,0); unsigned int written=0; mSaveFile->zeroFile(mFileEntry,(unsigned int)length,&written);}
void RegionFile::insertInitialSectors(){zero(0,0x2000);mNewFile=false;}
void RegionFile::setOffset(int x,int z,int offset){if(mNewFile)insertInitialSectors();if(!outOfBounds(x,z))mOffsets[x+z*32]=(unsigned int)offset;}
void RegionFile::setTimestamp(int x,int z,int timestamp){if(mNewFile)insertInitialSectors();if(!outOfBounds(x,z))mTimestamps[x+z*32]=(unsigned int)timestamp;}
void RegionFile::write(int x,int z,unsigned char* data,int length,bool compressed)
{
 unsigned int count=(unsigned int)((length+0x1007)>>12),sector=2;
 while(sector<mUsedSectors->size()&&(*mUsedSectors)[sector])++sector;
 if(sector+count>mUsedSectors->size())mUsedSectors->resize(sector+count,0);
 for(unsigned int i=0;i<count;++i)(*mUsedSectors)[sector+i]=1;
 write((int)sector,data,length,(unsigned int)length,compressed,false,true);
 setOffset(x,z,(int)(sector<<8|count)); setTimestamp(x,z,(int)System::processTimeInSeconds());
}
int RegionFile::fjCountSavedChunks()const{int n=0;for(int i=0;i<1024;++i)if(mOffsets[i])++n;return n;}
void RegionFile::writeAllOffsets()
{
 if(!mSaveFile||!mFileEntry)return;
 unsigned int written=0;
 mSaveFile->setFilePointer(mFileEntry,0,NULL,0);
 mSaveFile->writeFile(mFileEntry,mOffsets,0x1000,&written);
 mSaveFile->writeFile(mFileEntry,mTimestamps,0x1000,&written);
}
void RegionFile::close(){if(mSaveFile&&mFileEntry)mSaveFile->closeHandle(mFileEntry);}
void RegionFile::CreateNewThreadStorage(){TlsSetValue(gRegionFileThreadStorageIndex,new unsigned char[0x10000]);}
long long RegionFile::lastModified(){return mFileEntry!=NULL?mFileEntry->m_timestamp:0;}
int RegionFile::getSizeDelta(){return mSizeDelta;}
bool RegionFile::hasChunk(int x,int z){return getOffset(x,z)!=0;}
void RegionFile::ReleaseThreadStorage(){unsigned char* storage=(unsigned char*)TlsGetValue(gRegionFileThreadStorageIndex);if(storage!=NULL){delete[]storage;TlsSetValue(gRegionFileThreadStorageIndex,NULL);}}
RegionFile::ChunkBuffer::ChunkBuffer(int x,RegionFile* region,int z,bool compressed):ByteArrayOutputStream(0x10000),mX(x),mRegion(region),mZ(z),mCompressed(compressed){}
RegionFile::ChunkBuffer::~ChunkBuffer(){}
void RegionFile::ChunkBuffer::close(){mRegion->write(mX,mZ,mBuffer.data,(int)mCount,mCompressed);mBuffer.data=NULL;mBuffer.length=0;}
