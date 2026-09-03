#ifndef GAME_UI_UICONTROL_HPP
#define GAME_UI_UICONTROL_HPP

#include <string>

class UIScene;
class fuiRenderNode;

class UIControl
{
public:
    UIControl();
    virtual ~UIControl();
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void UpdateControl();
    virtual void tick();
    virtual void ReInit();
    virtual void setFocus(bool focus);
    virtual bool hasFocus();

    void setVisible(bool visible);
    double getWidth();
    double getHeight();
    double getXPos();
    double getYPos();
    bool getVisible();
    void removeUINodeRef();
    fuiRenderNode* getFuiRenderNode();
    void setWidth(float width);
    void setOpacity(float opacity);
    UIControl& operator=(const UIControl& other);

protected:
    unsigned int m_unknown0;
    unsigned char m_unknown2;
    unsigned char m_unknown3;
    void* m_unknown4;
    void* m_unknown8;
    std::string m_name;
    fuiRenderNode* m_fuiRenderNode;
    UIScene* m_scene;
    float m_x;
    float m_y;
    float m_width;
    float m_height;
    float m_opacity;
    bool m_visible;
    bool m_unknown49;
};

#endif
