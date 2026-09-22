#include "game/minigame/IUIScene_MiniGames.hpp"
#include "game/core/IUIPartial_OnlineRestricted.hpp"
#include "game/minigame/MiniGameDef.hpp"

IUIScene_MiniGames::Variant::Variant()
    : mPlayLabel(-1)
    , mDescription(-1)
    , mPlayLabelAlt(-1)
    , mDescriptionAlt(-1)
{
}

IUIScene_MiniGames::IUIScene_MiniGames()
{
}

unsigned int IUIScene_MiniGames::GetVariantPlayLabel(int index, bool alternate)
{
    return alternate ? mVariants[index].mPlayLabel : mVariants[index].mPlayLabelAlt;
}

unsigned int IUIScene_MiniGames::GetVariantDesc(int index, bool alternate)
{
    return alternate ? mVariants[index].mDescription : mVariants[index].mDescriptionAlt;
}

void IUIScene_MiniGames::SetUpVariants()
{
    MiniGameDef* miniGame = GetMiniGame();
    int optionsCount = miniGame->GetOptionsCount(eCustomGameModeHostOption_ModeVariant);
    unsigned int id = miniGame->GetId();
    if (id != 0)
    {
        if (id == 1)
        {
            mVariants[0].mPlayLabel = -1;
            mVariants[0].mDescription = -1;
            mVariants[0].mPlayLabelAlt = 0xa1b54cbe;
            mVariants[0].mDescriptionAlt = 0x7031de7d;
            if (optionsCount == 3)
            {
                mVariants[1].mPlayLabel = 0x7ec1ad4c;
                mVariants[1].mDescription = 0xfe25a58c;
                mVariants[1].mPlayLabelAlt = 0xa1b54cbe;
                mVariants[1].mDescriptionAlt = 0x353280c2;
                mVariants[2].mPlayLabel = 0x7ec1ad4c;
                mVariants[2].mDescription = 0xa7b858b2;
                mVariants[2].mPlayLabelAlt = 0xa1b54cbe;
                mVariants[2].mDescriptionAlt = 0x3cedfb4c;
                mVariants[3].mPlayLabel = 0x7ec1ad4c;
                mVariants[3].mDescription = 0x19ce3576;
                mVariants[3].mPlayLabelAlt = 0xa1b54cbe;
                mVariants[3].mDescriptionAlt = 0xa6fc9a18;
                return;
            }
            mVariants[1].mPlayLabel = 0x7ec1ad4c;
            mVariants[1].mDescription = 0xb6a97e24;
            mVariants[1].mDescriptionAlt = -1;
            mVariants[1].mPlayLabelAlt = -1;
            return;
        }
        if (id > 2)
        {
            if (id != 3)
                return;
            mVariants[0].mPlayLabel = -1;
            mVariants[0].mDescription = -1;
            mVariants[0].mPlayLabelAlt = 0xa1b54cbe;
            mVariants[0].mDescriptionAlt = 0x26fc3c0e;
            if (optionsCount == 2)
            {
                mVariants[1].mPlayLabel = 0x7ec1ad4c;
                mVariants[1].mDescription = 0xb746dec7;
                mVariants[1].mPlayLabelAlt = 0xa1b54cbe;
                mVariants[1].mDescriptionAlt = 0x4e09c09;
                mVariants[2].mPlayLabel = 0x7ec1ad4c;
                mVariants[2].mDescription = 0xb4516cb;
                mVariants[2].mPlayLabelAlt = 0xa1b54cbe;
                mVariants[2].mDescriptionAlt = 0x5840ccf5;
                return;
            }
            mVariants[1].mPlayLabel = 0x7ec1ad4c;
            mVariants[1].mDescription = 0xb6a97e24;
            mVariants[1].mPlayLabelAlt = -1;
            mVariants[1].mDescriptionAlt = -1;
            return;
        }
        mVariants[0].mPlayLabel = -1;
        mVariants[0].mDescription = -1;
        mVariants[0].mPlayLabelAlt = 0xa1b54cbe;
        mVariants[0].mDescriptionAlt = 0xf4cd599f;
        if (optionsCount == 4)
        {
            mVariants[1].mPlayLabel = 0x7ec1ad4c;
            mVariants[1].mDescription = 0x41afac5c;
            mVariants[1].mPlayLabelAlt = 0xa1b54cbe;
            mVariants[1].mDescriptionAlt = 0x4e43a572;
            mVariants[2].mPlayLabel = 0x7ec1ad4c;
            mVariants[2].mDescription = 0x9f01e42a;
            mVariants[2].mPlayLabelAlt = 0xa1b54cbe;
            mVariants[2].mDescriptionAlt = 0xc708bb04;
            mVariants[3].mPlayLabel = 0x7ec1ad4c;
            mVariants[3].mDescription = 0x89b0cf2c;
            mVariants[3].mPlayLabelAlt = 0xa1b54cbe;
            mVariants[3].mDescriptionAlt = 0x30045cc2;
            mVariants[4].mPlayLabel = 0x7ec1ad4c;
            mVariants[4].mDescription = 0x19ce3576;
            mVariants[4].mPlayLabelAlt = 0xa1b54cbe;
            mVariants[4].mDescriptionAlt = 0xa6fc9a18;
        }
    }
}
