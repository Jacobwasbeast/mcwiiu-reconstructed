#include "game/ui/UIGroup.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/MemoryTracker.hpp"

UIGroup::UIGroup(EUIGroup group, int player)
    : m_component(0), m_componentSecondary(0), m_componentTertiary(0),
      m_componentQuaternary(0), m_unknown30(0), m_viewportType(0),
      m_group((int)group), m_player(player), m_menuDisplayed(0), m_stackState1(0),
      m_stackState2(0), m_stackState3(0), m_stackState4(0), m_stackState5(0),
      m_focusTimer(0), m_displayList(0)
{
    int i;
    m_unknown45[0] = 0;
    m_unknown45[1] = 0;
    m_unknown45[2] = 0;
    for (i = 0; i < 8; ++i) m_layers[i] = new UILayer(this);
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
void UIGroup::getRenderDimensions(int& width,int& height) { gConsoleUIController.getRenderDimensions((C4JRender::eViewportType)m_viewportType,width,height); }
unsigned int UIGroup::GetLayerIndex(UILayer* layer) { int i; for(i=0;i<8;++i) if(m_layers[i]==layer)return (unsigned int)i; return 0; }
unsigned int UIGroup::RequestFocus(UILayer* layer) { unsigned int n=GetLayerIndex(layer),i; if(n==0)return 1; for(i=0;i<n;++i)if(m_layers[i]->HasFocus(0))return 0; return 1; }
void UIGroup::UpdateFocusState() { m_focusTimer=10; }
bool UIGroup::IsFullscreenGroup() { return m_group==1; }
unsigned int UIGroup::GetViewportType() { return (unsigned int)m_viewportType; }
void UIGroup::DestroyAll() { int i; for(i=0;i<8;++i)m_layers[i]->DestroyAll(); }
void UIGroup::ReloadAll() { int last=8,i; for(i=0;i<8;++i)if(m_layers[i]->hidesLowerScenes()){last=i+1;break;} for(i=last-1;i>=0;--i)m_layers[i]->ReloadAll(i!=4); }
void UIGroup::_UpdateFocusState() { int i; for(i=0;i<8;++i)if(m_layers[i]->updateFocusState(true))return; }
void UIGroup::tick() { int i; if(m_player<0){for(i=0;i<8;++i)m_layers[i]->tick();if(m_focusTimer>0&&--m_focusTimer==0)_UpdateFocusState();} }
void UIGroup::render()
{
    int width = 0;
    int height = 0;
    int layer;
    int firstLayer = 0;

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

    for (layer = firstLayer - 1; layer >= 0; --layer)
    {
        m_layers[layer]->render(width, height, (C4JRender::eViewportType)m_viewportType);
    }
}
int UIGroup::hidesLowerScenes() { int i; if(m_player>=0)return 0; for(i=7;i>=0;--i)if(m_layers[i]->hidesLowerScenes())return 1;return 0; }
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
    m_stackState1 = state[1];
    m_stackState2 = state[2];
    m_stackState3 = state[3];
    m_stackState4 = state[4];
}
bool UIGroup::NavigateToScene(int player,EUIScene scene,void* data,EUILayer layer){bool result=m_layers[(int)layer]->NavigateToScene(player,scene,data);updateStackStates();return result;}
int UIGroup::NavigateBack(int player,EUIScene scene,EUILayer layer){int i,result=0;for(i=0;i<8;++i)if((int)layer>=8||i==(int)layer)result=m_layers[i]->NavigateBack(player,scene)||result;updateStackStates();return result;}
void UIGroup::closeAllScenes(){int i;for(i=0;i<8;++i)if(i!=2)m_layers[i]->closeAllScenes();updateStackStates();}
UIScene* UIGroup::GetTopScene(EUILayer layer){return m_layers[(int)layer]->GetTopScene();}
bool UIGroup::IsSceneInStack(EUIScene scene){int i;for(i=0;i<8;++i)if(m_layers[i]->IsSceneInStack(scene))return true;return false;}
unsigned char UIGroup::GetMenuDisplayed(){return m_menuDisplayed;}
void UIGroup::handleInput(int player,int action,bool repeat,bool released,bool pressed,bool& handled){int i;if(m_player<0)for(i=0;i<8&&!handled;++i)m_layers[i]->handleInput(player,action,repeat,released,pressed,handled);}
UIScene* UIGroup::getCurrentScene(int player){int i;UIScene* scene;for(i=0;i<8;++i){scene=m_layers[i]->getCurrentScene(player);if(scene)return scene;}return 0;}
void UIGroup::showComponent(int player,EUIScene scene,EUILayer layer,bool show){m_layers[(int)layer]->showComponent(player,scene,show);}
void UIGroup::addComponent(int player,EUIScene scene,EUILayer layer){m_layers[(int)layer]->addComponent(player,scene,0);}
void UIGroup::removeComponent(EUIScene scene,EUILayer layer){m_layers[(int)layer]->removeComponent(scene);}
void UIGroup::ReloadAllLayers(){int i;for(i=0;i<8;++i)m_layers[i]->ReloadAll(true);}
void UIGroup::SetViewportType(C4JRender::eViewportType viewport){if(m_viewportType!=(int)viewport){m_viewportType=(int)viewport;ReloadAllLayers();}}
void UIGroup::HandleDLCMountingComplete(){int i;if(m_player<0)for(i=0;i<8;++i)m_layers[i]->HandleDLCMountingComplete();}
void UIGroup::HandleDLCInstalled(){int i;if(m_player<0)for(i=0;i<8;++i)m_layers[i]->HandleDLCInstalled();}
void UIGroup::HandleDLCLicenseChange(){int i;if(m_player<0)for(i=0;i<8;++i)m_layers[i]->HandleDLCLicenseChange();}
void UIGroup::HandleTMSBanFileRetrieved(){int i;if(m_player<0)for(i=0;i<8;++i)m_layers[i]->HandleTMSBanFileRetrieved();}
void UIGroup::HandleMessage(EUIMessage message,void* data){int i;if(m_player<0)for(i=0;i<8;++i)m_layers[i]->HandleMessage(message,data);}
void UIGroup::HandleSaveDeviceRemoved(int device){int i;for(i=0;i<8;++i)m_layers[i]->HandleSaveDeviceRemoved(device);}
UIScene* UIGroup::FindScene(EUIScene scene){int i;UIScene* found;for(i=0;i<8;++i){found=m_layers[i]->FindScene(scene);if(found)return found;}return 0;}
void UIGroup::ClearTooltips(){UIScene* scene=GetTopScene((EUILayer)2);if(scene)scene->handleTouchBoxRebuild();}
