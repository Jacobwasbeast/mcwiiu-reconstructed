#include "game/world/generation/IUIScene_PlayerList.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/KickPlayerPacket.hpp"
#include "game/ui/UIControl_ButtonList.hpp"

IUIScene_PlayerList::PlayerChange::PlayerChange(unsigned char smallIdValue, bool addedValue)
    : smallId(smallIdValue), added(addedValue)
{
}

IUIScene_PlayerList::IUIScene_PlayerList()
{
    InitializeCriticalSection(&mCriticalSection);
}

IUIScene_PlayerList::~IUIScene_PlayerList()
{
    std::vector<PlayerChange*>::iterator it;
    for (it = mPlayerChanges.begin(); it != mPlayerChanges.end(); ++it)
        delete *it;
    mPlayerChanges.clear();
    DeleteCriticalSection(&mCriticalSection);
}

uint32_t IUIScene_PlayerList::KickPlayerReturned(void* context, int playerIndex,
                                                  C4JStorage::EMessageResult result)
{
    unsigned char smallId = *static_cast<unsigned char*>(context);
    delete static_cast<unsigned char*>(context);
    if (result != C4JStorage::EMessageResult_4)
        return 0;

    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft == NULL)
        return 0;

    ClientPacketListener* listener = minecraft->getConnection(playerIndex);
    if (listener != NULL)
        listener->send(boost::shared_ptr<Packet>(new KickPlayerPacket(smallId)));
    return 0;
}

void IUIScene_PlayerList::OnPlayerChanged(void* context, INetworkPlayer* player, bool added)
{
    IUIScene_PlayerList* list = static_cast<IUIScene_PlayerList*>(context);
    EnterCriticalSection(&list->mCriticalSection);
    list->mPlayerChanges.push_back(new PlayerChange(player->GetSmallId(), added));
    LeaveCriticalSection(&list->mCriticalSection);
}

void IUIScene_PlayerList::tick()
{
    EnterCriticalSection(&mCriticalSection);
    unsigned int removed = 0;
    unsigned int index;
    for (index = 0; index < mPlayerChanges.size(); ++index)
    {
        PlayerChange* change = mPlayerChanges[index];
        int playerIndex = FindPlayer(change->smallId);
        if (!change->added && playerIndex < 0)
        {
            UIControl_ButtonList* list = GetPlayerList();
            if (list != NULL)
                list->addItem(std::wstring(L"buckets_"), static_cast<int>(list->getItemCount()));
        }
        else if (playerIndex >= 0)
        {
            ++removed;
            UIControl_ButtonList* list = GetPlayerList();
            if (list != NULL && removed < list->getItemCount())
                list->setCurrentSelection(list->getItemCount() - static_cast<int>(index) - 1, false, false);
        }
        delete change;
    }
    mPlayerChanges.clear();
    LeaveCriticalSection(&mCriticalSection);

    UIControl_ButtonList* list = GetPlayerList();
    if (list != NULL)
    {
        unsigned int item = 0;
        while (item < static_cast<unsigned int>(list->getItemCount()))
        {
            PlayerInfo* currentInfo = GetPlayer(static_cast<int>(item));
            if (currentInfo == NULL)
            {
                ++item;
                continue;
            }
            unsigned char smallId = currentInfo->smallId;
            INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(smallId);
            if (player != NULL)
            {
                PlayerInfo* info = CreatePlayerInfo();
                if (info != NULL)
                {
                    BuildPlayerInfo(player, info);
                    UpdatePlayer(static_cast<int>(item), GetPlayer(static_cast<int>(item)), info);
                    delete info;
                }
            }
            ++item;
        }
    }

    EnterCriticalSection(&mCriticalSection);
    LeaveCriticalSection(&mCriticalSection);
}

bool IUIScene_PlayerList::hasAPlayerChanged()
{
    return !mPlayerChanges.empty();
}

void IUIScene_PlayerList::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info)
{
    info->smallId = player->GetSmallId();
    info->voiceStatus = 0;
    info->name = player->GetDisplayName();
    if (player->HasVoice())
        info->voiceStatus = player->IsTalking() ? 3 : (player->IsMutedByLocalUser(0) ? 2 : 1);
    info->colour = static_cast<unsigned short>(g_pApp->GetPlayerColour(info->smallId));
}

PlayerInfo* IUIScene_PlayerList::CreatePlayerInfo()
{
    PlayerInfo* info = new PlayerInfo;
    info->smallId = 0;
    info->voiceStatus = 0;
    info->colour = 0;
    return info;
}

int IUIScene_PlayerList::FindPlayer(unsigned char smallId)
{
    UIControl_ButtonList* list = GetPlayerList();
    if (list == NULL)
        return -1;
    for (int index = 0; index < list->getItemCount(); ++index)
    {
        PlayerInfo* info = GetPlayer(index);
        if (info != NULL && info->smallId == smallId)
            return index;
    }
    return -1;
}

void IUIScene_PlayerList::UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo)
{
    int comparison = newInfo->name.compare(0, newInfo->name.size(), oldInfo->name.c_str(), oldInfo->name.size());

    if (comparison != 0 || hasAPlayerChanged())
    {
        oldInfo->name.assign(newInfo->name);
        GetPlayerList()->setButtonLabel(index, newInfo->name);
    }

    if (newInfo->smallId != oldInfo->smallId || hasAPlayerChanged())
    {
        oldInfo->smallId = newInfo->smallId;
        GetPlayerList()->setButtonLabel(index, newInfo->name);
    }
}
