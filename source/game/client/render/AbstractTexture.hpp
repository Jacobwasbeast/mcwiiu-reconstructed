#ifndef GAME_CLIENT_RENDER_ABSTRACTTEXTURE_HPP
#define GAME_CLIENT_RENDER_ABSTRACTTEXTURE_HPP

class ResourceManager;

class AbstractTexture
{
public:
    AbstractTexture();

    virtual void setFilter(bool blur, bool mipmap);
    virtual void pushFilter(bool blur, bool mipmap);
    virtual void popFilter();
    virtual void load(ResourceManager* resourceManager) = 0;
    virtual int getId();
    virtual void releaseId();

protected:
    int id;
    bool blur;
    bool mipmap;
    bool oldBlur;
    bool oldMipmap;
};

#endif
