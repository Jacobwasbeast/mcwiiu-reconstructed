#include "game/ui/FJ_EndPoem.hpp"

#include <cstdio>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_EndPoem::FJ_EndPoem(fuiRenderNode* renderNode)
    : FJ_Document(renderNode), tilePrototype(0), scrollDirection(false), waitingForLabel(false), tileCount(0), tileColumns(0), tileRows(0), currentLabel(0), scrollState(0), textNodes(), labelIndices(), hasFinished(false)
{
}

FJ_EndPoem::~FJ_EndPoem()
{
}

FJ_Document* FJ_EndPoem::Create(fuiRenderNode* renderNode)
{
    return new FJ_EndPoem(renderNode);
}

unsigned int FJ_EndPoem::getType()
{
    return 0x4d;
}

void FJ_EndPoem::RequestNextLabel(int label)
{
    currentLabel = label;
    waitingForLabel = true;
}

void FJ_EndPoem::UpdateTileBackground()
{
    if (textNodes.empty())
    {
        tileCount = 0;
        return;
    }
    tileCount = static_cast<int>(textNodes.size());
    if (tileColumns == 0)
    {
        FJ_FuiNodeStage* stage = getStage();
        const float width = tilePrototype == 0 ? 1.0f : tilePrototype->getWidth();
        const float height = tilePrototype == 0 ? 1.0f : tilePrototype->getHeight();
        tileColumns = width > 0.0f ? static_cast<int>(stage->getStageWidth() / width) + 1 : 1;
        tileRows = height > 0.0f ? static_cast<int>(stage->getStageHeight() / height) + 1 : 1;
    }
    if (scrollDirection)
    {
        std::vector<FJ_FuiNode*>::iterator it;
        for (it = textNodes.begin(); it != textNodes.end(); ++it)
        {
            if (*it != 0)
            {
                (*it)->incY(1.0f);
            }
        }
    }
    scrollDirection = !scrollDirection;
}

void FJ_EndPoem::UpdateScrolling(FJ_Event*)
{
    if (textNodes.empty())
    {
        return;
    }
    const float distance = scrollState < 0 ? -1.0f : 1.0f;
    std::vector<FJ_FuiNode*>::iterator it;
    for (it = textNodes.begin(); it != textNodes.end(); ++it)
    {
        if (*it != 0)
        {
            (*it)->incY(distance);
        }
    }
    UpdateTileBackground();
}

void FJ_EndPoem::_UpdateScrolling(FJ_FuiNode* node, FJ_Event* event)
{
    if (node != 0)
    {
        static_cast<FJ_EndPoem*>(node)->UpdateScrolling(event);
    }
}

void FJ_EndPoem::keyUpHandler(FJ_Event*)
{
    scrollState = 1;
}

void FJ_EndPoem::_keyUpHandler(FJ_FuiNode* node, FJ_Event* event)
{
    if (node != 0)
    {
        static_cast<FJ_EndPoem*>(node)->keyUpHandler(event);
    }
}

void FJ_EndPoem::init(FJ_Event* event)
{
    FJ_Document::init(event);
    tilePrototype = getChildByName(std::string("DirtTile"));
    textNodes.clear();
    labelIndices.clear();
    int i;
    for (i = 0; i < 20; ++i)
    {
        char name[32];
        std::sprintf(name, "EndPoemText_%d", i);
        FJ_FuiNode* node = getChildByName(std::string(name));
        if (node != 0)
        {
            textNodes.push_back(node);
            labelIndices.push_back(i);
            node->setVisible(false);
        }
    }
    tileCount = static_cast<int>(textNodes.size());
    tileColumns = 0;
    tileRows = 0;
    currentLabel = 0;
    scrollState = 0;
    waitingForLabel = false;
    hasFinished = false;
    UpdateTileBackground();
}

unsigned int FJ_EndPoem::SetNextLabel(const std::wstring& label, int index, bool append)
{
    if (index < 0 || index >= static_cast<int>(textNodes.size()))
    {
        return 0;
    }
    FJ_FuiNode* node = textNodes[index];
    if (node == 0)
    {
        return 0;
    }
    if (!append || !waitingForLabel)
    {
        node->asFJ_Base()->SetLabel(label.empty() ? std::wstring(L" ") : label);
    }
    node->setVisible(true);
    currentLabel = index;
    waitingForLabel = false;
    return 1;
}

void FJ_EndPoem::keyDownHandler(FJ_Event* event)
{
    if (event == 0)
    {
        return;
    }
    if (event->m_eType == eFJEventType_KeyDown)
    {
        scrollState = 3;
    }
}
