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
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonIconLeft_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonIconLeft_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonIconLeft_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonIconLeft_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonIconX2Left_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonIconX2Left_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonIconX2Left_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonIconX2Left_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonIconLeft"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_List_ListButtonIconLeft::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButton"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_List_ListButton::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonCheckbox"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_List_ListButtonCheckbox::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonDLC"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_List_ListButtonDLC::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonLayerIcon"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_List_ListButtonLayerIcon::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonLobby"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_List_ListButtonLobby::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonPlayerList"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_List_ListButtonPlayerList::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonScoreBoard"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_List_ListButtonScoreBoard::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_ListButtonAchievement"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_List_ListButtonAchievement::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_CheckpointMarker"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_CheckpointMarker::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_MenuButton"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_List_MenuButton::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BeaconEffectButton_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_BeaconEffectButton_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BeaconEffectButton_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_BeaconEffectButton_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_EnchantingButton_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_EnchantingButton_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_EnchantingButton_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_EnchantingButton_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_KeyboardButtons_Controls_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_KeyboardButtons_Controls_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_KeyboardButtons_Controls_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_KeyboardButtons_Controls_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_KeyboardButtons_Keys_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_KeyboardButtons_Keys_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_KeyboardButtons_Keys_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_KeyboardButtons_Keys_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_KeyboardButtons_Controls_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_KeyboardButtons_Controls_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LayoutButton"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LayoutButton::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LayoutButton_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LayoutButton_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonAchievement_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonAchievement_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonAchievement_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonAchievement_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonCheckbox_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonCheckbox_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonCheckbox_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonCheckbox_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonDLC_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonDLC_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonDLC_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonDLC_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonLayerIcon_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonLayerIcon_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonLayerIcon_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonLayerIcon_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonLobby_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonLobby_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonLobby_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonLobby_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonPlayerList_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonPlayerList_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonPlayerList_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonPlayerList_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonScoreBoard_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonScoreBoard_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonScoreBoard_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonScoreBoard_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonController_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonController_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListButtonController_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListButtonController_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LeaderboardList"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LeaderboardList::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LeaderboardEntryNormal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LeaderboardEntryNormal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LeaderboardEntrySmall"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LeaderboardEntrySmall::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LeaderboardHeaderNormal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LeaderboardHeaderNormal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LeaderboardHeaderSmall"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LeaderboardHeaderSmall::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_LeaderboardList"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LeaderboardList::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_IconSlot"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_IconSlot::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ToolTip_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ToolTip_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ToolTip_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ToolTip_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Blink_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Blink_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Blink_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Blink_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ScrollArrow"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ScrollArrow::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ScrollArrowH"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ScrollArrowH::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Slider_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Slider_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Slider_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Slider_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TexturePackList"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_TexturePackList::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TexturePackList_Slot"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_TexturePackList_Slot::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_SlotList"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SlotList::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_SlotList_Slot"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SlotList_Slot::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_CheckBox_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_CheckBox_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_CheckBox_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_CheckBox_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_SlotList"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SlotList::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_SlotList_Slot"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SlotList_Slot::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ExlamationMark"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ExlamationMark::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Cursor_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Cursor_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Cursor_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Cursor_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_MobEffect_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_MobEffect_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_MobEffect_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_MobEffect_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TouchControl"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_TouchControl::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TouchControlFrontend"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_TouchControlFrontend::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BitmapIcon"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_BitmapIcon::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_List_MultiList"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_List_MultiList::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("CreditsMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_CreditsMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("EndPoem"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_EndPoem::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("MainMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_MainMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("LoadMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LoadMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SaveMessage"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SaveMessage::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("ComponentLogo"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ComponentLogo::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("Panorama"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Panorama::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("ToolTips"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ToolTips::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("Intro"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Intro::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("LoadCreateJoinMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LoadCreateJoinMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("Hud"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Hud::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("HowToPlayMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_HowToPlayMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("HowToPlay"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_HowToPlay::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("CreativeMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_CreativeMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("CraftingMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_CraftingMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("InventoryMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_InventoryMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FullscreenProgress"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_FullScreenProgress::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("Eula"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Eula::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("MessageBox"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_MessageBox::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("PauseMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_PauseMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("HelpAndOptionsMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_HelpAndOptionsMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SettingsMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SettingsMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("AnvilMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_AnvilMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("BeaconMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_BeaconMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("BrewingStandMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_BrewingStandMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("ClassicCraftingMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ClassicCraftingMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("ChestMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ChestMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DispenserMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DispenserMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("EnchantingMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_EnchantingMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FurnaceMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_FurnaceMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("HopperMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_HopperMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("HorseInventoryMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_HorseInventoryMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("BookAndQuillMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_BookAndQuillMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("ControlsMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ControlsMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("PressStartToPlay"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_PressStartToPlay::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SuperFlatMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SuperFlatMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SuperFlatPresets"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SuperFlatPresets::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SuperFlatEditLayerMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SuperFlatEditLayerMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("QuadrantSignin"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_QuadrantSignin::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("LeaderboardMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LeaderboardMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("TradingMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_TradingMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("GameModeScoreboard"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_GameModeScoreboard::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("BiomeSelectMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_BiomeSelectMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("ComponentChat"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ComponentChat::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DeathMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DeathMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugBuildTools"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DebugBuildTools::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugCreateSchematic"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DebugCreateSchematic::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugFillArea"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DebugFillArea::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugMarketingGuide"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DebugMarketingGuide::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DebugMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugOptions"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DebugOptions::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugPuppeteer"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DebugPuppeteer::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DebugSetCamera"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DebugSetCamera::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DLCMainMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DLCMainMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("DLCOffersMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DLCOffersMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("GameModeSelection"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_GameModeSelection::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("Gamertag"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Gamertag::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("JoinGameFilterList"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_JoinGameFilterList::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("JoinMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_JoinMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("LoadOrJoinMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LoadOrJoinMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SaveMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SaveMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SettingsAudioMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SettingsAudioMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SettingsControlMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SettingsControlMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SettingsGraphicsMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SettingsGraphicsMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SettingsOptionsMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SettingsOptionsMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SettingsUIMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SettingsUIMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SignEntryMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SignEntryMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SocialPost"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SocialPost::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("AchievementsMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_AchievementsMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("AvatarSelectMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_AvatarSelectMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("CreateWorldMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_CreateWorldMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("LaunchMoreOptionsMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_LaunchMoreOptionsMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("TutorialPopup"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_TutorialPopup::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("MultilistMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_MultilistMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("SkinSelect"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SkinSelect::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("InGamePlayerOptions"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_InGamePlayerOptions::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("InGameHostOptions"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_InGameHostOptions::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("InGameInfoMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_InGameInfoMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("InGameTeleportMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_InGameTeleportMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("MapSelectMenu"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_MapSelectMenu::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DragonHealth_01"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DragonHealth_01::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DragonHealth_02"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DragonHealth_02::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DragonHealth_03"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DragonHealth_03::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DragonHealth_04"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DragonHealth_04::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DragonHealth_05"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DragonHealth_05::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HorseJumpBar"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_HorseJumpBar::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ExperienceBar"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ExperienceBar::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BrewingArrow"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_BrewingArrow::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BrewingArrow_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_BrewingArrow_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BrewingBubbles"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_BrewingBubbles::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BrewingBubbles_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_BrewingBubbles_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BrewingPlate"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_BrewingPlate::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_BrewingPlate_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_BrewingPlate_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_FurnaceArrow"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_FurnaceArrow::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_FurnaceArrow_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_FurnaceArrow_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_FurnaceFlame"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_FurnaceFlame::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_FurnaceFlame_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_FurnaceFlame_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_KillBar"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_KillBar::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_KillBarSmall"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_KillBarSmall::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ProgressBar_Loading"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ProgressBar_Loading::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_AttackIndicatorCrosshair"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_AttackIndicatorCrosshair::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_AttackIndicatorHotbar"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_AttackIndicatorHotbar::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_SpaceIndicatorBar"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SpaceIndicatorBar::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TextInput"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_TextInput::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TextInput_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_TextInput_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TextInput_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_TextInput_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_TextInput_Vita"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_TextInput_Vita::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlTextPanel"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_HtmlTextPanel::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlTextPanelLarge"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_HtmlTextPanelLarge::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_BigBlack"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_BigBlack::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_BigGrey"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_BigGrey::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_BigWhite"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_BigWhite::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Black"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Black::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Black_Centered"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Black_Centered::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Black_Centered_HUD"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Black_Centered_HUD::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Black_Small_Centered"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Black_Small_Centered::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Black_Small_Centered_HUD"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Black_Small_Centered_HUD::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Black_VerySmall_Centered"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Black_VerySmall_Centered::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Blink_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Blink_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Blink_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Blink_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Book_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Book_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Book_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Book_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Book_VerySmall"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Book_VerySmall::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Credits_10"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Credits_10::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Credits_10_Yellow"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Credits_10_Yellow::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Credits_20"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Credits_20::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Credits_20_Yellow"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Credits_20_Yellow::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Credits_30"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Credits_30::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_DragonHealth"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_DragonHealth::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_DragonHealthBig"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_DragonHealthBig::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_DragonHealthSmall"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_DragonHealthSmall::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_DragonHealthVerySmall"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_DragonHealthVerySmall::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Green"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Green::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Grey"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Grey::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Html"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Html::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HtmlEndPoem"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HtmlEndPoem::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HtmlEndPoemHD"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HtmlEndPoemHD::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HtmlSmall"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HtmlSmall::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_Level_01"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HUD_Level_01::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_Level_02"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HUD_Level_02::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_Level_03"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HUD_Level_03::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_Level_04"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HUD_Level_04::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_Level_05"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HUD_Level_05::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_SmallWhite"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HUD_SmallWhite::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_BigWhite"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HUD_BigWhite::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_Splash"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HUD_Splash::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_SplashBig"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HUD_SplashBig::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_SplashVita"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HUD_SplashVita::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_VerySmallWhite"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HUD_VerySmallWhite::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_HUD_White"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_HUD_White::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_OutlineLevel"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_OutlineLevel::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_OutlineLevel_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_OutlineLevel_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_OutlineTitle"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_OutlineTitle::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_OutlineTitle_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_OutlineTitle_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_Red"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_Red::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelect"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SkinSelect::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelectCenter"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SkinSelectCenter::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelectCenter_Off"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SkinSelectCenter_Off::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelectCenter_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SkinSelectCenter_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelectCenter_Small_Off"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SkinSelectCenter_Small_Off::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelect_Off"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SkinSelect_Off::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelect_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SkinSelect_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SkinSelect_Small_Off"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SkinSelect_Small_Off::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SmallBlack"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SmallBlack::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SmallGreen"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SmallGreen::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SmallGrey"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SmallGrey::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SmallRed"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SmallRed::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_SmallWhite"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_SmallWhite::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_VerySmallBlack"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_VerySmallBlack::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_VerySmallGrey"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_VerySmallGrey::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_VerySmallWhite"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_VerySmallWhite::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_White"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_White::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_WhiteHtml"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_WhiteHtml::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_WhiteHtmlSmall"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_WhiteHtmlSmall::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_White_Centered"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_White_Centered::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_White_Small_Centered"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_White_Small_Centered::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Label_White_VerySmall_Centered"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Label_White_VerySmall_Centered::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListLabel_Black"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListLabel_Black::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_ListLabel_SmallBlack"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_ListLabel_SmallBlack::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlText_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_HtmlText_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlText_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_HtmlText_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlText_WhiteNormal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_HtmlText_WhiteNormal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlText_WhiteSmall"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_HtmlText_WhiteSmall::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlText_WithPanel_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_HtmlText_WithPanel_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_HtmlText_WithPanel_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_HtmlText_WithPanel_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DynamicHtmlText"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DynamicHtmlText::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DynamicHtmlText_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DynamicHtmlText_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DynamicHtmlText_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DynamicHtmlText_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DynamicHtmlText_WithPanel_Normal"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DynamicHtmlText_WithPanel_Normal::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_DynamicHtmlText_WithPanel_Small"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_DynamicHtmlText_WithPanel_Small::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_SignInput"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_SignInput::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_panel9Grid_16Fade"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_panel9Grid_16Fade::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_Panel9GridFade"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_Panel9GridFade::Create)));
    g_fuiNodeFactories.insert(std::make_pair(std::string("FJ_PanelRecessFade"),
        reinterpret_cast<FJ_FactoryFunction>(&FJ_PanelRecessFade::Create)));
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

    // The original (0x02BD40E0) DOES discard a dotless lookup: after find(),
    // at L_02bd4200 it compares the post-backward-scan pointer against the
    // symbol start and clears the result when they are equal. That rule is
    // deliberately not reproduced here yet, because this permissive form
    // cannot break a lookup the original would have satisfied, whereas
    // restoring it without knowing whether the real .fui symbol is
    // "fourj.Intro" or a bare "Intro" risks re-breaking the boot. Settle it
    // against the FUI symbol table in a packet that owns it.
    //
    // The keys themselves are not derived from class names: the original
    // copies 278 prebuilt std::string globals, and the 81 screen roots are
    // keyed on the bare FUI symbol ("Intro" -> FJ_Intro::Create), while the
    // rest keep the FJ_ prefix. "FullscreenProgress" even maps to
    // FJ_FullScreenProgress, with a different capital S.
    FJ_FactoryMap::iterator factory = g_fuiNodeFactories.find(std::string(factoryName, end - factoryName));

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
