#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/Texture.hpp"
#include "game/core/BufferedReader.hpp"
#include "game/core/ByteBuffer.hpp"
#include <sstream>
#include <cstdlib>
#include "game/core/ClockAtlasSprite.hpp"
#include "game/core/CompassAtlasSprite.hpp"
#include "game/core/StringUtils.hpp"
#include "game/client/render/TextureManager.hpp"

TextureAtlasSprite::TextureAtlasSprite(const std::wstring& name,const std::wstring& parent,float u0,float v0,float u1,float v1,float du,float dv)
 : mName(name),mParent(parent),mSource(0),mFrames(0),mOwnTexture(0),mAnimationFrames(0),mFlags(0),mInterpolated(false),mRotated(false),mX(0),mY(0),mWidth(0),mHeight(0),mU0(u0),mU1(u1),mV0(v0),mV1(v1),mUOffset(0.0625f / du),mVOffset(0.0625f / dv),mUScale(0.0f),mVScale(0.0f),mFrame(0),mFrameTime(0) {}

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
 mU0=(float)x/(float)source->getWidth(); mU1=(float)(x+width)/(float)source->getWidth();
 mV0=(float)y/(float)source->getHeight(); mV1=(float)(y+height)/(float)source->getHeight();
 mUScale=(float)width*0.0625f; mVScale=(float)height*0.0625f;
 delete mOwnTexture; mOwnTexture=0;
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
float TextureAtlasSprite::getU(double x) const { return (getU1(false)-getU0(false))*(float(x)*0.0625f)+getU0(false); }
float TextureAtlasSprite::getV0(bool shrink) const { return shrink ? mV0+mVOffset : mV0; }
float TextureAtlasSprite::getV1(bool shrink) const { return shrink ? mV1-mVOffset : mV1; }
float TextureAtlasSprite::getV(double y) const { return (getV1(false)-getV0(false))*(float(y)*0.0625f)+getV0(false); }
void TextureAtlasSprite::adjustU(float& a,float& b) { if(a<b){a+=mUOffset;b-=mUOffset;} else if(a>b){b+=mUOffset;a-=mUOffset;} }
void TextureAtlasSprite::adjustV(float& a,float& b) { if(a<b){a+=mVOffset;b-=mVOffset;} else if(a>b){b+=mVOffset;a-=mVOffset;} }
void TextureAtlasSprite::adjustUVs(float& u0,float& u1,float& v0,float& v1) { adjustU(u0,u1); adjustV(v0,v1); }
std::wstring TextureAtlasSprite::getName() const { return mName; }
unsigned int TextureAtlasSprite::getSourceWidth() const { return mSource ? mSource->getWidth() : 32; }
unsigned int TextureAtlasSprite::getSourceHeight() const { return mSource ? mSource->getHeight() : 32; }

void TextureAtlasSprite::uploadInterpolatedFrame() {
 if (!mSource || !mOwnTexture || !mAnimationFrames || mAnimationFrames->empty()) {
  if (mSource && mOwnTexture) mSource->blit(mX, mY, mOwnTexture, mRotated);
  return;
 }
 const std::pair<int,int>& cur = (*mAnimationFrames)[mFrame];
 const std::pair<int,int>& next = (*mAnimationFrames)[(mFrame + 1) % mAnimationFrames->size()];
 Texture* first = getFrame(cur.first);
 Texture* second = getFrame(next.first);
 ByteBuffer* firstData = first ? reinterpret_cast<ByteBuffer*>(first->getData(0)) : NULL;
 ByteBuffer* secondData = second ? reinterpret_cast<ByteBuffer*>(second->getData(0)) : NULL;
 ByteBuffer* outputData = reinterpret_cast<ByteBuffer*>(mOwnTexture->getData(0));
 if (!firstData || !secondData || !outputData || cur.second <= 0) {
  mSource->blit(mX, mY, mOwnTexture, mRotated);
  return;
 }
 const int width = static_cast<int>(mOwnTexture->getWidth());
 const int height = static_cast<int>(mOwnTexture->getHeight());
 const int frameTime = mFrameTime < cur.second ? mFrameTime : cur.second;
 const int firstWeight = 256 - (frameTime * 256 / cur.second);
 const int secondWeight = 256 - firstWeight;
 unsigned char* a = firstData->getBuffer();
 unsigned char* b = secondData->getBuffer();
 unsigned char* out = outputData->getBuffer();
 for (int i = 0; i < width * height; ++i) {
  const int offset = i * 4;
  out[offset] = a[offset];
  for (int channel = 1; channel < 4; ++channel)
   out[offset + channel] = static_cast<unsigned char>((a[offset + channel] * firstWeight + b[offset + channel] * secondWeight) >> 8);
 }
 mSource->blit(mX, mY, mOwnTexture, mRotated);
}
void TextureAtlasSprite::cycleFrames() {
 if(!mAnimationFrames){ if(mFrames&&!mFrames->empty()){mFrame=(mFrame+1)%mFrames->size();mSource->blit(mX,mY,(*mFrames)[mFrame],mRotated);} return; }
 if(mAnimationFrames->empty()) return; std::pair<int,int>& p=(*mAnimationFrames)[mFrame]; ++mFrameTime;
 if(mFrameTime<p.second){if(mInterpolated)uploadInterpolatedFrame();return;} mFrameTime=0; mFrame=(mFrame+1)%mAnimationFrames->size(); int index=(*mAnimationFrames)[mFrame].first; if(index>=0&&mFrames&&index<(int)mFrames->size()) mSource->blit(mX,mY,(*mFrames)[index],mRotated);
}
int TextureAtlasSprite::getFrameCount() const { return mFrames ? (int)mFrames->size() : 0; }
Texture* TextureAtlasSprite::getSource() { return mSource; }
Texture* TextureAtlasSprite::getFrame(int index) { return mFrames&&index>=0&&index<(int)mFrames->size()?(*mFrames)[index]:0; }
int TextureAtlasSprite::getFrames() { return mFrames ? (int)mFrames->size() : 0; }

void TextureAtlasSprite::loadAnimationFrames(const std::wstring& text) {
 delete mAnimationFrames; mAnimationFrames=0; mFrame=0; mFrameTime=0; mInterpolated=false;
 std::wstring value=trimString(text); if(!value.empty()&&value[0]==L'#'){value=value.substr(1);mInterpolated=true;}
 mAnimationFrames=new std::vector<std::pair<int,int> >(); std::wstring::size_type start=0;
 while (start <= value.size()) {
  std::wstring::size_type end = value.find(L',', start);
  std::wstring token = value.substr(start,
   end == std::wstring::npos ? value.size() - start : end - start);
  token = trimString(token);
  if (token.empty()) { if (end == std::wstring::npos) break; start = end + 1; continue; }
  std::wstring::size_type star = token.find(L'*');
  if (star == std::wstring::npos) {
   mAnimationFrames->push_back(std::make_pair(
    std::wcstol(token.c_str(), 0, 10), 1));
  } else {
   mAnimationFrames->push_back(std::make_pair(
    std::wcstol(token.substr(0, star).c_str(), 0, 10),
    std::wcstol(token.substr(star + 1).c_str(), 0, 10)));
  }
  if (end == std::wstring::npos)
   break;
  start = end + 1;
 }
 if(mAnimationFrames->empty()||mAnimationFrames->size()>=600){delete mAnimationFrames;mAnimationFrames=0;}
}
void TextureAtlasSprite::loadAnimationFrames(BufferedReader* reader) { if(!reader)return; std::wstring all,line; while(!(line=reader->readLine()).empty()){if(!all.empty())all+=L",";all+=line;} loadAnimationFrames(all); }
unsigned int TextureAtlasSprite::getFlags() const { return mFlags; }
void TextureAtlasSprite::setFlags(int flags) { mFlags=flags; }
void TextureAtlasSprite::freeFrameTextures() { if(mFrames){for(std::vector<Texture*>::iterator i=mFrames->begin();i!=mFrames->end();++i) { TextureManager::getInstance()->unregisterTexture(L"", *i); delete *i; }delete mFrames;mFrames=0;} delete mOwnTexture;mOwnTexture=0; }
unsigned int TextureAtlasSprite::hasOwnData() { return 1; }
