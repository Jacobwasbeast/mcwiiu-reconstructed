#include "game/ui/FJ_Factory.hpp"

#include <boost/unordered_map.hpp>
#include <cstdio>
#include <cstring>
#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_AttackIndicatorCrosshair.hpp"
#include "game/ui/FJ_BeaconEffectButton_Normal.hpp"
#include "game/ui/FJ_BrewingPlate_Small.hpp"
#include "game/ui/FJ_ClassicCraftingMenu.hpp"
#include "game/ui/FJ_ComponentLogo.hpp"
#include "game/ui/FJ_ControlsMenu.hpp"
#include "game/ui/FJ_CreativeMenu.hpp"
#include "game/ui/FJ_DebugFillArea.hpp"
#include "game/ui/FJ_DLCMainMenu.hpp"
#include "game/ui/FJ_DragonHealth_01.hpp"
#include "game/ui/FJ_ExlamationMark.hpp"
#include "game/ui/FJ_ExperienceBar.hpp"
#include "game/ui/FJ_HtmlText_WithPanel_Normal.hpp"
#include "game/ui/FJ_KeyboardButtons_Controls_Normal.hpp"
#include "game/ui/FJ_Label_Black_Centered_HUD.hpp"
#include "game/ui/FJ_Label_Book_Normal.hpp"
#include "game/ui/FJ_Label_Credits_20_Yellow.hpp"
#include "game/ui/FJ_Label_Html.hpp"
#include "game/ui/FJ_Label_HUD_Level_03.hpp"
#include "game/ui/FJ_Label_HUD_VerySmallWhite.hpp"
#include "game/ui/FJ_Label_SkinSelectCenter_Small_Off.hpp"
#include "game/ui/FJ_Label_SkinSelect_Small.hpp"
#include "game/ui/FJ_Label_SmallWhite.hpp"
#include "game/ui/FJ_Label_White_Small_Centered.hpp"
#include "game/ui/FJ_LeaderboardEntryNormal.hpp"
#include "game/ui/FJ_LeaderboardMenu.hpp"
#include "game/ui/FJ_ListButtonCheckbox_Small.hpp"
#include "game/ui/FJ_ListButtonPlayerList_Small.hpp"
#include "game/ui/FJ_ListButton_Medium.hpp"
#include "game/ui/FJ_ListButton_Normal.hpp"
#include "game/ui/FJ_ListButton_Small.hpp"
#include "game/ui/FJ_List_ListButtonDLC.hpp"
#include "game/ui/FJ_List_MultiList.hpp"
#include "game/ui/FJ_LoadOrJoinMenu.hpp"
#include "game/ui/FJ_MenuButton_Normal.hpp"
#include "game/ui/FJ_MenuButton_Small.hpp"
#include "game/ui/FJ_PauseMenu.hpp"
#include "game/ui/FJ_panel9Grid_16Fade.hpp"
#include "game/ui/FJ_Slider_Normal.hpp"
#include "game/ui/FJ_SocialPost.hpp"
#include "game/ui/FJ_TextInput_Vita.hpp"
#include "game/ui/FJ_TexturePackList.hpp"
#include "game/ui/FJ_TutorialPopup.hpp"
#include "game/ui/FJ_UpsellButton_Normal.hpp"
#include "game/ui/FJ_UpsellButton_Small.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"

typedef FJ_FuiNode* (*FJ_FactoryFunction)(fuiRenderNode*);
typedef boost::unordered_map<std::string, FJ_FactoryFunction> FJ_FactoryMap;

static FJ_FactoryMap g_fuiNodeFactories;
static unsigned int g_factoryNodeId;

void FJ_Factory::initialise()
{
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_FuiNode"),
        &FJ_FuiNode::Create));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_MenuButton_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_MenuButton_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_MenuButton_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_MenuButton_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButton_Medium"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButton_Medium::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButton_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButton_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButton_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButton_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_UpsellButton_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_UpsellButton_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_UpsellButton_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_UpsellButton_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonDLC"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_List_ListButtonDLC::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BeaconEffectButton_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_BeaconEffectButton_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_KeyboardButtons_Controls_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_KeyboardButtons_Controls_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonCheckbox_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonCheckbox_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonPlayerList_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonPlayerList_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LeaderboardEntryNormal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LeaderboardEntryNormal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Slider_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Slider_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TexturePackList"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_TexturePackList::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ExlamationMark"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ExlamationMark::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_MultiList"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_List_MultiList::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ComponentLogo"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ComponentLogo::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_CreativeMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_CreativeMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_PauseMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_PauseMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ClassicCraftingMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ClassicCraftingMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ControlsMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ControlsMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LeaderboardMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LeaderboardMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DebugFillArea"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DebugFillArea::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DLCMainMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DLCMainMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LoadOrJoinMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LoadOrJoinMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_SocialPost"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SocialPost::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TutorialPopup"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_TutorialPopup::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DragonHealth_01"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DragonHealth_01::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ExperienceBar"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ExperienceBar::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BrewingPlate_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_BrewingPlate_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_AttackIndicatorCrosshair"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_AttackIndicatorCrosshair::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TextInput_Vita"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_TextInput_Vita::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Black_Centered_HUD"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Black_Centered_HUD::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Book_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Book_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Credits_20_Yellow"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Credits_20_Yellow::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Html"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Html::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_Level_03"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HUD_Level_03::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_VerySmallWhite"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HUD_VerySmallWhite::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelectCenter_Small_Off"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SkinSelectCenter_Small_Off::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelect_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SkinSelect_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SmallWhite"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SmallWhite::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_White_Small_Centered"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_White_Small_Centered::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlText_WithPanel_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_HtmlText_WithPanel_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_panel9Grid_16Fade"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_panel9Grid_16Fade::Create)));
}

FJ_FuiNode* FJ_Factory::createObject(const char* symbol, fuiRenderNode* renderNode,
    bool createFromSymbol, const char* className)
{
    static bool initialized = false;
    if (!initialized)
    {
        initialise();
        initialized = true;
    }

    const char* factoryName = symbol;
    const char* end = symbol + std::strlen(symbol);
    const char* separator = std::strrchr(symbol, '.');
    if (separator != NULL)
        factoryName = separator + 1;

    FJ_FactoryMap::iterator factory = g_fuiNodeFactories.find(std::string(factoryName, end - factoryName));
    if (factoryName == symbol)
        factory = g_fuiNodeFactories.end();

    if (!createFromSymbol)
    {
        if (factory == g_fuiNodeFactories.end())
            return new FJ_FuiNode(renderNode);
        return factory->second(renderNode);
    }

    fuiRenderNode* nodeRender = (fuiRenderNode*)gFui->createNodeFromSymbol(symbol, renderNode);
    FJ_FuiNode* node;
    if (factory == g_fuiNodeFactories.end())
        node = new FJ_FuiNode(nodeRender);
    else
        node = factory->second(nodeRender);

    nodeRender->m_node = node;
    node->ASConstructor();
    nodeRender->m_flags &= ~0x40;

    if (className == NULL)
        ::snprintf(nodeRender->m_name, 0x40, "Fac_%d", g_factoryNodeId++);
    else
        std::strncpy(nodeRender->m_name, className, 0x40);

    return node;
}
