#include "game/ui/UIControl_BitmapIcon.hpp"

#include "game/ui/FJ_BitmapIcon.hpp"
#include "game/ui/fuiRenderNode.hpp"

class C4JThread
{
public:
    static bool isMainThread();
};

UIControl_BitmapIcon::UIControl_BitmapIcon()
    : m_textureNamePending(false)
    , m_textureName()
{
    m_textureName = L"";
}

UIControl_BitmapIcon::~UIControl_BitmapIcon()
{
}

void UIControl_BitmapIcon::setTextureName(const std::wstring& textureName, bool immediate)
{
    m_textureName.assign(textureName, 0, std::wstring::npos);
    if (!immediate) {
        m_textureNamePending = true;
    } else {
        m_textureNamePending = false;
        if (m_fuiRenderNode != 0) {
            FJ_BitmapIcon* bitmapIcon = static_cast<FJ_BitmapIcon*>(m_fuiRenderNode->getRoot());
            bitmapIcon->SetTextureName(textureName);
        }
    }
}

void UIControl_BitmapIcon::tick()
{
    if (m_textureNamePending && C4JThread::isMainThread()) {
        setTextureName(m_textureName, true);
    }
}

bool UIControl_BitmapIcon::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_unknown0 = 19;
    return UIControl::setupControl(scene, node, name);
}

void UIControl_BitmapIcon::ReInit()
{
    setTextureName(m_textureName, false);
    UIControl::ReInit();
}
