#include "game/ui/UIControl_TexturePackList.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_TexturePackList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_TexturePackList::UIControl_TexturePackList()
    : UIControl_Base()
    , m_packDetails()
{
}

UIControl_TexturePackList::~UIControl_TexturePackList()
{
}

void UIControl_TexturePackList::init(const std::wstring& label, int id)
{
    m_label = UIString(label);
    m_unknown0 = (unsigned int)id;
    static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getRoot())->Init(label, id);
    int sceneType = m_scene->getSceneType();
    if (sceneType == 4 || (sceneType > 5 && sceneType < 8)) {
        g_ConsoleUIController.TouchBoxAdd(this, m_scene);
    }
}

void UIControl_TexturePackList::setEnabled(bool enabled)
{
    if (m_fuiRenderNode == NULL) {
        return;
    }
    static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getRoot())->EnableSelector(enabled);
}

bool UIControl_TexturePackList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_unknown0 = 18;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_TexturePackList::addPack(int id, const std::wstring& name, bool alreadyAdded)
{
    if (m_fuiRenderNode == NULL) {
        return;
    }
    FJ_TexturePackList* list = static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getRoot());
    list->addPack(id, name);
    if (alreadyAdded) {
        return;
    }
    PackDetails details;
    details.id = id;
    details.name = name;
    m_packDetails.push_back(details);
}

void UIControl_TexturePackList::ReInit()
{
    std::wstring label((const wchar_t*)(long)m_label.getString());
    init(label, (int)m_unknown0);
    for (unsigned int i = 0; i < m_packDetails.size(); ++i) {
        addPack(m_packDetails[i].id, m_packDetails[i].name, true);
    }
    UIControl::ReInit();
}

void UIControl_TexturePackList::selectSlot(int slot)
{
    if (m_fuiRenderNode == NULL) {
        return;
    }
    static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getRoot())->SelectSlot(slot);
}

void UIControl_TexturePackList::clearSlots()
{
    if (m_fuiRenderNode != NULL) {
        static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getRoot())->removeAllItems();
    }
    m_packDetails.clear();
}

void UIControl_TexturePackList::SetTouchFocus(int x, int y, bool focus)
{
    if (m_fuiRenderNode == NULL) {
        return;
    }
    static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getRoot())->SetTouchFocus(x, y, focus);
}

unsigned int UIControl_TexturePackList::CanTouchTrigger(int x, int y)
{
    if (m_fuiRenderNode == NULL) {
        return 0;
    }
    return static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getRoot())->CanTouchTrigger(x, y);
}

int UIControl_TexturePackList::GetRealHeight()
{
    if (m_fuiRenderNode == NULL) {
        return 0;
    }
    return (int)static_cast<FJ_TexturePackList*>(m_fuiRenderNode->getRoot())->GetRealHeight();
}
