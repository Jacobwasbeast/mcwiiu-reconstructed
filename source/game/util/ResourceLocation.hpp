#ifndef GAME_UTIL_RESOURCELOCATION_HPP
#define GAME_UTIL_RESOURCELOCATION_HPP

#include <string>

#include "game/world/biome/arrayWithLength.hpp"

enum _TEXTURE_NAME {};

class ResourceLocation {
public:
    ResourceLocation();
    ResourceLocation(const ResourceLocation& other);
    ResourceLocation(_TEXTURE_NAME textureName);
    ResourceLocation(const std::wstring& name);
    ResourceLocation(const std::wstring& nameSpace, const std::wstring& path);
    ResourceLocation(arrayWithLength<int> textures);
    virtual ~ResourceLocation();

    ResourceLocation& operator=(const ResourceLocation& other);

    const std::wstring& getNamespace() const;
    const std::wstring& getPath() const;
    bool isDefault() const;

    int getTexture() const;
    int getTexture(int index) const;
    int getTextureCount() const;
    bool isPreloaded() const;

    virtual std::wstring toString() const;

    int hashCode() const;
    bool equals(const ResourceLocation* other) const;

protected:
    static arrayWithLength<std::wstring> decompose(const std::wstring& name);

private:
    void _init(const ResourceLocation& other);

    int* textures;
    int textureCount;
    std::wstring path;
    std::wstring nameSpace;
    bool preloaded;
};

#endif
