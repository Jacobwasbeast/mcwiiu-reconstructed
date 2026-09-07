#include "game/ui/UIControl_Progress.hpp"

#include "game/ui/FJ_ProgressBar.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_Progress::UIControl_Progress()
    : UIControl_Base(), m_minimum(0), m_maximum(100), m_progress(0),
      m_progressRatio(0.0f), m_showBar(true)
{
    m_controlType = 13;
}

UIControl_Progress::~UIControl_Progress()
{
}

bool UIControl_Progress::setupControl(UIScene* scene, fuiRenderNode* node,
                                      const std::string& name)
{
    m_controlType = 13;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_Progress::init(UIString label, int controlId, int minimum, int maximum,
                              int progress)
{
    m_label = label;
    m_controlId = controlId;
    m_minimum = minimum;
    m_maximum = maximum;
    m_progress = progress;

    if (m_fuiRenderNode != NULL)
    {
        FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
        if (node != NULL)
        {
            FJ_ProgressBar* progressBar = node->asFJ_ProgressBar();
            if (progressBar != NULL)
                progressBar->Init(label.getString());
        }
    }
}

void UIControl_Progress::setProgress(int progress)
{
    m_progress = progress;
    float ratio = 0.0f;
    if (m_maximum != m_minimum)
        ratio = (float)(progress - m_minimum) / (float)(m_maximum - m_minimum);

    if (ratio == m_progressRatio)
        return;

    m_progressRatio = ratio;
    if (m_fuiRenderNode != NULL)
    {
        FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
        if (node != NULL)
        {
            FJ_ProgressBar* progressBar = node->asFJ_ProgressBar();
            if (progressBar != NULL)
                progressBar->setProgress(m_progressRatio);
        }
    }
}

void UIControl_Progress::showBar(bool show)
{
    if (show == m_showBar)
        return;

    m_showBar = show;
    if (m_fuiRenderNode == NULL)
        return;

    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node == NULL)
        return;

    FJ_ProgressBar* progressBar = node->asFJ_ProgressBar();
    if (progressBar != NULL)
        progressBar->ShowBar(show);
}

void UIControl_Progress::ReInit()
{
    UIControl::ReInit();
    UIString label = m_label;
    init(label, m_controlId, m_minimum, m_maximum, m_progress);
}

unsigned int UIControl_Progress::getMax()
{
    return (unsigned int)m_maximum;
}

void UIControl_Progress::setMax(int maximum)
{
    m_maximum = maximum;
}
