#include "game/ui/UIScene.hpp"
#include <ctime>

UIScene::_TimerInfo::_TimerInfo() : duration(0), elapsed(0), complete(false) {}

UIScene::UIScene(int type, UILayer* layer)
    : m_sceneType(type), m_layer(layer), m_safeZoneLeft(0), m_safeZoneTop(0),
      m_safeZoneRight(0), m_safeZoneBottom(0), m_viewportTouchOffset(1),
      m_movie(0), m_movieLoaded(false), m_itemRenderer(0), m_controlFocus(-1),
      m_controlChildFocus(0), m_opacity(1), m_needsReload(false), m_visible(true),
      m_focusInitialized(false), m_backScene(0), m_resolution(0), m_stageWidth(0),
      m_hasInitialized(false), m_resizePending(false), m_touchBoxDirty(false),
      m_controlsUpdated(false), m_mainPanel(0), m_sceneActive(false),
      m_sceneVisible(false), m_sceneReady(false), m_cacheDirty(false),
      m_destroyed(false), m_callbackId(0) {}
UIScene::~UIScene() {}
void UIScene::reloadMovie(bool value) { m_needsReload = value || m_needsReload; }
bool UIScene::needsReloaded() { return m_needsReload; }
bool UIScene::hasMovie() { return m_movie != 0; }
void UIScene::updateSafeZone() { m_resizePending = false; }
void UIScene::updateViewportTouchOffset() { m_viewportTouchOffset = 0; }
double UIScene::getSafeZoneHalfHeight() { return (m_safeZoneBottom - m_safeZoneTop) * 0.5; }
double UIScene::getSafeZoneHalfWidth() { return (m_safeZoneRight - m_safeZoneLeft) * 0.5; }
unsigned int UIScene::mapElementsAndNames() { return 1; }
unsigned int UIScene::getSubSceneType() const { return 0; }
void UIScene::tick() { tickTimers(); UpdateSceneControls(); }
void UIScene::SetFocusToElement(int value) { _handleFocusChange(value, 0); }
void UIScene::handleTimerComplete(int) {}
bool UIScene::stealsFocus() { return true; }
bool UIScene::hasFocus(int value) { return m_focusInitialized && value == m_sceneType; }
void UIScene::updateTooltips() {}
void UIScene::updateComponents() {}
void UIScene::handleGainFocus(bool) {}
void UIScene::handleLoseFocus() {}
bool UIScene::hidesLowerScenes() { return m_sceneVisible; }
bool UIScene::blocksInput() { return false; }
unsigned int UIScene::GetMainPanel() { return m_mainPanel; }
void UIScene::render(int, int, C4JRender::eViewportType) {}
void UIScene::customDraw(const char*, fuiRect*) {}
bool UIScene::allowRepeat(int value) { return value > 3 && (value < 20 || value > 21); }
void UIScene::handleInput(int, int, bool, bool, bool, bool&) {}
void UIScene::handleDestroy() {}
void UIScene::handlePreUnloadForReload() {}
void UIScene::handlePreReload() {}
void UIScene::handleReload() {}
void UIScene::handlePress(int, int) {}
void UIScene::handleFocusChange(int first, int second) { _handleFocusChange(first, second); }
void UIScene::handleInitFocus(int first, int second) { _handleInitFocus(first, second); }
void UIScene::handleCheckboxToggled(int, bool) {}
void UIScene::handleCheckboxElementToggled(int, int, bool) {}
void UIScene::handleSliderMove(int, int) {}
void UIScene::handleSliderElementMove(int, int, int) {}
void UIScene::handleAnimationEnd() {}
void UIScene::handleSelectionChanged(int) {}
void UIScene::handleRequestMoreData(int, bool) {}
void UIScene::handleTouchBoxRebuild() {}
void UIScene::HandleDLCMountingComplete() {}
void UIScene::HandleDLCInstalled() {}
void UIScene::HandleDLCLicenseChange() {}
void UIScene::HandleTMSBanFileRetrieved() {}
void UIScene::HandleMessage(EUIMessage, void*) {}
void UIScene::HandleSaveDeviceRemoved(int) {}
void UIScene::handleUnlockFullVersion() {}
void UIScene::handleTouchInput(unsigned int, int, int, int, bool, bool, bool) {}
bool UIScene::isReadyToDelete() { return true; }
void UIScene::customDrawFui(void* scene, const char* name, fuiRect* rect) { if (scene != 0) static_cast<UIScene*>(scene)->customDraw(name, rect); }
void UIScene::tickTimers() {
    const int now = static_cast<int>(std::clock() * 1000 / CLOCKS_PER_SEC);
    boost::unordered_map<int, _TimerInfo>::iterator it = m_timers.begin();
    while (it != m_timers.end()) {
        if (!it->second.complete) {
            ++it;
        } else if (it->second.elapsed < now) {
            handleTimerComplete(it->first);
            it->second.elapsed = now + it->second.duration;
            ++it;
        } else {
            ++it;
        }
    }
}
void UIScene::navigateBack() { if (m_backScene != 0) m_backScene->gainFocus(); }
bool UIScene::hasRegisteredSubstitutionTexture(const std::wstring& name) { return m_substitutionTextures.find(name) != m_substitutionTextures.end(); }
void UIScene::unregisterSubstitutionTexture(const std::wstring& name, bool remove) { if (remove) m_substitutionTextures.erase(name); else m_substitutionTextures[name] = false; }
void UIScene::registerSubstitutionTexture(const std::wstring& name,
    unsigned char*, unsigned int, bool value) {
    m_substitutionTextures[name] = value;
}
void UIScene::loadMovie() { m_movieLoaded = true; m_needsReload = false; }
void UIScene::initialiseMovie() { loadMovie(); mapElementsAndNames(); updateSafeZone(); m_hasInitialized = true; }
void UIScene::addTimer(int id, int duration) { _TimerInfo& info = m_timers[id]; info.duration = duration; info.elapsed = static_cast<int>(std::clock() * 1000 / CLOCKS_PER_SEC) + duration; info.complete = true; }
void UIScene::killTimer(int id) { boost::unordered_map<int, _TimerInfo>::iterator it = m_timers.find(id); if (it != m_timers.end()) it->second.complete = false; }
unsigned int UIScene::convertGameActionToFuiKeycode(int value) { if (value == 20) return 0; if (value == 21) return 7; if (value == 12) return 100; return value; }
void UIScene::sendInputToMovie(int, bool, bool, bool) {}
unsigned long long UIScene::GetCallbackUniqueId() { return (unsigned long long)m_callbackId; }
bool UIScene::controlHasFocus(int value) { return m_controlFocus == value; }
unsigned int UIScene::getControlChildFocus() { return m_controlChildFocus; }
void UIScene::customDrawSlotControlFui(fuiRect*, int, not_null_ptr<ItemInstance>, float, bool, bool, bool) {}
void UIScene::destroyMovie() { m_movie = 0; m_movieLoaded = false; }
void UIScene::UpdateSceneControls() { m_controlsUpdated = true; }
void UIScene::removeControl(UIControl_Base*, bool) {}
void UIScene::doHorizontalResizeCheck() { m_resizePending = false; }
UIScene* UIScene::getBackScene() { return m_backScene; }
void UIScene::gainFocus() { handleGainFocus(true); m_sceneActive = true; }
void UIScene::loseFocus() { handleLoseFocus(); m_sceneActive = false; }
void UIScene::setBackScene(UIScene* scene) { m_backScene = scene; }
void UIScene::setSafeZone(double left, double top, double right, double bottom) { m_safeZoneLeft = left; m_safeZoneTop = top; m_safeZoneRight = right; m_safeZoneBottom = bottom; m_resizePending = true; }
void UIScene::resetHorizontalResizeCheck() { m_resizePending = false; }
void UIScene::_handleFocusChange(int first, int second) { m_controlFocus = first; m_controlChildFocus = second; m_focusInitialized = true; }
void UIScene::_handleInitFocus(int first, int second) { _handleFocusChange(first, second); }
void UIScene::controlHasFocus(UIControl_Base*) { m_focusInitialized = true; }
unsigned int UIScene::getControlFocus() { return m_controlFocus; }
void UIScene::setOpacity(float value) { m_opacity = value; }
void UIScene::setVisible(bool value) { m_visible = value; }
bool UIScene::hasTimer(int id) { boost::unordered_map<int, _TimerInfo>::iterator it = m_timers.find(id); return it != m_timers.end() && it->second.complete; }
bool UIScene::canCacheSWF() { return false; }
unsigned long long UIScene::slideLeft() { return 0; }
unsigned long long UIScene::slideRight() { return 0; }
