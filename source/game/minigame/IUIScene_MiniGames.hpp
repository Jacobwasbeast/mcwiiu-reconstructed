#ifndef GAME_MINIGAME_IUISCENE_MINIGAMES_HPP
#define GAME_MINIGAME_IUISCENE_MINIGAMES_HPP

class IUIPartial_OnlineRestricted;

class IUIScene_MiniGames
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
    unsigned int GetVariantPlayLabel(int index, bool alternate);
    unsigned int GetVariantDesc(int index, bool alternate);
    void SetUpVariants();

private:
    Variant mVariants[5];
    const wchar_t* mVariantLabel;
    IUIPartial_OnlineRestricted* mOnlineRestricted;
    const wchar_t* mVariantName;
};

#endif
