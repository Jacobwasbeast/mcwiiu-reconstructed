#ifndef GAME_CORE_FLIPPEDICON_HPP
#define GAME_CORE_FLIPPEDICON_HPP

#include "game/client/render/TextureAtlasSprite.hpp"

class FlippedIcon : public TextureAtlasSprite {
public:
    FlippedIcon(TextureAtlasSprite* sprite, bool flipU, bool flipV);
    virtual ~FlippedIcon();
    virtual unsigned int getX() const;
    virtual unsigned int getY() const;
    virtual unsigned int getWidth() const;
    virtual unsigned int getHeight() const;
    virtual double getU0(bool) const;
    virtual double getU1(bool) const;
    virtual double getU(double) const;
    virtual double getV0(bool) const;
    virtual double getV1(bool) const;
    virtual double getV(double) const;
    virtual std::wstring getName() const;
    virtual unsigned int getSourceWidth() const;
    virtual unsigned int getSourceHeight() const;
    virtual unsigned int getFlags() const;
    virtual void setFlags(int);
private:
    TextureAtlasSprite* mSprite;
    bool mFlipU;
    bool mFlipV;
};

#endif
