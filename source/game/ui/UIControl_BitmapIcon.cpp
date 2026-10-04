#include "game/ui/UIControl_BitmapIcon.hpp"

#include "game/ui/FJ_BitmapIcon.hpp"
#include "platform/wiiu/C4JThread.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_BitmapIcon::UIControl_BitmapIcon()
    : m_textureNamePending(false)
    , m_textureName()
{
    m_textureName = L"";
}

void UIControl_BitmapIcon::setTextureName(const std::wstring& textureName, bool immediate)
{
    m_textureName.assign(textureName, 0, std::wstring::npos);
    if (immediate) {
        m_textureNamePending = false;
        if (m_fuiRenderNode != 0) {
            static_cast<FJ_BitmapIcon*>(m_fuiRenderNode->getFuiNode())->SetTextureName(textureName);
        }
    } else {
        m_textureNamePending = true;
    }
}

void UIControl_BitmapIcon::tick()
{
    if (m_textureNamePending && C4JThread::isMainThread()) {
        setTextureName(m_textureName, true);
    }
}

void UIControl_BitmapIcon::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_BitmapIcon;
    UIControl::setupControl(scene, node, name);
}

void UIControl_BitmapIcon::ReInit()
{
    setTextureName(m_textureName, false);
    UIControl::ReInit();
}
