#include "game/ui/UIScene_SuperFlatPresets.hpp"
#include "game/core/StringIds.hpp"

#include <cstdio>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DefaultPresets.hpp"
#include "game/core/L10N.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/core/GameSettings.hpp"

UIScene_SuperFlatPresets::UIScene_SuperFlatPresets(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer), m_generator(static_cast<SuperflatConfig*>(0)),
      m_functionPresetIds(), m_customPresets(), m_defaultPresets(), m_presetList(),
      m_presetTitle(), m_selectedPreset(-1)
{
    SuperflatConfig* config = static_cast<SuperflatConfig**>(data)[1];
    m_generator = FlatGeneratorInfo(config, false);
    initialiseMovie();
    m_presetTitle.init(UIString(IDS_TOOLTIPS_PRESETS));
    m_presetList.init(0);
    m_presetList.UseIggyIcons();

    const DefaultPresets::Preset presets[] = {
        DefaultPresets::CLASSIC,
        DefaultPresets::TUNNELERS_DREAM,
        DefaultPresets::WATER_WORLD,
        DefaultPresets::OVERWORLD,
        DefaultPresets::SNOWY_KINGDOM,
        DefaultPresets::DESERT,
        DefaultPresets::REDSTONE_READY
    };
    unsigned int i;
    for (i = 0; i < sizeof(presets) / sizeof(presets[0]); ++i)
    {
        SuperflatConfig* preset = new SuperflatConfig();
        preset->makePreset(presets[i]);
        m_defaultPresets.push_back(preset);
    }
    PopulatePresetList();
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_SuperFlatPresets::~UIScene_SuperFlatPresets()
{
    std::vector<SuperflatConfig*>::iterator it;
    for (it = m_customPresets.begin(); it != m_customPresets.end(); ++it)
        delete *it;
    for (it = m_defaultPresets.begin(); it != m_defaultPresets.end(); ++it)
        delete *it;
}

unsigned int UIScene_SuperFlatPresets::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_presetList.setupControl(this, root, "PresetsList");
    m_controls.push_back(&m_presetList);
    m_presetTitle.setupControl(this, root, "PresetsTitle");
    m_controls.push_back(&m_presetTitle);
    return 1;
}

int UIScene_SuperFlatPresets::GetPresetCount() const
{
    return static_cast<int>(m_functionPresetIds.size() + m_customPresets.size() +
                            m_defaultPresets.size());
}

int UIScene_SuperFlatPresets::SafeGetIndex(unsigned int count, int start, int index) const
{
    if (index >= start && index < start + static_cast<int>(count))
        return index - start;
    return -1;
}

int UIScene_SuperFlatPresets::GetFunctionPresetId(int index) const
{
    return SafeGetIndex(static_cast<unsigned int>(m_functionPresetIds.size()), 0, index);
}

int UIScene_SuperFlatPresets::GetCustomPresetId(int index) const
{
    return SafeGetIndex(static_cast<unsigned int>(m_customPresets.size()),
                        static_cast<int>(m_functionPresetIds.size()), index);
}

int UIScene_SuperFlatPresets::GetDefaultPresetId(int index) const
{
    return SafeGetIndex(static_cast<unsigned int>(m_defaultPresets.size()),
                        static_cast<int>(m_functionPresetIds.size() + m_customPresets.size()), index);
}

std::wstring UIScene_SuperFlatPresets::GetDisplayName(int index) const
{
    int functionId = GetFunctionPresetId(index);
    int customId = GetCustomPresetId(index);
    int defaultId = GetDefaultPresetId(index);
    if (functionId == 0)
        return L10N::GetString(0x477A886A);
    if (customId >= 0)
        return m_customPresets[customId]->getDisplayname();
    if (defaultId < 0)
        return L"[UNKNOWN_PRESET]";

    static const unsigned int displayNames[] = {
        IDS_PRESET_CLASSIC_FLAT, 0x1D3AAD7A, IDS_PRESET_WATER_WORLD, IDS_PRESET_OVERWORLD,
        IDS_PRESET_SNOWY_KINGDOM, IDS_PRESET_DESERT, IDS_PRESET_REDSTONE_READY
    };
    if (defaultId < static_cast<int>(sizeof(displayNames) / sizeof(displayNames[0])))
        return L10N::GetString(static_cast<int>(displayNames[defaultId]));
    return L"[UNKNOWN_PRESET]";
}

void UIScene_SuperFlatPresets::PopulatePresetList()
{
    m_presetList.clearList();
    int i;
    for (i = 0; i < GetPresetCount(); ++i)
        m_presetList.AddNewItem(GetDisplayName(i), i, L"slot_0_%d");
}

unsigned int UIScene_SuperFlatPresets::GetFunction(int function) const
{
    int selection = m_presetList.getCurrentSelection();
    int functionId = GetFunctionPresetId(selection);
    int customId = GetCustomPresetId(selection);
    int defaultId = GetDefaultPresetId(selection);
    if (functionId < 0)
    {
        if (customId >= 0 || defaultId >= 0)
        {
            if (function == 0)
                return 1;
            if (customId >= 0 && function > 1 && function < 4)
                return static_cast<unsigned int>(function);
        }
    }
    else if (functionId == 0 && function == 0)
        return static_cast<unsigned int>(function);
    return 0xFFFFFFFF;
}

int UIScene_SuperFlatPresets::GetTooltip(int function) const
{
    static const unsigned int tooltips[] = { IDS_SUPERFLAT_SAVE_PRESET, IDS_SUPERFLAT_LOAD_PRESET, IDS_SUPERFLAT_OVERWRITE, IDS_SUPERFLAT_DELETE_PRESET };
    unsigned int result = GetFunction(function);
    return result < 4 ? static_cast<int>(tooltips[result]) : -1;
}

void UIScene_SuperFlatPresets::updateTooltips()
{
    gConsoleUIController.SetTooltips(0, GetTooltip(0), 0x04F6BCF5, GetTooltip(2), GetTooltip(3),
                                      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_SuperFlatPresets::updateComponents()
{
}

SuperflatConfig* UIScene_SuperFlatPresets::GetSuperflatConfig(int index) const
{
    int functionId = GetFunctionPresetId(index);
    int customId = GetCustomPresetId(index);
    int defaultId = GetDefaultPresetId(index);
    if (functionId < 0)
    {
        if (customId >= 0)
            return m_customPresets[customId];
        if (defaultId >= 0)
            return m_defaultPresets[defaultId];
    }
    return 0;
}

int UIScene_SuperFlatPresets::LoadConfirmMessageCallback(void* data, int, C4JStorage::EMessageResult result)
{
    UIScene_SuperFlatPresets* scene = static_cast<UIScene_SuperFlatPresets*>(data);
    if (result == C4JStorage::EMessageResult_Accept)
    {
        scene->m_generator.setEqualTo(scene->GetSuperflatConfig(scene->m_selectedPreset));
        gConsoleUIController.NavigateBack(scene->m_playerId, false, EUIScene_None,
                                           EUILayer_Count);
    }
    return 0;
}

void UIScene_SuperFlatPresets::HandlePresetListPress(int function)
{
    int selection = m_presetList.getCurrentSelection();
    if (GetFunction(function) == 1)
    {
        int parameters[2] = { static_cast<int>(IDS_CONFIRM_OK), static_cast<int>(IDS_CONFIRM_CANCEL) };
        m_selectedPreset = selection;
        gConsoleUIController.RequestAlertMessage(IDS_SUPERFLAT_LOAD_PRESET, 0xCD41C121, parameters, 2,
                                                  m_playerId, LoadConfirmMessageCallback, this, 0, true);
    }
}

void UIScene_SuperFlatPresets::handleInput(int, int action, bool repeat, bool released, bool pressed,
                                           bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    switch (action)
    {
    case EControllerActions_MenuX:
    case EControllerActions_MenuY:
        if (released)
        {
            HandlePresetListPress(action);
            handled = true;
        }
        break;
    case EControllerActions_MenuCancel:
        if (released)
        {
            navigateBack();
            handled = true;
        }
        break;
    default:
        sendInputToMovie(action, repeat, released, pressed);
        break;
    }
}

void UIScene_SuperFlatPresets::handleFocusChange(int, int childFocus)
{
    m_presetList.updateChildFocus(childFocus);
    updateTooltips();
}

void UIScene_SuperFlatPresets::handlePress(int controlId, int)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (controlId == 0)
        HandlePresetListPress(0);
}

int UIScene_SuperFlatPresets::GetIcon(int index) const
{
    int functionId = GetFunctionPresetId(index);
    int customId = GetCustomPresetId(index);
    int defaultId = GetDefaultPresetId(index);
    if (functionId == 0)
        return Item::getBlockId(Item::byBlock(Blocks::crafting_table));
    if (customId >= 0)
        return Item::getId(Items::painting);
    static const int icons[] = { 2, 1, 9, 31, 78, 12, 331 };
    if (defaultId >= 0 && defaultId < static_cast<int>(sizeof(icons) / sizeof(icons[0])))
        return icons[defaultId];
    return Item::getBlockId(Item::byBlock(Blocks::command_block));
}

void UIScene_SuperFlatPresets::customDraw(const char* name, fuiRect* rect)
{
    int slot = -1;
    std::sscanf(name, "slot_0_%d", &slot);
    if (slot != -1)
    {
        not_null_ptr<ItemInstance> item(new ItemInstance(Item::byId(GetIcon(slot)), 1, 0));
        if (!item->isEmpty())
            customDrawSlotControlFui(rect, m_playerId, item, 1.0f, false, false, false);
    }
}

std::wstring UIScene_SuperFlatPresets::getMoviePath()
{
    return L"SuperFlatPresets";
}

int UIScene_SuperFlatPresets::getSceneType()
{
    return EUIScene_SuperFlatPresets;
}
