#ifndef GAME_UI_UILAYER_HPP
#define GAME_UI_UILAYER_HPP
#include <vector>
#include <utility>
#include <boost/unordered_map.hpp>
#include "game/ui/UIScene.hpp"
class UIGroup;
enum EUIScene { EUIScene_Unknown = 0 };
class UILayer {
public:
 UILayer(UIGroup*); void getRenderDimensions(int&,int&); UIScene* addComponent(int,EUIScene,void*); void showComponent(int,EUIScene,bool); void tick(); void render(int,int,C4JRender::eViewportType); void DestroyAll(); void ReloadAll(bool); bool IsSceneInStack(EUIScene); bool updateFocusState(bool); void closeAllScenes(); bool NavigateToScene(int,EUIScene,void*); void removeScene(UIScene*); bool NavigateBack(int,EUIScene); UIScene* GetTopScene(); bool HasFocus(int); bool hidesLowerScenes(); bool isComponentVisible(EUIScene); void removeComponent(EUIScene); void handleInput(int,int,bool,bool,bool,bool&); UIScene* getCurrentScene(int); bool IsFullscreenGroup(); C4JRender::eViewportType getViewport(); void HandleDLCMountingComplete(); void HandleDLCInstalled(); void HandleDLCLicenseChange(); void HandleTMSBanFileRetrieved(); void HandleMessage(EUIMessage,void*); void HandleSaveDeviceRemoved(int); void handleUnlockFullVersion(); UIScene* FindScene(EUIScene); void DestroyAllScenes(); bool GetMenuDisplayed();
private:
 std::vector<UIScene*> m_scenes,m_components,m_pendingDestroy,m_pendingReload;
 boost::unordered_map<EUIScene,std::pair<int,bool> > m_componentStates;
 bool m_focusChanged,m_menuDisplayed,m_fullscreen,m_viewportChanged,m_reloadPending,m_inputHandled;
 UIGroup* m_group;
};
#endif
