#include "game/ui/UIControl_SpaceIndicatorBar.hpp"
#include "game/ui/FJ_SpaceIndicatorBar.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_SpaceIndicatorBar::UIControl_SpaceIndicatorBar()
    : UIControl_Base()
    , m_startOffset(0)
    , m_endOffset(100)
    , m_saveSize(0)
    , m_totalSize(0)
    , m_saveGameOffset(0.0f)
    , m_saves()
{
}

UIControl_SpaceIndicatorBar::~UIControl_SpaceIndicatorBar()
{
}

void UIControl_SpaceIndicatorBar::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = 13;
    UIControl_Base::setupControl(scene, node, name);
}

void UIControl_SpaceIndicatorBar::setSaveSize(long long size)
{
    m_saveSize = size;
    long long numerator = m_saveSize - m_startOffset;
    long long denominator = m_endOffset - m_startOffset;
    ((FJ_SpaceIndicatorBar*)m_fuiRenderNode->getRoot())->setSaveGameSize((float)numerator / (float)denominator);
}

void UIControl_SpaceIndicatorBar::setSaveGameOffset(float offset)
{
    m_saveGameOffset = offset;
    ((FJ_SpaceIndicatorBar*)m_fuiRenderNode->getRoot())->setSaveGameOffset(offset);
}

void UIControl_SpaceIndicatorBar::selectSave(int index)
{
    if (index >= 0 && !m_saves.empty() && (unsigned int)index < m_saves.size()) {
        setSaveSize(m_saves[index].first);
        setSaveGameOffset(m_saves[index].second);
    } else {
        setSaveSize(0);
        setSaveGameOffset(0.0f);
    }
}

void UIControl_SpaceIndicatorBar::setTotalSize(long long size)
{
    long long numerator = m_totalSize - m_startOffset;
    long long denominator = m_endOffset - m_startOffset;
    ((FJ_SpaceIndicatorBar*)m_fuiRenderNode->getRoot())->setTotalSize((float)numerator / (float)denominator);
}

void UIControl_SpaceIndicatorBar::reset()
{
    m_saves.clear();
    m_totalSize = 0;
    setTotalSize(0);
    setSaveSize(0);
    setSaveGameOffset(0.0f);
}

void UIControl_SpaceIndicatorBar::addSave(long long size)
{
    long long numerator = m_totalSize - m_startOffset;
    long long denominator = m_endOffset - m_startOffset;
    m_saves.push_back(std::make_pair(size, (float)numerator / (float)denominator));
    m_totalSize += size;
    setTotalSize(m_totalSize);
}

void UIControl_SpaceIndicatorBar::init(UIString label, int controlId, long long startOffset, long long totalOffset)
{
    m_label = label;
    m_controlType = (unsigned short)controlId;
    m_startOffset = startOffset;
    m_endOffset = totalOffset;
    ((FJ_SpaceIndicatorBar*)m_fuiRenderNode->getRoot())->Init(std::wstring((const wchar_t*)(long)label.getString()));
}

void UIControl_SpaceIndicatorBar::ReInit()
{
    UIControl::ReInit();
    UIString label = m_label;
    init(label, (int)m_controlType, m_startOffset, m_endOffset);
    setSaveSize(m_saveSize);
    setTotalSize(m_totalSize);
    setSaveGameOffset(m_saveGameOffset);
}
