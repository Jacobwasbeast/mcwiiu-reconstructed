#include "game/ui/UIScene_SuperFlatMenu.hpp"
#include "game/core/StringIds.hpp"
#include "game/ui/FJ_SuperFlatMenu.hpp"
#include "game/ui/SuperflatMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/L10N.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/core/GameSettings.hpp"

#include <cstdio>
#include <cstdlib>

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
    : UIScene(sceneType, layer),
      m_tickActions(),
      m_layersTouch(),
      m_biomesTouch(),
      m_propertiesTouch(),
      m_activeTab(eTabLayers),
      m_layersTab(), m_layersPanel(), m_layersList(), m_layersHelpTitle(), m_layersHelp(), m_layersTimer(),
      m_biomesTab(), m_biomesPanel(), m_biomesList(), m_biomesHelpTitle(), m_biomesHelp(), m_biomesTimer(),
      m_propertiesTab(), m_propertiesPanel(), m_propertiesList(), m_propertiesHelpTitle(), m_propertiesHelp(),
      m_propertiesTimer(),
      m_generating(false),
      m_editingLayer(false),
      m_pendingSelection(0),
      m_paramMappings(),
      m_presetsFlag(false),
      m_config(NULL),
      m_generatorInfo(NULL),
      m_initData(data)
{
    initialiseMovie();
    _init();
    m_config = new SuperflatConfig();
    LoadPreset(reinterpret_cast<SuperflatConfig*>(static_cast<char*>(data) + 0xe0));
}

UIScene_SuperFlatMenu::~UIScene_SuperFlatMenu()
{
    for (boost::unordered_map<int, SuperflatMenu::Mapping*>::iterator it = m_paramMappings.begin();
         it != m_paramMappings.end(); ++it)
        delete it->second;
    delete m_config;
}

unsigned int UIScene_SuperFlatMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();

    m_layersTab.setupControl(this, root, "LayersTabTitle");
    m_controls.push_back(&m_layersTab);
    m_biomesTab.setupControl(this, root, "BiomesTabTitle");
    m_controls.push_back(&m_biomesTab);
    m_propertiesTab.setupControl(this, root, "PropertiesTabTitle");
    m_controls.push_back(&m_propertiesTab);

    m_layersPanel.setupControl(this, root, "Layers");
    m_controls.push_back(&m_layersPanel);
    fuiRenderNode* panel = m_layersPanel.getFuiRenderNode();
    m_layersList.setupControl(this, panel, "LayersList");
    m_controls.push_back(&m_layersList);
    m_layersHelpTitle.setupControl(this, panel, "LayersHelpTitle");
    m_controls.push_back(&m_layersHelpTitle);
    m_layersHelp.setupControl(this, panel, "LayersHelpHtml");
    m_controls.push_back(&m_layersHelp);
    m_layersTimer.setupControl(this, panel, "LayersTimer");
    m_controls.push_back(&m_layersTimer);
    m_layersTouch.setupControl(this, panel, "TouchTabLayers");
    m_controls.push_back(&m_layersTouch);

    m_biomesPanel.setupControl(this, root, "Biomes");
    m_controls.push_back(&m_biomesPanel);
    panel = m_biomesPanel.getFuiRenderNode();
    m_biomesList.setupControl(this, panel, "BiomesList");
    m_controls.push_back(&m_biomesList);
    m_biomesHelpTitle.setupControl(this, panel, "BiomesHelpTitle");
    m_controls.push_back(&m_biomesHelpTitle);
    m_biomesHelp.setupControl(this, panel, "BiomesHelpHtml");
    m_controls.push_back(&m_biomesHelp);
    m_biomesTimer.setupControl(this, panel, "BiomesTimer");
    m_controls.push_back(&m_biomesTimer);
    m_biomesTouch.setupControl(this, panel, "TouchTabBiomes");
    m_controls.push_back(&m_biomesTouch);

    m_propertiesPanel.setupControl(this, root, "Properties");
    m_controls.push_back(&m_propertiesPanel);
    panel = m_propertiesPanel.getFuiRenderNode();
    m_propertiesList.setupControl(this, panel, "PropertiesList");
    m_controls.push_back(&m_propertiesList);
    m_propertiesHelpTitle.setupControl(this, panel, "PropertiesHelpTitle");
    m_controls.push_back(&m_propertiesHelpTitle);
    m_propertiesHelp.setupControl(this, panel, "PropertiesHelpHtml");
    m_controls.push_back(&m_propertiesHelp);
    m_propertiesTimer.setupControl(this, panel, "PropertiesTimer");
    m_controls.push_back(&m_propertiesTimer);
    m_propertiesTouch.setupControl(this, panel, "TouchTabProperties");
    m_controls.push_back(&m_propertiesTouch);

    return 1;
}

void UIScene_SuperFlatMenu::PopulateBiomesList()
{
    SuperflatMenu::BiomeInfo* biomes = SuperflatMenu::getBiomeInfoA();
    int index = 0;
    for (int i = 0; biomes[i].mBiome >= 0; ++i)
    {
        m_biomesList.AddNewItem(biomes[i].getName(), index, false, L"");
        ++index;
    }
}

int UIScene_SuperFlatMenu::ParentParam(int index)
{
    if (index < 0)
        return -1;
    return index;
}

void UIScene_SuperFlatMenu::CreateParamMappings()
{
    m_paramMappings.clear();
}

void UIScene_SuperFlatMenu::PopulateParamsList()
{
    CreateParamMappings();
    m_propertiesList.clearList();
}

void UIScene_SuperFlatMenu::_init()
{
    m_layersTab.init(UIString(IDS_LAYERS));
    m_layersHelpTitle.init(UIString(L" "));
    m_layersList.init(0);
    m_layersList.UseIggyIcons();
    m_biomesTab.init(UIString(IDS_BIOMES));
    m_biomesHelpTitle.init(UIString(IDS_ACHIEVEMENTS));
    m_biomesList.init(1);
    m_biomesList.UseIggyIcons();
    PopulateBiomesList();
    m_propertiesTab.init(UIString(IDS_PROPERTIES));
    m_propertiesHelpTitle.init(UIString(IDS_ACHIEVEMENTS));
    m_propertiesList.init(2);
    CreateParamMappings();
    PopulateParamsList();
    m_layersTimer.setVisible(false);
    m_biomesTimer.setVisible(false);
    m_propertiesTimer.setVisible(false);
    m_layersTouch.init(3);
    m_biomesTouch.init(4);
    m_propertiesTouch.init(5);
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_SuperFlatMenu::setLayerButton(int index, const FlatLayerInfo& layer)
{
    unsigned int count = m_layersList.getItemCount();
    if (index > (int)count)
        return;
    if (index < (int)count)
        m_layersList.SetItem(index, layer.getNameText(), layer.getHeightText(), L"");
    else
        m_layersList.AddNewItem(layer.getNameText(), layer.getHeightText(), index, L"");
}

void UIScene_SuperFlatMenu::PopulateLayersList()
{
    std::vector<FlatLayerInfo> layers;
    m_generatorInfo.getLayers(layers);
    m_layersList.clearList();
    int index = 0;
    for (std::vector<FlatLayerInfo>::reverse_iterator it = layers.rbegin(); it != layers.rend(); ++it)
    {
        setLayerButton(index, *it);
        ++index;
    }
}

void UIScene_SuperFlatMenu::UpdateBiomesList()
{
    SuperflatMenu::BiomeInfo* biomes = SuperflatMenu::getBiomeInfoA();
    int index = 0;
    for (int i = 0; biomes[i].mBiome >= 0; ++i)
    {
        bool checked = biomes[i].mBiome == m_generatorInfo.getBiome();
        if (!checked && biomes[i].mBiome == 0 && m_generatorInfo.getBiome() == 0x18)
            checked = true;
        m_biomesList.SetCheckbox(index, checked);
        ++index;
    }
}

void UIScene_SuperFlatMenu::UpdateParamsList()
{
}

void UIScene_SuperFlatMenu::LoadPreset(SuperflatConfig* config)
{
    m_presetsFlag = false;
    *m_config = *config;
    m_generatorInfo.setEqualTo(m_config);
    PopulateLayersList();
    UpdateBiomesList();
    UpdateParamsList();
}

void UIScene_SuperFlatMenu::SetActiveTab(eTabs tab)
{
    FJ_SuperFlatMenu::Create(m_movie->getRootNode())->SetActiveTab(tab);
    m_activeTab = tab;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_SuperFlatMenu::handleTouchInput(unsigned int playerId, int action, int x, int y, bool repeat,
                                             bool released, bool pressed)
{
    if (pressed && !m_editingLayer && action > 2 && action < 6)
    {
        if (m_activeTab != action - 3)
        {
            m_activeTab = static_cast<eTabs>(action - 3);
            updateTooltips();
            SetActiveTab(static_cast<eTabs>(action - 3));
        }
    }
}

UIControl* UIScene_SuperFlatMenu::GetMainPanel()
{
    if (m_activeTab == eTabBiomes)
        return &m_biomesPanel;
    if (m_activeTab != eTabProperties)
        return &m_layersPanel;
    return &m_propertiesPanel;
}

void UIScene_SuperFlatMenu::updateTooltips()
{
    int tooltipIdForPrimaryAction = -1;
    int tooltipIdForSecondaryAction = -1;
    int tooltipIdForLayerAction = -1;
    int tooltipIdForDisabledAction = -1;
    if (!m_editingLayer)
    {
        tooltipIdForDisabledAction = -2;
        tooltipIdForLayerAction = IDS_TOOLTIPS_CHANGE_TAB;
        tooltipIdForSecondaryAction = IDS_TOOLTIPS_PRESETS;
    }
    if (m_activeTab == eTabLayers)
    {
        if (controlHasFocus(UIControl::eUIControlType_LayerIconButtonList))
        {
            unsigned int selection = m_layersList.getCurrentSelection();
            unsigned int count = m_layersList.getItemCount();
            if (m_editingLayer)
            {
                gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_PLACE, IDS_TOOLTIPS_BACK, -1, tooltipIdForSecondaryAction, IDS_TOOLTIPS_PAGE_UP,
                    IDS_TOOLTIPS_PAGE_DOWN, tooltipIdForDisabledAction, tooltipIdForLayerAction, -1, -1, -1, -1, -1, -1, -1, 0x4eb3a99c, false, false);
                return;
            }
            tooltipIdForPrimaryAction = IDS_SUPERFLAT_ADD_LAYER;
            if (selection != count - 1)
            {
                gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_LAYER_OPTIONS, IDS_TOOLTIPS_BACK, IDS_SUPERFLAT_MOVE_LAYER, tooltipIdForSecondaryAction, -1, -1,
                    tooltipIdForDisabledAction, tooltipIdForLayerAction, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
                return;
            }
        }
    }
    else
    {
        if (m_activeTab > eTabProperties)
        {
            gConsoleUIController.SetTooltips(0, -1, IDS_TOOLTIPS_BACK, -1, tooltipIdForSecondaryAction, -1, -1, tooltipIdForDisabledAction, tooltipIdForLayerAction, -1,
                -1, -1, -1, -1, -1, -1, -1, false, false);
            return;
        }
        tooltipIdForPrimaryAction = IDS_TOOLTIPS_SELECT;
    }
    gConsoleUIController.SetTooltips(0, tooltipIdForPrimaryAction, IDS_TOOLTIPS_BACK, -1, tooltipIdForSecondaryAction, -1, -1, tooltipIdForDisabledAction, tooltipIdForLayerAction, -1, -1,
        -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_SuperFlatMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

void UIScene_SuperFlatMenu::tick()
{
    UIScene::tick();
    for (std::vector<TickAction*>::iterator it = m_tickActions.begin(); it != m_tickActions.end(); ++it)
    {
        (*it)->Execute(this);
        delete *it;
    }
    m_tickActions.clear();
}

int UIScene_SuperFlatMenu::GetLayerIndex(int index)
{
    return m_generatorInfo.numLayers() - index - 1;
}

void UIScene_SuperFlatMenu::UpdateLayersHelpText()
{
    int index = GetLayerIndex(m_layersList.getCurrentSelection());
    FlatLayerInfo layer = m_generatorInfo.getLayer(index);
    m_layersHelpTitle.setLabel(UIString(layer.getNameText()), false, false);
    m_layersHelp.setLabel(UIString(layer.getDescriptionText()), false, false);
}

void UIScene_SuperFlatMenu::UpdateBiomeHelpText()
{
    SuperflatMenu::BiomeInfo* biomes = SuperflatMenu::getBiomeInfoA();
    SuperflatMenu::BiomeInfo& biome = biomes[m_biomesList.getCurrentSelection()];
    m_biomesHelpTitle.setLabel(UIString(biome.getName()), false, false);
    m_biomesHelp.setLabel(UIString(L10N::GetString(biome.mDescription)), false, false);
}

void UIScene_SuperFlatMenu::UpdateParamHelpText()
{
}

void UIScene_SuperFlatMenu::handleReload()
{
    int selection = m_pendingSelection;
    _init();
    PopulateLayersList();
    UpdateBiomesList();
    UpdateParamsList();
    FJ_SuperFlatMenu::Create(m_movie->getRootNode())->SetActiveTab(m_activeTab);
    m_pendingSelection = selection;
    if (m_activeTab == eTabLayers)
    {
        m_layersList.setCurrentSelection(selection, false, true);
        UpdateLayersHelpText();
    }
    else if (m_activeTab == eTabBiomes)
    {
        m_biomesList.setCurrentSelection(selection, false, true);
        UpdateBiomeHelpText();
    }
    else if (m_activeTab == eTabProperties)
    {
        m_propertiesList.setCurrentSelection(selection, false, true);
        UpdateParamHelpText();
    }
}

void UIScene_SuperFlatMenu::ExecuteOnTick(MethodAction::Func function)
{
    m_tickActions.push_back(new MethodAction(function));
}

void UIScene_SuperFlatMenu::MoveCurrentLayer(int amount)
{
    if (amount == 0)
        return;
    int direction = (amount > 0) ? -1 : 1;
    int selection = m_layersList.getCurrentSelection();
    int layers = m_generatorInfo.numLayers();
    int maxIndex = layers - 1;
    int target = (layers - (selection + direction * std::abs(amount))) - 1;
    int limit = (layers - selection) - 1;
    if (target > maxIndex)
        target = maxIndex;
    if (target < 1)
        target = 1;
    if (limit > maxIndex)
        limit = maxIndex;
    if (limit < 1)
        limit = 1;
    if (target == limit || limit == 0)
    {
        ExecuteOnTick<const SoundEvent*>(&UIScene_SuperFlatMenu::PlaySound, SoundEvents::UI_PRESS);
        return;
    }
    ExecuteOnTick<const SoundEvent*>(&UIScene_SuperFlatMenu::PlaySound, SoundEvents::UI_PRESS);
    FlatLayerInfo moving = m_generatorInfo.getLayer(limit);
    for (; limit != target; limit -= direction)
        m_generatorInfo.setLayer(limit, m_generatorInfo.getLayer(limit - direction));
    m_generatorInfo.setLayer(target, moving);
    ExecuteOnTick(&UIScene_SuperFlatMenu::PopulateLayersList);
    ExecuteOnTick<int>(&UIScene_SuperFlatMenu::SetSelectedLayer, (layers - target) - 1);
}

void UIScene_SuperFlatMenu::NavActiveTab(int direction)
{
    int tab = (int)m_activeTab + direction;
    while (tab < 0)
        tab += 3;
    while (tab >= 3)
        tab -= 3;
    SetActiveTab((eTabs)tab);
}

int UIScene_SuperFlatMenu::CbGenerationWarning(void* context, int pad, C4JStorage::EMessageResult result)
{
    UIScene_SuperFlatMenu* scene = static_cast<UIScene_SuperFlatMenu*>(context);
    int ret = 0;
    if (result < C4JStorage::EMessageResult_Cancelled)
    {
        ret = -1;
    }
    else if (result != C4JStorage::EMessageResult_Cancelled)
    {
        if (result < C4JStorage::EMessageResult_Decline)
        {
            scene->ReturnResults(false);
            scene->m_generating = false;
            return 0;
        }
        if (result != C4JStorage::EMessageResult_Decline)
            ret = -1;
    }
    scene->m_generating = false;
    return ret;
}

void UIScene_SuperFlatMenu::ReturnResults(bool accepted)
{
    unsigned int warning = SuperflatMenu::CustomSuperflatBlocks::verify(
        static_cast<LaunchMoreOptionsMenuInitData*>(m_initData), m_config);
    if (accepted && warning != 0)
    {
        m_generating = true;
        int params[2] = { static_cast<int>(IDS_CONFIRM_OK), static_cast<int>(IDS_CONFIRM_CANCEL) };
        gConsoleUIController.RequestAlertMessage(static_cast<int>(IDS_SUPERFLAT_CONFIGURATION_PROBLEMS), static_cast<int>(warning),
            params, 2, m_playerId, &UIScene_SuperFlatMenu::CbGenerationWarning, this, NULL, true);
        return;
    }
    SuperflatMenu::CustomSuperflatBlocks::sanitise(m_generatorInfo);
    *reinterpret_cast<SuperflatConfig*>(static_cast<char*>(m_initData) + 0xe0) = *m_config;
    SuperflatMenu::PresetCache::SetCachedPreset(m_playerId, *m_config);
    navigateBack();
}

void UIScene_SuperFlatMenu::handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                                        bool& handled)
{
    if (m_generating)
        return;
    if (!m_tickActions.empty())
        return;
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, released, pressed);
    switch (action)
    {
    case EControllerActions_MenuX:
        if (m_layersList.getCurrentSelection() != m_layersList.getItemCount() - 1 && !m_editingLayer &&
            m_layersList.hasFocus() && pressed)
        {
            m_editingLayer = true;
            updateTooltips();
            handled = true;
            return;
        }
        break;
    case EControllerActions_MenuY:
        if (!m_editingLayer && pressed)
        {
            ExecuteOnTick(&UIScene_SuperFlatMenu::OpenPresetsDialog);
            handled = true;
            return;
        }
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
        if (m_editingLayer && pressed)
        {
            if (action == EControllerActions_MenuDown)
                MoveCurrentLayer(-1);
            else if (action != EControllerActions_MenuPageUp)
                MoveCurrentLayer(action == EControllerActions_MenuPageDown ? -10 : 1);
            else
                MoveCurrentLayer(10);
            return;
        }
        sendInputToMovie(action, repeat, released, pressed);
        handled = true;
        return;
    case EControllerActions_MenuB:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuOtherStickUp:
        if (!m_editingLayer)
        {
            sendInputToMovie(action, repeat, released, pressed);
            handled = true;
        }
        return;
    case EControllerActions_MenuRightScroll:
    case EControllerActions_MenuLeftScroll:
        if (!m_editingLayer && pressed)
        {
            NavActiveTab(action == EControllerActions_MenuRightScroll ? 1 : -1);
            updateTooltips();
            handled = true;
            return;
        }
        break;
    case EControllerActions_MenuOtherStickDown:
        if (!m_editingLayer)
        {
            sendInputToMovie(action, repeat, released, pressed);
            handled = true;
        }
        break;
    case EControllerActions_MenuOk:
        if (m_editingLayer && pressed)
        {
            m_editingLayer = false;
            handled = true;
            updateTooltips();
            return;
        }
        sendInputToMovie(action, repeat, released, pressed);
        handled = true;
        return;
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            ReturnResults(true);
            handled = true;
        }
        break;
    default:
        break;
    }
}

int UIScene_SuperFlatMenu::CbLayerOperation(void* context, int pad, C4JStorage::EMessageResult result)
{
    UIScene_SuperFlatMenu* scene = static_cast<UIScene_SuperFlatMenu*>(context);
    int ret = 0;
    if (result < C4JStorage::EMessageResult_Cancelled)
    {
        ret = -1;
    }
    else if (result != C4JStorage::EMessageResult_Cancelled)
    {
        if (result < C4JStorage::EMessageResult_Decline)
        {
            scene->ExecuteOnTick<bool>(&UIScene_SuperFlatMenu::OpenEditLayerDialog, false);
            scene->m_generating = false;
            return 0;
        }
        if (result == C4JStorage::EMessageResult_Decline)
        {
            if (scene->m_generatorInfo.getHeight() < 0x80)
            {
                scene->ExecuteOnTick<bool>(&UIScene_SuperFlatMenu::OpenEditLayerDialog, true);
                scene->m_generating = false;
                return 0;
            }
            scene->ExecuteOnTick(&UIScene_SuperFlatMenu::OpenMaxHeightDialog);
            scene->m_generating = false;
            return 0;
        }
        if (result == C4JStorage::EMessageResult_ThirdOption)
        {
            scene->ExecuteOnTick(&UIScene_SuperFlatMenu::DeleteLayer);
            scene->m_generating = false;
            return 0;
        }
        ret = -1;
    }
    scene->m_generating = false;
    return ret;
}

void UIScene_SuperFlatMenu::ChangeBiome(int biome)
{
    SuperflatMenu::BiomeInfo* biomes = SuperflatMenu::getBiomeInfoA();
    m_generatorInfo.setBiome(biomes[biome].mBiome);
    ExecuteOnTick(&UIScene_SuperFlatMenu::UpdateBiomesList);
}

void UIScene_SuperFlatMenu::HandleChangeParamList(int index)
{
    ExecuteOnTick<int>(&UIScene_SuperFlatMenu::WritePropertyChanges, index);
}

void UIScene_SuperFlatMenu::handlePress(int controlId, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (controlId == 0)
    {
        if (!m_editingLayer)
        {
            unsigned int selection = m_layersList.getCurrentSelection();
            unsigned int count = m_layersList.getItemCount();
            if (selection != count - 1)
            {
                int params[3] = { static_cast<int>(IDS_SUPERFLAT_EDIT_LAYER), static_cast<int>(IDS_SUPERFLAT_ADD_LAYER),
                                  static_cast<int>(IDS_SUPERFLAT_DELETE_LAYER) };
                gConsoleUIController.RequestAlertMessage(static_cast<int>(IDS_SUPERFLAT_LAYER_OPTIONS_TITLE),
                    static_cast<int>(0x78a7e3b3U), params, 3, m_playerId,
                    &UIScene_SuperFlatMenu::CbLayerOperation, this, NULL, true);
                m_generating = true;
                return;
            }
            if (m_generatorInfo.getHeight() < 0x80)
            {
                ExecuteOnTick<bool>(&UIScene_SuperFlatMenu::OpenEditLayerDialog, true);
                return;
            }
            ExecuteOnTick(&UIScene_SuperFlatMenu::OpenMaxHeightDialog);
        }
    }
    else if (controlId == 1)
    {
        ChangeBiome(childId);
    }
    else if (controlId == 2)
    {
        HandleChangeParamList(childId);
    }
}

void UIScene_SuperFlatMenu::handleCheckboxElementToggled(int first, int second, bool checked)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (first == 2)
        HandleChangeParamList(second);
}

void UIScene_SuperFlatMenu::handleSliderElementMove(int first, int second, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    if (first == 2)
        HandleChangeParamList(second);
}

void UIScene_SuperFlatMenu::handleSelectionChanged(int selection)
{
    if (m_activeTab == eTabBiomes)
    {
        SuperflatMenu::BiomeInfo* biomes = SuperflatMenu::getBiomeInfoA();
        m_biomesHelp.setLabel(UIString(biomes[selection].mDescription), false, false);
        return;
    }
    if (m_activeTab == eTabProperties)
    {
        UpdateParamHelpText();
        return;
    }
    if (m_activeTab == eTabLayers)
        controlHasFocus(&m_layersList);
}

void UIScene_SuperFlatMenu::handleFocusChange(int focus, int childFocus)
{
    UIControl_List* list;
    if (focus == eTabLayers)
        list = &m_layersList;
    else if (focus == eTabBiomes)
        list = &m_biomesList;
    else if (focus != eTabProperties)
    {
        m_pendingSelection = static_cast<UIControl_List*>(NULL)->getListIndex(childFocus);
        updateTooltips();
        return;
    }
    else
        list = &m_propertiesList;
    list->updateChildFocus(childFocus);
    m_pendingSelection = list->getListIndex(childFocus);
    if (focus == eTabLayers)
        ExecuteOnTick(&UIScene_SuperFlatMenu::UpdateLayersHelpText);
    else if (focus == eTabBiomes)
        ExecuteOnTick(&UIScene_SuperFlatMenu::UpdateBiomeHelpText);
    else
        ExecuteOnTick(&UIScene_SuperFlatMenu::UpdateParamHelpText);
    updateTooltips();
}

void UIScene_SuperFlatMenu::handleGainFocus(bool hadFocus)
{
    UIScene::handleGainFocus(hadFocus);
    if (hadFocus)
    {
        SetActiveTab(eTabLayers);
        PopulateLayersList();
        UpdateBiomesList();
        UpdateParamsList();
    }
}

void UIScene_SuperFlatMenu::customDraw(const char* name, fuiRect* rect)
{
    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    int slot = -1;
    if (m_activeTab == eTabLayers)
        std::sscanf(name, "slot_0_%d", &slot);
    if (m_activeTab == eTabBiomes)
    {
        std::sscanf(name, "slot_1_%d", &slot);
        if (slot != -1)
            item = SuperflatMenu::getBiomeInfoA()[slot].mItem;
    }
    if (!item->isEmpty())
        customDrawSlotControlFui(rect, m_playerId, item, 1.0f, false, false, false);
}

void UIScene_SuperFlatMenu::SetSelectedLayer(int index)
{
    if (m_layersList.hasFocus() && index < (int)m_layersList.getItemCount())
        m_layersList.setCurrentSelection(index, false, true);
}

void UIScene_SuperFlatMenu::DeleteLayer()
{
    int index = GetLayerIndex(m_layersList.getCurrentSelection());
    m_generatorInfo.remLayer(index);
    PopulateLayersList();
}

void UIScene_SuperFlatMenu::WritePropertyChanges(int index)
{
    m_paramMappings[index]->writeConfig(m_generatorInfo);
}

void UIScene_SuperFlatMenu::PlaySound(const SoundEvent* sound)
{
    gConsoleUIController.PlayUISFX(sound);
}

void UIScene_SuperFlatMenu::OpenPresetsDialog()
{
    struct PresetsData { bool flag; SuperflatConfig* config; };
    PresetsData* data = new PresetsData();
    data->flag = m_presetsFlag;
    data->config = m_config;
    gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SuperFlatPresets, data,
        EUILayer_Scene, EUIGroup_PlayerDefault);
}

void UIScene_SuperFlatMenu::OpenEditLayerDialog(bool insert)
{
    struct EditLayerData { bool insert; int index; SuperflatConfig* config; };
    EditLayerData* data = new EditLayerData();
    data->insert = insert;
    data->index = (m_generatorInfo.numLayers() - 1) - m_layersList.getCurrentSelection();
    data->config = m_config;
    gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SuperFlatEditLayer, data,
        EUILayer_Scene, EUIGroup_PlayerDefault);
}

void UIScene_SuperFlatMenu::OpenMaxHeightDialog()
{
    int params[1] = { static_cast<int>(IDS_CONFIRM_OK) };
    gConsoleUIController.RequestAlertMessage(static_cast<int>(0xf2637855U), static_cast<int>(0x373ccbf0U),
        params, 1, m_playerId, NULL, NULL, NULL, true);
}

std::wstring UIScene_SuperFlatMenu::getMoviePath()
{
    return std::wstring(L"SuperFlatMenu");
}

int UIScene_SuperFlatMenu::getSceneType()
{
    return EUIScene_SuperFlatMenu;
}

template void UIScene_SuperFlatMenu::ExecuteOnTick<const SoundEvent*>(MethodActionArg1<const SoundEvent*>::Func function, const SoundEvent* value);
template void UIScene_SuperFlatMenu::ExecuteOnTick<int>(MethodActionArg1<int>::Func function, int value);
template void UIScene_SuperFlatMenu::ExecuteOnTick<bool>(MethodActionArg1<bool>::Func function, bool value);
