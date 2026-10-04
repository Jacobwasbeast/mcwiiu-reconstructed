#include "game/ui/FJ_Factory.hpp"

#include <boost/unordered_map.hpp>
#include <cstdio>
#include <cstring>
#include <string>

#include "game/ui/FJ_AchievementsMenu.hpp"
#include "game/ui/FJ_AnvilMenu.hpp"
#include "game/ui/FJ_AttackIndicatorCrosshair.hpp"
#include "game/ui/FJ_AttackIndicatorHotbar.hpp"
#include "game/ui/FJ_AvatarSelectMenu.hpp"
#include "game/ui/FJ_BeaconEffectButton_Normal.hpp"
#include "game/ui/FJ_BeaconEffectButton_Small.hpp"
#include "game/ui/FJ_BeaconMenu.hpp"
#include "game/ui/FJ_BiomeSelectMenu.hpp"
#include "game/ui/FJ_BitmapIcon.hpp"
#include "game/ui/FJ_BookAndQuillMenu.hpp"
#include "game/ui/FJ_BrewingArrow.hpp"
#include "game/ui/FJ_BrewingArrow_Small.hpp"
#include "game/ui/FJ_BrewingBubbles.hpp"
#include "game/ui/FJ_BrewingBubbles_Small.hpp"
#include "game/ui/FJ_BrewingPlate.hpp"
#include "game/ui/FJ_BrewingPlate_Small.hpp"
#include "game/ui/FJ_BrewingStandMenu.hpp"
#include "game/ui/FJ_CheckBox_Normal.hpp"
#include "game/ui/FJ_CheckBox_Small.hpp"
#include "game/ui/FJ_CheckpointMarker.hpp"
#include "game/ui/FJ_ChestMenu.hpp"
#include "game/ui/FJ_ClassicCraftingMenu.hpp"
#include "game/ui/FJ_ComponentChat.hpp"
#include "game/ui/FJ_ComponentLogo.hpp"
#include "game/ui/FJ_ControlsMenu.hpp"
#include "game/ui/FJ_CraftingMenu.hpp"
#include "game/ui/FJ_CreateWorldMenu.hpp"
#include "game/ui/FJ_CreativeMenu.hpp"
#include "game/ui/FJ_CreditsMenu.hpp"
#include "game/ui/FJ_Cursor_Normal.hpp"
#include "game/ui/FJ_Cursor_Small.hpp"
#include "game/ui/FJ_DLCMainMenu.hpp"
#include "game/ui/FJ_DLCOffersMenu.hpp"
#include "game/ui/FJ_DeathMenu.hpp"
#include "game/ui/FJ_DebugBuildTools.hpp"
#include "game/ui/FJ_DebugCreateSchematic.hpp"
#include "game/ui/FJ_DebugFillArea.hpp"
#include "game/ui/FJ_DebugMarketingGuide.hpp"
#include "game/ui/FJ_DebugMenu.hpp"
#include "game/ui/FJ_DebugOptions.hpp"
#include "game/ui/FJ_DebugPuppeteer.hpp"
#include "game/ui/FJ_DebugSetCamera.hpp"
#include "game/ui/FJ_DispenserMenu.hpp"
#include "game/ui/FJ_DragonHealth_01.hpp"
#include "game/ui/FJ_DragonHealth_02.hpp"
#include "game/ui/FJ_DragonHealth_03.hpp"
#include "game/ui/FJ_DragonHealth_04.hpp"
#include "game/ui/FJ_DragonHealth_05.hpp"
#include "game/ui/FJ_DynamicHtmlText.hpp"
#include "game/ui/FJ_DynamicHtmlText_Normal.hpp"
#include "game/ui/FJ_DynamicHtmlText_Small.hpp"
#include "game/ui/FJ_DynamicHtmlText_WithPanel_Normal.hpp"
#include "game/ui/FJ_DynamicHtmlText_WithPanel_Small.hpp"
#include "game/ui/FJ_EnchantingButton_Normal.hpp"
#include "game/ui/FJ_EnchantingButton_Small.hpp"
#include "game/ui/FJ_EnchantingMenu.hpp"
#include "game/ui/FJ_EndPoem.hpp"
#include "game/ui/FJ_Eula.hpp"
#include "game/ui/FJ_ExlamationMark.hpp"
#include "game/ui/FJ_ExperienceBar.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FullScreenProgress.hpp"
#include "game/ui/FJ_FurnaceArrow.hpp"
#include "game/ui/FJ_FurnaceArrow_Small.hpp"
#include "game/ui/FJ_FurnaceFlame.hpp"
#include "game/ui/FJ_FurnaceFlame_Small.hpp"
#include "game/ui/FJ_FurnaceMenu.hpp"
#include "game/ui/FJ_GameModeScoreboard.hpp"
#include "game/ui/FJ_GameModeSelection.hpp"
#include "game/ui/FJ_Gamertag.hpp"
#include "game/ui/FJ_HelpAndOptionsMenu.hpp"
#include "game/ui/FJ_HopperMenu.hpp"
#include "game/ui/FJ_HorseInventoryMenu.hpp"
#include "game/ui/FJ_HorseJumpBar.hpp"
#include "game/ui/FJ_HowToPlay.hpp"
#include "game/ui/FJ_HowToPlayMenu.hpp"
#include "game/ui/FJ_HtmlTextPanel.hpp"
#include "game/ui/FJ_HtmlTextPanelLarge.hpp"
#include "game/ui/FJ_HtmlText_Normal.hpp"
#include "game/ui/FJ_HtmlText_Small.hpp"
#include "game/ui/FJ_HtmlText_WhiteNormal.hpp"
#include "game/ui/FJ_HtmlText_WhiteSmall.hpp"
#include "game/ui/FJ_HtmlText_WithPanel_Normal.hpp"
#include "game/ui/FJ_HtmlText_WithPanel_Small.hpp"
#include "game/ui/FJ_Hud.hpp"
#include "game/ui/FJ_IconSlot.hpp"
#include "game/ui/FJ_InGameHostOptions.hpp"
#include "game/ui/FJ_InGameInfoMenu.hpp"
#include "game/ui/FJ_InGamePlayerOptions.hpp"
#include "game/ui/FJ_InGameTeleportMenu.hpp"
#include "game/ui/FJ_Intro.hpp"
#include "game/ui/FJ_InventoryMenu.hpp"
#include "game/ui/FJ_JoinGameFilterList.hpp"
#include "game/ui/FJ_JoinMenu.hpp"
#include "game/ui/FJ_KeyboardButtons_Controls_Normal.hpp"
#include "game/ui/FJ_KeyboardButtons_Controls_Small.hpp"
#include "game/ui/FJ_KeyboardButtons_Keys_Normal.hpp"
#include "game/ui/FJ_KeyboardButtons_Keys_Small.hpp"
#include "game/ui/FJ_KillBar.hpp"
#include "game/ui/FJ_KillBarSmall.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_Label_BigBlack.hpp"
#include "game/ui/FJ_Label_BigGrey.hpp"
#include "game/ui/FJ_Label_BigWhite.hpp"
#include "game/ui/FJ_Label_Black.hpp"
#include "game/ui/FJ_Label_Black_Centered.hpp"
#include "game/ui/FJ_Label_Black_Centered_HUD.hpp"
#include "game/ui/FJ_Label_Black_Small_Centered.hpp"
#include "game/ui/FJ_Label_Black_Small_Centered_HUD.hpp"
#include "game/ui/FJ_Label_Black_VerySmall_Centered.hpp"
#include "game/ui/FJ_Label_Blink_Normal.hpp"
#include "game/ui/FJ_Label_Blink_Small.hpp"
#include "game/ui/FJ_Label_Book_Normal.hpp"
#include "game/ui/FJ_Label_Book_Small.hpp"
#include "game/ui/FJ_Label_Book_VerySmall.hpp"
#include "game/ui/FJ_Label_Credits_10.hpp"
#include "game/ui/FJ_Label_Credits_10_Yellow.hpp"
#include "game/ui/FJ_Label_Credits_20.hpp"
#include "game/ui/FJ_Label_Credits_20_Yellow.hpp"
#include "game/ui/FJ_Label_Credits_30.hpp"
#include "game/ui/FJ_Label_DragonHealth.hpp"
#include "game/ui/FJ_Label_DragonHealthBig.hpp"
#include "game/ui/FJ_Label_DragonHealthSmall.hpp"
#include "game/ui/FJ_Label_DragonHealthVerySmall.hpp"
#include "game/ui/FJ_Label_Green.hpp"
#include "game/ui/FJ_Label_Grey.hpp"
#include "game/ui/FJ_Label_HUD_BigWhite.hpp"
#include "game/ui/FJ_Label_HUD_Level_01.hpp"
#include "game/ui/FJ_Label_HUD_Level_02.hpp"
#include "game/ui/FJ_Label_HUD_Level_03.hpp"
#include "game/ui/FJ_Label_HUD_Level_04.hpp"
#include "game/ui/FJ_Label_HUD_Level_05.hpp"
#include "game/ui/FJ_Label_HUD_SmallWhite.hpp"
#include "game/ui/FJ_Label_HUD_Splash.hpp"
#include "game/ui/FJ_Label_HUD_SplashBig.hpp"
#include "game/ui/FJ_Label_HUD_SplashVita.hpp"
#include "game/ui/FJ_Label_HUD_VerySmallWhite.hpp"
#include "game/ui/FJ_Label_HUD_White.hpp"
#include "game/ui/FJ_Label_Html.hpp"
#include "game/ui/FJ_Label_HtmlEndPoem.hpp"
#include "game/ui/FJ_Label_HtmlEndPoemHD.hpp"
#include "game/ui/FJ_Label_HtmlSmall.hpp"
#include "game/ui/FJ_Label_OutlineLevel.hpp"
#include "game/ui/FJ_Label_OutlineLevel_Small.hpp"
#include "game/ui/FJ_Label_OutlineTitle.hpp"
#include "game/ui/FJ_Label_OutlineTitle_Small.hpp"
#include "game/ui/FJ_Label_Red.hpp"
#include "game/ui/FJ_Label_SkinSelect.hpp"
#include "game/ui/FJ_Label_SkinSelectCenter.hpp"
#include "game/ui/FJ_Label_SkinSelectCenter_Off.hpp"
#include "game/ui/FJ_Label_SkinSelectCenter_Small.hpp"
#include "game/ui/FJ_Label_SkinSelectCenter_Small_Off.hpp"
#include "game/ui/FJ_Label_SkinSelect_Off.hpp"
#include "game/ui/FJ_Label_SkinSelect_Small.hpp"
#include "game/ui/FJ_Label_SkinSelect_Small_Off.hpp"
#include "game/ui/FJ_Label_SmallBlack.hpp"
#include "game/ui/FJ_Label_SmallGreen.hpp"
#include "game/ui/FJ_Label_SmallGrey.hpp"
#include "game/ui/FJ_Label_SmallRed.hpp"
#include "game/ui/FJ_Label_SmallWhite.hpp"
#include "game/ui/FJ_Label_VerySmallBlack.hpp"
#include "game/ui/FJ_Label_VerySmallGrey.hpp"
#include "game/ui/FJ_Label_VerySmallWhite.hpp"
#include "game/ui/FJ_Label_White.hpp"
#include "game/ui/FJ_Label_WhiteHtml.hpp"
#include "game/ui/FJ_Label_WhiteHtmlSmall.hpp"
#include "game/ui/FJ_Label_White_Centered.hpp"
#include "game/ui/FJ_Label_White_Small_Centered.hpp"
#include "game/ui/FJ_Label_White_VerySmall_Centered.hpp"
#include "game/ui/FJ_LaunchMoreOptionsMenu.hpp"
#include "game/ui/FJ_LayoutButton.hpp"
#include "game/ui/FJ_LayoutButton_Small.hpp"
#include "game/ui/FJ_LeaderboardEntryNormal.hpp"
#include "game/ui/FJ_LeaderboardEntrySmall.hpp"
#include "game/ui/FJ_LeaderboardHeaderNormal.hpp"
#include "game/ui/FJ_LeaderboardHeaderSmall.hpp"
#include "game/ui/FJ_LeaderboardList.hpp"
#include "game/ui/FJ_LeaderboardMenu.hpp"
#include "game/ui/FJ_ListButtonAchievement_Normal.hpp"
#include "game/ui/FJ_ListButtonAchievement_Small.hpp"
#include "game/ui/FJ_ListButtonCheckbox_Normal.hpp"
#include "game/ui/FJ_ListButtonCheckbox_Small.hpp"
#include "game/ui/FJ_ListButtonController_Normal.hpp"
#include "game/ui/FJ_ListButtonController_Small.hpp"
#include "game/ui/FJ_ListButtonDLC_Normal.hpp"
#include "game/ui/FJ_ListButtonDLC_Small.hpp"
#include "game/ui/FJ_ListButtonIconLeft_Normal.hpp"
#include "game/ui/FJ_ListButtonIconLeft_Small.hpp"
#include "game/ui/FJ_ListButtonIconX2Left_Normal.hpp"
#include "game/ui/FJ_ListButtonIconX2Left_Small.hpp"
#include "game/ui/FJ_ListButtonLayerIcon_Normal.hpp"
#include "game/ui/FJ_ListButtonLayerIcon_Small.hpp"
#include "game/ui/FJ_ListButtonLobby_Normal.hpp"
#include "game/ui/FJ_ListButtonLobby_Small.hpp"
#include "game/ui/FJ_ListButtonPlayerList_Normal.hpp"
#include "game/ui/FJ_ListButtonPlayerList_Small.hpp"
#include "game/ui/FJ_ListButtonScoreBoard_Normal.hpp"
#include "game/ui/FJ_ListButtonScoreBoard_Small.hpp"
#include "game/ui/FJ_ListButton_Medium.hpp"
#include "game/ui/FJ_ListButton_Normal.hpp"
#include "game/ui/FJ_ListButton_Small.hpp"
#include "game/ui/FJ_ListLabel_Black.hpp"
#include "game/ui/FJ_ListLabel_SmallBlack.hpp"
#include "game/ui/FJ_List_ListButton.hpp"
#include "game/ui/FJ_List_ListButtonAchievement.hpp"
#include "game/ui/FJ_List_ListButtonCheckbox.hpp"
#include "game/ui/FJ_List_ListButtonDLC.hpp"
#include "game/ui/FJ_List_ListButtonIconLeft.hpp"
#include "game/ui/FJ_List_ListButtonLayerIcon.hpp"
#include "game/ui/FJ_List_ListButtonLobby.hpp"
#include "game/ui/FJ_List_ListButtonPlayerList.hpp"
#include "game/ui/FJ_List_ListButtonScoreBoard.hpp"
#include "game/ui/FJ_List_MenuButton.hpp"
#include "game/ui/FJ_List_MultiList.hpp"
#include "game/ui/FJ_LoadCreateJoinMenu.hpp"
#include "game/ui/FJ_LoadMenu.hpp"
#include "game/ui/FJ_LoadOrJoinMenu.hpp"
#include "game/ui/FJ_MainMenu.hpp"
#include "game/ui/FJ_MapSelectMenu.hpp"
#include "game/ui/FJ_MenuButton_Normal.hpp"
#include "game/ui/FJ_MenuButton_Small.hpp"
#include "game/ui/FJ_MessageBox.hpp"
#include "game/ui/FJ_MobEffect_Normal.hpp"
#include "game/ui/FJ_MobEffect_Small.hpp"
#include "game/ui/FJ_MultilistMenu.hpp"
#include "game/ui/FJ_Panel9GridFade.hpp"
#include "game/ui/FJ_PanelRecessFade.hpp"
#include "game/ui/FJ_Panorama.hpp"
#include "game/ui/FJ_PauseMenu.hpp"
#include "game/ui/FJ_PressStartToPlay.hpp"
#include "game/ui/FJ_ProgressBar_Loading.hpp"
#include "game/ui/FJ_QuadrantSignin.hpp"
#include "game/ui/FJ_SaveMenu.hpp"
#include "game/ui/FJ_SaveMessage.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/FJ_ScrollArrowH.hpp"
#include "game/ui/FJ_SettingsAudioMenu.hpp"
#include "game/ui/FJ_SettingsControlMenu.hpp"
#include "game/ui/FJ_SettingsGraphicsMenu.hpp"
#include "game/ui/FJ_SettingsMenu.hpp"
#include "game/ui/FJ_SettingsOptionsMenu.hpp"
#include "game/ui/FJ_SettingsUIMenu.hpp"
#include "game/ui/FJ_SignEntryMenu.hpp"
#include "game/ui/FJ_SignInput.hpp"
#include "game/ui/FJ_SkinSelect.hpp"
#include "game/ui/FJ_Slider_Normal.hpp"
#include "game/ui/FJ_Slider_Small.hpp"
#include "game/ui/FJ_SlotList.hpp"
#include "game/ui/FJ_SlotList_Slot.hpp"
#include "game/ui/FJ_SocialPost.hpp"
#include "game/ui/FJ_SpaceIndicatorBar.hpp"
#include "game/ui/FJ_SuperFlatEditLayerMenu.hpp"
#include "game/ui/FJ_SuperFlatMenu.hpp"
#include "game/ui/FJ_SuperFlatPresets.hpp"
#include "game/ui/FJ_TextInput.hpp"
#include "game/ui/FJ_TextInput_Normal.hpp"
#include "game/ui/FJ_TextInput_Small.hpp"
#include "game/ui/FJ_TextInput_Vita.hpp"
#include "game/ui/FJ_TexturePackList.hpp"
#include "game/ui/FJ_TexturePackList_Slot.hpp"
#include "game/ui/FJ_ToolTip_Normal.hpp"
#include "game/ui/FJ_ToolTip_Small.hpp"
#include "game/ui/FJ_ToolTips.hpp"
#include "game/ui/FJ_TouchControl.hpp"
#include "game/ui/FJ_TouchControlFrontend.hpp"
#include "game/ui/FJ_TradingMenu.hpp"
#include "game/ui/FJ_TutorialPopup.hpp"
#include "game/ui/FJ_UpsellButton_Normal.hpp"
#include "game/ui/FJ_UpsellButton_Small.hpp"
#include "game/ui/FJ_panel9Grid_16Fade.hpp"
#include "platform/wiiu/FJ_WiiUDRC.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"

typedef FJ_FuiNode* (*FJ_FactoryFunction)(fuiRenderNode*);
typedef boost::unordered_map<std::string, FJ_FactoryFunction> FJ_FactoryMap;

template <typename Node>
static FJ_FuiNode* createFactoryNode(fuiRenderNode* parent)
{
    return Node::Create(parent);
}

static FJ_FactoryMap g_fuiNodeFactories;
static unsigned int g_factoryNodeId;

void FJ_Factory::initialise()
{
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_FuiNode"),
        &FJ_FuiNode::Create));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_MenuButton_Normal"),
        &createFactoryNode<FJ_MenuButton_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_MenuButton_Small"),
        &createFactoryNode<FJ_MenuButton_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButton_Medium"),
        &createFactoryNode<FJ_ListButton_Medium>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButton_Normal"),
        &createFactoryNode<FJ_ListButton_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButton_Small"),
        &createFactoryNode<FJ_ListButton_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_UpsellButton_Normal"),
        &createFactoryNode<FJ_UpsellButton_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_UpsellButton_Small"),
        &createFactoryNode<FJ_UpsellButton_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonIconLeft_Normal"),
        &createFactoryNode<FJ_ListButtonIconLeft_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonIconLeft_Small"),
        &createFactoryNode<FJ_ListButtonIconLeft_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonIconX2Left_Normal"),
        &createFactoryNode<FJ_ListButtonIconX2Left_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonIconX2Left_Small"),
        &createFactoryNode<FJ_ListButtonIconX2Left_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonIconLeft"),
        &createFactoryNode<FJ_List_ListButtonIconLeft>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButton"),
        &createFactoryNode<FJ_List_ListButton>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonCheckbox"),
        &createFactoryNode<FJ_List_ListButtonCheckbox>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonDLC"),
        &createFactoryNode<FJ_List_ListButtonDLC>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonLayerIcon"),
        &createFactoryNode<FJ_List_ListButtonLayerIcon>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonLobby"),
        &createFactoryNode<FJ_List_ListButtonLobby>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonPlayerList"),
        &createFactoryNode<FJ_List_ListButtonPlayerList>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonScoreBoard"),
        &createFactoryNode<FJ_List_ListButtonScoreBoard>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonAchievement"),
        &createFactoryNode<FJ_List_ListButtonAchievement>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_CheckpointMarker"),
        &createFactoryNode<FJ_CheckpointMarker>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_MenuButton"),
        &createFactoryNode<FJ_List_MenuButton>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BeaconEffectButton_Normal"),
        &createFactoryNode<FJ_BeaconEffectButton_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BeaconEffectButton_Small"),
        &createFactoryNode<FJ_BeaconEffectButton_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_EnchantingButton_Normal"),
        &createFactoryNode<FJ_EnchantingButton_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_EnchantingButton_Small"),
        &createFactoryNode<FJ_EnchantingButton_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_KeyboardButtons_Controls_Normal"),
        &createFactoryNode<FJ_KeyboardButtons_Controls_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_KeyboardButtons_Controls_Small"),
        &createFactoryNode<FJ_KeyboardButtons_Controls_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_KeyboardButtons_Keys_Normal"),
        &createFactoryNode<FJ_KeyboardButtons_Keys_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_KeyboardButtons_Keys_Small"),
        &createFactoryNode<FJ_KeyboardButtons_Keys_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_KeyboardButtons_Controls_Normal"),
        &createFactoryNode<FJ_KeyboardButtons_Controls_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LayoutButton"),
        &createFactoryNode<FJ_LayoutButton>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LayoutButton_Small"),
        &createFactoryNode<FJ_LayoutButton_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonAchievement_Normal"),
        &createFactoryNode<FJ_ListButtonAchievement_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonAchievement_Small"),
        &createFactoryNode<FJ_ListButtonAchievement_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonCheckbox_Normal"),
        &createFactoryNode<FJ_ListButtonCheckbox_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonCheckbox_Small"),
        &createFactoryNode<FJ_ListButtonCheckbox_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonDLC_Normal"),
        &createFactoryNode<FJ_ListButtonDLC_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonDLC_Small"),
        &createFactoryNode<FJ_ListButtonDLC_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonLayerIcon_Normal"),
        &createFactoryNode<FJ_ListButtonLayerIcon_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonLayerIcon_Small"),
        &createFactoryNode<FJ_ListButtonLayerIcon_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonLobby_Normal"),
        &createFactoryNode<FJ_ListButtonLobby_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonLobby_Small"),
        &createFactoryNode<FJ_ListButtonLobby_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonPlayerList_Normal"),
        &createFactoryNode<FJ_ListButtonPlayerList_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonPlayerList_Small"),
        &createFactoryNode<FJ_ListButtonPlayerList_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonScoreBoard_Normal"),
        &createFactoryNode<FJ_ListButtonScoreBoard_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonScoreBoard_Small"),
        &createFactoryNode<FJ_ListButtonScoreBoard_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonController_Normal"),
        &createFactoryNode<FJ_ListButtonController_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonController_Small"),
        &createFactoryNode<FJ_ListButtonController_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LeaderboardList"),
        &createFactoryNode<FJ_LeaderboardList>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LeaderboardEntryNormal"),
        &createFactoryNode<FJ_LeaderboardEntryNormal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LeaderboardEntrySmall"),
        &createFactoryNode<FJ_LeaderboardEntrySmall>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LeaderboardHeaderNormal"),
        &createFactoryNode<FJ_LeaderboardHeaderNormal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LeaderboardHeaderSmall"),
        &createFactoryNode<FJ_LeaderboardHeaderSmall>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LeaderboardList"),
        &createFactoryNode<FJ_LeaderboardList>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_IconSlot"),
        &createFactoryNode<FJ_IconSlot>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ToolTip_Normal"),
        &createFactoryNode<FJ_ToolTip_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ToolTip_Small"),
        &createFactoryNode<FJ_ToolTip_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label"),
        &createFactoryNode<FJ_Label>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Blink_Normal"),
        &createFactoryNode<FJ_Label_Blink_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Blink_Small"),
        &createFactoryNode<FJ_Label_Blink_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ScrollArrow"),
        &createFactoryNode<FJ_ScrollArrow>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ScrollArrowH"),
        &createFactoryNode<FJ_ScrollArrowH>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Slider_Normal"),
        &createFactoryNode<FJ_Slider_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Slider_Small"),
        &createFactoryNode<FJ_Slider_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TexturePackList"),
        &createFactoryNode<FJ_TexturePackList>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TexturePackList_Slot"),
        &createFactoryNode<FJ_TexturePackList_Slot>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_SlotList"),
        &createFactoryNode<FJ_SlotList>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_SlotList_Slot"),
        &createFactoryNode<FJ_SlotList_Slot>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_CheckBox_Normal"),
        &createFactoryNode<FJ_CheckBox_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_CheckBox_Small"),
        &createFactoryNode<FJ_CheckBox_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_SlotList"),
        &createFactoryNode<FJ_SlotList>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_SlotList_Slot"),
        &createFactoryNode<FJ_SlotList_Slot>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ExlamationMark"),
        &createFactoryNode<FJ_ExlamationMark>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Cursor_Normal"),
        &createFactoryNode<FJ_Cursor_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Cursor_Small"),
        &createFactoryNode<FJ_Cursor_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_MobEffect_Small"),
        &createFactoryNode<FJ_MobEffect_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_MobEffect_Normal"),
        &createFactoryNode<FJ_MobEffect_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TouchControl"),
        &createFactoryNode<FJ_TouchControl>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TouchControlFrontend"),
        &createFactoryNode<FJ_TouchControlFrontend>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BitmapIcon"),
        &createFactoryNode<FJ_BitmapIcon>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_MultiList"),
        &createFactoryNode<FJ_List_MultiList>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("CreditsMenu"),
        &createFactoryNode<FJ_CreditsMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("EndPoem"),
        &createFactoryNode<FJ_EndPoem>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("MainMenu"),
        &createFactoryNode<FJ_MainMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("LoadMenu"),
        &createFactoryNode<FJ_LoadMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SaveMessage"),
        &createFactoryNode<FJ_SaveMessage>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("ComponentLogo"),
        &createFactoryNode<FJ_ComponentLogo>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("Panorama"),
        &createFactoryNode<FJ_Panorama>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("ToolTips"),
        &createFactoryNode<FJ_ToolTips>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("Intro"),
        &createFactoryNode<FJ_Intro>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("LoadCreateJoinMenu"),
        &createFactoryNode<FJ_LoadCreateJoinMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("Hud"),
        &createFactoryNode<FJ_Hud>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("HowToPlayMenu"),
        &createFactoryNode<FJ_HowToPlayMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("HowToPlay"),
        &createFactoryNode<FJ_HowToPlay>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("CreativeMenu"),
        &createFactoryNode<FJ_CreativeMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("CraftingMenu"),
        &createFactoryNode<FJ_CraftingMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("InventoryMenu"),
        &createFactoryNode<FJ_InventoryMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FullscreenProgress"),
        &createFactoryNode<FJ_FullScreenProgress>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("Eula"),
        &createFactoryNode<FJ_Eula>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("MessageBox"),
        &createFactoryNode<FJ_MessageBox>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("PauseMenu"),
        &createFactoryNode<FJ_PauseMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("HelpAndOptionsMenu"),
        &createFactoryNode<FJ_HelpAndOptionsMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SettingsMenu"),
        &createFactoryNode<FJ_SettingsMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("AnvilMenu"),
        &createFactoryNode<FJ_AnvilMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("BeaconMenu"),
        &createFactoryNode<FJ_BeaconMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("BrewingStandMenu"),
        &createFactoryNode<FJ_BrewingStandMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("ClassicCraftingMenu"),
        &createFactoryNode<FJ_ClassicCraftingMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("ChestMenu"),
        &createFactoryNode<FJ_ChestMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DispenserMenu"),
        &createFactoryNode<FJ_DispenserMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("EnchantingMenu"),
        &createFactoryNode<FJ_EnchantingMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FurnaceMenu"),
        &createFactoryNode<FJ_FurnaceMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("HopperMenu"),
        &createFactoryNode<FJ_HopperMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("HorseInventoryMenu"),
        &createFactoryNode<FJ_HorseInventoryMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("BookAndQuillMenu"),
        &createFactoryNode<FJ_BookAndQuillMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("ControlsMenu"),
        &createFactoryNode<FJ_ControlsMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("PressStartToPlay"),
        &createFactoryNode<FJ_PressStartToPlay>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SuperFlatMenu"),
        &createFactoryNode<FJ_SuperFlatMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SuperFlatPresets"),
        &createFactoryNode<FJ_SuperFlatPresets>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SuperFlatEditLayerMenu"),
        &createFactoryNode<FJ_SuperFlatEditLayerMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("QuadrantSignin"),
        &createFactoryNode<FJ_QuadrantSignin>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("LeaderboardMenu"),
        &createFactoryNode<FJ_LeaderboardMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("TradingMenu"),
        &createFactoryNode<FJ_TradingMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("GameModeScoreboard"),
        &createFactoryNode<FJ_GameModeScoreboard>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("BiomeSelectMenu"),
        &createFactoryNode<FJ_BiomeSelectMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("ComponentChat"),
        &createFactoryNode<FJ_ComponentChat>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DeathMenu"),
        &createFactoryNode<FJ_DeathMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugBuildTools"),
        &createFactoryNode<FJ_DebugBuildTools>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugCreateSchematic"),
        &createFactoryNode<FJ_DebugCreateSchematic>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugFillArea"),
        &createFactoryNode<FJ_DebugFillArea>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugMarketingGuide"),
        &createFactoryNode<FJ_DebugMarketingGuide>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugMenu"),
        &createFactoryNode<FJ_DebugMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugOptions"),
        &createFactoryNode<FJ_DebugOptions>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugPuppeteer"),
        &createFactoryNode<FJ_DebugPuppeteer>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugSetCamera"),
        &createFactoryNode<FJ_DebugSetCamera>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DLCMainMenu"),
        &createFactoryNode<FJ_DLCMainMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DLCOffersMenu"),
        &createFactoryNode<FJ_DLCOffersMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("GameModeSelection"),
        &createFactoryNode<FJ_GameModeSelection>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("Gamertag"),
        &createFactoryNode<FJ_Gamertag>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("JoinGameFilterList"),
        &createFactoryNode<FJ_JoinGameFilterList>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("JoinMenu"),
        &createFactoryNode<FJ_JoinMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("LoadOrJoinMenu"),
        &createFactoryNode<FJ_LoadOrJoinMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SaveMenu"),
        &createFactoryNode<FJ_SaveMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SettingsAudioMenu"),
        &createFactoryNode<FJ_SettingsAudioMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SettingsControlMenu"),
        &createFactoryNode<FJ_SettingsControlMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SettingsGraphicsMenu"),
        &createFactoryNode<FJ_SettingsGraphicsMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SettingsOptionsMenu"),
        &createFactoryNode<FJ_SettingsOptionsMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SettingsUIMenu"),
        &createFactoryNode<FJ_SettingsUIMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SignEntryMenu"),
        &createFactoryNode<FJ_SignEntryMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SocialPost"),
        &createFactoryNode<FJ_SocialPost>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("AchievementsMenu"),
        &createFactoryNode<FJ_AchievementsMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("AvatarSelectMenu"),
        &createFactoryNode<FJ_AvatarSelectMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("CreateWorldMenu"),
        &createFactoryNode<FJ_CreateWorldMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("LaunchMoreOptionsMenu"),
        &createFactoryNode<FJ_LaunchMoreOptionsMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("TutorialPopup"),
        &createFactoryNode<FJ_TutorialPopup>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("MultilistMenu"),
        &createFactoryNode<FJ_MultilistMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SkinSelect"),
        &createFactoryNode<FJ_SkinSelect>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("InGamePlayerOptions"),
        &createFactoryNode<FJ_InGamePlayerOptions>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("InGameHostOptions"),
        &createFactoryNode<FJ_InGameHostOptions>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("InGameInfoMenu"),
        &createFactoryNode<FJ_InGameInfoMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("InGameTeleportMenu"),
        &createFactoryNode<FJ_InGameTeleportMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("MapSelectMenu"),
        &createFactoryNode<FJ_MapSelectMenu>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DragonHealth_01"),
        &createFactoryNode<FJ_DragonHealth_01>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DragonHealth_02"),
        &createFactoryNode<FJ_DragonHealth_02>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DragonHealth_03"),
        &createFactoryNode<FJ_DragonHealth_03>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DragonHealth_04"),
        &createFactoryNode<FJ_DragonHealth_04>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DragonHealth_05"),
        &createFactoryNode<FJ_DragonHealth_05>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HorseJumpBar"),
        &createFactoryNode<FJ_HorseJumpBar>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ExperienceBar"),
        &createFactoryNode<FJ_ExperienceBar>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BrewingArrow"),
        &createFactoryNode<FJ_BrewingArrow>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BrewingArrow_Small"),
        &createFactoryNode<FJ_BrewingArrow_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BrewingBubbles"),
        &createFactoryNode<FJ_BrewingBubbles>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BrewingBubbles_Small"),
        &createFactoryNode<FJ_BrewingBubbles_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BrewingPlate"),
        &createFactoryNode<FJ_BrewingPlate>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BrewingPlate_Small"),
        &createFactoryNode<FJ_BrewingPlate_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_FurnaceArrow"),
        &createFactoryNode<FJ_FurnaceArrow>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_FurnaceArrow_Small"),
        &createFactoryNode<FJ_FurnaceArrow_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_FurnaceFlame"),
        &createFactoryNode<FJ_FurnaceFlame>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_FurnaceFlame_Small"),
        &createFactoryNode<FJ_FurnaceFlame_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_KillBar"),
        &createFactoryNode<FJ_KillBar>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_KillBarSmall"),
        &createFactoryNode<FJ_KillBarSmall>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ProgressBar_Loading"),
        &createFactoryNode<FJ_ProgressBar_Loading>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_AttackIndicatorCrosshair"),
        &createFactoryNode<FJ_AttackIndicatorCrosshair>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_AttackIndicatorHotbar"),
        &createFactoryNode<FJ_AttackIndicatorHotbar>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_SpaceIndicatorBar"),
        &createFactoryNode<FJ_SpaceIndicatorBar>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TextInput"),
        &createFactoryNode<FJ_TextInput>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TextInput_Small"),
        &createFactoryNode<FJ_TextInput_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TextInput_Normal"),
        &createFactoryNode<FJ_TextInput_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TextInput_Vita"),
        &createFactoryNode<FJ_TextInput_Vita>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlTextPanel"),
        &createFactoryNode<FJ_HtmlTextPanel>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlTextPanelLarge"),
        &createFactoryNode<FJ_HtmlTextPanelLarge>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_BigBlack"),
        &createFactoryNode<FJ_Label_BigBlack>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_BigGrey"),
        &createFactoryNode<FJ_Label_BigGrey>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_BigWhite"),
        &createFactoryNode<FJ_Label_BigWhite>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Black"),
        &createFactoryNode<FJ_Label_Black>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Black_Centered"),
        &createFactoryNode<FJ_Label_Black_Centered>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Black_Centered_HUD"),
        &createFactoryNode<FJ_Label_Black_Centered_HUD>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Black_Small_Centered"),
        &createFactoryNode<FJ_Label_Black_Small_Centered>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Black_Small_Centered_HUD"),
        &createFactoryNode<FJ_Label_Black_Small_Centered_HUD>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Black_VerySmall_Centered"),
        &createFactoryNode<FJ_Label_Black_VerySmall_Centered>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Blink_Normal"),
        &createFactoryNode<FJ_Label_Blink_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Blink_Small"),
        &createFactoryNode<FJ_Label_Blink_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Book_Normal"),
        &createFactoryNode<FJ_Label_Book_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Book_Small"),
        &createFactoryNode<FJ_Label_Book_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Book_VerySmall"),
        &createFactoryNode<FJ_Label_Book_VerySmall>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Credits_10"),
        &createFactoryNode<FJ_Label_Credits_10>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Credits_10_Yellow"),
        &createFactoryNode<FJ_Label_Credits_10_Yellow>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Credits_20"),
        &createFactoryNode<FJ_Label_Credits_20>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Credits_20_Yellow"),
        &createFactoryNode<FJ_Label_Credits_20_Yellow>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Credits_30"),
        &createFactoryNode<FJ_Label_Credits_30>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_DragonHealth"),
        &createFactoryNode<FJ_Label_DragonHealth>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_DragonHealthBig"),
        &createFactoryNode<FJ_Label_DragonHealthBig>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_DragonHealthSmall"),
        &createFactoryNode<FJ_Label_DragonHealthSmall>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_DragonHealthVerySmall"),
        &createFactoryNode<FJ_Label_DragonHealthVerySmall>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Green"),
        &createFactoryNode<FJ_Label_Green>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Grey"),
        &createFactoryNode<FJ_Label_Grey>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Html"),
        &createFactoryNode<FJ_Label_Html>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HtmlEndPoem"),
        &createFactoryNode<FJ_Label_HtmlEndPoem>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HtmlEndPoemHD"),
        &createFactoryNode<FJ_Label_HtmlEndPoemHD>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HtmlSmall"),
        &createFactoryNode<FJ_Label_HtmlSmall>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_Level_01"),
        &createFactoryNode<FJ_Label_HUD_Level_01>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_Level_02"),
        &createFactoryNode<FJ_Label_HUD_Level_02>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_Level_03"),
        &createFactoryNode<FJ_Label_HUD_Level_03>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_Level_04"),
        &createFactoryNode<FJ_Label_HUD_Level_04>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_Level_05"),
        &createFactoryNode<FJ_Label_HUD_Level_05>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_SmallWhite"),
        &createFactoryNode<FJ_Label_HUD_SmallWhite>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_BigWhite"),
        &createFactoryNode<FJ_Label_HUD_BigWhite>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_Splash"),
        &createFactoryNode<FJ_Label_HUD_Splash>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_SplashBig"),
        &createFactoryNode<FJ_Label_HUD_SplashBig>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_SplashVita"),
        &createFactoryNode<FJ_Label_HUD_SplashVita>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_VerySmallWhite"),
        &createFactoryNode<FJ_Label_HUD_VerySmallWhite>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_White"),
        &createFactoryNode<FJ_Label_HUD_White>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_OutlineLevel"),
        &createFactoryNode<FJ_Label_OutlineLevel>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_OutlineLevel_Small"),
        &createFactoryNode<FJ_Label_OutlineLevel_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_OutlineTitle"),
        &createFactoryNode<FJ_Label_OutlineTitle>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_OutlineTitle_Small"),
        &createFactoryNode<FJ_Label_OutlineTitle_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Red"),
        &createFactoryNode<FJ_Label_Red>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelect"),
        &createFactoryNode<FJ_Label_SkinSelect>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelectCenter"),
        &createFactoryNode<FJ_Label_SkinSelectCenter>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelectCenter_Off"),
        &createFactoryNode<FJ_Label_SkinSelectCenter_Off>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelectCenter_Small"),
        &createFactoryNode<FJ_Label_SkinSelectCenter_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelectCenter_Small_Off"),
        &createFactoryNode<FJ_Label_SkinSelectCenter_Small_Off>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelect_Off"),
        &createFactoryNode<FJ_Label_SkinSelect_Off>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelect_Small"),
        &createFactoryNode<FJ_Label_SkinSelect_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelect_Small_Off"),
        &createFactoryNode<FJ_Label_SkinSelect_Small_Off>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SmallBlack"),
        &createFactoryNode<FJ_Label_SmallBlack>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SmallGreen"),
        &createFactoryNode<FJ_Label_SmallGreen>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SmallGrey"),
        &createFactoryNode<FJ_Label_SmallGrey>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SmallRed"),
        &createFactoryNode<FJ_Label_SmallRed>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SmallWhite"),
        &createFactoryNode<FJ_Label_SmallWhite>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_VerySmallBlack"),
        &createFactoryNode<FJ_Label_VerySmallBlack>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_VerySmallGrey"),
        &createFactoryNode<FJ_Label_VerySmallGrey>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_VerySmallWhite"),
        &createFactoryNode<FJ_Label_VerySmallWhite>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_White"),
        &createFactoryNode<FJ_Label_White>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_WhiteHtml"),
        &createFactoryNode<FJ_Label_WhiteHtml>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_WhiteHtmlSmall"),
        &createFactoryNode<FJ_Label_WhiteHtmlSmall>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_White_Centered"),
        &createFactoryNode<FJ_Label_White_Centered>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_White_Small_Centered"),
        &createFactoryNode<FJ_Label_White_Small_Centered>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_White_VerySmall_Centered"),
        &createFactoryNode<FJ_Label_White_VerySmall_Centered>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListLabel_Black"),
        &createFactoryNode<FJ_ListLabel_Black>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListLabel_SmallBlack"),
        &createFactoryNode<FJ_ListLabel_SmallBlack>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlText_Normal"),
        &createFactoryNode<FJ_HtmlText_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlText_Small"),
        &createFactoryNode<FJ_HtmlText_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlText_WhiteNormal"),
        &createFactoryNode<FJ_HtmlText_WhiteNormal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlText_WhiteSmall"),
        &createFactoryNode<FJ_HtmlText_WhiteSmall>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlText_WithPanel_Normal"),
        &createFactoryNode<FJ_HtmlText_WithPanel_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlText_WithPanel_Small"),
        &createFactoryNode<FJ_HtmlText_WithPanel_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DynamicHtmlText"),
        &createFactoryNode<FJ_DynamicHtmlText>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DynamicHtmlText_Normal"),
        &createFactoryNode<FJ_DynamicHtmlText_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DynamicHtmlText_Small"),
        &createFactoryNode<FJ_DynamicHtmlText_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DynamicHtmlText_WithPanel_Normal"),
        &createFactoryNode<FJ_DynamicHtmlText_WithPanel_Normal>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DynamicHtmlText_WithPanel_Small"),
        &createFactoryNode<FJ_DynamicHtmlText_WithPanel_Small>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_SignInput"),
        &createFactoryNode<FJ_SignInput>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_panel9Grid_16Fade"),
        &createFactoryNode<FJ_panel9Grid_16Fade>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Panel9GridFade"),
        &createFactoryNode<FJ_Panel9GridFade>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_PanelRecessFade"),
        &createFactoryNode<FJ_PanelRecessFade>));
    g_fuiNodeFactories.insert(std::make_pair(std::string("WiiUDRC"),
        &createFactoryNode<FJ_WiiUDRC>));
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

    bool hasFactory = separator != NULL && factory != g_fuiNodeFactories.end();

    if (!createFromSymbol)
    {
        if (!hasFactory)
            return new FJ_FuiNode(renderNode);
        return factory->second(renderNode);
    }

    fuiRenderNode* nodeRender = (fuiRenderNode*)gFui->createNodeFromSymbol(symbol, renderNode);
    FJ_FuiNode* node;
    if (!hasFactory)
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
