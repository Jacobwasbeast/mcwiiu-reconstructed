#ifndef GAME_UI_UISCENE_SUPERFLATMENU_HPP
#define GAME_UI_UISCENE_SUPERFLATMENU_HPP

#include <string>
#include <vector>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_CheckboxButtonList.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_LayerIconButtonList.hpp"
#include "game/ui/UIControl_MultiList.hpp"
#include "game/ui/UIControl_Touch.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/core/ConsoleUIController.hpp"

class FlatLayerInfo;
class SoundEvent;

class UIScene_SuperFlatMenu : public UIScene
{
public:
    enum eTabs { eTabLayers, eTabBiomes, eTabProperties };

    class TickAction
    {
    public:
        virtual ~TickAction();
        virtual void Execute(UIScene_SuperFlatMenu* scene) = 0;
    };

    class MethodAction : public TickAction
    {
    public:
        typedef void (UIScene_SuperFlatMenu::*Func)();
        MethodAction(Func function);
        virtual ~MethodAction();
        virtual void Execute(UIScene_SuperFlatMenu* scene);
    private:
        Func m_function;
    };

    template<class T>
    class MethodActionArg1 : public TickAction
    {
    public:
        typedef void (UIScene_SuperFlatMenu::*Func)(T);
        MethodActionArg1(Func function, T value) : m_function(function), m_value(value) {}
        virtual ~MethodActionArg1() {}
        virtual void Execute(UIScene_SuperFlatMenu* scene) { (scene->*m_function)(m_value); }
    private:
        Func m_function;
        T m_value;
    };

    UIScene_SuperFlatMenu(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_SuperFlatMenu();
    virtual unsigned int mapElementsAndNames();
    void PopulateBiomesList();
    int ParentParam(int index);
    void CreateParamMappings();
    void PopulateParamsList();
    void _init();
    void setLayerButton(int index, const FlatLayerInfo& layer);
    void PopulateLayersList();
    void UpdateBiomesList();
    void UpdateParamsList();
    void LoadPreset(SuperflatConfig* config);
    void SetActiveTab(eTabs tab);
    virtual void handleTouchInput(unsigned int playerId, int action, int x, int y, bool repeat, bool released, bool pressed);
    virtual unsigned int GetMainPanel();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void tick();
    int GetLayerIndex(int index);
    void UpdateLayersHelpText();
    void UpdateBiomeHelpText();
    void UpdateParamHelpText();
    virtual void handleReload();
    void ExecuteOnTick(MethodAction::Func function);
    template<class T>
    void ExecuteOnTick(typename MethodActionArg1<T>::Func function, T value);
    void MoveCurrentLayer(int amount);
    void NavActiveTab(int direction);
    static int CbGenerationWarning(void* context, int button, C4JStorage::EMessageResult result);
    void ReturnResults(bool accepted);
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed, bool& handled);
    static int CbLayerOperation(void* context, int button, C4JStorage::EMessageResult result);
    void ChangeBiome(int biome);
    void HandleChangeParamList(int index);
    virtual void handlePress(int first, int second);
    virtual void handleCheckboxElementToggled(int first, int second, bool checked);
    virtual void handleSliderElementMove(int first, int second, int value);
    virtual void handleSelectionChanged(int selection);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handleGainFocus(bool hadFocus);
    virtual void customDraw(const char* name, fuiRect* rect);
    unsigned int SetSelectedLayer(int index);
    void DeleteLayer();
    void WritePropertyChanges(int index);
    void PlaySound(const SoundEvent* sound);
    unsigned int OpenPresetsDialog();
    unsigned int OpenEditLayerDialog(bool insert);
    unsigned int OpenMaxHeightDialog();
    virtual std::wstring getMoviePath();
    virtual int getSceneType();
    void InsertNewLayer();
    static int CbPresetName(void* context, bool accepted);

private:
    std::vector<TickAction*> m_tickActions;
    int m_activeTab;
    int m_selectedLayer;
    int m_selectedBiome;
    int m_selectedParam;
    UIControl_Label m_layersTab;
    UIControl m_layersPanel;
    UIControl_LayerIconButtonList m_layersList;
    UIControl_Touch m_layersTouch;
    UIControl_Label m_layersHelpTitle;
    UIControl_HTMLLabel m_layersHelp;
    UIControl m_layersTimer;
    UIControl_Label m_biomesTab;
    UIControl m_biomesPanel;
    UIControl_CheckboxButtonList m_biomesList;
    UIControl_Touch m_biomesTouch;
    UIControl_Label m_biomesHelpTitle;
    UIControl_HTMLLabel m_biomesHelp;
    UIControl m_biomesTimer;
    UIControl_Label m_propertiesTab;
    UIControl m_propertiesPanel;
    UIControl_MultiList m_propertiesList;
    UIControl_Touch m_propertiesTouch;
    UIControl_Label m_propertiesHelpTitle;
    UIControl_HTMLLabel m_propertiesHelp;
    UIControl m_propertiesTimer;
    bool m_generating;
    bool m_editingLayer;
    int m_currentPanel;
    int m_pendingSelection;
    std::vector<int> m_paramParents;
    std::vector<int> m_paramMappings;
    SuperflatConfig* m_config;
    FlatGeneratorInfo m_generatorInfo;
    void* m_initData;
};

template<class T>
void UIScene_SuperFlatMenu::ExecuteOnTick(typename MethodActionArg1<T>::Func function, T value)
{
    m_tickActions.push_back(new MethodActionArg1<T>(function, value));
}

#endif
