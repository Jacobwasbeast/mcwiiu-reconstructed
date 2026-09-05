#ifndef GAME_UI_FJ_BASE_HPP
#define GAME_UI_FJ_BASE_HPP

#include <stdint.h>
#include <string>
#include <vector>
#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;
class fuiRenderNodeEditText;

class FJ_Base : public FJ_FuiNode
{
public:
    explicit FJ_Base(fuiRenderNode* renderNode);
    virtual ~FJ_Base();
    FJ_FuiNode* getObjListParent();
    uint32_t getId();
    uint64_t LostFocus();
    uint32_t getNewWidth();
    void ResizingComplete();
    void ResetResizing();
    void setObjListParent(FJ_FuiNode* parent);
    uint32_t getListId();
    void setListId(int id);
    void setPreScaleWidth(int width);
    void setId(int id);
    uint8_t getLabelTooWide();
    int getOldWidth();
    uint64_t CorrectPixelPosition();
    void InitBase(const std::wstring& name, int id);
    fuiRenderNodeEditText* GetTextField();
    void CheckLabelWidth(const std::wstring& label);
    void SetLabelAlignment(int alignment);
    uint64_t DoManualWordWrapCheck(FJ_FuiNode* container, fuiRenderNodeEditText* textField);
    uint64_t DoMultilineWordWrap(FJ_FuiNode* container, fuiRenderNodeEditText* textField);
    uint64_t DoEllipsisCheck(FJ_FuiNode* container, fuiRenderNodeEditText* textField);
    virtual void SetLabel(const std::wstring& label);
    int GetLabel();
    void SetTextOnly(const std::wstring& text);
    virtual void GetLabelTooWide(fuiRenderNodeEditText* textField);
    virtual void SetNewObjectWidth(int width);
    virtual void SetNewObjectHeight(int height);
    virtual void HandleCharInput(unsigned int character);
    virtual uint32_t getType();
protected:
    std::wstring m_name;
    std::wstring m_textContainerLabel;
    std::wstring m_textContainerText;
    std::wstring m_ellipsisText;
    std::wstring m_wrappedText;
    int m_labelWidth;
    int m_textWidth;
    int m_textHeight;
    int m_id;
    int m_labelAlignment;
    bool m_isLabel;
    bool m_textContainerVisible;
    bool m_textContainerChanged;
    bool m_textChanged;
    bool m_centered;
    bool m_textOnly;
    int m_lineCount;
    std::vector<int> m_lineWidths;
    bool m_manualWrap;
    bool m_labelTooWide;
    bool m_resizing;
    std::wstring m_label;
    int m_newWidth;
    int m_oldWidth;
    float m_objectWidth;
    float m_objectHeight;
    float m_textScale;
    bool m_widthChanged;
    bool m_heightChanged;
    int m_wrapWidth;
    int m_wrapHeight;
    int m_preScaleWidth;
    int m_maxLabelWidth;
    std::vector<FJ_FuiNode*> m_objects;
    FJ_FuiNode* m_objListParent;
    int m_listId;
};

#endif
