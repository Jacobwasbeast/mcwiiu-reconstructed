#include "game/ui/UIScene_BeaconMenu.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/Packet.hpp"
#include "game/network/ServerboundCustomPayloadPacket.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/BeaconMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"

#include <cstdio>

extern CMinecraftApp* g_pApp;

struct BeaconMenuData
{
    boost::shared_ptr<Container> inventory;
    boost::shared_ptr<Container> beacon;
    int playerIndex;
    bool splitscreen;
};

UIScene_BeaconMenu::UIScene_BeaconMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_activatorSlot(), m_activatorList(),
      m_primaryPowerLabel(), m_secondaryPowerLabel(), m_powerButtons(), m_confirmButton(),
      m_beaconContainer(), m_primaryEffect(-1), m_secondaryEffect(-1), m_beaconLevel(-1)
{
    BeaconMenuData* sceneData = static_cast<BeaconMenuData*>(data);
    initialiseMovie();
    m_primaryPowerLabel.init(UIString(0xb3c82373));
    m_secondaryPowerLabel.init(UIString(0x7b8c1d5d));
    int index;
    for (index = 0; index < 7; ++index)
        m_powerButtons[index].setVisible(false);
    if (sceneData != 0)
    {
        if (Minecraft::GetInstance()->mGameModes[sceneData->playerIndex] != 0)
        {
            Tutorial* tutorial =
                Minecraft::GetInstance()->mGameModes[sceneData->playerIndex]->getTutorial();
            if (tutorial != 0)
            {
                m_previousTutorialState = tutorial->currentState;
                tutorial->changeTutorialState(static_cast<eTutorial_State>(0x22), this);
            }
        }
        m_beaconContainer = sceneData->beacon;
        Initialize(false, sceneData->playerIndex,
                   new BeaconMenu(sceneData->inventory, sceneData->beacon), true, 1,
                   static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x38),
                   static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x43), false);
        m_activatorSlot.addSlots(0, 1);
        m_activatorList.addSlots(static_cast<unsigned int>(m_menu->m_slots.size()), 4);
        delete sceneData;
    }
}

UIScene_BeaconMenu::~UIScene_BeaconMenu()
{
}

bool UIScene_BeaconMenu::IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    return section > static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x37) &&
           section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3b);
}

unsigned int UIScene_BeaconMenu::GetPowerButtonId(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3b) ||
        section > static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x41))
        return 0;
    return m_powerButtons[static_cast<int>(section) - 0x3b].GetData();
}

unsigned int UIScene_BeaconMenu::IsVisible(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    return section != static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x41) ||
           GetPowerButtonId(section) != 0;
}

unsigned int UIScene_BeaconMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    static const unsigned char columns[] = { 9, 9, 1 };
    if (section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x38) ||
        section > static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3a))
        return 0;
    return columns[static_cast<int>(section) - 0x38];
}

unsigned int UIScene_BeaconMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x38)) return 1;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x39)) return GetInventoryRows();
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3a)) return 1;
    return 0;
}

unsigned int UIScene_BeaconMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection target = section;
    int dx = 0;
    const int value = static_cast<int>(section);

    if (value == 0x38)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            if (*x < 2) target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3b);
            else if (*x < 5) target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3c);
            else if (*x < 9) target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x40);
            else target = GetPowerButtonId(static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x41)) > 0 ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x41) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x40);
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (m_inventorySmall == 0)
                target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x39);
            else if (*x < 4)
                target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3f);
            else if (*x > 6)
                target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x42);
            else
                target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3a);
        }
    }
    else if (value == 0x39)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x38);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (*x < 4) target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3f);
            else if (*x > 6) target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x42);
            else target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3a);
        }
    }
    else if (value == 0x3a)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_inventorySmall != 0 ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x38) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x39);
            dx = -5;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x40);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left ||
                 state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x42);
    }
    else if (value == 0x3b)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3d);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x38);
            dx = -1;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left ||
                 state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3c);
    }
    else if (value == 0x3c)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3e);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x38);
            dx = -3;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left ||
                 state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3b);
    }
    else if (value == 0x3d)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3f);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3b);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = GetPowerButtonId(static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x41)) > 0 ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x41) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x40);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3e);
    }
    else if (value == 0x3e)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3f);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3c);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3d);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x40);
    }
    else if (value == 0x3f)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_inventorySmall != 0 ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x38) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x39);
            dx = -3;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3d);
    }
    else if (value == 0x40)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x38);
            dx = -7;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3e);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3e);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = GetPowerButtonId(static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x41)) > 0 ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x41) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3d);
    }
    else if (value == 0x41)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x38);
            dx = -8;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x40);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x40);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3d);
    }
    else if (value == 0x42)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_inventorySmall != 0 ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x38) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x39);
            dx = -8;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x40);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left ||
                 state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3a);
    }

    updateSlotPosition(section, target, state, x, y, dx, 0);
    return static_cast<unsigned int>(target);
}

void UIScene_BeaconMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    UIControl* control = getSection(section);
    if (control != 0)
    {
        position->x = static_cast<float>(control->getXPos());
        position->y = static_cast<float>(control->getYPos());
    }
}

void UIScene_BeaconMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                            _UIVec2D* position, _UIVec2D* size)
{
    UIControl* control = getSection(section);
    if (control == 0 || position == 0 || size == 0)
        return;
    const float width = static_cast<float>(control->getWidth());
    const float height = static_cast<float>(control->getHeight());
    if (!IsSectionSlotList(section))
    {
        GetPositionOfSection(section, position);
        size->x = width;
        size->y = height;
        return;
    }
    unsigned int columns = getSectionColumns(section);
    unsigned int rows = getSectionRows(section);
    if (columns == 0 || rows == 0)
        return;
    size->x = width / columns;
    size->y = height / rows;
    position->x = static_cast<float>(slot % static_cast<int>(columns)) * size->x;
    position->y = static_cast<float>(slot / static_cast<int>(columns)) * size->y;
}

unsigned int UIScene_BeaconMenu::GetPowerButtonTier(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    static const unsigned char tiers[] = { 1, 1, 2, 3, 3 };
    if (section <= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3c) ||
        section >= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x42))
        return 0;
    return tiers[static_cast<int>(section) - 0x3d];
}

MobEffect* UIScene_BeaconMenu::GetPowerButtonEffect(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3b)) return MobEffects::MOVEMENT_SPEED;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3c)) return MobEffects::DIG_SPEED;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3d)) return MobEffects::DAMAGE_RESISTANCE;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3e)) return MobEffects::JUMP;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3f)) return MobEffects::DAMAGE_BOOST;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x40)) return MobEffects::REGENERATION;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x41) && m_beaconContainer)
        return MobEffect::byId(m_beaconContainer->getContainerData(1));
    return 0;
}

bool UIScene_BeaconMenu::IsPowerButtonSelected(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3b) ||
        section > static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x41))
        return false;
    return m_powerButtons[static_cast<int>(section) - 0x3b].IsButtonSelected() != 0;
}

void UIScene_BeaconMenu::SetPowerButtonSelected(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int i;
    if (section >= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3b) &&
        section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x40))
        for (i = 0; i < 5; ++i) m_powerButtons[i].SetButtonSelected(false);
    else if (section >= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x40) &&
             section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x42))
        for (i = 5; i < 7; ++i) m_powerButtons[i].SetButtonSelected(false);
    if (section >= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3b) &&
        section <= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x41))
        m_powerButtons[static_cast<int>(section) - 0x3b].SetButtonSelected(true);
}

void UIScene_BeaconMenu::AddPowerButton(int id, int icon, int tier, int column, bool active, bool selected)
{
    UIControl_BeaconEffectButton* button = 0;
    if (tier == 0) button = &m_powerButtons[column == 0 ? 0 : 1];
    else if (tier == 1) button = &m_powerButtons[column == 0 ? 2 : 3];
    else if (tier == 2) button = &m_powerButtons[4];
    else if (tier == 3) button = &m_powerButtons[column == 0 ? 5 : 6];
    if (button != 0)
    {
        button->SetData(id, icon, active, selected);
        button->setVisible(true);
    }
}

void UIScene_BeaconMenu::handleOtherClicked(int,
                                            IUIScene_AbstractContainerMenu::ESceneSection section,
                                            int, bool)
{
    if (section >= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3b) &&
        section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x42))
    {
        if (IsPowerButtonSelected(section))
            return;
        const int tier = GetPowerButtonTier(section);
        MobEffect* effect = GetPowerButtonEffect(section);
        if (m_beaconContainer->getContainerData(0) <= tier)
            return;
        const int effectId = MobEffect::getId(effect);
        if (tier < 3)
        {
            m_beaconContainer->setContainerData(1, effectId);
            if (IsPowerButtonSelected(static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x41)))
                m_beaconContainer->setContainerData(2, effectId);
        }
        else
            m_beaconContainer->setContainerData(2, effectId);
        SetPowerButtonSelected(section);

        const int primaryId = m_beaconContainer->getContainerData(1);
        if (primaryId > 0)
        {
            MobEffect* primary = MobEffect::byId(primaryId);
            const bool active = m_beaconContainer->getContainerData(0) >= 4;
            const bool selected = active && primaryId == m_beaconContainer->getContainerData(2);
            AddPowerButton(m_nextButtonId, primary->getIcon(), 3, 1, active, selected);
        }
        return;
    }
    if (section != static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x42))
        return;

    const int primaryId = m_beaconContainer->getContainerData(1);
    const int secondaryId = m_beaconContainer->getContainerData(2);
    if (m_beaconContainer->getItem(0)->isEmpty() ||
        (m_primaryEffect == primaryId && m_secondaryEffect == secondaryId))
        return;

    ByteArrayOutputStream bytes;
    DataOutputStream output(&bytes);
    output.writeInt(primaryId);
    output.writeInt(secondaryId);
    ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(getPad());
    if (connection != 0)
        connection->send(boost::shared_ptr<Packet>(new ServerboundCustomPayloadPacket(
            std::wstring(L"MC|Beacon"), bytes.toByteArray())));
    m_primaryEffect = primaryId;
    m_secondaryEffect = secondaryId;
    m_beaconContainer->setContainerData(1, primaryId);
}

int UIScene_BeaconMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    static const unsigned char offsets[] = { 0x1c, 1, 0 };
    return section >= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x38) &&
           section <= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3a) ?
           offsets[static_cast<int>(section) - 0x38] : 0;
}

void UIScene_BeaconMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    if (wrap)
    {
        UIControl_SlotList* list = section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x38) ?
            &m_hotbarList : section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x39) ?
            &m_inventoryList : section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3a) ?
            &m_activatorSlot : 0;
        if (list != 0) list->setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
    }
}

std::vector<HtmlString>* UIScene_BeaconMenu::GetSectionHoverText(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3b) ||
        section > static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x41)) return 0;
    std::wstring suffix;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x41))
    {
        if (GetPowerButtonId(section) == 0)
            return 0;
        suffix = L" II";
    }
    MobEffect* effect = GetPowerButtonEffect(section);
    std::vector<HtmlString>* text = new std::vector<HtmlString>();
    text->push_back(HtmlString(std::wstring(L10N::GetString(effect->getDescriptionId(-1))) + suffix,
                               static_cast<eMinecraftColour>(0x1b5), false, false, 0));
    return text;
}

unsigned int UIScene_BeaconMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_activatorSlot.setupControl(this, panel, "ActivatorSlot"); m_controls.push_back(&m_activatorSlot);
    m_activatorList.setupControl(this, panel, "ActivatorList"); m_controls.push_back(&m_activatorList);
    static const char* names[] = { "Primary_Slot_01", "Primary_Slot_02", "Primary_Slot_03",
                                   "Primary_Slot_04", "Primary_Slot_05", "Secondary_Slot_01",
                                   "Secondary_Slot_02" };
    int index;
    for (index = 0; index < 7; ++index)
    {
        m_powerButtons[index].setupControl(this, panel, names[index]);
        m_controls.push_back(&m_powerButtons[index]);
    }
    m_confirmButton.setupControl(this, panel, "ConfirmButton"); m_controls.push_back(&m_confirmButton);
    m_primaryPowerLabel.setupControl(this, panel, "PrimaryPowerLabel"); m_controls.push_back(&m_primaryPowerLabel);
    m_secondaryPowerLabel.setupControl(this, panel, "SecondaryPowerLabel"); m_controls.push_back(&m_secondaryPowerLabel);
    return 1;
}

std::wstring UIScene_BeaconMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"BeaconMenu") : std::wstring(L"BeaconMenuSplit");
}

void UIScene_BeaconMenu::handleReload()
{
    Initialize(true, m_playerId, m_menu, true, 1,
               static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x38),
               static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x43), false);
    m_activatorSlot.addSlots(0, 1);
    m_activatorList.addSlots(static_cast<unsigned int>(m_menu->m_slots.size()), 4);
}

void UIScene_BeaconMenu::SetConfirmButtonEnabled(bool enabled)
{
    m_confirmButton.SetButtonActive(enabled);
}

void UIScene_BeaconMenu::handleTick()
{
    int level = m_beaconContainer->getContainerData(0);
    MobEffect* primary = MobEffect::byId(m_beaconContainer->getContainerData(1));
    MobEffect* secondary = MobEffect::byId(m_beaconContainer->getContainerData(2));
    if (level != m_beaconLevel)
    {
        m_nextButtonId = 100;
        MobEffect* effects[] = { MobEffects::MOVEMENT_SPEED, MobEffects::DIG_SPEED,
                                 MobEffects::DAMAGE_RESISTANCE, MobEffects::JUMP,
                                 MobEffects::DAMAGE_BOOST };
        int tiers[] = { 0, 0, 1, 1, 2 };
        int columns[] = { 0, 1, 0, 1, 0 };
        int index;
        for (index = 0; index < 5; ++index)
        {
            bool active = tiers[index] < level;
            bool selected = active && effects[index] == primary;
            if (selected && m_primaryEffect == -1)
                m_primaryEffect = m_beaconContainer->getContainerData(1);
            AddPowerButton(m_nextButtonId++, effects[index]->getIcon(), tiers[index], columns[index],
                           active, selected);
        }
        bool selected = level >= 4 && MobEffects::REGENERATION == secondary;
        if (selected && m_secondaryEffect == -1)
            m_secondaryEffect = m_beaconContainer->getContainerData(2);
        AddPowerButton(m_nextButtonId++, MobEffects::REGENERATION->getIcon(), 3, 0,
                       level >= 4, selected);
        if (primary != 0)
        {
            selected = level >= 4 && primary == secondary;
            if (selected && m_secondaryEffect == -1)
                m_secondaryEffect = m_beaconContainer->getContainerData(2);
            AddPowerButton(m_nextButtonId++, primary->getIcon(), 3, 1, level >= 4,
                           selected);
        }
    }
    m_beaconLevel = level;
    int primaryId = m_beaconContainer->getContainerData(1);
    int secondaryId = m_beaconContainer->getContainerData(2);
    SetConfirmButtonEnabled((m_primaryEffect != primaryId || m_secondaryEffect != secondaryId) &&
                            !m_beaconContainer->getItem(0)->isEmpty());
}

void UIScene_BeaconMenu::tick()
{
    UIScene_AbstractContainerMenu::tick();
    handleTick();
}

UIControl* UIScene_BeaconMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x38)) return &m_hotbarList;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x39)) return &m_inventoryList;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3a)) return &m_activatorSlot;
    if (section >= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x3b) &&
        section <= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x41))
        return &m_powerButtons[static_cast<int>(section) - 0x3b];
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x42)) return &m_confirmButton;
    return 0;
}

void UIScene_BeaconMenu::customDraw(const char* name, fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    const int player = getPad();
    if (minecraft->mLocalPlayers[player] != 0 && minecraft->mGameModes[player] != 0)
    {
        int slot;
        std::sscanf(name, "slot_%d", &slot);
        UIScene_AbstractContainerMenu::customDraw(name, rect);
    }
}

void UIScene_BeaconMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_BeaconMenu::getSceneType()
{
    return 0x36;
}

unsigned int UIScene_BeaconMenu::GetBaseSlotCount()
{
    return 4;
}
