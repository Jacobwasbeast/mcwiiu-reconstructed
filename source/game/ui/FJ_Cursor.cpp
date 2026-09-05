#include "game/ui/FJ_Cursor.hpp"

#include <string>
#include <utility>

#include "game/ui/FJ_Document.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

FJ_Cursor::FJ_Cursor(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode)
{
}

FJ_Cursor::~FJ_Cursor()
{
}

void FJ_Cursor::TopAlignPanel()
{
    FJ_FuiNode* parent = getParent();
    int panelBottom = (int)(parent->getY() + getY() - m_textPanel->getHeight());
    FJ_Document* document = parent->getParent()->asFJ_Document();
    int safeTop = document->getISafeZoneTop();
    m_topAlignment = panelBottom < safeTop ? safeTop - panelBottom : 0;
}

void FJ_Cursor::AlignPanel(bool bRight)
{
    std::pair<float, float> boundaries = m_textField->getCharBoundaries(0);
    float textWidth = boundaries.second - boundaries.first;
    const float scale = 0.5f;
    const float cursorScale = 0.35f;

    if (bRight)
    {
        m_textPanel->setX((float)Math::round(m_textPanel->getWidth() * scale * 0.8f));
        m_textPanel->setY((float)m_topAlignment - m_textPanel->getHeight());
        m_textField->setX((float)Math::round(textWidth * cursorScale + m_textPanel->getX()));
        m_textField->setY((float)Math::round(textWidth * scale + m_textPanel->getY()));
        return;
    }

    m_textPanel->setX((float)(-Math::round(m_pointerTextPanel->getWidth() * scale) - m_textPanel->getWidth()));
    m_textPanel->setY((float)m_topAlignment - m_textPanel->getHeight());
    m_textField->setX((float)Math::round(textWidth * 1.675f + m_textPanel->getX()));
    m_textField->setY((float)Math::round((float)m_textPanel->getY() + textWidth * scale));
}

void FJ_Cursor::UpdatePanelAlignment()
{
    if (!m_textField->getText().empty())
    {
        float right = getX() + getWidth() + (float)m_panelAlignment;
        float stageWidth = (float)getStage()->getStageWidth();
        FJ_Document* document = getParent()->asFJ_Document();
        int safeRight = document->getISafeZoneRight();
        if (getParent() != 0)
        {
            TopAlignPanel();
            AlignPanel(right <= (int)(stageWidth - (float)safeRight));
        }
    }
}

void FJ_Cursor::SetLabel(const std::wstring& label)
{
    m_textField->setHTMLText(label);
    m_textField->setAutoSize(TextFieldAutoSizeLeft);
    m_textPanel->setVisible(!label.empty());
    if (m_textPanel->isVisible())
    {
        AlignPanel(true);
        std::pair<float, float> boundaries = m_textField->getCharBoundaries(0);
        m_textPanel->setScaleX(1.0f);
        m_textPanel->setScaleY(1.0f);
        float textHeight = m_textField->getHeight();
        float textWidth = boundaries.second - boundaries.first;
        int width = Math::round(textWidth * 0.8f + m_textField->getWidth());
        m_textPanel->getChildAt(0)->setWidth((float)width);
        m_textPanel->getChildAt(0)->setHeight(textHeight + textWidth);
        UpdatePanelAlignment();
    }
}

void FJ_Cursor::ASConstructor()
{
    FJ_FuiNode* textContainer = getChildByName(std::string("FJ_TextContainer"))->asMovieClip();
    FJ_FuiNode* textFieldContainer = textContainer->getChildAt(0)->asMovieClip();
    m_textField = static_cast<fuiRenderNodeEditText*>(textFieldContainer->getRenderNodeChildByName(std::string("FJ_TextField")));
    addChild(m_textField);
    m_textField->setY(textFieldContainer->getY());
    removeChild(textContainer, true);

    m_textPanel = getChildByName(std::string("PointerTextPanel"))->asMovieClip();
    m_pointerTextPanel = getChildByName(std::string("iggy_pointerIcon"))->asMovieClip();
    m_cursorImage = getChildByName(std::string("CursorImage"))->asMovieClip();
    m_topAlignment = 0;
    m_panelAlignment = 0;
    SetLabel(std::wstring(L""));
}
