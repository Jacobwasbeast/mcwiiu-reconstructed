#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_URLPROBELIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_URLPROBELIST_HPP

#include <list>

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/URLProbe.hpp"

namespace nn
{
namespace nex
{

class URLProbeList : public RootObject
{
public:
    typedef std::list<URLProbe, MemAllocator<URLProbe> > ProbeList;

    URLProbeList();
    virtual ~URLProbeList();

    URLProbe* FindProbe(const StationURL& rStationURL);
    URLProbe* FindProbe(unsigned int uiConnectionID);

    ProbeList m_probes;
};

}
}

#endif
