#ifndef GAME_UI_FJ_CREDITSMENU_HPP
#define GAME_UI_FJ_CREDITSMENU_HPP

#include <string>
#include <vector>

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_CreditsMenu : public FJ_Document
{
private:
    float m_scrollAmount;
    FJ_FuiNode* m_smallLabels[25];
    FJ_FuiNode* m_mediumLabels[12];
    FJ_FuiNode* m_largeLabels[28];
    FJ_FuiNode* m_logoIggy;
    FJ_FuiNode* m_logoMiles;
    FJ_FuiNode* m_logoDolby;
    FJ_FuiNode* m_logoNintendo;
    std::vector<FJ_FuiNode*> m_scrollingNodes;
    int m_scrollTop;
    int m_scrollBottom;
    FJ_FuiNode* m_creditsContent;
    int m_unused;
    bool m_requestingLabel;

public:
    explicit FJ_CreditsMenu(fuiRenderNode* renderNode);
    virtual ~FJ_CreditsMenu();
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void RequestNextLabel();
    void UpdateScrolling(FJ_Event* event);
    static void _UpdateScrolling(FJ_FuiNode* node, FJ_Event* event);
    virtual void init(FJ_Event* event);
    void AddImage(int image, bool requestNext);
    unsigned int SetNextLabel(std::wstring label, int type, bool requestNext);
    void SetLogoLanguage(int language);
};

#endif
