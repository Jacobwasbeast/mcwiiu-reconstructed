#include "game/ui/UIScene_TeleportMenu.hpp"

#include <string>

#include "game/command/TeleportCommand.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"

UIScene_TeleportMenu::UIScene_TeleportMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer), m_splitScreen(false), m_playerInfo(), m_playerList(), m_title()
{
    initialiseMovie();
    m_splitScreen = data != 0 && static_cast<unsigned char*>(data)[4] != 0;
    delete static_cast<unsigned char*>(data);

    m_title.init(UIString(L10N::GetString(m_splitScreen ? 0xdd235a1d : 0x4296e646)));
    m_playerList.init(0);

    INetworkPlayer* localPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    const int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    for (int i = 0; i < playerCount; ++i)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerByIndex(i);
        if (player != 0 && player != localPlayer)
        {
            _PlayerInfo* info = BuildPlayerInfo(player);
            m_playerInfo.push_back(info);
            m_playerList.addItem(info->name, info->colour, info->voiceStatus, true);
        }
    }
    CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(m_playerId, OnPlayerChanged, this);
    m_layer->addComponent(m_playerId, static_cast<EUIScene>(0x48), 0);
    updateTooltips();
    gConsoleUIController.HidePressStart();
}

UIScene_TeleportMenu::~UIScene_TeleportMenu()
{
    for (std::vector<_PlayerInfo*>::iterator it = m_playerInfo.begin(); it != m_playerInfo.end(); ++it)
        delete *it;
}

unsigned int UIScene_TeleportMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_playerList.setupControl(this, root, std::string("GamePlayers"));
    m_controls.push_back(&m_playerList);
    m_title.setupControl(this, root, std::string("Title"));
    m_controls.push_back(&m_title);
    return 1;
}

UIScene_TeleportMenu::_PlayerInfo* UIScene_TeleportMenu::BuildPlayerInfo(INetworkPlayer* player)
{
    _PlayerInfo* info = new _PlayerInfo;
    info->smallId = player->GetSmallId();
    info->name = player->GetDisplayName();
    info->voiceStatus = 0;
    if (player->HasVoice())
    {
        if (player->IsMutedByLocalUser(m_playerId))
            info->voiceStatus = 3;
        else
            info->voiceStatus = player->IsTalking() ? 2 : 1;
    }
    info->colour = static_cast<unsigned short>(g_pApp->GetPlayerColour(info->smallId));
    return info;
}

void UIScene_TeleportMenu::OnPlayerChanged(void* context, INetworkPlayer* player, bool removed)
{
    UIScene_TeleportMenu* scene = static_cast<UIScene_TeleportMenu*>(context);
    unsigned int index = 0;
    for (std::vector<_PlayerInfo*>::iterator it = scene->m_playerInfo.begin();
         it != scene->m_playerInfo.end(); ++it, ++index)
    {
        if ((*it)->smallId == player->GetSmallId())
        {
            if (scene->m_playerList.getCurrentSelection() == scene->m_playerList.getItemCount() - 1)
                scene->m_playerList.setCurrentSelection(scene->m_playerList.getItemCount() - 2, false, true);
            delete *it;
            scene->m_playerInfo.erase(it);
            scene->m_playerList.removeItem(index, true);
            break;
        }
    }
    if (removed)
        return;
    _PlayerInfo* info = scene->BuildPlayerInfo(player);
    scene->m_playerInfo.push_back(info);
    scene->m_playerList.addItem(info->name, info->colour, info->voiceStatus, true);
}

void UIScene_TeleportMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, 0x8d6323ed, 0x4f6bcf5e, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_TeleportMenu::handleReload()
{
    for (std::vector<_PlayerInfo*>::iterator it = m_playerInfo.begin(); it != m_playerInfo.end(); ++it)
        delete *it;
    m_playerInfo.clear();
    INetworkPlayer* localPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    const int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    for (int i = 0; i < playerCount; ++i)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerByIndex(i);
        if (player != 0 && player != localPlayer)
        {
            _PlayerInfo* info = BuildPlayerInfo(player);
            m_playerInfo.push_back(info);
            m_playerList.addItem(info->name, info->colour, info->voiceStatus, true);
        }
    }
    if (controlHasFocus(0))
        m_playerList.setCurrentSelection(getControlChildFocus(), false, true);
}

void UIScene_TeleportMenu::tick()
{
    UIScene::tick();
    INetworkPlayer* localPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    unsigned int item = 0;
    for (int playerIndex = 0; item < m_playerInfo.size(); ++playerIndex, ++item)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerByIndex(playerIndex);
        if (player == localPlayer)
            ++playerIndex;
        player = CGameNetworkManager::sInstance.GetPlayerByIndex(playerIndex);
        if (player == 0)
            continue;
        _PlayerInfo* info = BuildPlayerInfo(player);
        _PlayerInfo* old = m_playerInfo[item];
        old->smallId = info->smallId;
        if (old->colour != info->colour)
        {
            old->colour = info->colour;
            m_playerList.setPlayerIcon(item, info->colour);
        }
        if (old->name != info->name)
        {
            m_playerList.SetItemLabel(item, UIString(info->name), true);
            old->name = info->name;
        }
        delete info;
    }
}

std::wstring UIScene_TeleportMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"InGameTeleportMenu") :
                                               std::wstring(L"InGameTeleportMenuSplit");
}

void UIScene_TeleportMenu::handleInput(int, int action, bool repeat, bool released, bool pressed,
                                       bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (action <= 3 || (action >= 6 && action < 8))
        return;
    if ((action <= 9 || action == 0x14))
    {
        sendInputToMovie(action, repeat, released, pressed);
        return;
    }
    if (action == 0x15 && released && !repeat)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
        navigateBack();
    }
}

void UIScene_TeleportMenu::handleGainFocus(bool hadFocus)
{
    UIScene::handleGainFocus(hadFocus);
    if (hadFocus)
        CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(m_playerId, OnPlayerChanged, this);
}

void UIScene_TeleportMenu::handlePress(int controlId, int itemIndex)
{
    if (controlId != 0)
        return;
    INetworkPlayer* target = CGameNetworkManager::sInstance.GetPlayerBySmallId(m_playerInfo[itemIndex]->smallId);
    INetworkPlayer* local = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
    if (m_splitScreen)
        connection->send(TeleportCommand::preparePacket(local->GetUID(), target->GetUID()));
    else
        connection->send(TeleportCommand::preparePacket(target->GetUID(), local->GetUID()));
}

void UIScene_TeleportMenu::handleDestroy()
{
    CGameNetworkManager::sInstance.UnRegisterPlayerChangedCallback(m_playerId, OnPlayerChanged, this);
    m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x48), false);
}

int UIScene_TeleportMenu::getSceneType()
{
    return 0x34;
}

unsigned int UIScene_TeleportMenu::getSubSceneType() const
{
    return m_splitScreen ? 1 : 0;
}
