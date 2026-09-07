#ifndef GAME_UI_UISCENE_BIOMEPREVIEWMENU_HPP
#define GAME_UI_UISCENE_BIOMEPREVIEWMENU_HPP

#include <string>
#include <vector>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Base.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/world/biome/BiomeSource.hpp"

struct _UIVec2D
{
    float x;
    float y;
};

class UIScene_BiomePreviewMenu : public UIScene
{
public:
    class CustomImage
    {
    public:
        CustomImage();
        ~CustomImage();

        unsigned char mTextureLoaded;
        unsigned char mPadding[3];
        UIControl_BitmapIcon mImage;
        int mTexture;
        unsigned char* mTextureData;
        unsigned int mTextureDataSize;
    };

    enum ENavDir
    {
        NAV_UP,
        NAV_DOWN,
        NAV_LEFT,
        NAV_RIGHT
    };

    UIScene_BiomePreviewMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_BiomePreviewMenu();

    void setNewPointerPos(float x, float y);
    void setNewPointerPos(const _UIVec2D& position);
    void UpdatePixelCaches(int sectionX, int sectionZ, bool atVerticalEdge, bool atHorizontalEdge);
    static unsigned int UpdateRegenBiomes(void* data);

    CustomImage* getImage(bool wholeMap);
    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual std::wstring getMoviePath();
    virtual void customDraw(const char* name, fuiRect* rect);
    void dmouse(float& x, float& y, float& width, float& height);
    void componentExtents(UIControl& control, float& left, float& top, float& right, float& bottom);
    bool isOverComponent(UIControl& control, float x, float y);
    void onMouseTick();
    void UpdatePreviewImage(const arrayWithLength<unsigned char>& pixels, bool wholeMap);
    virtual void tick();
    void updateMapNavigatorFromOverrideSettings();
    void handleScale(bool increase);
    void handleZoom(bool zoomIn);
    void handleNavigate(ENavDir direction);
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                             bool& handled);
    virtual unsigned int GetMainPanel();
    virtual int getSceneType();
    void SetMapNavigator(int x, int z, int scale);

private:
    CustomImage mBiomeMap;
    CustomImage mNavigatorMap;
    UIControl mMainPanel;
    UIControl mTimer;
    UIControl mScrollUp;
    UIControl mScrollDown;
    UIControl mScrollLeft;
    UIControl mScrollRight;
    UIControl_Label mPlayableAreaLabel;
    UIControl_Label mCoordinatesLabel;
    UIControl_Label mFullMapLabel;
    UIControl_HTMLLabel mHelpLabel;
    UIControl_Base mCursor;
    BiomeSource::LayerOverrideSettings mOverrideSettings;
    bool mRegenerating;
    bool mUpdatePending;
    bool mZoomPending;
    unsigned char mPad;
    float mPointerX;
    float mPointerY;
    float mPointerWidth;
    float mPointerHeight;
    float mMapWidth;
    float mMapHeight;
    float mNavigatorWidth;
    float mNavigatorHeight;
    OSFastMutex mCriticalSection;
};

#endif
