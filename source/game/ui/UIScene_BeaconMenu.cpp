#include "game/ui/UIScene_BeaconMenu.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
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
#include "game/item/Items.hpp"
#include "game/item/Item.hpp"
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

struct BeaconMenuData
{
    boost::shared_ptr<Container> inventory;
    boost::shared_ptr<Container> beacon;
    int playerIndex;
    bool splitscreen;
};

UIScene_BeaconMenu::UIScene_BeaconMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_activatorSlot(), m_activatorList(),
      m_primaryPowerLabel(), m_secondaryPowerLabel(), m_powerButtons(), m_confirmButton()
{
    m_beaconContainer = boost::shared_ptr<Container>();
    m_primaryEffect = -1;
    m_beaconLevel = -1;
    m_secondaryEffect = -1;
    BeaconMenuData* sceneData = static_cast<BeaconMenuData*>(data);
    initialiseMovie();
    m_primaryPowerLabel.init(UIString(0xb3c82373));
    m_secondaryPowerLabel.init(UIString(0x7b8c1d5d));
    int index;
    for (index = 0; index < 7; ++index)
        m_powerButtons[index].setVisible(false);
    if (Minecraft::GetInstance()->mGameModes[sceneData->playerIndex] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[type]->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_BeaconMenu, this);
        }
    }
    m_beaconContainer = sceneData->beacon;
    BeaconMenu* menu = new BeaconMenu(sceneData->inventory, sceneData->beacon);
    Initialize(false, sceneData->playerIndex, menu, true, 1,
               IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_BeaconMax, false);
    m_activatorSlot.addSlots(0, 1);
    m_activatorList.addSlots(static_cast<unsigned int>(m_menu->m_slots.size()), 4);
    delete sceneData;
}

UIScene_BeaconMenu::~UIScene_BeaconMenu()
{
}

bool UIScene_BeaconMenu::IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator:
        return true;
    default:
        return false;
    }
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
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2 &&
        GetPowerButtonId(IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2) == 0)
        return 0;
    return 1;
}

unsigned int UIScene_BeaconMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    unsigned int columns = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory:
        columns = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator:
        columns = 1;
        break;
    default:
        break;
    }
    return columns;
}

unsigned int UIScene_BeaconMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    unsigned int rows = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar:
        rows = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory:
        rows = GetInventoryRows();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator:
        rows = 1;
        break;
    default:
        break;
    }
    return rows;
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
    float width;
    float height;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar:
        width = m_hotbarList.getWidth();
        height = m_hotbarList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory:
        width = m_inventoryList.getWidth();
        height = m_inventoryList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator:
        width = m_activatorSlot.getWidth();
        height = m_activatorSlot.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1:
        width = m_powerButtons[0].getWidth();
        height = m_powerButtons[0].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2:
        width = m_powerButtons[1].getWidth();
        height = m_powerButtons[1].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3:
        width = m_powerButtons[2].getWidth();
        height = m_powerButtons[2].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4:
        width = m_powerButtons[3].getWidth();
        height = m_powerButtons[3].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5:
        width = m_powerButtons[4].getWidth();
        height = m_powerButtons[4].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1:
        width = m_powerButtons[5].getWidth();
        height = m_powerButtons[5].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2:
        width = m_powerButtons[6].getWidth();
        height = m_powerButtons[6].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm:
        width = m_confirmButton.getWidth();
        height = m_confirmButton.getHeight();
        break;
    }
    if (!IsSectionSlotList(section))
    {
        GetPositionOfSection(section, position);
        size->x = width;
        size->y = height;
        return;
    }
    int rows = getSectionRows(section);
    int columns = getSectionColumns(section);
    size->y = height / rows;
    size->x = width / columns;
    position->x = static_cast<float>(slot % columns) * size->x;
    position->y = static_cast<float>(slot / columns) * size->y;
}

unsigned int UIScene_BeaconMenu::GetPowerButtonTier(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4:
        return 1;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5:
        return 2;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2:
        return 3;
    default:
        return 0;
    }
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
    switch (tier)
    {
    case 0:
        if (column == 0)
        {
            m_powerButtons[0].SetData(id, icon, active, selected);
            m_powerButtons[0].setVisible(true);
        }
        else
        {
            m_powerButtons[1].SetData(id, icon, active, selected);
            m_powerButtons[1].setVisible(true);
        }
        break;
    case 1:
        if (column == 0)
        {
            m_powerButtons[2].SetData(id, icon, active, selected);
            m_powerButtons[2].setVisible(true);
        }
        else
        {
            m_powerButtons[3].SetData(id, icon, active, selected);
            m_powerButtons[3].setVisible(true);
        }
        break;
    case 2:
        m_powerButtons[4].SetData(id, icon, active, selected);
        m_powerButtons[4].setVisible(true);
        break;
    case 3:
        if (column == 0)
        {
            m_powerButtons[5].SetData(id, icon, active, selected);
            m_powerButtons[5].setVisible(true);
        }
        else
        {
            m_powerButtons[6].SetData(id, icon, active, selected);
            m_powerButtons[6].setVisible(true);
        }
        break;
    }
}

void UIScene_BeaconMenu::handleOtherClicked(int,
                                            IUIScene_AbstractContainerMenu::ESceneSection section,
                                            int, bool)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2:
    {
        if (IsPowerButtonSelected(section))
            return;
        int tier = GetPowerButtonTier(section);
        int effectId = MobEffect::getId(GetPowerButtonEffect(section));
        if (m_beaconContainer->getContainerData(0) <= tier)
            return;
        if (tier < 3)
        {
            m_beaconContainer->setContainerData(1, effectId);
            if (IsPowerButtonSelected(IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2))
                m_beaconContainer->setContainerData(2, effectId);
        }
        else
        {
            m_beaconContainer->setContainerData(2, effectId);
        }
        SetPowerButtonSelected(section);
        if (m_beaconContainer->getContainerData(1) > 0)
        {
            int primaryId = m_beaconContainer->getContainerData(1);
            bool active = true;
            bool selected = false;
            if (m_beaconContainer->getContainerData(0) > 3)
                selected = primaryId == m_beaconContainer->getContainerData(2);
            else
                active = false;
            int buttonId = m_nextButtonId;
            AddPowerButton(buttonId, MobEffect::byId(m_beaconContainer->getContainerData(1))->getIcon(),
                           3, 1, active, selected);
        }
        return;
    }
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm:
    {
        int primaryId = m_beaconContainer->getContainerData(1);
        int secondaryId = m_beaconContainer->getContainerData(2);
        bool unchanged = true;
        if (!m_beaconContainer->getItem(0)->isEmpty() &&
            (m_primaryEffect != primaryId || m_secondaryEffect != secondaryId))
            unchanged = false;
        if (unchanged)
            return;

        ByteArrayOutputStream bytes;
        DataOutputStream output(&bytes);
        output.writeInt(primaryId);
        output.writeInt(secondaryId);
        Minecraft::GetInstance()->getConnection(getPad())->send(
            boost::shared_ptr<Packet>(new ServerboundCustomPayloadPacket(std::wstring(L"MC|Beacon"),
                                                                         bytes.toByteArray())));
        m_primaryEffect = primaryId;
        m_secondaryEffect = secondaryId;
        m_beaconContainer->setContainerData(1, primaryId);
        return;
    }
    default:
        return;
    }
}

int UIScene_BeaconMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int offset = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar:
        offset = 0x1c;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory:
        offset = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator:
        offset = 0;
        break;
    default:
        break;
    }
    return offset;
}

void UIScene_BeaconMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    int columns = static_cast<int>(getSectionColumns(section));
    int index = direction * columns + slot;
    UIControl_SlotList* list = 0;
    if (section >= IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar)
    {
        if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar)
            list = &m_hotbarList;
        else if (section < IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator)
            list = &m_inventoryList;
        else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator)
            list = &m_activatorSlot;
    }
    if (wrap)
        list->setHighlightSlot(index);
}

std::vector<HtmlString>* UIScene_BeaconMenu::GetSectionHoverText(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    std::vector<HtmlString>* text = 0;
    std::wstring suffix(L"");
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2:
        if (GetPowerButtonId(IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2) == 0)
            break;
        suffix = L" II";
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1:
        {
            MobEffect* effect = GetPowerButtonEffect(section);
            text = new std::vector<HtmlString>;
            std::wstring description(L10N::GetString(effect->getDescriptionId(-1)));
            description += suffix;
            HtmlString line(description, eMinecraftColour_HTMLColor_White, false, false, 0);
            text->push_back(line);
        }
        break;
    default:
        break;
    }
    return text;
}

unsigned int UIScene_BeaconMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    {
        std::string name("ActivatorSlot");
        m_activatorSlot.setupControl(this, panel, name);
    }
    m_controls.push_back(&m_activatorSlot);
    {
        std::string name("ActivatorList");
        m_activatorList.setupControl(this, panel, name);
    }
    m_controls.push_back(&m_activatorList);
    {
        std::string name("Primary_Slot_01");
        m_powerButtons[0].setupControl(this, panel, name);
    }
    m_controls.push_back(&m_powerButtons[0]);
    {
        std::string name("Primary_Slot_02");
        m_powerButtons[1].setupControl(this, panel, name);
    }
    m_controls.push_back(&m_powerButtons[1]);
    {
        std::string name("Primary_Slot_03");
        m_powerButtons[2].setupControl(this, panel, name);
    }
    m_controls.push_back(&m_powerButtons[2]);
    {
        std::string name("Primary_Slot_04");
        m_powerButtons[3].setupControl(this, panel, name);
    }
    m_controls.push_back(&m_powerButtons[3]);
    {
        std::string name("Primary_Slot_05");
        m_powerButtons[4].setupControl(this, panel, name);
    }
    m_controls.push_back(&m_powerButtons[4]);
    {
        std::string name("Secondary_Slot_01");
        m_powerButtons[5].setupControl(this, panel, name);
    }
    m_controls.push_back(&m_powerButtons[5]);
    {
        std::string name("Secondary_Slot_02");
        m_powerButtons[6].setupControl(this, panel, name);
    }
    m_controls.push_back(&m_powerButtons[6]);
    {
        std::string name("ConfirmButton");
        m_confirmButton.setupControl(this, panel, name);
    }
    m_controls.push_back(&m_confirmButton);
    {
        std::string name("PrimaryPowerLabel");
        m_primaryPowerLabel.setupControl(this, panel, name);
    }
    m_controls.push_back(&m_primaryPowerLabel);
    {
        std::string name("SecondaryPowerLabel");
        m_secondaryPowerLabel.setupControl(this, panel, name);
    }
    m_controls.push_back(&m_secondaryPowerLabel);
    return 1;
}

std::wstring UIScene_BeaconMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return L"BeaconMenuSplit";
    return L"BeaconMenu";
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
    bool enabled = false;
    if (!m_beaconContainer->getItem(0)->isEmpty() &&
        (m_primaryEffect != primaryId || m_secondaryEffect != secondaryId))
        enabled = true;
    SetConfirmButtonEnabled(enabled);
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
    if (minecraft->mLocalPlayers[m_playerId] == NULL || minecraft->mGameModes[m_playerId] == NULL)
        return;

    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    int slotId = -1;
    std::sscanf(name, "slot_%d", &slotId);
    if (slotId >= 0 && static_cast<unsigned int>(slotId) >= m_menu->m_slots.size())
    {
        switch (slotId - m_menu->m_slots.size())
        {
        case 0:
            item = not_null_ptr<ItemInstance>(new ItemInstance(Items::emerald));
            break;
        case 1:
            item = not_null_ptr<ItemInstance>(new ItemInstance(Items::diamond));
            break;
        case 2:
            item = not_null_ptr<ItemInstance>(new ItemInstance(Items::goldIngot));
            break;
        case 3:
            item = not_null_ptr<ItemInstance>(new ItemInstance(Items::ironIngot));
            break;
        }
        if (!item->isEmpty())
            customDrawSlotControlFui(rect, m_playerId, item, 1.0f, item->getItem()->isFoil(item), true, false);
    }
    else
    {
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
