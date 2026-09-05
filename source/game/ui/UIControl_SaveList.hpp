#ifndef GAME_UI_UICONTROL_SAVELIST_HPP
#define GAME_UI_UICONTROL_SAVELIST_HPP

#include <string>

class UIScene;
class fuiRenderNode;

class UIControl_SaveList
{
public:
    virtual ~UIControl_SaveList();
    virtual void setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void addItem(const std::wstring& label, const std::wstring& textureName, int icon);
    void addItem(const std::wstring& label, const std::wstring& textureName);
    void setTextureName(int item, const std::wstring& textureName);
    void addItem(const std::wstring& label);
    void addItem(const std::string& label, const std::wstring& textureName, int icon);
    void addItem(const std::string& label, const std::wstring& textureName);
    void addItem(const std::string& label);
    void addItem(const std::wstring& label, int icon);
    void addItem(const std::string& label, int icon);

private:
    int m_controlType;
    void* m_listRenderNode;
    int m_itemIndex;
};

#endif
