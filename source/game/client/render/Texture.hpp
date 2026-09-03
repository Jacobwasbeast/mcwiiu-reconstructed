#ifndef GAME_CLIENT_RENDER_TEXTURE_HPP
#define GAME_CLIENT_RENDER_TEXTURE_HPP

#include <string>
#include "game/core/BufferedImage.hpp"
#include "game/core/Rect2i.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class ByteBuffer;

class Texture
{
public:
    Texture(const std::wstring& name, int type, int width, int height, int format, int filter,
            int wrap, int target, BufferedImage* image, bool mipmap);
    Texture(const std::wstring& name, int type, int width, int height, int format, int filter,
            int wrap, int target, int mipLevels, BufferedImage* image, bool mipmap);
    ~Texture();

    void _init(const std::wstring& name, int type, int width, int height, int format, int filter,
               int wrap, int target, bool mipmap);
    void updateOnGPU();
    static unsigned int crispBlend(int first, int second);
    void transferFromImage(BufferedImage* image);
    void _init(const std::wstring& name, int type, int width, int height, int format, int filter,
               int wrap, int target, int mipLevels, BufferedImage* image, bool mipmap);
    unsigned int getData(unsigned int mipLevel);
    unsigned int getHeight();
    unsigned int getWidth();
    void blit(int x, int y, Texture* texture, bool flip);
    unsigned int getManagerId();
    unsigned int getGlId();
    void bind(int textureUnit);
    Texture(const std::wstring& name, int type, int width, int height, int format, int filter,
            int wrap, int target, bool mipmap);
    Rect2i* getRect();
    void fill(const Rect2i* rect, int color);
    void writeAsBMP(const std::wstring& fileName);
    void writeAsPNG(const std::wstring& fileName);
    void blit(int x, int y, Texture* texture);
    void transferFromBuffer(arrayWithLength<int> pixels);
    std::wstring getName();
    void setImmediateUpdate(bool immediate);

private:
    unsigned int m_glId;
    unsigned int m_managerId;
    int m_type;
    int m_width;
    int m_height;
    int m_format;
    int m_filter;
    int m_wrap;
    int m_target;
    bool m_mipmap;
    bool m_hasMipmaps;
    bool m_immediateUpdate;
    std::wstring m_name;
    Rect2i* m_rect;
    unsigned int m_mipLevels;
    ByteBuffer* m_data[10];
    bool m_dirty;
};

#endif
