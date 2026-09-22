#ifndef GAME_MINIGAME_IUISCENE_MINIGAMES_HPP
#define GAME_MINIGAME_IUISCENE_MINIGAMES_HPP

#include "game/core/IUIPartial_OnlineRestricted.hpp"

class IUIScene_MiniGames : public virtual IUIPartial_OnlineRestricted
{
public:
    class Variant
    {
    public:
        int mPlayLabel;
        int mDescription;
        int mPlayLabelAlt;
        int mDescriptionAlt;

        Variant();
    };

    IUIScene_MiniGames();
    virtual void AddVariantButtons(bool alternate) = 0;
    unsigned int GetVariantPlayLabel(int index, bool alternate);
    unsigned int GetVariantDesc(int index, bool alternate);
    void SetUpVariants();

private:
    Variant mVariants[5];
};

#endif
