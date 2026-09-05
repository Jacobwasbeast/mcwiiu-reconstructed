#include "game/ui/FJ_ListButtonController.hpp"

FJ_ListButtonController::FJ_ListButtonController(fuiRenderNode* renderNode)
    : FJ_Button(renderNode)
{
}

FJ_ListButtonController::~FJ_ListButtonController()
{
}

double FJ_ListButtonController::getOriginalHeight()
{
    return static_cast<double>(m_originalHeight);
}

void FJ_ListButtonController::Init(const std::wstring& name, int id)
{
    SetLabelAlignment(4);
    m_isLabel = true;
    FJ_Button::Init(name, id);
    m_originalHeight = getHeight();
}
