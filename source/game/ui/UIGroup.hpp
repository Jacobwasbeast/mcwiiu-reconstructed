#ifndef GAME_UI_UIGROUP_HPP
#define GAME_UI_UIGROUP_HPP

#include "game/ui/UILayer.hpp"

enum EUIGroup { EUIGroup_Unknown = 0 };
enum EUILayer { EUILayer_Unknown = 0 };

class UIComponent_WiiUDRC;

class UIGroup
{
    friend class ConsoleUIController;

public:
    UIGroup(EUIGroup group, int player);
    void getRenderDimensions(int& width, int& height);
    unsigned int GetLayerIndex(UILayer* layer);
    unsigned int RequestFocus(UILayer* layer);
    void UpdateFocusState();
    bool IsFullscreenGroup();
    unsigned int GetViewportType();
    void DestroyAll();
    void ReloadAll();
    void _UpdateFocusState();
    void tick();
    void render();
    int hidesLowerScenes();
    void updateStackStates();
    bool NavigateToScene(int player, EUIScene scene, void* data, EUILayer layer);
    int NavigateBack(int player, EUIScene scene, EUILayer layer);
    void closeAllScenes();
    UIScene* GetTopScene(EUILayer layer);
    bool IsSceneInStack(EUIScene scene);
    unsigned char GetMenuDisplayed();
    void handleInput(int player, int action, bool repeat, bool released, bool pressed, bool& handled);
    UIScene* getCurrentScene(int player);
    void showComponent(int player, EUIScene scene, EUILayer layer, bool show);
    UIScene* addComponent(int player, EUIScene scene, EUILayer layer);
    void removeComponent(EUIScene scene, EUILayer layer);
    void ReloadAllLayers();
    void SetViewportType(C4JRender::eViewportType viewport);
    void HandleDLCMountingComplete();
    void HandleDLCInstalled();
    void HandleDLCLicenseChange();
    void HandleTMSBanFileRetrieved();
    void HandleMessage(EUIMessage message, void* data);
    void HandleSaveDeviceRemoved(int device);
    UIScene* FindScene(EUIScene scene);
    void ClearTooltips();
    bool HasFocus(int player);
    void handleUnlockFullVersion();
    void* getCommandBufferList();
private:
    UILayer* m_layers[8];
    UIScene* m_component;
    UIScene* m_componentSecondary;
    UIScene* m_componentTertiary;
    UIScene* m_componentQuaternary;
    UIComponent_WiiUDRC* m_drcComponent;
    int m_viewportType;
    int m_group;
    int m_player;
    unsigned char m_menuDisplayed;
    bool m_pauseMenuDisplayed;
    bool m_containerMenuDisplayed;
    bool m_ignoreAutosaveMenuDisplayed;
    bool m_ignorePlayerJoinMenuDisplayed;
    unsigned char m_stackState5;
    unsigned char m_unknown45[3];
    int m_focusTimer;
    int m_displayList;
};

#endif
