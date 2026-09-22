#ifndef GAME_CORE_COMPASSATLASSprite_HPP
#define GAME_CORE_COMPASSATLASSprite_HPP

#include "game/client/render/TextureAtlasSprite.hpp"

class Level;

class CompassAtlasSprite : public TextureAtlasSprite
{
public:
    CompassAtlasSprite();
    CompassAtlasSprite(int frameCount, CompassAtlasSprite* parent);
    virtual ~CompassAtlasSprite();

    void updateFromPosition(Level* level, double x, double y, double z, bool hasLevel, bool force);
    virtual void cycleFrames();
    virtual unsigned int getSourceWidth() const;
    virtual unsigned int getSourceHeight() const;
    virtual int getFrames();
    virtual void freeFrameTextures();
    virtual unsigned int hasOwnData();

private:
    int mFrameCount;
    CompassAtlasSprite* mParent;
    unsigned char mReserved[4];
    double mAngle;
    double mTargetAngle;
};

#endif
