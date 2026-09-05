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
    virtual void setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
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
    unsigned short m_controlType;
    bool m_controlFlags;
    void* m_controlData;
    bool m_flag;
    std::string m_name;
    fuiRenderNode* m_fuiRenderNode;
    UIScene* m_scene;
    float m_x;
    float m_y;
    float m_width;
    float m_height;
    float m_opacity;
    bool m_visible;
    bool m_renderState;
};

#endif
