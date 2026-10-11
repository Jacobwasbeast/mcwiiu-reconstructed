#include "nn/nex/ZLibPlugin.hpp"

#include "nn/nex/PluginObject.hpp"
#include "nn/nex/ZLibCompression.hpp"

namespace nn
{
namespace nex
{

PluginObject* ZLibPlugin::CreatePluginObject()
{
    ZLibCompression* pCompression = new ZLibCompression;
    pCompression->SetPlugin(this);
    return pCompression;
}

void ZLibPlugin::DeletePluginObject(PluginObject* pObject)
{
    delete pObject;
}

}
}
