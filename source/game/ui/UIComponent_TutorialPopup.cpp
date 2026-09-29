#include "game/ui/UIComponent_TutorialPopup.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/ui/FJ_TutorialPopup.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/UIString.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "vendor/_TutorialPopupInfo.hpp"

UIComponent_TutorialPopup::UIComponent_TutorialPopup(int sceneType, void* callback, UILayer* layer)
    : UIScene(sceneType, layer), m_interactScene(0), m_previousInteractScene(0),
      m_interactScenePosition(false), m_showTitleTimer(false),
      m_showExitScreenshot(false), m_description(), m_tutorial(0), m_icon(),
      m_showIcon(false), m_containerMenuDisplayed(false), m_hasDescription(false),
      m_splitScreen(false), m_title(), m_iconHolder(EIcons_None),
      m_descriptionLabel(), m_titleLabel(), m_iconHolderControl(), m_exitScreenshotControl()
{
    m_itemRenderer = static_cast<ItemRenderer*>(callback);
    initialiseMovie();
    m_interactScene = 0;
    m_icon.reset();
    m_splitScreen = false;
    m_showIcon = false;
    m_hasDescription = false;
    m_showTitleTimer = false;
    m_showExitScreenshot = false;
    m_iconHolder = EIcons_None;
    m_titleLabel.init(UIString(L""));
}

UIComponent_TutorialPopup::~UIComponent_TutorialPopup()
{
}

void UIComponent_TutorialPopup::SetVisible(bool visible)
{
    if (m_layer != 0)
        m_layer->showComponent(0, static_cast<EUIScene>(getSceneType()), visible);
    if (visible && m_showTitleTimer)
        addTimer(0, 7000);
}

void UIComponent_TutorialPopup::setupIconHolder(EIcons icon)
{
    if (m_movie != 0 && m_movie->getRootNode() != 0)
    {
        FJ_TutorialPopup* popup = static_cast<FJ_TutorialPopup*>(m_movie->getRootNode()->getFuiNode());
        if (popup != 0)
            popup->SetupIconHolder(static_cast<int>(icon));
    }
    m_iconHolder = icon;
}

std::wstring UIComponent_TutorialPopup::_SetIcon(not_null_ptr<ItemInstance> item, int itemId,
                                                 int auxValue, bool showIcon,
                                                 const wchar_t* image)
{
    std::wstring text(image);
    bool usesIconHolderPreset = false;
    bool parseImage = false;

    if (itemId == -1)
    {
        m_showIcon = showIcon;
        parseImage = item->isEmpty();
    }

    if (parseImage)
    {
        m_icon = ItemInstance::EMPTY;
        std::wstring openTag(L"{*ICON*}");
        std::wstring closeTag(L"{*/ICON*}");
        int iconStart = text.find(openTag);
        int idStart = iconStart + openTag.length();
        if (iconStart > 0 && idStart < static_cast<int>(text.length()))
        {
            int iconEnd = text.find(closeTag, idStart);
            if (idStart < iconEnd && iconEnd < static_cast<int>(text.length()))
            {
                std::wstring idText = text.substr(idStart, iconEnd - idStart);
                std::vector<std::wstring> idParts = stringSplit(idText, L':');
                int iconId = _fromString<int>(idParts[0]);
                int iconAuxValue = 0;
                if (idParts.size() >= 2)
                    iconAuxValue = _fromString<int>(idParts[1]);
                ItemInstance* icon;
                if (Item::byId(iconId) != 0)
                    icon = new ItemInstance(Item::byId(iconId), 1);
                else
                    icon = new ItemInstance(Block::byId(iconId), 1);
                icon->setAuxValue(iconAuxValue);
                m_icon = not_null_ptr<ItemInstance>(icon);
                text.replace(iconStart, (iconEnd - iconStart) + closeTag.length(), L"");
            }
        }
        else if (text.find(L"{*CraftingTableIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::crafting_table, 1, 0));
        }
        else if (text.find(L"{*SticksIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::stick, 1, 0));
        }
        else if (text.find(L"{*PlanksIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::planks, 1, 0));
        }
        else if (text.find(L"{*WoodenShovelIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::woodenShovel, 1, 0));
        }
        else if (text.find(L"{*WoodenHatchetIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::woodenAxe, 1, 0));
        }
        else if (text.find(L"{*WoodenPickaxeIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::woodenPickaxe, 1, 0));
        }
        else if (text.find(L"{*FurnaceIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::furnace, 1, 0));
        }
        else if (text.find(L"{*WoodenDoorIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::woodenDoor, 1, 0));
        }
        else if (text.find(L"{*TorchIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::torch, 1, 0));
        }
        else if (text.find(L"{*BoatIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::boat, 1, 0));
        }
        else if (text.find(L"{*FishingRodIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::fishingRod, 1, 0));
        }
        else if (text.find(L"{*FishIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::fish, 1, 0));
        }
        else if (text.find(L"{*MinecartIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::minecart, 1, 0));
        }
        else if (text.find(L"{*RailIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::rail, 1, 0));
        }
        else if (text.find(L"{*PoweredRailIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::golden_rail, 1, 0));
        }
        else if (text.find(L"{*StructuresIcon*}") != std::wstring::npos)
        {
            usesIconHolderPreset = true;
            setupIconHolder(static_cast<EIcons>(9));
        }
        else if (text.find(L"{*ToolsIcon*}") != std::wstring::npos)
        {
            usesIconHolderPreset = true;
            setupIconHolder(static_cast<EIcons>(10));
        }
        else if (text.find(L"{*BannerIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::banner, 1, 0));
        }
        else if (text.find(L"{*SlimeBlockIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::slime, 1, 0));
        }
        else if (text.find(L"{*PistonIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::piston, 1, 0));
        }
        else if (text.find(L"{*BlazePowderIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::blazePowder, 1, 0));
        }
        else if (text.find(L"{*ElytraIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::elytra, 1, 0));
        }
        else
        {
            m_icon = ItemInstance::EMPTY;
        }
    }
    else
    {
        m_showIcon = false;
        m_icon = item;
        if (m_icon->isEmpty())
        {
            Item* fallbackItem = Item::byId(itemId);
            if (fallbackItem == 0)
                m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Block::byId(itemId), 1));
            else
                m_icon = not_null_ptr<ItemInstance>(new ItemInstance(fallbackItem, 1));
            m_icon->setAuxValue(auxValue);
        }
    }

    if (!usesIconHolderPreset && !m_icon->isEmpty())
        setupIconHolder(EIcons_None);

    m_iconHolderControl.setVisible(usesIconHolderPreset || !m_icon->isEmpty());
    return text;
}

std::wstring UIComponent_TutorialPopup::_SetImage(std::wstring& image)
{
    return image;
}

void UIComponent_TutorialPopup::ParseDescription(int, std::wstring& description)
{
    static const wchar_t* tokens[] = {
        L"{*CraftingTableIcon*}", L"{*SticksIcon*}", L"{*PlanksIcon*}",
        L"{*WoodenShovelIcon*}", L"{*WoodenHatchetIcon*}", L"{*WoodenPickaxeIcon*}",
        L"{*FurnaceIcon*}", L"{*WoodenDoorIcon*}", L"{*TorchIcon*}",
        L"{*MinecartIcon*}", L"{*BoatIcon*}", L"{*FishingRodIcon*}",
        L"{*FishIcon*}", L"{*RailIcon*}", L"{*PoweredRailIcon*}",
        L"{*StructuresIcon*}", L"{*ToolsIcon*}", L"{*StoneIcon*}",
        L"{*BannerIcon*}", L"{*SlimeBlockIcon*}", L"{*PistonIcon*}",
        L"{*BlazePowderIcon*}", L"{*ElytraIcon*}"
    };
    std::size_t i;
    for (i = 0; i < sizeof(tokens) / sizeof(tokens[0]); ++i)
    {
        description = replaceAll(description, tokens[i], L"{*ICON*}");
    }

    bool hasExitPicture = description.find(L"{*EXIT_PICTURE*}") != std::wstring::npos;
    description = replaceAll(description, L"{*EXIT_PICTURE*}", L"{*ICON*}");
    m_exitScreenshotControl.setVisible(hasExitPicture);

    m_title = description;
}

void UIComponent_TutorialPopup::_SetDescription(UIScene* scene, const std::wstring& title,
                                                const std::wstring& description, bool showTitle,
                                                bool showExitScreenshot, int displayTime)
{
    m_interactScene = scene;
    m_description = description;
    if (m_interactScene != m_previousInteractScene)
        m_previousInteractScene = 0;

    if (title.empty())
    {
        SetVisible(false);
        addTimer(1, 500);
        killTimer(0);
        return;
    }

    SetVisible(true);
    addTimer(1, 500);
    if (!showTitle)
    {
        killTimer(0);
        m_showTitleTimer = false;
        m_showExitScreenshot = showExitScreenshot;
    }
    else
    {
        addTimer(0, displayTime < 0 ? 7000 : displayTime);
        m_showTitleTimer = showTitle;
        m_showExitScreenshot = showExitScreenshot;
    }

    std::wstring titleText;
    if (!showExitScreenshot)
    {
        titleText = title;
    }
    else
    {
        titleText = L10N::GetString(IDS_TUTORIAL_REMINDER);
        titleText += title;
    }
    stripWhitespaceForHtml(titleText, true);

    wchar_t colorTag[40];
    swprintf(colorTag, 40, L"<font color=\"#%08x\">", gApp.GetHTMLColour(eMinecraftColour_HTMLColor_White));
    std::wstring coloredTitle(colorTag);
    coloredTitle += titleText;

#if defined(RECON_NATIVE)
    FJ_FuiNode* titleNode = m_movie->getRootNode()->getFuiNode()->getChildByName("Title");
    if (titleNode != 0 && titleNode->isFJ_Label())
        titleNode->asFJ_Label()->setUseHtmlText(true);
#endif
    m_descriptionLabel.setLabel(UIString(coloredTitle), true, false);
    m_titleLabel.setLabel(UIString(description), true, false);
    m_titleLabel.setVisible(!description.empty());

    m_hasDescription = GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_SplitscreenNicknames) != 0;

    FJ_TutorialPopup* popup = static_cast<FJ_TutorialPopup*>(m_movie->getRootNode()->getFuiNode());
    popup->AdjustLayout(m_hasDescription && !m_containerMenuDisplayed);
}

void UIComponent_TutorialPopup::SetTutorialDescription(_TutorialPopupInfo* info)
{
    m_interactScene = info->scene;
    std::wstring text = _SetIcon(info->icon, info->iconItemId, info->iconAuxValue, info->showIcon, info->title);
    text = _SetImage(text);
    ParseDescription(m_playerId, text);
    std::wstring formattedTitle = gApp.FormatHTMLString(m_playerId, m_title, -1);
    if (formattedTitle.empty())
        _SetDescription(info->scene, L"", L"", info->showTitle, info->showExitScreenshot, info->displayTime);
    else
        _SetDescription(info->scene, formattedTitle, info->description, info->showTitle,
                        info->showExitScreenshot, info->displayTime);
}

void UIComponent_TutorialPopup::RemoveInteractSceneReference(UIScene* scene)
{
    if (m_interactScene == scene)
        m_interactScene = 0;
}

void UIComponent_TutorialPopup::UpdateTutorialPopup()
{
    bool splitScreen = GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_SplitscreenNicknames) != 0;
    if (m_splitScreen != splitScreen)
    {
        m_splitScreen = splitScreen;
        handleReload();
    }
}

bool UIComponent_TutorialPopup::IsVisible()
{
    return m_layer != 0 && m_layer->isComponentVisible(static_cast<EUIScene>(getSceneType()));
}

unsigned int UIComponent_TutorialPopup::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    if (m_movie == 0 || m_movie->getRootNode() == 0)
        return 1;
    fuiRenderNode* root = m_movie->getRootNode();
    m_titleLabel.setupControl(this, root, "Description");
    m_descriptionLabel.setupControl(this, root, "Title");
    m_iconHolderControl.setupControl(this, root, "IconHolder");
    m_exitScreenshotControl.setupControl(this, root, "ExitScreenShot");
    m_controls.push_back(&m_titleLabel);
    m_controls.push_back(&m_descriptionLabel);
    m_controls.push_back(&m_iconHolderControl);
    m_controls.push_back(&m_exitScreenshotControl);
    return 1;
}

std::wstring UIComponent_TutorialPopup::getMoviePath()
{
    unsigned int viewport = m_layer->getViewport();
    if (viewport == C4JRender::eViewportType_FullScreen || viewport > C4JRender::eViewportType_BottomRightQuarter)
        return L"TutorialPopup";
    return L"TutorialPopupSplit";
}

void UIComponent_TutorialPopup::handleReload()
{
    if (IsVisible())
    {
        std::wstring formattedTitle = gApp.FormatHTMLString(m_playerId, m_title, -1);
        if (formattedTitle.empty())
        {
            _SetDescription(m_interactScene, L"", L"", m_showTitleTimer, m_showExitScreenshot, -1);
        }
        else
        {
            _SetDescription(m_interactScene, formattedTitle, m_description, m_showTitleTimer,
                            m_showExitScreenshot, -1);
        }
    }

    bool adjustSplit = GameSettings::GetGameSettings(m_playerId, eGameSetting_SplitscreenNicknames) != 0 &&
                        !m_containerMenuDisplayed;
    FJ_TutorialPopup* popup = static_cast<FJ_TutorialPopup*>(m_movie->getRootNode()->getFuiNode());
    popup->AdjustLayout(adjustSplit);
    setupIconHolder(m_iconHolder);
}

void UIComponent_TutorialPopup::render(int width, int height, C4JRender::eViewportType viewport)
{
    if (viewport == C4JRender::eViewportType_FullScreen)
    {
        UIScene::render(width, height, viewport);
        return;
    }

    gConsoleUIController.setupRenderPosition(0, static_cast<int>(getSafeZoneHalfHeight()));

    if (gFui != 0)
        gFui->render(m_movie, 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
}

void UIComponent_TutorialPopup::customDraw(const char*, fuiRect* rect)
{
    if (m_icon->getItem() != 0)
    {
        if (m_icon->getCount() == 0)
            m_icon->setCount(1);
        bool highlight = m_icon->getItem()->isFoil(m_icon) || m_showIcon;
        customDrawSlotControlFui(rect, m_playerId, m_icon, 1.0f, highlight, false, false);
    }
}

void UIComponent_TutorialPopup::UpdateInteractScenePosition(bool visible)
{
    if (m_interactScene == 0)
        return;

    bool craftingMenu = m_interactScene->getSceneType() == EUIScene_CraftingMenu2x2
        || m_interactScene->getSceneType() == EUIScene_CraftingMenu;
    int sceneType = m_interactScene->getSceneType();
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[m_playerId];
    if (player->mViewport >= C4JRender::eViewportType_LeftHalf
        && (craftingMenu || sceneType == EUIScene_CreativeMenu || sceneType == EUIScene_TradingMenu))
        return;

    if (m_previousInteractScene == m_interactScene)
    {
        if (m_interactScenePosition == visible)
            return;
        if (!visible)
        {
            m_interactScene->slideRight();
            m_interactScenePosition = false;
            m_previousInteractScene = m_interactScene;
            return;
        }
    }
    else if (!visible)
    {
        return;
    }

    m_interactScene->slideLeft();
    m_interactScenePosition = visible;
    m_previousInteractScene = m_interactScene;
}

void UIComponent_TutorialPopup::handleTimerComplete(int timerId)
{
    if (timerId == 0)
    {
        SetVisible(false);
        killTimer(0);
        addTimer(1, 500);
    }
    else if (timerId == 1)
    {
        UpdateInteractScenePosition(IsVisible());
        killTimer(1);
    }
}

int UIComponent_TutorialPopup::getSceneType() { return EUIScene_TutorialPopup; }
bool UIComponent_TutorialPopup::stealsFocus() { return false; }
bool UIComponent_TutorialPopup::hasFocus(int) { return false; }
bool UIComponent_TutorialPopup::hidesLowerScenes() { return false; }
