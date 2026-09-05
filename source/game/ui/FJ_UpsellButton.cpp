#include "game/ui/FJ_UpsellButton.hpp"

#include "game/ui/FJ_Base.hpp"

FJ_UpsellButton::FJ_UpsellButton(fuiRenderNode* renderNode)
    : FJ_Button(renderNode)
{
    m_wrapWidth = 0;
    m_textContainerVisible = true;
    m_textChanged = true;
    m_resizing = true;
}

FJ_UpsellButton::~FJ_UpsellButton()
{
}

void FJ_UpsellButton::Init(const std::wstring& name, int id)
{
    FJ_Base::SetLabelAlignment(2);
    FJ_Button::Init(name, id);
}
