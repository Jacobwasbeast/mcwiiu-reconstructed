#include "game/ui/FJ_SpaceIndicatorBar.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"

FJ_SpaceIndicatorBar::FJ_SpaceIndicatorBar(fuiRenderNode* renderNode)
    : FJ_Base(renderNode)
{
    GetBarFiller();

    m_barWidth = m_saveBar->getWidth();

    fuiRect rect = { 0.0f, m_saveBar->getWidth(), 0.0f, m_saveBar->getHeight() };
    fuiRenderNodeCodeGenRect* greenNode = new fuiRenderNodeCodeGenRect(
        getStage()->m_pRenderNode, &rect, "fillerMask_Green");
    m_saveGameFiller = new FJ_FuiNode(greenNode);
    m_saveBar->addMask(m_saveGameFiller);

    rect.xMax = m_saveBarFiller->getWidth();
    rect.yMax = m_saveBarFiller->getHeight();
    fuiRenderNodeCodeGenRect* yellowNode = new fuiRenderNodeCodeGenRect(
        getStage()->m_pRenderNode, &rect, "fillerMask_Yellow");
    m_totalFiller = new FJ_FuiNode(yellowNode);
    m_saveBarFiller->addMask(m_totalFiller);

    setSaveGameSize(0.0f);
    setTotalSize(0.0f);
    setSaveGameOffset(0.0f);
}

void FJ_SpaceIndicatorBar::GetBarFiller()
{
    m_saveBar = getChildByName("SaveBarFiller")->asMovieClip();
    m_saveBarFiller = getChildByName("SaveBarFiller2")->asMovieClip();
}

void FJ_SpaceIndicatorBar::setSaveGameSize(float size)
{
    const float maximum = 1.0f;
    const float minimum = 0.0f;
    float width;

    if (size <= maximum)
    {
        if (size < minimum)
            size = minimum;
        m_saveGameFiller->setWidth((m_barWidth / m_saveBar->getScaleX()) * size);
        width = m_saveGameFiller->getWidth();
    }
    else
    {
        m_saveGameFiller->setWidth((m_barWidth / m_saveBar->getScaleX()) * maximum);
        width = m_saveGameFiller->getWidth();
        size = maximum;
    }

    if (width < maximum && size > minimum)
        m_saveGameFiller->setWidth(maximum);
}

void FJ_SpaceIndicatorBar::setTotalSize(float size)
{
    const float maximum = 1.0f;
    const float minimum = 0.0f;

    if (size <= maximum)
    {
        if (size < minimum)
            size = minimum;
        m_totalFiller->setWidth((m_barWidth / m_saveBarFiller->getScaleX()) * size);
    }
    else
    {
        m_totalFiller->setWidth((m_barWidth / m_saveBarFiller->getScaleX()) * maximum);
    }
}

void FJ_SpaceIndicatorBar::setSaveGameOffset(float offset)
{
    m_saveGameFiller->setX(m_barWidth * offset);
}

FJ_SpaceIndicatorBar* FJ_SpaceIndicatorBar::Create(fuiRenderNode* renderNode)
{
    return new FJ_SpaceIndicatorBar(renderNode);
}

unsigned int FJ_SpaceIndicatorBar::getType()
{
    return 0x800a;
}

void FJ_SpaceIndicatorBar::Init(const std::wstring& name)
{
    SetLabelAlignment(3);
    InitBase(name, -1);
}
