#include "game/ui/UIGroup.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/ui/UIComponent_Tooltips.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/minigame/Tutorial.hpp"


UIGroup::UIGroup(EUIGroup group, int player)
    : m_component(0), m_componentSecondary(0), m_componentTertiary(0),
      m_componentQuaternary(0), m_drcComponent(0), m_viewportType(0),
      m_group((int)group), m_player(player), m_menuDisplayed(0), m_pauseMenuDisplayed(false),
      m_containerMenuDisplayed(false), m_ignoreAutosaveMenuDisplayed(false),
      m_ignorePlayerJoinMenuDisplayed(false), m_stackState5(0),
      m_focusTimer(0), m_displayList(0)
{
    int i;
    m_unknown45[0] = 0;
    m_unknown45[1] = 0;
    m_unknown45[2] = 0;
    for (i = 0; i < 8; ++i)
    {
        m_layers[i] = new UILayer(this);
        m_layers[i]->m_layerId = i;
    }
    if (m_group == 0 || m_group == 1) m_component = m_layers[1]->addComponent(0, (EUIScene)0x46, 0);
    else m_component = m_layers[1]->addComponent(m_player, (EUIScene)0x46, 0);
    if (m_group == 0) {
        m_componentQuaternary = m_layers[4]->addComponent(m_player, (EUIScene)0x41, 0);
        m_displayList = MemoryTracker::genLists(1);
    } else if (m_group == 1) {
        m_componentTertiary = m_layers[1]->addComponent(0, (EUIScene)0x47, 0);
        m_displayList = MemoryTracker::genLists(1);
    } else {
        m_componentSecondary = m_layers[5]->addComponent(m_player, (EUIScene)0x19, 0);
        m_componentQuaternary = m_layers[7]->addComponent(m_player, (EUIScene)0x31, 0);
        m_displayList = MemoryTracker::genLists(1);
    }
}

void UIGroup::getRenderDimensions(int& width, int& height)
{
    gConsoleUIController.getRenderDimensions(
        static_cast<C4JRender::eViewportType>(m_viewportType), width, height);
}

unsigned int UIGroup::GetLayerIndex(UILayer* layer)
{
    for (int i = 0; i < 8; ++i)
        if (m_layers[i] == layer)
            return static_cast<unsigned int>(i);
    return 0;
}

unsigned int UIGroup::RequestFocus(UILayer* layer)
{
    unsigned int index = GetLayerIndex(layer);
    if (index == 0)
        return 1;
    for (unsigned int i = 0; i < index; ++i)
        if (m_layers[i]->HasFocus(0))
            return 0;
    return 1;
}

void UIGroup::UpdateFocusState()
{
    m_focusTimer = 10;
}

bool UIGroup::IsFullscreenGroup()
{
    return m_group == 1;
}

unsigned int UIGroup::GetViewportType()
{
    return static_cast<unsigned int>(m_viewportType);
}

void UIGroup::DestroyAll()
{
    for (int i = 0; i < 8; ++i)
        m_layers[i]->DestroyAll();
}

void UIGroup::ReloadAll()
{
    int lastLayer;
    int i;

    if (m_layers[0]->hidesLowerScenes() || m_layers[1]->hidesLowerScenes() ||
        m_layers[2]->hidesLowerScenes() || m_layers[3]->hidesLowerScenes())
    {
        lastLayer = 4;
    }
    else
    {
        lastLayer = 8;
        for (i = 4; i < 8; ++i)
        {
            if (m_layers[i]->hidesLowerScenes())
            {
                lastLayer = i;
                break;
            }
        }
    }

    for (i = lastLayer; i >= 0; --i)
    {
        if (i < 8)
            m_layers[i]->ReloadAll(i != 4);
    }
}

void UIGroup::_UpdateFocusState()
{
    for (int i = 0; i < 8; ++i)
        if (m_layers[i]->updateFocusState(true))
            return;
}

void UIGroup::tick()
{
    if (m_player < 0 || gProfile->IsSignedIn(m_player))
    {
        for (int i = 0; i < 8; ++i)
            m_layers[i]->tick();
        if (m_focusTimer > 0 && --m_focusTimer == 0)
            _UpdateFocusState();
    }
}
void UIGroup::render()
{
    int width = 0;
    int height = 0;
    int layer;
    int firstLayer = 0;

    if (m_player >= 0 && !gProfile->IsSignedIn(m_player))
    {
        return;
    }

    if (m_group != 0)
    {
        getRenderDimensions(width, height);
    }
    else
    {
        width = 0x356;
        height = 0x1e0;
    }

    for (layer = 0; layer < 8; ++layer)
    {
        if (m_layers[layer]->hidesLowerScenes())
        {
            firstLayer = layer;
            break;
        }
        firstLayer = layer + 1;
    }

    for (layer = firstLayer; layer >= 0; --layer)
    {
        if (layer < 8)
        {
            m_layers[layer]->render(width, height, (C4JRender::eViewportType)m_viewportType);
        }
    }
}
int UIGroup::hidesLowerScenes()
{
    if (m_player >= 0 && !gProfile->IsSignedIn(m_player))
        return 0;
    for (int i = 7; i >= 0; --i)
        if (m_layers[i]->hidesLowerScenes())
            return 1;
    return 0;
}
void UIGroup::updateStackStates()
{
    bool state[5] = { false, false, false, false, false };
    int layer;

    for (layer = 0; layer < 8; ++layer)
    {
        state[0] = state[0] || m_layers[layer]->GetMenuDisplayed();
        state[1] = state[1] || m_layers[layer]->IsFullscreenGroup();
        state[2] = state[2] || m_layers[layer]->hidesLowerScenes();
        state[3] = state[3] || m_layers[layer]->isComponentVisible((EUIScene)0x46);
        state[4] = state[4] || m_layers[layer]->HasFocus(0);
    }

    m_menuDisplayed = state[0];
    m_pauseMenuDisplayed = state[1];
    m_containerMenuDisplayed = state[2];
    m_ignoreAutosaveMenuDisplayed = state[3];
    m_ignorePlayerJoinMenuDisplayed = state[4];
}
bool UIGroup::NavigateToScene(int player, EUIScene scene, void* data, EUILayer layer)
{
    bool result = m_layers[static_cast<int>(layer)]->NavigateToScene(player, scene, data);
    updateStackStates();
    return result;
}

int UIGroup::NavigateBack(int player, EUIScene scene, EUILayer layer)
{
    int result = 0;
    for (int i = 0; i < 8; ++i)
    {
        if (static_cast<int>(layer) < 8 && i != static_cast<int>(layer))
            continue;
        result = m_layers[i]->NavigateBack(player, scene);
        if (result)
            break;
    }
    updateStackStates();
    return result;
}

void UIGroup::closeAllScenes()
{
    Minecraft* mc = Minecraft::GetInstance();
    if (m_player >= 0 && mc != 0 && mc->mGameModes[m_player] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[m_player]->getTutorial();
        if (tutorial != 0)
            tutorial->showTutorialPopup(true);
    }

    for (int i = 0; i < 8; ++i)
        if (i != 2)
            m_layers[i]->closeAllScenes();
    updateStackStates();
}
UIScene* UIGroup::GetTopScene(EUILayer layer)
{
    return m_layers[static_cast<int>(layer)]->GetTopScene();
}

bool UIGroup::IsSceneInStack(EUIScene scene)
{
    for (int i = 0; i < 8; ++i)
        if (m_layers[i]->IsSceneInStack(scene))
            return true;
    return false;
}

unsigned char UIGroup::GetMenuDisplayed()
{
    return m_menuDisplayed;
}

void UIGroup::handleInput(int player, int action, bool repeat, bool released, bool pressed, bool& handled)
{
    if (m_player >= 0 && !gProfile->IsSignedIn(m_player))
        return;
    for (int i = 0; i < 8; ++i)
    {
        m_layers[i]->handleInput(player, action, repeat, released, pressed, handled);
        if (handled)
            return;
    }
}

UIScene* UIGroup::getCurrentScene(int player)
{
    for (int i = 0; i < 8; ++i)
    {
        UIScene* scene = m_layers[i]->getCurrentScene(player);
        if (scene != 0)
            return scene;
    }
    return 0;
}
void UIGroup::showComponent(int player, EUIScene scene, EUILayer layer, bool show)
{
    m_layers[static_cast<int>(layer)]->showComponent(player, scene, show);
}
UIScene* UIGroup::addComponent(int player,EUIScene scene,EUILayer layer){return m_layers[(int)layer]->addComponent(player,scene,0);}
void UIGroup::removeComponent(EUIScene scene,EUILayer layer){m_layers[(int)layer]->removeComponent(scene);}
void UIGroup::ReloadAllLayers(){int i;for(i=0;i<8;++i)m_layers[i]->ReloadAll(true);}
void UIGroup::SetViewportType(C4JRender::eViewportType viewport){if(m_viewportType!=(int)viewport){m_viewportType=(int)viewport;ReloadAllLayers();}}
void UIGroup::HandleDLCMountingComplete()
{
    if (m_player < 0 || gProfile->IsSignedIn(m_player))
        for (int i = 0; i < 8; ++i)
            m_layers[i]->HandleDLCMountingComplete();
}

void UIGroup::HandleDLCInstalled()
{
    if (m_player < 0 || gProfile->IsSignedIn(m_player))
        for (int i = 0; i < 8; ++i)
            m_layers[i]->HandleDLCInstalled();
}

void UIGroup::HandleDLCLicenseChange()
{
    if (m_player < 0 || gProfile->IsSignedIn(m_player))
        for (int i = 0; i < 8; ++i)
            m_layers[i]->HandleDLCLicenseChange();
}

void UIGroup::HandleTMSBanFileRetrieved()
{
    if (m_player < 0 || gProfile->IsSignedIn(m_player))
        for (int i = 0; i < 8; ++i)
            m_layers[i]->HandleTMSBanFileRetrieved();
}

void UIGroup::HandleMessage(EUIMessage message, void* data)
{
    if (m_player < 0 || gProfile->IsSignedIn(m_player))
        for (int i = 0; i < 8; ++i)
            m_layers[i]->HandleMessage(message, data);
}

void UIGroup::HandleSaveDeviceRemoved(int device)
{
    for (int i = 0; i < 8; ++i)
        m_layers[i]->HandleSaveDeviceRemoved(device);
}

UIScene* UIGroup::FindScene(EUIScene scene)
{
    for (int i = 0; i < 8; ++i)
    {
        UIScene* found = m_layers[i]->FindScene(scene);
        if (found != 0)
            return found;
    }
    return 0;
}

void UIGroup::ClearTooltips()
{
    UIScene* scene = GetTopScene(static_cast<EUILayer>(2));
    if (scene == 0)
    {
        static_cast<UIComponent_Tooltips*>(m_component)->SetTooltips(
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false);
    }
}
