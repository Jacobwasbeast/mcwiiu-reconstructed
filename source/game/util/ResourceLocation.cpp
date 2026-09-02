#include "game/util/ResourceLocation.hpp"

#include <cstring>

int javaStringHashCode(const std::wstring& text);

ResourceLocation::ResourceLocation() {
    textures = NULL;
    textureCount = 0;
    preloaded = false;

    path = L"";
    nameSpace = L"minecraft";
}

ResourceLocation::ResourceLocation(const ResourceLocation& other) {
    textures = NULL;
    textureCount = 0;

    _init(other);
}

ResourceLocation::ResourceLocation(_TEXTURE_NAME textureName) {
    textures = NULL;
    textureCount = 0;

    path = L"";
    nameSpace = L"minecraft";

    textures = new int[1];
    textures[0] = textureName;
    preloaded = true;
    textureCount = 1;
}

ResourceLocation::ResourceLocation(const std::wstring& name) {
    textures = NULL;
    textureCount = 0;

    arrayWithLength<std::wstring> parts = decompose(name);

    path = parts.data[1];
    nameSpace = parts.data[0].length() == 0 ? L"minecraft" : parts.data[0];

    delete[] parts.data;

    preloaded = false;
}

ResourceLocation::ResourceLocation(const std::wstring& nameSpace, const std::wstring& path) {
    textures = NULL;
    textureCount = 0;

    this->nameSpace = nameSpace;

    if (this->nameSpace.length() == 0) {
        this->nameSpace = L"minecraft";
    }

    this->path = path;
}

ResourceLocation::ResourceLocation(arrayWithLength<int> textures) {
    this->textures = NULL;
    textureCount = 0;

    this->textures = new int[textures.length]();
    textureCount = textures.length;

    for (int i = 0; i < textureCount; i++) {
        this->textures[i] = textures.data[i];
    }

    preloaded = true;
}

ResourceLocation::~ResourceLocation() {
    delete textures;
}

void ResourceLocation::_init(const ResourceLocation& other) {
    if (other.textureCount != 0) {
        textures = new int[other.textureCount]();
        textureCount = other.textureCount;
        memcpy(textures, other.textures, other.textureCount * sizeof(int));
    } else {
        textures = other.textures;
        textureCount = 0;
    }

    path = other.path;
    nameSpace = other.nameSpace;
    preloaded = other.preloaded;
}

ResourceLocation& ResourceLocation::operator=(const ResourceLocation& other) {
    _init(other);

    return *this;
}

arrayWithLength<std::wstring> ResourceLocation::decompose(const std::wstring& name) {
    arrayWithLength<std::wstring> parts(2, true);

    parts.data[0] = L"";
    parts.data[1] = name;

    int index = name.find(L':');

    if (index >= 0) {
        parts.data[1] = name.substr(index + 1, name.length());

        if (index > 1) {
            parts.data[0] = name.substr(0, index);
        }
    }

    return parts;
}

const std::wstring& ResourceLocation::getNamespace() const {
    return nameSpace;
}

const std::wstring& ResourceLocation::getPath() const {
    return path;
}

bool ResourceLocation::isDefault() const {
    return getNamespace() == L"minecraft" && getPath() == L"";
}

int ResourceLocation::getTexture() const {
    return textures[0];
}

int ResourceLocation::getTexture(int index) const {
    return textures[index];
}

int ResourceLocation::getTextureCount() const {
    return textureCount;
}

bool ResourceLocation::isPreloaded() const {
    return preloaded;
}

std::wstring ResourceLocation::toString() const {
    std::wstring text = getNamespace() + L':';

    return text + getPath();
}

int ResourceLocation::hashCode() const {
    return javaStringHashCode(getNamespace()) * 31 + javaStringHashCode(getPath());
}

bool ResourceLocation::equals(const ResourceLocation* other) const {
    if (this == other) {
        return true;
    }

    return getNamespace() == other->getNamespace() && getPath() == other->getPath();
}
