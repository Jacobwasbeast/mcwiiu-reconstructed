#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/Texture.hpp"
#include "game/core/BufferedReader.hpp"
#include "game/core/ByteBuffer.hpp"
#include <sstream>
#include <cstdlib>
#include "game/core/ClockAtlasSprite.hpp"
#include "game/core/CompassAtlasSprite.hpp"
#include "game/core/StringUtils.hpp"
#include "game/core/StringConversion.hpp"
#include "game/client/render/TextureManager.hpp"

TextureAtlasSprite::TextureAtlasSprite(const std::wstring& name,const std::wstring& parent,float u0,float v0,float u1,float v1,float du,float dv)
 : mName(name),mParent(),mSource(0),mFrames(0),mAnimationFrames(0),mFlags(0),mRotated(false),mX(0),mY(0),mWidth(0),mHeight(0),mU0(u0),mU1(u1),mV0(v0),mV1(v1),mUOffset(0.0625f / du),mVOffset(0.0625f / dv),mUScale(0.0f),mVScale(0.0f),mFrame(0),mFrameTime(0)
{
 mParent = parent;
 mOwnTexture = 0;
 mInterpolated = false;
}

TextureAtlasSprite::~TextureAtlasSprite() {
 if (mFrames) {
  for (std::vector<Texture*>::iterator i = mFrames->begin(); i != mFrames->end(); ++i) delete *i;
  delete mFrames;
 }
}

TextureAtlasSprite* TextureAtlasSprite::create(const std::wstring& name) {
 if (name==L"clock") return new ClockAtlasSprite();
 if (name==L"compass") return new CompassAtlasSprite();
 return new TextureAtlasSprite(name,name,0,0,0,0,256,256);
}

void TextureAtlasSprite::initUVs(float u0,float v0,float u1,float v1) { mU0=u0; mU1=u1; mV0=v0; mV1=v1; }

void TextureAtlasSprite::init(Texture* source,std::vector<Texture*>* frames,int x,int y,int width,int height,bool rotated) {
 mSource=source; mFrames=frames; mX=x; mY=y; mWidth=width; mHeight=height; mRotated=rotated; mFrame=-1;
 float e=0.0f;
 mU0=(float)x/(float)(int)source->getWidth()+e; mU1=(float)(x+width)/(float)(int)source->getWidth()-e;
 mV0=(float)y/(float)(int)source->getHeight()+e; mV1=(float)(y+height)/(float)(int)source->getHeight()-e;
 mUScale=(float)width*0.0625f; mVScale=(float)height*0.0625f;
 if (mOwnTexture) { delete mOwnTexture; mOwnTexture=0; }
 mOwnTexture=new Texture(source->m_name,2,width,height,source->m_depth,source->m_wrap,source->m_format,source->m_minFilter,source->m_magFilter,0,source->m_mipmap);
}

void TextureAtlasSprite::replaceWith(TextureAtlasSprite* sprite) {
 if (!sprite)
  return;
 mName = sprite->mName;
 mParent = sprite->mParent;
 mSource = sprite->mSource;
 mFrames = sprite->mFrames;
 mOwnTexture = sprite->mOwnTexture;
 mAnimationFrames = sprite->mAnimationFrames;
 mFlags = sprite->mFlags;
 mInterpolated = sprite->mInterpolated;
 mRotated = sprite->mRotated;
 mX = sprite->mX;
 mY = sprite->mY;
 mWidth = sprite->mWidth;
 mHeight = sprite->mHeight;
 mU0 = sprite->mU0;
 mU1 = sprite->mU1;
 mV0 = sprite->mV0;
 mV1 = sprite->mV1;
 mUOffset = sprite->mUOffset;
 mVOffset = sprite->mVOffset;
 mUScale = sprite->mUScale;
 mVScale = sprite->mVScale;
 mFrame = sprite->mFrame;
 mFrameTime = sprite->mFrameTime;
}
unsigned int TextureAtlasSprite::getX() const { return mX; }
unsigned int TextureAtlasSprite::getY() const { return mY; }
unsigned int TextureAtlasSprite::getWidth() const { return mWidth; }
unsigned int TextureAtlasSprite::getHeight() const { return mHeight; }
float TextureAtlasSprite::getU0(bool shrink) const { return shrink ? mU0+mUOffset : mU0; }
float TextureAtlasSprite::getU1(bool shrink) const { return shrink ? mU1-mUOffset : mU1; }
float TextureAtlasSprite::getU(double x) const {
 float width = getU1(false) - getU0(false);
 return width * (float(x) * 0.0625f) + getU0(false);
}
float TextureAtlasSprite::getV0(bool shrink) const { return shrink ? mV0+mVOffset : mV0; }
float TextureAtlasSprite::getV1(bool shrink) const { return shrink ? mV1-mVOffset : mV1; }
float TextureAtlasSprite::getV(double y) const {
 float height = getV1(false) - getV0(false);
 return height * (float(y) * 0.0625f) + getV0(false);
}
void TextureAtlasSprite::adjustU(float& a,float& b) { if(a<b){a+=mUOffset;b-=mUOffset;} else if(a>b){b+=mUOffset;a-=mUOffset;} }
void TextureAtlasSprite::adjustV(float& a,float& b) { if(a<b){a+=mVOffset;b-=mVOffset;} else if(a>b){b+=mVOffset;a-=mVOffset;} }
void TextureAtlasSprite::adjustUVs(float& u0,float& u1,float& v0,float& v1) {
 if(u0<u1){u0+=mUOffset;u1-=mUOffset;} else if(u0>u1){u1+=mUOffset;u0-=mUOffset;}
 if(v0<v1){v0+=mVOffset;v1-=mVOffset;} else if(v0>v1){v1+=mVOffset;v0-=mVOffset;}
}
std::wstring TextureAtlasSprite::getName() const { return mName; }
unsigned int TextureAtlasSprite::getSourceWidth() const { return mSource ? mSource->getWidth() : 32; }
unsigned int TextureAtlasSprite::getSourceHeight() const { return mSource ? mSource->getHeight() : 32; }

void TextureAtlasSprite::uploadInterpolatedFrame() {
 std::pair<int,int>& current = mAnimationFrames->at(mFrame);
 int duration = current.second;
 int firstFrame = current.first;
 double progress = 1.0 - (double)mFrameTime / (double)duration;
 int firstWeight = (int)(256.0 * progress);
 int secondWeight = (int)(256.0 * (1.0 - progress));
 int nextIndex = (mFrame + 1) % mAnimationFrames->size();
 int nextFrame = mAnimationFrames->at(nextIndex).first;
 Texture* first = mFrames->at(firstFrame);
 Texture* second = mFrames->at(nextFrame);
 for (int level = 0; level < (int)mSource->m_mipLevels; ++level) {
  ByteBuffer* firstData = reinterpret_cast<ByteBuffer*>(first->getData(level));
  ByteBuffer* secondData = reinterpret_cast<ByteBuffer*>(second->getData(level));
  ByteBuffer* outputData = reinterpret_cast<ByteBuffer*>(mOwnTexture->getData(level));
  if (firstData == 0 || secondData == 0) {
   mSource->blit(mX, mY, mOwnTexture, mRotated);
   return;
  }
  unsigned char* a = firstData->getBuffer();
  unsigned char* b = secondData->getBuffer();
  unsigned char* out = outputData->getBuffer();
  int channels[8] = { 3, 0, 1, 2, 3, 2, 1, 0 };
  int* order = channels;
  if (first->m_format == 1)
   order = channels + 4;
  unsigned int width = mWidth >> level;
  int height = mHeight >> level;
  for (int y = 0; y < height; ++y) {
   for (int x = 0; x < (int)width; ++x) {
    int source = (y * width + x) * 4;
    int target = source;
    if (mRotated)
     target = (x * width + (height - y)) * 4;
    out[target + order[0]] = a[source + order[0]];
    for (int channel = 1; channel < 4; ++channel)
     out[target + order[channel]] = (a[source + order[channel]] * firstWeight + b[source + order[channel]] * secondWeight) / 256;
   }
  }
 }
 mSource->blit(mX, mY, mOwnTexture, mRotated);
}
void TextureAtlasSprite::cycleFrames() {
 if (mAnimationFrames == 0) {
  int previous = mFrame;
  mFrame = (mFrame + 1) % mFrames->size();
  if (previous != mFrame)
   mSource->blit(mX, mY, mFrames->at(mFrame), mRotated);
 } else {
  std::pair<int,int> current = mAnimationFrames->at(mFrame);
  if (mFrameTime + 1 < current.second) {
   mFrameTime = mFrameTime + 1;
   if (mInterpolated)
    uploadInterpolatedFrame();
  } else {
   mFrame = (mFrame + 1) % mAnimationFrames->size();
   mFrameTime = 0;
   int next = mAnimationFrames->at(mFrame).first;
   if (current.first != next) {
    if (next >= 0 && (unsigned int)next < mFrames->size())
     mSource->blit(mX, mY, mFrames->at(next), mRotated);
   }
  }
 }
}
int TextureAtlasSprite::getFrameCount() const { return mFrames ? (int)mFrames->size() : 0; }
Texture* TextureAtlasSprite::getSource() { return mSource; }
Texture* TextureAtlasSprite::getFrame(int index) { return mFrames&&index>=0&&index<(int)mFrames->size()?(*mFrames)[index]:0; }
int TextureAtlasSprite::getFrames() { return mFrames ? (int)mFrames->size() : 0; }

void TextureAtlasSprite::loadAnimationFrames(const std::wstring& text) {
 if (mAnimationFrames) { delete mAnimationFrames; mAnimationFrames = 0; }
 mFrameTime = 0;
 mFrame = 0;
 std::vector<std::pair<int,int> >* frames = new std::vector<std::pair<int,int> >();
 std::wstring value = trimString(text);
 if (value[0] == L'#') {
  value = value.substr(1);
  mInterpolated = true;
 }
 std::vector<std::wstring> tokens = stringSplit(value, L',');
 for (std::vector<std::wstring>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
  std::wstring token = trimString(*it);
  if (!token.empty()) {
   std::wstring::size_type star = token.find(L'*');
   if (star == std::wstring::npos) {
    frames->push_back(std::pair<int,int>(_fromString<int>(token), 1));
   } else {
    int frame = _fromString<int>(token.substr(0, star));
    int time = _fromString<int>(token.substr(star + 1));
    frames->push_back(std::pair<int,int>(frame, time));
   }
  }
 }
 if (frames->empty() || frames->size() > 599) {
  delete frames;
 } else {
  mAnimationFrames = frames;
 }
}
void TextureAtlasSprite::loadAnimationFrames(BufferedReader* reader) { if(!reader)return; std::wstring all,line; while(!(line=reader->readLine()).empty()){if(!all.empty())all+=L",";all+=line;} loadAnimationFrames(all); }
unsigned int TextureAtlasSprite::getFlags() const { return mFlags; }
void TextureAtlasSprite::setFlags(int flags) { mFlags=flags; }
void TextureAtlasSprite::freeFrameTextures() {
 if (mFrames) {
  for (std::vector<Texture*>::iterator i = mFrames->begin(); i != mFrames->end(); ++i) {
   TextureManager::getInstance()->unregisterTexture(L"", *i);
   delete *i;
  }
  delete mFrames;
 }
}
unsigned int TextureAtlasSprite::hasOwnData() { return 1; }
