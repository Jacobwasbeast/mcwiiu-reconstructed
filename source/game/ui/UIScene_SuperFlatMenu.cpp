#include "game/ui/UIScene_SuperFlatMenu.hpp"
#include "game/ui/FJ_SuperFlatMenu.hpp"
#include "game/ui/SuperflatMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/client/audio/SoundEvent.hpp"

UIScene_SuperFlatMenu::TickAction::~TickAction()
{
}

UIScene_SuperFlatMenu::MethodAction::MethodAction(Func function) : m_function(function)
{
}

UIScene_SuperFlatMenu::MethodAction::~MethodAction()
{
}

void UIScene_SuperFlatMenu::MethodAction::Execute(UIScene_SuperFlatMenu* scene)
{
    (scene->*m_function)();
}

UIScene_SuperFlatMenu::UIScene_SuperFlatMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer), m_tickActions(), m_activeTab(eTabLayers), m_selectedLayer(0),
      m_selectedBiome(0), m_selectedParam(0), m_layersTab(), m_layersPanel(), m_layersList(),
      m_layersTouch(), m_layersHelpTitle(), m_layersHelp(), m_layersTimer(), m_biomesTab(),
      m_biomesPanel(), m_biomesList(), m_biomesTouch(), m_biomesHelpTitle(), m_biomesHelp(),
      m_biomesTimer(), m_propertiesTab(), m_propertiesPanel(), m_propertiesList(),
      m_propertiesTouch(), m_propertiesHelpTitle(), m_propertiesHelp(), m_propertiesTimer(),
      m_generating(false), m_editingLayer(false), m_currentPanel(0), m_pendingSelection(0),
      m_paramParents(), m_paramMappings(), m_config(new SuperflatConfig()), m_generatorInfo(m_config),
      m_initData(data)
{
    _init();
}

UIScene_SuperFlatMenu::~UIScene_SuperFlatMenu()
{
    for (std::vector<TickAction*>::iterator it = m_tickActions.begin(); it != m_tickActions.end(); ++it)
        delete *it;
    m_tickActions.clear();
    delete m_config;
}

unsigned int UIScene_SuperFlatMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    m_layersTab.setupControl(this, m_movie->getRootNode(), "LayersTabTitle");
    m_biomesTab.setupControl(this, m_movie->getRootNode(), "BiomesTabTitle");
    m_propertiesTab.setupControl(this, m_movie->getRootNode(), "PropertiesTabTitle");
    m_layersPanel.setupControl(this, m_movie->getRootNode(), "Layers");
    m_layersList.setupControl(this, m_layersPanel.getFuiRenderNode(), "LayersList");
    m_layersHelpTitle.setupControl(this, m_layersPanel.getFuiRenderNode(), "LayersHelpTitle");
    m_layersHelp.setupControl(this, m_layersPanel.getFuiRenderNode(), "LayersHelpHtml");
    m_layersTimer.setupControl(this, m_layersPanel.getFuiRenderNode(), "LayersTimer");
    m_biomesPanel.setupControl(this, m_movie->getRootNode(), "Biomes");
    m_biomesList.setupControl(this, m_biomesPanel.getFuiRenderNode(), "BiomesList");
    m_biomesHelpTitle.setupControl(this, m_biomesPanel.getFuiRenderNode(), "BiomesHelpTitle");
    m_biomesHelp.setupControl(this, m_biomesPanel.getFuiRenderNode(), "BiomesHelpHtml");
    m_biomesTimer.setupControl(this, m_biomesPanel.getFuiRenderNode(), "BiomesTimer");
    m_propertiesPanel.setupControl(this, m_movie->getRootNode(), "Properties");
    m_propertiesList.setupControl(this, m_propertiesPanel.getFuiRenderNode(), "PropertiesList");
    m_propertiesHelpTitle.setupControl(this, m_propertiesPanel.getFuiRenderNode(), "PropertiesHelpTitle");
    m_propertiesHelp.setupControl(this, m_propertiesPanel.getFuiRenderNode(), "PropertiesHelpHtml");
    m_propertiesTimer.setupControl(this, m_propertiesPanel.getFuiRenderNode(), "PropertiesTimer");
    PopulateLayersList();
    PopulateBiomesList();
    PopulateParamsList();
    return 0;
}

void UIScene_SuperFlatMenu::PopulateBiomesList()
{
    m_biomesList.clearList();
    m_biomesList.setCurrentSelection(m_selectedBiome, false, true);
}

int UIScene_SuperFlatMenu::ParentParam(int index)
{
    if (index < 0 || index >= (int)m_paramParents.size()) return index;
    return m_paramParents[index];
}

void UIScene_SuperFlatMenu::CreateParamMappings()
{
    m_paramParents.clear();
    m_paramMappings.clear();
    for (int i = 0; i < 16; ++i) { m_paramParents.push_back(i); m_paramMappings.push_back(0); }
}

void UIScene_SuperFlatMenu::PopulateParamsList()
{
    CreateParamMappings();
    m_propertiesList.clearList();
    m_propertiesList.setCurrentSelection(m_selectedParam, false, true);
}

void UIScene_SuperFlatMenu::_init()
{
    m_activeTab = eTabLayers;
    m_selectedLayer = 0;
    m_selectedBiome = 0;
    m_selectedParam = 0;
    m_generating = false;
    m_editingLayer = false;
    m_generatorInfo.setEqualTo(m_config);
}

void UIScene_SuperFlatMenu::setLayerButton(int index, const FlatLayerInfo& layer)
{
    if (index >= 0 && index < (int)m_layersList.getItemCount())
        m_layersList.SetItem(index, layer.getNameText(), layer.getHeightText(), L"");
}

void UIScene_SuperFlatMenu::PopulateLayersList()
{
    m_layersList.clearList();
    for (int i = m_generatorInfo.numLayers() - 1; i >= 0; --i) {
        FlatLayerInfo layer = m_generatorInfo.getLayer(i);
        m_layersList.AddNewItem(layer.getNameText(), layer.getHeightText(), i, L"");
    }
    m_layersList.setCurrentSelection(m_selectedLayer, false, true);
}

void UIScene_SuperFlatMenu::UpdateBiomesList()
{
    m_biomesList.setCurrentSelection(m_selectedBiome, false, true);
    UpdateBiomeHelpText();
}

void UIScene_SuperFlatMenu::UpdateParamsList()
{
    m_propertiesList.setCurrentSelection(m_selectedParam, false, true);
    UpdateParamHelpText();
}

void UIScene_SuperFlatMenu::LoadPreset(SuperflatConfig* config)
{
    if (config != NULL) {
        *m_config = *config;
        m_generatorInfo.setEqualTo(m_config);
    }
    PopulateLayersList();
    UpdateBiomesList();
    UpdateParamsList();
}

void UIScene_SuperFlatMenu::SetActiveTab(eTabs tab)
{
    m_activeTab = tab;
    if (m_movie != NULL) FJ_SuperFlatMenu::Create(m_movie->getRootNode())->SetActiveTab((int)tab);
}

void UIScene_SuperFlatMenu::handleTouchInput(unsigned int playerId, int action, int x, int y, bool repeat, bool released, bool pressed)
{
    UIScene::handleTouchInput(playerId, action, x, y, repeat, released, pressed);
}

unsigned int UIScene_SuperFlatMenu::GetMainPanel()
{
    return m_currentPanel;
}

void UIScene_SuperFlatMenu::updateTooltips()
{
    UIScene::updateTooltips();
    UpdateLayersHelpText();
    UpdateBiomeHelpText();
    UpdateParamHelpText();
}

void UIScene_SuperFlatMenu::updateComponents()
{
    m_layersList.UpdateControl();
    m_biomesList.UpdateControl();
    m_propertiesList.UpdateControl();
}

void UIScene_SuperFlatMenu::tick()
{
    UIScene::tick();
    for (std::vector<TickAction*>::iterator it = m_tickActions.begin(); it != m_tickActions.end(); ++it)
        (*it)->Execute(this);
    m_tickActions.clear();
}

int UIScene_SuperFlatMenu::GetLayerIndex(int index)
{
    return m_generatorInfo.numLayers() - index - 1;
}

void UIScene_SuperFlatMenu::UpdateLayersHelpText()
{
    int index = GetLayerIndex(m_layersList.getCurrentSelection());
    if (index >= 0 && index < m_generatorInfo.numLayers()) {
        FlatLayerInfo layer = m_generatorInfo.getLayer(index);
        m_layersHelpTitle.setLabel(UIString(layer.getNameText()), false, false);
        m_layersHelp.setLabel(UIString(layer.getDescriptionText()), false, false);
    }
}

void UIScene_SuperFlatMenu::UpdateBiomeHelpText()
{
    if (m_biomesList.getItemCount() != 0) {
        UIString label = m_biomesList.GetItemLabel(m_biomesList.getCurrentSelection());
        m_biomesHelpTitle.setLabel(label, false, false);
    }
}

void UIScene_SuperFlatMenu::UpdateParamHelpText()
{
    if (m_propertiesList.getItemCount() != 0) {
        UIString label = m_propertiesList.GetItemLabel(m_propertiesList.getCurrentSelection());
        m_propertiesHelpTitle.setLabel(label, false, false);
    }
}

void UIScene_SuperFlatMenu::handleReload()
{
    unsigned int selection = m_layersList.getCurrentSelection();
    _init();
    PopulateLayersList();
    UpdateBiomesList();
    UpdateParamsList();
    if (m_activeTab == eTabLayers) m_layersList.setCurrentSelection(selection, false, true);
}

void UIScene_SuperFlatMenu::ExecuteOnTick(MethodAction::Func function)
{
    m_tickActions.push_back(new MethodAction(function));
}

void UIScene_SuperFlatMenu::MoveCurrentLayer(int amount)
{
    if (amount == 0) return;
    int oldIndex = GetLayerIndex(m_layersList.getCurrentSelection());
    int newIndex = oldIndex - (amount > 0 ? 1 : -1);
    if (oldIndex < 0 || newIndex < 0 || oldIndex >= m_generatorInfo.numLayers() || newIndex >= m_generatorInfo.numLayers()) return;
    FlatLayerInfo oldLayer = m_generatorInfo.getLayer(oldIndex);
    FlatLayerInfo newLayer = m_generatorInfo.getLayer(newIndex);
    m_generatorInfo.setLayer(oldIndex, newLayer);
    m_generatorInfo.setLayer(newIndex, oldLayer);
    PopulateLayersList();
    SetSelectedLayer(m_generatorInfo.numLayers() - newIndex - 1);
}

void UIScene_SuperFlatMenu::NavActiveTab(int direction)
{
    int tab = (int)m_activeTab + (direction > 0 ? 1 : -1);
    if (tab < 0) tab = 2;
    if (tab > 2) tab = 0;
    SetActiveTab((eTabs)tab);
}

int UIScene_SuperFlatMenu::CbGenerationWarning(void* context, int button, C4JStorage::EMessageResult result)
{
    UIScene_SuperFlatMenu* scene = static_cast<UIScene_SuperFlatMenu*>(context);
    if (scene != NULL) scene->ReturnResults(result == C4JStorage::EMessageResult_4);
    return 0;
}

void UIScene_SuperFlatMenu::ReturnResults(bool accepted)
{
    m_generating = !accepted;
    if (accepted) m_generatorInfo.setEqualTo(m_config);
}

void UIScene_SuperFlatMenu::handleInput(int playerId, int action, bool repeat, bool released, bool pressed, bool& handled)
{
    UIScene::handleInput(playerId, action, repeat, released, pressed, handled);
    if (!handled && pressed) { NavActiveTab(action > 0 ? 1 : -1); handled = true; }
}

int UIScene_SuperFlatMenu::CbLayerOperation(void* context, int button, C4JStorage::EMessageResult result)
{
    UIScene_SuperFlatMenu* scene = static_cast<UIScene_SuperFlatMenu*>(context);
    if (scene != NULL && result == C4JStorage::EMessageResult_4) scene->m_editingLayer = true;
    return 0;
}

void UIScene_SuperFlatMenu::ChangeBiome(int biome)
{
    m_selectedBiome = biome;
    m_generatorInfo.setBiome(biome);
    UpdateBiomesList();
}

void UIScene_SuperFlatMenu::HandleChangeParamList(int index)
{
    m_selectedParam = index;
    UpdateParamsList();
}

void UIScene_SuperFlatMenu::handlePress(int first, int second)
{
    if (m_activeTab == eTabLayers) SetSelectedLayer(first);
    else if (m_activeTab == eTabBiomes) ChangeBiome(first);
    else HandleChangeParamList(first);
}

void UIScene_SuperFlatMenu::handleCheckboxElementToggled(int first, int second, bool checked)
{
    if (m_activeTab == eTabProperties) m_propertiesList.SetCheckboxValue(second, checked, true);
}

void UIScene_SuperFlatMenu::handleSliderElementMove(int first, int second, int value)
{
    if (m_activeTab == eTabProperties) m_propertiesList.SetSliderValue(second, value, true);
}

void UIScene_SuperFlatMenu::handleSelectionChanged(int selection)
{
    if (m_activeTab == eTabLayers) { m_selectedLayer = selection; UpdateLayersHelpText(); }
    else if (m_activeTab == eTabBiomes) { m_selectedBiome = selection; UpdateBiomeHelpText(); }
    else { m_selectedParam = selection; UpdateParamHelpText(); }
}

void UIScene_SuperFlatMenu::handleFocusChange(int focus, int childFocus)
{
    UIScene::handleFocusChange(focus, childFocus);
    m_currentPanel = focus;
}

void UIScene_SuperFlatMenu::handleGainFocus(bool hadFocus)
{
    UIScene::handleGainFocus(hadFocus);
    updateComponents();
}

void UIScene_SuperFlatMenu::customDraw(const char* name, fuiRect* rect)
{
    UIScene::customDraw(name, rect);
}

unsigned int UIScene_SuperFlatMenu::SetSelectedLayer(int index)
{
    m_selectedLayer = index;
    m_layersList.setCurrentSelection(index, true, true);
    UpdateLayersHelpText();
    return index;
}

void UIScene_SuperFlatMenu::DeleteLayer()
{
    int index = GetLayerIndex(m_selectedLayer);
    if (index >= 0 && index < m_generatorInfo.numLayers()) { m_generatorInfo.remLayer(index); PopulateLayersList(); }
}

void UIScene_SuperFlatMenu::WritePropertyChanges(int index)
{
    m_generatorInfo.setEqualTo(m_config);
    UpdateParamsList();
}

void UIScene_SuperFlatMenu::PlaySound(const SoundEvent* sound)
{
    if (sound != NULL) m_pendingSelection = sound->getIndex();
}

unsigned int UIScene_SuperFlatMenu::OpenPresetsDialog()
{
    m_editingLayer = false;
    return 1;
}

unsigned int UIScene_SuperFlatMenu::OpenEditLayerDialog(bool insert)
{
    m_editingLayer = true;
    if (insert) m_pendingSelection = 1;
    return 1;
}

unsigned int UIScene_SuperFlatMenu::OpenMaxHeightDialog()
{
    return (unsigned int)m_generatorInfo.getHeight();
}

std::wstring UIScene_SuperFlatMenu::getMoviePath()
{
    return std::wstring(L"SuperFlatMenu");
}

int UIScene_SuperFlatMenu::getSceneType()
{
    return 57;
}

template void UIScene_SuperFlatMenu::ExecuteOnTick<const SoundEvent*>(MethodActionArg1<const SoundEvent*>::Func function, const SoundEvent* value);
template void UIScene_SuperFlatMenu::ExecuteOnTick<int>(MethodActionArg1<int>::Func function, int value);
template void UIScene_SuperFlatMenu::ExecuteOnTick<bool>(MethodActionArg1<bool>::Func function, bool value);
