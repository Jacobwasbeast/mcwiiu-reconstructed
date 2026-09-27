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
#include "game/ui/UILayer.hpp"

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
                tutorial->changeTutorialState(eTutorial_State_BeaconMenu, this);
            }
        }
        m_beaconContainer = sceneData->beacon;
        Initialize(false, sceneData->playerIndex,
                   new BeaconMenu(sceneData->inventory, sceneData->beacon), true, 1,
                   IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar,
                   IUIScene_AbstractContainerMenu::ESceneSection_BeaconMax, false);
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
    return section > IUIScene_AbstractContainerMenu::ESceneSection_AnvilMax &&
           section < IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1;
}

unsigned int UIScene_BeaconMenu::GetPowerButtonId(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1 ||
        section > IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2)
        return 0;
    return m_powerButtons[static_cast<int>(section) - IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1].GetData();
}

unsigned int UIScene_BeaconMenu::IsVisible(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    return section != IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2 ||
           GetPowerButtonId(section) != 0;
}

unsigned int UIScene_BeaconMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    static const unsigned char columns[] = { 9, 9, 1 };
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar ||
        section > IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator)
        return 0;
    return columns[static_cast<int>(section) - IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar];
}

unsigned int UIScene_BeaconMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar) return 1;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory) return GetInventoryRows();
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator) return 1;
    return 0;
}

unsigned int UIScene_BeaconMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection target = section;
    int dx = 0;

    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            if (*x < 2) target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1;
            else if (*x < 5) target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2;
            else if (*x < 9) target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1;
            else target = GetPowerButtonId(IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2) > 0 ?
                IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2 :
                IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (m_smallInventory == 0)
                target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory;
            else if (*x < 4)
                target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5;
            else if (*x > 6)
                target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm;
            else
                target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator;
        }
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (*x < 4) target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5;
            else if (*x > 6) target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm;
            else target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator;
        }
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_smallInventory != 0 ?
                IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar :
                IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory;
            dx = -5;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left ||
                 state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm;
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar;
            dx = -1;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left ||
                 state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2;
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar;
            dx = -3;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left ||
                 state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1;
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = GetPowerButtonId(IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2) > 0 ?
                IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2 :
                IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4;
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1;
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_smallInventory != 0 ?
                IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar :
                IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory;
            dx = -3;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3;
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar;
            dx = -7;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = GetPowerButtonId(IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2) > 0 ?
                IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2 :
                IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3;
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar;
            dx = -8;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3;
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_smallInventory != 0 ?
                IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar :
                IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory;
            dx = -8;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left ||
                 state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator;
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
    if (section <= IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2 ||
        section >= IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm)
        return 0;
    return tiers[static_cast<int>(section) - IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3];
}

MobEffect* UIScene_BeaconMenu::GetPowerButtonEffect(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1) return MobEffects::MOVEMENT_SPEED;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2) return MobEffects::DIG_SPEED;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3) return MobEffects::DAMAGE_RESISTANCE;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4) return MobEffects::JUMP;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5) return MobEffects::DAMAGE_BOOST;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1) return MobEffects::REGENERATION;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2 && m_beaconContainer)
        return MobEffect::byId(m_beaconContainer->getContainerData(1));
    return 0;
}

bool UIScene_BeaconMenu::IsPowerButtonSelected(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1 ||
        section > IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2)
        return false;
    return m_powerButtons[static_cast<int>(section) - IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1].IsButtonSelected() != 0;
}

void UIScene_BeaconMenu::SetPowerButtonSelected(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int i;
    if (section >= IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1 &&
        section < IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1)
        for (i = 0; i < 5; ++i) m_powerButtons[i].SetButtonSelected(false);
    else if (section >= IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1 &&
             section < IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm)
        for (i = 5; i < 7; ++i) m_powerButtons[i].SetButtonSelected(false);
    if (section >= IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1 &&
        section <= IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2)
        m_powerButtons[static_cast<int>(section) - IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1].SetButtonSelected(true);
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
    if (section >= IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1 &&
        section < IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm)
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
            if (IsPowerButtonSelected(IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2))
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
    if (section != IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm)
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
    return section >= IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar &&
           section <= IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator ?
           offsets[static_cast<int>(section) - IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar] : 0;
}

void UIScene_BeaconMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    if (wrap)
    {
        UIControl_SlotList* list = section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar ?
            &m_hotbarList : section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory ?
            &m_inventoryList : section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator ?
            &m_activatorSlot : 0;
        if (list != 0) list->setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
    }
}

std::vector<HtmlString>* UIScene_BeaconMenu::GetSectionHoverText(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1 ||
        section > IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2) return 0;
    std::wstring suffix;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2)
    {
        if (GetPowerButtonId(section) == 0)
            return 0;
        suffix = L" II";
    }
    MobEffect* effect = GetPowerButtonEffect(section);
    std::vector<HtmlString>* text = new std::vector<HtmlString>();
    text->push_back(HtmlString(std::wstring(L10N::GetString(effect->getDescriptionId(-1))) + suffix,
                               eMinecraftColour_HTMLColor_White, false, false, 0));
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
               IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_BeaconMax, false);
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
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar) return &m_hotbarList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory) return &m_inventoryList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator) return &m_activatorSlot;
    if (section >= IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1 &&
        section <= IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2)
        return &m_powerButtons[static_cast<int>(section) - IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1];
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm) return &m_confirmButton;
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
    return EUIScene_BeaconMenu;
}

unsigned int UIScene_BeaconMenu::GetBaseSlotCount()
{
    return 4;
}
