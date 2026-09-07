#include "game/ui/UIComponent_TutorialPopup.hpp"

#include <cwchar>

#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/FJ_TutorialPopup.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/UIString.hpp"
#include "vendor/_TutorialPopupInfo.hpp"

void replaceAll(std::wstring& result, const std::wstring& source,
                const std::wstring& search, const std::wstring& replacement);
void stripWhitespaceForHtml(std::wstring& text, bool collapse);

static void replaceTutorialToken(std::wstring& text, const wchar_t* token)
{
    std::wstring empty;
    std::wstring key(token);
    std::size_t position = text.find(key);
    while (position != std::wstring::npos)
    {
        text.replace(position, key.size(), empty);
        position = text.find(key, position);
    }
}

UIComponent_TutorialPopup::UIComponent_TutorialPopup(int sceneType, void* callback, UILayer* layer)
    : UIScene(sceneType, layer), m_interactScene(0), m_previousInteractScene(0),
      m_interactScenePosition(false), m_showTitleTimer(false),
      m_showExitScreenshot(false), m_description(), m_icon(), m_showIcon(false),
      m_hasDescription(false), m_splitScreen(false), m_title(), m_iconHolder(EIcons_None),
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
    m_titleLabel.init(UIString(L"Title"));
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

void UIComponent_TutorialPopup::_SetIcon(not_null_ptr<ItemInstance> item, int itemId,
                                         int descriptionId, bool showIcon,
                                         const wchar_t* image)
{
    m_showIcon = showIcon;
    if (descriptionId == -1 && !item->isEmpty())
    {
        if (item->getCount() == 0)
            item->setCount(1);
        m_icon = item;
        m_description = image == 0 ? L"" : image;
        m_hasDescription = true;
        return;
    }
    m_icon.reset();
    m_description.clear();
    m_hasDescription = false;
}

std::wstring UIComponent_TutorialPopup::_SetImage(std::wstring& image)
{
    return image;
}

void UIComponent_TutorialPopup::ParseDescription(int, std::wstring& description)
{
    static const wchar_t* tokens[] = {
        L"__CraftingTableIcon__", L"__SticksIcon__", L"__PlanksIcon__",
        L"__WoodenShovelIcon__", L"__WoodenHatchetIcon__", L"__WoodenPickaxeIcon__",
        L"__FurnaceIcon__", L"__WoodenDoorIcon__", L"__TorchIcon__",
        L"__MinecartIcon__", L"__StoneIcon__", L"__BannerIcon__",
        L"__SlimeBlockIcon__", L"__PistonIcon__", L"__BlazePowderIcon__",
        L"__ElytraIcon__"
    };
    std::size_t i;
    for (i = 0; i < sizeof(tokens) / sizeof(tokens[0]); ++i)
        replaceTutorialToken(description, tokens[i]);
    replaceTutorialToken(description, L"__EXIT_PICTURE__");
}

void UIComponent_TutorialPopup::_SetDescription(UIScene* scene, const std::wstring& title,
                                                const std::wstring& description, bool showTitle,
                                                bool showExitScreenshot, int descriptionId)
{
    m_interactScene = scene;
    m_title = title;
    m_description = description;
    m_hasDescription = !description.empty();
    if (m_interactScene != m_previousInteractScene)
        m_previousInteractScene = 0;
    if (!m_hasDescription)
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
        addTimer(0, descriptionId < 0 ? 7000 : descriptionId);
        m_showTitleTimer = true;
        m_showExitScreenshot = showExitScreenshot;
    }
    m_descriptionLabel.setLabel(UIString(m_title), true, false);
    m_titleLabel.setLabel(UIString(m_description), true, false);
    m_titleLabel.setVisible(!m_description.empty());
    m_exitScreenshotControl.setVisible(showExitScreenshot);
    m_splitScreen = GameSettings::GetGameSettings(0, eGameSetting_SkinAdjustments) != 0;
    if (m_movie != 0 && m_movie->getRootNode() != 0)
    {
        FJ_TutorialPopup* popup = static_cast<FJ_TutorialPopup*>(m_movie->getRootNode()->getFuiNode());
        if (popup != 0)
            popup->AdjustLayout(m_splitScreen && !m_showExitScreenshot);
    }
}

void UIComponent_TutorialPopup::SetTutorialDescription(_TutorialPopupInfo* info)
{
    if (info == 0)
        return;
    m_interactScene = info->scene;
    std::wstring title(info->title == 0 ? L"" : info->title);
    std::wstring description(info->description == 0 ? L"" : info->description);
    _SetIcon(not_null_ptr<ItemInstance>(info->icon.get()), info->iconItemId,
             info->descriptionId, info->showIcon, L"");
    description = _SetImage(description);
    ParseDescription(info->descriptionId, description);
    _SetDescription(info->scene, title, description, info->showTitle,
                    info->showExitScreenshot, info->descriptionId);
}

void UIComponent_TutorialPopup::RemoveInteractSceneReference(UIScene* scene)
{
    if (m_interactScene == scene)
        m_interactScene = 0;
}

void UIComponent_TutorialPopup::UpdateTutorialPopup()
{
    bool splitScreen = GameSettings::GetGameSettings(0, eGameSetting_SkinAdjustments) != 0;
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
    if (m_layer != 0 && m_layer->getViewport() > 0 && m_layer->getViewport() <= 8)
        return L"TutorialPopupSplit";
    return L"TutorialPopup";
}

void UIComponent_TutorialPopup::handleReload()
{
    if (IsVisible())
        _SetDescription(m_interactScene, m_title, m_description, m_showTitleTimer,
                        m_showExitScreenshot, -1);
    setupIconHolder(m_iconHolder);
}

void UIComponent_TutorialPopup::render(int width, int height, C4JRender::eViewportType viewport)
{
    if (viewport == C4JRender::eViewportType_FullScreen)
    {
        UIScene::render(width, height, viewport);
        return;
    }
    if (gFui != 0)
        gFui->render(m_movie, 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
}

void UIComponent_TutorialPopup::customDraw(const char*, fuiRect* rect)
{
    if (m_icon && !m_icon->isEmpty())
        customDrawSlotControlFui(rect, 0, m_icon, 1.0f, true, false, false);
}

void UIComponent_TutorialPopup::UpdateInteractScenePosition(bool visible)
{
    if (m_interactScene == 0)
        return;
    if (m_previousInteractScene == 0 ||
        m_interactScene->getSceneType() != m_previousInteractScene->getSceneType() ||
        m_interactScenePosition != visible)
    {
        if (!visible)
            slideRight();
        else
            slideLeft();
        m_interactScenePosition = visible;
        m_previousInteractScene = m_interactScene;
    }
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

int UIComponent_TutorialPopup::getSceneType() { return 25; }
bool UIComponent_TutorialPopup::stealsFocus() { return false; }
bool UIComponent_TutorialPopup::hasFocus(int) { return false; }
bool UIComponent_TutorialPopup::hidesLowerScenes() { return false; }
