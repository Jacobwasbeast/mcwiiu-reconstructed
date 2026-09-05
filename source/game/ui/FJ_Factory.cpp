#include "game/ui/FJ_Factory.hpp"

#include <boost/unordered_map.hpp>
#include <cstdio>
#include <cstring>
#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"

typedef FJ_FuiNode* (*FJ_FactoryFunction)(fuiRenderNode*);
typedef boost::unordered_map<std::string, FJ_FactoryFunction> FJ_FactoryMap;

static FJ_FactoryMap g_fuiNodeFactories;
static unsigned int g_factoryNodeId;

void FJ_Factory::initialise()
{
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_FuiNode"),
        &FJ_FuiNode::Create));
}

FJ_FuiNode* FJ_Factory::createObject(const char* symbol, fuiRenderNode* renderNode,
    bool createFromSymbol, const char* className)
{
    static bool initialized = false;
    if (!initialized)
    {
        initialise();
        initialized = true;
    }

    const char* factoryName = symbol;
    const char* end = symbol + std::strlen(symbol);
    const char* separator = std::strrchr(symbol, '.');
    if (separator != NULL)
        factoryName = separator + 1;

    FJ_FactoryMap::iterator factory = g_fuiNodeFactories.find(std::string(factoryName, end - factoryName));
    if (factoryName == symbol)
        factory = g_fuiNodeFactories.end();

    if (!createFromSymbol)
    {
        if (factory == g_fuiNodeFactories.end())
            return new FJ_FuiNode(renderNode);
        return factory->second(renderNode);
    }

    fuiRenderNode* nodeRender = (fuiRenderNode*)gFui->createNodeFromSymbol(symbol, renderNode);
    FJ_FuiNode* node;
    if (factory == g_fuiNodeFactories.end())
        node = new FJ_FuiNode(nodeRender);
    else
        node = factory->second(nodeRender);

    nodeRender->m_node = node;
    node->ASConstructor();
    nodeRender->m_flags &= ~0x40;

    if (className == NULL)
        ::snprintf(nodeRender->m_name, 0x40, "Fac_%d", g_factoryNodeId++);
    else
        std::strncpy(nodeRender->m_name, className, 0x40);

    return node;
}
