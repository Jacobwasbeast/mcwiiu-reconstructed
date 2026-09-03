#include "game/ui/fuiRenderNodeStage.hpp"
#include "game/ui/fuiFile.hpp"

fuiRenderNodeStage::fuiRenderNodeStage(fuiFile* file)
    : fuiRenderNode(0, 0, 0, 0, (eFuiObjectType)0, 0, 0, 0, false, file), m_child(0)
{
}

fuiRenderNodeStage::~fuiRenderNodeStage()
{
    if (m_child != 0)
    {
        m_child->render(0, 0);
        m_child->shutdown();
    }
}

void fuiRenderNodeStage::addChild(fuiRenderNode* child)
{
    m_child = child;
}

fuiRenderNode* fuiRenderNodeStage::getChild()
{
    return m_child;
}

void fuiRenderNodeStage::gather(bool force)
{
    if (m_child != 0)
        m_child->gather(force);
}

void fuiRenderNodeStage::preRender()
{
    if (m_child != 0)
        m_child->preRender();
}

void fuiRenderNodeStage::render(fuiRect* rect, fuiMatrix* matrix)
{
    if (m_child != 0)
        m_child->render(rect, matrix);
}

fuiRenderNode* fuiRenderNodeStage::findNode(const char* name)
{
    if (m_child != 0)
        return m_child->findNode(name);
    return 0;
}

double fuiRenderNodeStage::getStageWidth()
{
    return m_file->getStageWidth();
}

double fuiRenderNodeStage::getStageHeight()
{
    return m_file->getStageHeight();
}
