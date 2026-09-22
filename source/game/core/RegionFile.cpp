#include "game/core/RegionFile.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/Compression.hpp"
#include "game/core/System.hpp"
#include "game/data/ConsoleSavePath.hpp"
#include <algorithm>
#include <cstring>
static unsigned int gRegionFileThreadStorageIndex=0;
void* TlsGetValue(unsigned int index); int TlsSetValue(unsigned int index,void* value);
static unsigned int encodeSaveWord(unsigned int value)
{
    if (FileHeader::getNativeEndian() != eByteOrder_BigEndian)
        System::ReverseULONG(&value);
    return value;
}
RegionFile::RegionFile(ConsoleSaveFile* saveFile,File* file)
 :mFileEntry(NULL),mFile(file),mSaveFile(saveFile),mOffsets(new unsigned int[1024]),
  mTimestamps(new unsigned int[1024]),mUsedSectors(new std::vector<bool>()),
  mSizeDelta(0),mUnknown32(0),mUnknown36(0),mNewFile(false)
{
 std::memset(mOffsets,0,4096);
 std::memset(mTimestamps,0,4096);
 ConsoleSavePath path(file->getPath());
 mFileEntry=saveFile->createFile(path);
 mSaveFile->setFilePointer(mFileEntry,0,NULL,2);
 unsigned int fileSize=mFileEntry->m_fileSize;
 if(fileSize>0xfff){
  mNewFile=false;
 }else{
  mSizeDelta+=0x2000;
  mNewFile=true;
 }
 if((fileSize&0xfff)!=0){
  unsigned int padSize=0x1000-(fileSize&0xfff);
  unsigned char* pad=new unsigned char[padSize];
  std::memset(pad,0,padSize);
  unsigned int written=0;
  mSaveFile->writeFile(mFileEntry,pad,padSize,&written);
  delete[]pad;
 }
 int sectorCount;
 if(mNewFile){
  sectorCount=2;
 }else{
  sectorCount=(int)mFileEntry->m_fileSize/0x1000;
 }
 mUsedSectors->reserve(sectorCount);
 for(int i=0;i<sectorCount;++i)mUsedSectors->push_back(true);
 mUsedSectors->at(0)=false;
 mUsedSectors->at(1)=false;
 if(!mNewFile)mSaveFile->setFilePointer(mFileEntry,0,NULL,0);
 for(int i=0;i<1024;++i){
  unsigned int offsetValue=0;
  unsigned int read=0;
  if(!mNewFile){
   mSaveFile->readFile(mFileEntry,&offsetValue,4,&read);
   if(mSaveFile->isSaveEndianDifferent())System::ReverseULONG(&offsetValue);
  }
  mOffsets[i]=offsetValue;
  if(offsetValue!=0){
   unsigned int sector=offsetValue>>8,count=offsetValue&0xff;
   if(sector+count<=mUsedSectors->size()&&count!=0){
    for(unsigned int j=0;j<count;++j)mUsedSectors->at(sector+j)=false;
   }
  }
 }
 for(int i=0;i<1024;++i){
  int timestampValue=0;
  unsigned int read=0;
  if(!mNewFile){
   mSaveFile->readFile(mFileEntry,&timestampValue,4,&read);
   if(mSaveFile->isSaveEndianDifferent())System::ReverseINT(&timestampValue);
  }
  mTimestamps[i]=(unsigned int)timestampValue;
 }
}
RegionFile::~RegionFile(){delete[]mOffsets;delete[]mTimestamps;delete mUsedSectors;if(mSaveFile&&mFileEntry)mSaveFile->closeHandle(mFileEntry);}
unsigned int RegionFile::getOffset(int x,int z){return outOfBounds(x,z)?0:mOffsets[x+z*32];}
unsigned int RegionFile::outOfBounds(int x,int z){return(x<0||x>=32||z<0||z>=32)?1:0;}
DataInputStream* RegionFile::getChunkDataInputStream(int x,int z,bool* compressed){
    const unsigned int encoded = getOffset(x, z);
    if (!encoded || !mSaveFile || !mFileEntry)
        return NULL;

    const unsigned int sector = encoded >> 8;
    const unsigned int sectors = encoded & 255;
    if (sectors == 0 || sector + sectors > mUsedSectors->size())
        return NULL;

    unsigned int rawLength = 0;
    unsigned int read = 0;
    mSaveFile->setFilePointer(mFileEntry, static_cast<long>(sector << 12), NULL, 0);
    if (!mSaveFile->readFile(mFileEntry, &rawLength, 4, &read) || read != 4)
        return NULL;
    if (mSaveFile->isSaveEndianDifferent())
        System::ReverseULONG(&rawLength);

    const bool useRle = (rawLength & 0x80000000U) != 0;
    if (compressed != NULL)
        *compressed = (rawLength & 0x40000000U) != 0;
    const unsigned int length = rawLength & 0x3fffffffU;
    if ((sectors << 12) < length)
        return NULL;

    unsigned int size = 0;
    if (!mSaveFile->readFile(mFileEntry, &size, 4, &read) || read != 4)
        return NULL;
    if (mSaveFile->isSaveEndianDifferent())
        System::ReverseULONG(&size);

    unsigned char* source = new unsigned char[length];
    if (!mSaveFile->readFile(mFileEntry, source, length, &read))
    {
        delete[] source;
        return NULL;
    }

    unsigned char* result = new unsigned char[size];
    Compression* compression = Compression::getCompression();
    compression->SetDecompressionType(mSaveFile->getSavePlatform());
    if (useRle)
        compression->DecompressLZXRLE(result, &size, source, length);
    else
        compression->Decompress(result, &size, source, length);
    compression->SetDecompressionType(eSavePlatform_WIIU);
    delete[] source;

    arrayWithLength<unsigned char> bytes;
    bytes.data = result;
    bytes.length = size;
    return new DataInputStream((InputStream*)new ByteArrayInputStream(bytes));
}
DataOutputStream* RegionFile::getChunkDataOutputStream(int x,int z,bool compressed){return new DataOutputStream(new ChunkBuffer(x,this,z,compressed));}
void* RegionFile::GetThreadCompressedDataStorage(){return TlsGetValue(gRegionFileThreadStorageIndex);}
void RegionFile::write(int sector,unsigned char* data,int length,unsigned int offset,bool compressed,bool rle,bool deleteData)
{
    unsigned int header = offset;
    unsigned int written = 0;
    mSaveFile->setFilePointer(mFileEntry, static_cast<long>(sector << 12), NULL, 0);
    if (!compressed)
        header &= 0x7fffffffU;
    else
        header |= 0x80000000U;
    if (rle)
        header |= 0x40000000U;

    const unsigned int serializedHeader = encodeSaveWord(header);
    const unsigned int serializedLength = encodeSaveWord(static_cast<unsigned int>(length));
    mSaveFile->writeFile(mFileEntry, &serializedHeader, 4, &written);
    header &= 0x3fffffffU;
    mSaveFile->writeFile(mFileEntry, &serializedLength, 4, &written);
    mSaveFile->writeFile(mFileEntry, data, header, &written);
    if (deleteData)
        delete[] data;
}
void RegionFile::zero(int sector,int length)
{if(!mSaveFile||!mFileEntry)return; mSaveFile->setFilePointer(mFileEntry,(long)(sector<<12),NULL,0); unsigned int written=0; mSaveFile->zeroFile(mFileEntry,(unsigned int)length,&written);}
void RegionFile::insertInitialSectors()
{
 unsigned char buffer[0x1000];
 unsigned int written=0;
 mSaveFile->setFilePointer(mFileEntry,0,NULL,0);
 std::memset(buffer,0,0x1000);
 mSaveFile->writeFile(mFileEntry,buffer,0x1000,&written);
 mSaveFile->writeFile(mFileEntry,buffer,0x1000,&written);
 mNewFile=false;
}
void RegionFile::setOffset(int x, int z, int offset)
{
    if (mNewFile)
        insertInitialSectors();
    if (outOfBounds(x, z))
        return;

    const unsigned int index = static_cast<unsigned int>(x + z * 32);
    mOffsets[index] = static_cast<unsigned int>(offset);
    const unsigned int serialized = encodeSaveWord(static_cast<unsigned int>(offset));
    unsigned int written = 0;
    mSaveFile->setFilePointer(mFileEntry, static_cast<long>(index * 4), NULL, 0);
    mSaveFile->writeFile(mFileEntry, &serialized, 4, &written);
}

void RegionFile::setTimestamp(int x, int z, int timestamp)
{
    if (mNewFile)
        insertInitialSectors();
    if (outOfBounds(x, z))
        return;

    const unsigned int index = static_cast<unsigned int>(x + z * 32);
    mTimestamps[index] = static_cast<unsigned int>(timestamp);
    const unsigned int serialized = encodeSaveWord(static_cast<unsigned int>(timestamp));
    unsigned int written = 0;
    mSaveFile->setFilePointer(mFileEntry, static_cast<long>(index * 4 + 0x1000), NULL, 0);
    mSaveFile->writeFile(mFileEntry, &serialized, 4, &written);
}
void RegionFile::write(int x,int z,unsigned char* data,int length,bool compressed)
{
 unsigned int bufferSize=(unsigned int)length+0x800;
 bool ownsBuffer=false;
 unsigned char* buffer;
 if(bufferSize<0x10000){
  buffer=(unsigned char*)GetThreadCompressedDataStorage();
  if(buffer==NULL){ownsBuffer=true;buffer=new unsigned char[bufferSize];}
 }else{
  ownsBuffer=true;
  buffer=new unsigned char[bufferSize];
 }
 Compression* c=Compression::getCompression();
 unsigned int compressedSize=bufferSize;
 int rleResult=c->CompressLZXRLE(buffer,&compressedSize,data,(unsigned int)length,NULL);
 if(rleResult!=0)c->Compress(buffer,&compressedSize,data,(unsigned int)length);
 bool rleUsed=rleResult==0;
 unsigned int sectorsNeeded=((compressedSize+8)>>12)+1;
 if(sectorsNeeded<0x100){
  unsigned int existing=getOffset(x,z);
  unsigned int existingSector=existing>>8,existingCount=existing&0xff;
  unsigned int sector=existingSector;
  if(existingSector==0||existingCount!=sectorsNeeded){
   for(unsigned int i=0;i<existingCount;++i)mUsedSectors->at(existingSector+i)=false;
   zero((int)existingSector,(int)(existingCount<<12));
   std::vector<bool>::iterator it=std::find(mUsedSectors->begin()+2,mUsedSectors->end(),true);
   unsigned int scan=(unsigned int)(it-mUsedSectors->begin());
   unsigned int runStart=scan;
   int run=0;
   if(it!=mUsedSectors->end()){
    while(scan<mUsedSectors->size()){
     if(mUsedSectors->at(scan)){
      if(run==0)runStart=scan;
      ++run;
      if((int)sectorsNeeded<=run)break;
     }else{
      run=0;
     }
     ++scan;
    }
   }
   if(run<(int)sectorsNeeded){
    mSaveFile->setFilePointer(mFileEntry,0,NULL,2);
    unsigned int newSector=(unsigned int)mUsedSectors->size();
    unsigned int written=0;
    unsigned char zeroSector[0x1000];
    std::memset(zeroSector,0,0x1000);
    for(unsigned int i=0;i<sectorsNeeded;++i){
     mSaveFile->writeFile(mFileEntry,zeroSector,0x1000,&written);
     mUsedSectors->push_back(false);
    }
    mSizeDelta+=(int)(sectorsNeeded<<12);
    write((int)newSector,buffer,(int)length,compressedSize,rleUsed,compressed,ownsBuffer);
    setOffset(x,z,(int)(newSector<<8|sectorsNeeded));
    setTimestamp(x,z,(int)System::processTimeInSeconds());
    return;
   }
   sector=runStart;
   setOffset(x,z,(int)(sector<<8|sectorsNeeded));
   for(unsigned int i=0;i<sectorsNeeded;++i)mUsedSectors->at(sector+i)=false;
  }
  write((int)sector,buffer,(int)length,compressedSize,rleUsed,compressed,ownsBuffer);
  setTimestamp(x,z,(int)System::processTimeInSeconds());
 }
}
int RegionFile::fjCountSavedChunks()const{int n=0;for(int i=0;i<1024;++i)if(mOffsets[i])++n;return n;}
void RegionFile::writeAllOffsets()
{
    if (!mSaveFile || !mFileEntry || mNewFile)
        return;

    unsigned int offsets[1024];
    unsigned int timestamps[1024];
    std::memcpy(offsets, mOffsets, 0x1000);
    std::memcpy(timestamps, mTimestamps, 0x1000);
    if (FileHeader::getNativeEndian() != eByteOrder_BigEndian)
    {
        for (int i = 0; i < 1024; ++i)
        {
            offsets[i] = encodeSaveWord(offsets[i]);
            timestamps[i] = encodeSaveWord(timestamps[i]);
        }
    }

    unsigned int written = 0;
    mSaveFile->setFilePointer(mFileEntry, 0, NULL, 0);
    mSaveFile->writeFile(mFileEntry, offsets, 0x1000, &written);
    mSaveFile->setFilePointer(mFileEntry, 0x1000, NULL, 0);
    mSaveFile->writeFile(mFileEntry, timestamps, 0x1000, &written);
    mSaveFile->finalizeWrite();
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
