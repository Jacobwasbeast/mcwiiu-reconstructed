#include "game/ui/UIControl_Progress.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

#include <string>

class FJ_ProgressBar
{
public:
    void Init(const std::wstring& label);
    void ShowBar(bool show);
    void setProgress(float progress);
};

UIControl_Progress::UIControl_Progress()
    : m_minimum(0)
    , m_maximum(100)
    , m_progress(0)
    , m_progressRatio(0.0f)
    , m_showBar(true)
{
    m_unknown0 = 13;
}

UIControl_Progress::~UIControl_Progress()
{
}

bool UIControl_Progress::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_unknown0 = 13;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_Progress::init(UIString label, int controlId, int minimum, int maximum, int progress)
{
    m_label = label;
    m_unknown2 = (unsigned char)controlId;
    m_minimum = minimum;
    m_maximum = maximum;
    m_progress = progress;
    if (m_fuiRenderNode != NULL) {
        FJ_FuiNode* root = m_fuiRenderNode->getRoot();
        if (root != NULL) {
            FJ_ProgressBar* bar = root->asFJ_ProgressBar();
            if (bar != NULL) {
                const wchar_t* text = (const wchar_t*)(long)m_label.getString();
                bar->Init(std::wstring(text == NULL ? L"" : text));
            }
        }
    }
}

void UIControl_Progress::setProgress(int progress)
{
    m_progress = progress;
    float ratio = 0.0f;
    if (m_maximum != m_minimum) {
        ratio = (float)(progress - m_minimum) / (float)(m_maximum - m_minimum);
    }
    if (ratio == m_progressRatio)
        return;
    m_progressRatio = ratio;
    if (m_fuiRenderNode != NULL) {
        FJ_FuiNode* root = m_fuiRenderNode->getRoot();
        if (root != NULL) {
            FJ_ProgressBar* bar = root->asFJ_ProgressBar();
            if (bar != NULL)
                bar->setProgress(m_progressRatio);
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
    FJ_FuiNode* root = m_fuiRenderNode->getRoot();
    if (root == NULL)
        return;
    FJ_ProgressBar* bar = root->asFJ_ProgressBar();
    if (bar != NULL)
        bar->ShowBar(show);
}

void UIControl_Progress::ReInit()
{
    UIControl::ReInit();
    UIString label = m_label;
    init(label, (int)m_unknown2, m_minimum, m_maximum, m_progress);
}

unsigned int UIControl_Progress::getMax()
{
    return (unsigned int)m_maximum;
}

void UIControl_Progress::setMax(int maximum)
{
    m_maximum = maximum;
}
