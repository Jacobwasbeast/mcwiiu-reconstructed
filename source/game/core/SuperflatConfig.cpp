#include "game/core/SuperflatConfig.hpp"

#include "game/data/ByteArrayTag.hpp"

SuperflatConfig::SuperflatConfig() : mVersion(1), mDisplayname(), mOptions(), layers() {}
SuperflatConfig::~SuperflatConfig() {}

bool SuperflatConfig::hasParam(unsigned char parameter) const
{
    return mOptions.find(parameter) != mOptions.end();
}

SuperflatConfig* SuperflatConfig::setDisplayname(const std::wstring& name)
{
    mDisplayname = name;
    return this;
}

bool SuperflatConfig::hasOption(unsigned char parameter, unsigned char option) const
{
    std::map<unsigned char, std::map<unsigned char, unsigned char> >::const_iterator param = mOptions.find(parameter);
    return param != mOptions.end() && param->second.find(option) != param->second.end();
}

void SuperflatConfig::addParam(unsigned char parameter)
{
    mOptions[parameter];
}

void SuperflatConfig::setOption(unsigned char parameter, unsigned char option, unsigned char value)
{
    mOptions[parameter][option] = value;
}

void SuperflatConfig::removeParam(unsigned char parameter)
{
    mOptions.erase(parameter);
}

void SuperflatConfig::updateLayers()
{
    unsigned int start = 0;
    for (std::vector<FlatLayerInfo>::iterator it = layers.begin(); it != layers.end(); ++it)
    {
        it->setStart(start);
        start += it->getHeight();
    }
}

void SuperflatConfig::clear()
{
    mOptions.clear();
    layers.clear();
    mDisplayname.clear();
    mVersion = 1;
}

std::wstring SuperflatConfig::getDisplayname() const { return mDisplayname; }
void SuperflatConfig::setVersion(unsigned char version) { mVersion = version; }
unsigned char SuperflatConfig::getOption(unsigned char parameter, unsigned char option) const
{
    std::map<unsigned char, std::map<unsigned char, unsigned char> >::const_iterator param = mOptions.find(parameter);
    if (param == mOptions.end()) return 0;
    std::map<unsigned char, unsigned char>::const_iterator value = param->second.find(option);
    return value == param->second.end() ? 0 : value->second;
}
unsigned int SuperflatConfig::maybeOption(unsigned char parameter, unsigned char option, unsigned char fallback) const
{
    return hasOption(parameter, option) ? getOption(parameter, option) : fallback;
}

arrayWithLength<unsigned char> SuperflatConfig::toBlob() const
{
    std::vector<unsigned char> bytes;
    bytes.push_back(mVersion);
    bytes.push_back((unsigned char)mOptions.size());
    for (std::map<unsigned char, std::map<unsigned char, unsigned char> >::const_iterator p = mOptions.begin(); p != mOptions.end(); ++p)
    {
        bytes.push_back(p->first);
        bytes.push_back((unsigned char)p->second.size());
        for (std::map<unsigned char, unsigned char>::const_iterator o = p->second.begin(); o != p->second.end(); ++o)
        {
            bytes.push_back(o->first);
            bytes.push_back(o->second);
        }
    }
    return arrayWithLength<unsigned char>(bytes);
}

void SuperflatConfig::fromBlob(arrayWithLength<unsigned char> data)
{
    clear();
    if (data.data == NULL || data.length < 2) return;
    unsigned int index = 0;
    mVersion = data.data[index++];
    unsigned int params = data.data[index++];
    for (unsigned int p = 0; p < params && index < data.length; ++p)
    {
        unsigned char parameter = data.data[index++];
        if (index >= data.length) break;
        unsigned int options = data.data[index++];
        for (unsigned int o = 0; o < options && index + 1 < data.length; ++o)
        {
            setOption(parameter, data.data[index], data.data[index + 1]);
            index += 2;
        }
    }
}

ByteArrayTag* SuperflatConfig::toTag() const
{
    return new ByteArrayTag(toBlob(), true);
}

void SuperflatConfig::fromTag(ByteArrayTag* tag)
{
    if (tag != NULL) fromBlob(tag->getAsByteArray());
}

SuperflatConfig* SuperflatConfig::makeDefaults() { clear(); return this; }
SuperflatConfig* SuperflatConfig::makePreset(DefaultPresets::Preset) { return makeDefaults(); }
unsigned char SuperflatConfig::getVersion() const { return mVersion; }
SuperflatConfig* SuperflatConfig::makeDebug() { clear(); return this; }
