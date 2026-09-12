#include "game/ui/UILayer.hpp"
#include <algorithm>
#include "game/ui/UIScene_Intro.hpp"
#include "game/ui/UIScene_SaveMessage.hpp"
#include "game/ui/UIScene_MainMenu.hpp"
#include "game/ui/UIScene_FullscreenProgress.hpp"
#include "game/ui/UIScene_PauseMenu.hpp"
#include "game/ui/UIScene_CraftingMenu.hpp"

// TEMPORARY boot tracing; remove once the boot reaches the title screen.
extern "C" void OSReport(const char*, ...);
#define LAYER_TRACE(step) OSReport("[boot][layer_trace] %s\n", step)

UILayer::UILayer(UIGroup* group) : m_focusChanged(false), m_menuDisplayed(false), m_fullscreen(false), m_viewportChanged(false), m_reloadPending(false), m_inputHandled(false), m_group(group) {}
void UILayer::getRenderDimensions(int& width, int& height) { width = 0; height = 0; }
UIScene* UILayer::addComponent(int, EUIScene scene, void*) {
    boost::unordered_map<EUIScene,std::pair<int,bool> >::iterator state = m_componentStates.find(scene);
    if (state != m_componentStates.end()) {
        ++state->second.first;
        for (std::vector<UIScene*>::iterator it=m_components.begin(); it!=m_components.end(); ++it)
            if (*it && (*it)->getSceneType()==(int)scene) return *it;
        return NULL;
    }
    m_componentStates[scene] = std::make_pair(1,true);
    return NULL;
}
void UILayer::showComponent(int id, EUIScene scene, bool visible) {
    boost::unordered_map<EUIScene,std::pair<int,bool> >::iterator it=m_componentStates.find(scene);
    if (it!=m_componentStates.end()) it->second.second=visible;
    else if (visible) addComponent(id,scene,NULL);
}
void UILayer::tick() {
    for (std::vector<UIScene*>::iterator it=m_pendingDestroy.begin(); it!=m_pendingDestroy.end(); ++it) if (*it) (*it)->destroyMovie();
    m_pendingDestroy.clear();
    for (std::vector<UIScene*>::iterator it=m_pendingReload.begin(); it!=m_pendingReload.end(); ++it) if (*it) (*it)->reloadMovie(true);
    m_pendingReload.clear();
    for (std::vector<UIScene*>::iterator it=m_components.begin(); it!=m_components.end(); ++it) if (*it) (*it)->UpdateSceneControls();
    for (std::vector<UIScene*>::iterator it=m_components.begin(); it!=m_components.end(); ++it) if (*it) (*it)->tick();
    for (std::vector<UIScene*>::iterator it=m_scenes.begin(); it!=m_scenes.end(); ++it) if (*it) (*it)->tick();
}
void UILayer::render(int width,int height,C4JRender::eViewportType viewport) {
    for (std::vector<UIScene*>::iterator it=m_components.begin(); it!=m_components.end(); ++it)
        if (*it && isComponentVisible((EUIScene)(*it)->getSceneType()) && (*it)->hasMovie()) (*it)->render(width,height,viewport);
    for (std::vector<UIScene*>::reverse_iterator it=m_scenes.rbegin(); it!=m_scenes.rend(); ++it) {
        if (*it && (*it)->isReadyToDelete()==false) (*it)->render(width,height,viewport);
        if (*it && (*it)->hidesLowerScenes()) break;
    }
}
void UILayer::DestroyAll() {
    for (std::vector<UIScene*>::iterator it=m_components.begin(); it!=m_components.end(); ++it)
        if (*it) (*it)->destroyMovie();
    for (std::vector<UIScene*>::iterator it=m_scenes.begin(); it!=m_scenes.end(); ++it)
        if (*it) (*it)->destroyMovie();
}
void UILayer::ReloadAll(bool force) {
    for (std::vector<UIScene*>::iterator it=m_components.begin(); it!=m_components.end(); ++it)
        if (*it) (*it)->reloadMovie(force);
    for (std::vector<UIScene*>::iterator it=m_scenes.begin(); it!=m_scenes.end(); ++it)
        if (*it) (*it)->reloadMovie(force);
}
bool UILayer::IsSceneInStack(EUIScene scene) { return FindScene(scene)!=NULL; }
bool UILayer::updateFocusState(bool requestFocus) {
    m_focusChanged=requestFocus;
    for (std::vector<UIScene*>::reverse_iterator it=m_scenes.rbegin(); it!=m_scenes.rend(); ++it) {
        if (*it) {
            if (requestFocus) (*it)->gainFocus(); else (*it)->loseFocus();
            break;
        }
    }
    return requestFocus;
}
void UILayer::closeAllScenes() { while(!m_scenes.empty()) removeScene(m_scenes.back()); }
bool UILayer::NavigateToScene(int player, EUIScene scene, void* data)
{
    LAYER_TRACE("NavigateToScene entered");
    UIScene* created = NULL;
    switch (static_cast<int>(scene))
    {
    LAYER_TRACE("switching on scene");
    case 1: created = new UIScene_Intro(0, data, this); break;
    case 2: created = new UIScene_SaveMessage(0, data, this); break;
    case 3: created = new UIScene_MainMenu(0, data, this); break;
    case 4: created = new UIScene_FullscreenProgress(0, data, this); break;
    case 5: created = new UIScene_PauseMenu(0, data, this); break;
    case 6:
    case 7: created = new UIScene_CraftingMenu(0, data, this); break;
    default: return false;
    }
    LAYER_TRACE("scene CONSTRUCTED");
    if (created == NULL)
        return false;
    if (!m_scenes.empty())
        created->setBackScene(m_scenes.back());
    LAYER_TRACE("backScene set");
    m_scenes.push_back(created);
    LAYER_TRACE("pushed to stack");
    updateFocusState(false);
    LAYER_TRACE("focus state updated");
    created->handleInitFocus(player, 0);
    return true;
}
void UILayer::removeScene(UIScene* scene) {
    std::vector<UIScene*>::iterator it=std::find(m_scenes.begin(),m_scenes.end(),scene);
    if (it!=m_scenes.end()) {
        m_pendingDestroy.push_back(scene);
        m_scenes.erase(it);
        scene->handleDestroy();
        updateFocusState(false);
    }
}
bool UILayer::NavigateBack(int,EUIScene scene) {
    while (!m_scenes.empty()) {
        UIScene* top=m_scenes.back();
        if (top->getSceneType()==(int)scene) return true;
        if (!top->blocksInput()) removeScene(top); else return false;
    }
    return false;
}
UIScene* UILayer::GetTopScene() { return m_scenes.empty()?NULL:m_scenes.back(); }
bool UILayer::HasFocus(int controllerId) {
    for (std::vector<UIScene*>::reverse_iterator it=m_scenes.rbegin(); it!=m_scenes.rend(); ++it)
        if (*it && (*it)->hasFocus(controllerId)) return true;
    return false;
}
bool UILayer::hidesLowerScenes() {
    for (std::vector<UIScene*>::reverse_iterator it=m_components.rbegin(); it!=m_components.rend(); ++it)
        if (*it && (*it)->hidesLowerScenes()) return true;
    for (std::vector<UIScene*>::reverse_iterator it=m_scenes.rbegin(); it!=m_scenes.rend(); ++it)
        if (*it && (*it)->hidesLowerScenes()) return true;
    return false;
}
bool UILayer::isComponentVisible(EUIScene scene) {
    boost::unordered_map<EUIScene,std::pair<int,bool> >::iterator it=m_componentStates.find(scene);
    return it!=m_componentStates.end() && it->second.second;
}
void UILayer::removeComponent(EUIScene scene) { m_componentStates.erase(scene); }
void UILayer::handleInput(int c,int a,bool r,bool p,bool q,bool& handled) { UIScene* scene=GetTopScene(); if(scene) scene->handleInput(c,a,r,p,q,handled); }
UIScene* UILayer::getCurrentScene(int controllerId) {
    for (std::vector<UIScene*>::reverse_iterator it=m_scenes.rbegin(); it!=m_scenes.rend(); ++it)
        if (*it && (*it)->hasFocus(controllerId)) return *it;
    return NULL;
}
bool UILayer::IsFullscreenGroup() { return m_group!=NULL && m_fullscreen; }
C4JRender::eViewportType UILayer::getViewport() { return C4JRender::eViewportType_FullScreen; }
void UILayer::HandleDLCMountingComplete() { for(std::vector<UIScene*>::reverse_iterator it=m_scenes.rbegin();it!=m_scenes.rend();++it)if(*it)(*it)->HandleDLCMountingComplete(); }
void UILayer::HandleDLCInstalled() { for(std::vector<UIScene*>::reverse_iterator it=m_scenes.rbegin();it!=m_scenes.rend();++it)if(*it)(*it)->HandleDLCInstalled(); }
void UILayer::HandleDLCLicenseChange() { for(std::vector<UIScene*>::reverse_iterator it=m_scenes.rbegin();it!=m_scenes.rend();++it)if(*it)(*it)->HandleDLCLicenseChange(); }
void UILayer::HandleTMSBanFileRetrieved() { for(std::vector<UIScene*>::reverse_iterator it=m_scenes.rbegin();it!=m_scenes.rend();++it)if(*it)(*it)->HandleTMSBanFileRetrieved(); }
void UILayer::HandleMessage(EUIMessage message,void* data) { for(std::vector<UIScene*>::reverse_iterator it=m_scenes.rbegin();it!=m_scenes.rend();++it)if(*it)(*it)->HandleMessage(message,data); }
void UILayer::HandleSaveDeviceRemoved(int device) { for(std::vector<UIScene*>::reverse_iterator it=m_scenes.rbegin();it!=m_scenes.rend();++it)if(*it)(*it)->HandleSaveDeviceRemoved(device); }
void UILayer::handleUnlockFullVersion() { for(std::vector<UIScene*>::iterator it=m_scenes.begin();it!=m_scenes.end();++it)if(*it)(*it)->handleUnlockFullVersion(); }
UIScene* UILayer::FindScene(EUIScene scene) { for(std::vector<UIScene*>::iterator it=m_scenes.begin();it!=m_scenes.end();++it)if(*it&&(*it)->getSceneType()==(int)scene)return *it; return NULL; }
void UILayer::DestroyAllScenes() { closeAllScenes(); }
bool UILayer::GetMenuDisplayed() { return m_menuDisplayed; }
