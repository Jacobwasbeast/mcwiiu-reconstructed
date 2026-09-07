#ifndef GAME_UI_UISCENE_SKINSELECTMENU_HPP
#define GAME_UI_UISCENE_SKINSELECTMENU_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/ui/UIScene.hpp"
#include "game/ui/IUIScene_DlcSelectMenu.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_MultiList.hpp"
#include "game/ui/UIControl_PlayerSkinPreview.hpp"
#include "game/ui/UIControl_Touch.hpp"

struct SKIN_BOX;

class UIScene_SkinSelectMenu : public UIScene
{
public:
    UIScene_SkinSelectMenu(int player, void* data, UILayer* layer);
    virtual ~UIScene_SkinSelectMenu();

    virtual unsigned int mapElementsAndNames();
    virtual std::wstring getMoviePath();
    virtual void tick();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual void handleInput(int player, int action, bool repeat, bool released, bool pressed,
                             bool& handled);
    virtual void handleReload();
    virtual void handlePress(int first, int second);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handleAnimationEnd();
    virtual void HandleDLCMountingComplete();
    virtual void HandleDLCInstalled();
    virtual void HandleDLCLicenseChange();
    virtual void handleTouchInput(unsigned int player, int action, int x, int y, bool repeat,
                                  bool released, bool pressed);
    virtual int getSceneType();

    void setCharacterLocked(bool locked);
    void setCharacterSelected(bool selected);
    void setCharacterFavourite(bool favourite);
    void setCharacterBlocked(bool blocked);
    void setActivePackIndex();
    unsigned int registerTexture(std::wstring textureName);
    unsigned char getTextureId(int index);
    unsigned int getNextSkinIndex(unsigned int index);
    int getPreviousSkinIndex(unsigned int index);
    void handleSkinIndexChanged();
    unsigned int IsTrialDiscontinuedPack(int packId);
    unsigned int GetIsNewPack(int packId);
    static bool SORTNEWEST(int first, int second);
    static bool SORTALPHABETICALLY(int first, int second);
    void SetSkinPackButtonList();
    void setPackLabel();
    void updatePackDisplay();
    void handlePackIndexChanged();
    void InputActionOK(unsigned int player);
    static unsigned int ManageFavoritesReturned(void* context, int player,
                                                C4JStorage::EMessageResult result);
    void SetFavoriteSkin(int player, int skin);
    void InputActionFavorite(unsigned int player);
    unsigned int IsFavoriteSkin(int player, int skin);

private:
    IUIScene_DlcSelectMenu m_dlcScene;
    UIControl_PlayerSkinPreview m_characterPreviews[7];
    UIControl_Label m_skinTitle1;
    UIControl_Label m_skinTitle2;
    UIControl_Label m_packName;
    UIControl_Label m_packType;
    UIControl m_timer;
    UIControl m_characterPanel;
    UIControl_Touch m_touch;
    UIControl_MultiList m_skinButtonList;
    UIControl_BitmapIcon m_packIcon;
    std::vector<int> m_packIds;
    std::vector<int> m_newPackIds;
    std::vector<int> m_selectedPackIds;
    int m_activePackId;
    int m_packTypeId;
    int m_skinIndex;
    int m_selectedSkinId;
    std::wstring m_skinName;
    std::wstring m_defaultSkinName;
    std::wstring m_textureName;
    std::vector<SKIN_BOX*>* m_skinBoxes;
    bool m_characterFacing;
    bool m_skinChanging;
    bool m_favouriteChanged;
    bool m_focusChanged;
    bool m_packChanged;
    bool m_dlcChanged;
    bool m_hasSelectedPack;
    int m_packButtonX;
    int m_packButtonY;
    bool m_inputPending;
    int m_player;
};

#endif
