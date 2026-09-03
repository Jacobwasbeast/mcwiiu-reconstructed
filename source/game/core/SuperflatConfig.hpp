#ifndef GAME_CORE_SUPERFLATCONFIG_HPP
#define GAME_CORE_SUPERFLATCONFIG_HPP

#include <string>
#include <vector>
#include <map>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/FlatLayerInfo.hpp"
#include "game/core/DefaultPresets.hpp"

class ByteArrayTag;
class DataFixerUpper;
class SuperflatConfig
{
public:
    SuperflatConfig();
    ~SuperflatConfig();
    bool hasParam(unsigned char parameter) const;
    SuperflatConfig* setDisplayname(const std::wstring& name);
    bool hasOption(unsigned char parameter, unsigned char option) const;
    void addParam(unsigned char parameter);
    void setOption(unsigned char parameter, unsigned char option, unsigned char value);
    void removeParam(unsigned char parameter);
    void updateLayers();
    void clear();
    std::wstring getDisplayname() const;
    void setVersion(unsigned char version);
    unsigned char getOption(unsigned char parameter, unsigned char option) const;
    unsigned int maybeOption(unsigned char parameter, unsigned char option, unsigned char fallback) const;
    arrayWithLength<unsigned char> toBlob() const;
    void fromBlob(arrayWithLength<unsigned char> data);
    ByteArrayTag* toTag() const;
    void fromTag(ByteArrayTag* tag);
    SuperflatConfig* makeDefaults();
    SuperflatConfig* makePreset(DefaultPresets::Preset preset);
    unsigned char getVersion() const;
    SuperflatConfig* makeDebug();

    std::vector<FlatLayerInfo> layers;

private:
    unsigned char mVersion;
    std::wstring mDisplayname;
    std::map<unsigned char, std::map<unsigned char, unsigned char> > mOptions;
};

#endif
