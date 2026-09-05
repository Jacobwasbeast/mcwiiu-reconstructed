#include "game/ui/FJ_SuperFlatMenu.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"

class FJ_Base
{
public:
    FJ_Base* getObjListParent();
    int getId();
};

class FJ_CheckBox : public FJ_Base
{
};

class FJ_HtmlText
{
public:
    void setText(const std::wstring& text);
};

class UIScene
{
public:
    virtual void handleCheckboxElementToggled(int first, int second, bool checked);
};

FJ_SuperFlatMenu::FJ_SuperFlatMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode), m_activeTab(0), m_currentTab(-1), m_helpTextSet(false),
      m_layersFocus(-1), m_biomesFocus(-1), m_propertiesFocus(-1)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);

    getChildByName("Layers.TabLeft")->getChildAt(0)->gotoAndStop(1);
    getChildByName("Biomes.TabMiddle")->getChildAt(0)->gotoAndStop(2);
    getChildByName("Properties.TabRight")->getChildAt(0)->gotoAndStop(3);
    SetActiveTab(m_activeTab);
}

FJ_SuperFlatMenu::~FJ_SuperFlatMenu()
{
}

void FJ_SuperFlatMenu::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
    {
        FJ_Base* listItem = checkBox->getObjListParent();
        m_pCallbackScene->handleCheckboxElementToggled(listItem->getId(), checkBox->getId(), checkBox->getId());
    }
}

void FJ_SuperFlatMenu::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_SuperFlatMenu*>(node)->handleCheckboxToggled(event);
}

void FJ_SuperFlatMenu::SetActiveTab(int tab)
{
    FJ_FuiNode* layers = getChildByName("Layers");
    FJ_FuiNode* layersHelp = layers->getChildByName("LayersHelpHtml");
    FJ_FuiNode* layersList = layers->getChildByName("LayersList");
    FJ_FuiNode* biomes = getChildByName("Biomes");
    FJ_FuiNode* biomesHelp = biomes->getChildByName("BiomesHelpHtml");
    FJ_FuiNode* biomesList = biomes->getChildByName("BiomesList");
    FJ_FuiNode* properties = getChildByName("Properties");
    FJ_FuiNode* propertiesHelp = properties->getChildByName("PropertiesHelpHtml");
    FJ_FuiNode* propertiesList = properties->getChildByName("PropertiesList");

    if (!m_helpTextSet)
    {
        static_cast<FJ_HtmlText*>(layersHelp->asFJ_HtmlText())->setText(L"");
        static_cast<FJ_HtmlText*>(biomesHelp->asFJ_HtmlText())->setText(L"");
        static_cast<FJ_HtmlText*>(propertiesHelp->asFJ_HtmlText())->setText(L"");
        m_helpTextSet = true;
    }

    FJ_FuiNode* active = layers;
    FJ_FuiNode* activeList = layersList;
    int focus = m_layersFocus;
    if (tab == 1)
    {
        active = biomes;
        activeList = biomesList;
        focus = m_biomesFocus;
        layers->setVisible(false);
        biomes->setVisible(true);
        properties->setVisible(false);
        layersHelp->setVisible(layers->isVisible());
        biomesHelp->setVisible(biomes->isVisible());
        propertiesHelp->setVisible(properties->isVisible());
        layersList->setVisible(layers->isVisible());
        biomesList->setVisible(biomes->isVisible());
        propertiesList->setVisible(properties->isVisible());
        gotoAndStop("Biomes");
    }
    else if (tab == 2)
    {
        active = properties;
        activeList = propertiesList;
        focus = m_propertiesFocus;
        layers->setVisible(false);
        biomes->setVisible(false);
        properties->setVisible(true);
        layersHelp->setVisible(layers->isVisible());
        biomesHelp->setVisible(biomes->isVisible());
        propertiesHelp->setVisible(properties->isVisible());
        layersList->setVisible(layers->isVisible());
        biomesList->setVisible(biomes->isVisible());
        propertiesList->setVisible(properties->isVisible());
        gotoAndStop("Properties");
    }
    else if (tab == 0)
    {
        layers->setVisible(true);
        biomes->setVisible(false);
        properties->setVisible(false);
        layersHelp->setVisible(layers->isVisible());
        biomesHelp->setVisible(biomes->isVisible());
        propertiesHelp->setVisible(properties->isVisible());
        layersList->setVisible(layers->isVisible());
        biomesList->setVisible(biomes->isVisible());
        propertiesList->setVisible(properties->isVisible());
        gotoAndStop("Layers");
    }
    else
    {
        m_currentTab = tab;
        m_activeTab = tab;
        return;
    }

    FJ_FuiNode* focusNode = getStage()->getFocus();
    if (focusNode->isFJ_Base())
    {
        FJ_Base* focusedBase = static_cast<FJ_Base*>(focusNode->asFJ_Base());
        if (m_currentTab == 0)
            m_layersFocus = focusedBase->getId();
        else if (m_currentTab == 1)
            m_biomesFocus = focusedBase->getId();
        else if (m_currentTab == 2)
            m_propertiesFocus = focusedBase->getId();
    }

    removeEventListener(eFJEventType_Press, this, FJ_Document::_handlePress);
    activeList->addEventListener(eFJEventType_Press, this, FJ_Document::_handlePress, false);
    SetFocus(focus);
    if (focus >= 0)
        setVisible(true);

    m_currentTab = tab;
    m_activeTab = tab;
}

FJ_SuperFlatMenu* FJ_SuperFlatMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_SuperFlatMenu(renderNode);
}

unsigned int FJ_SuperFlatMenu::getType()
{
    return 39;
}
