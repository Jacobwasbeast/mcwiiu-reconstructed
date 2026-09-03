#ifndef GAME_CLIENT_RENDER_TEXTUREATLASSprite_HPP
#define GAME_CLIENT_RENDER_TEXTUREATLASSprite_HPP
#include <string>
#include <vector>
class Texture;
class BufferedReader;
class TextureAtlasSprite {
public:
 TextureAtlasSprite(const std::wstring&,const std::wstring&,float,float,float,float,float,float); virtual ~TextureAtlasSprite();
 static TextureAtlasSprite* create(const std::wstring&); void initUVs(float,float,float,float);
 void init(Texture*,std::vector<Texture*>*,int,int,int,int,bool); void replaceWith(TextureAtlasSprite*);
 unsigned int getX() const; unsigned int getY() const; unsigned int getWidth() const; unsigned int getHeight() const;
 double getU0(bool) const; double getU1(bool) const; double getU(double) const; double getV0(bool) const; double getV1(bool) const; double getV(double) const;
 void adjustUVs(float&,float&,float&,float&); void adjustU(float&,float&); void adjustV(float&,float&);
 std::wstring getName() const; unsigned int getSourceWidth() const; unsigned int getSourceHeight() const; void uploadInterpolatedFrame(); void cycleFrames();
 int getFrameCount() const; Texture* getSource(); Texture* getFrame(int); int getFrames(); void loadAnimationFrames(BufferedReader*); void loadAnimationFrames(const std::wstring&);
 unsigned int getFlags() const; void setFlags(int); void freeFrameTextures(); unsigned int hasOwnData();
private:
 std::wstring mName; std::wstring mParent; Texture* mSource; std::vector<Texture*>* mFrames; Texture* mOwnTexture; std::vector<std::pair<int,int> >* mAnimationFrames;
 int mFlags; bool mInterpolated; bool mRotated; int mX; int mY; int mWidth; int mHeight; float mU0; float mU1; float mV0; float mV1; float mUOffset; float mVOffset; float mUScale; float mVScale; int mFrame; int mFrameTime;
};
#endif
