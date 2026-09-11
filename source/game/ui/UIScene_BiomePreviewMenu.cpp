#include "game/ui/UIScene_BiomePreviewMenu.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>

#include "game/core/ConsoleUIController.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/CMinecraftApp.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

UIScene_BiomePreviewMenu::CustomImage::CustomImage()
    : mTextureLoaded(0), mPadding(), mImage(), mTexture(0), mTextureData(0), mTextureDataSize(0)
{
    mTexture = GlStateManager::genTexture();
}

UIScene_BiomePreviewMenu::CustomImage::~CustomImage()
{
    delete[] mTextureData;
    mTextureData = 0;
    mTextureDataSize = 0;
}

UIScene_BiomePreviewMenu::UIScene_BiomePreviewMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer), mBiomeMap(), mNavigatorMap(), mMainPanel(), mTimer(), mScrollUp(),
      mScrollDown(), mScrollLeft(), mScrollRight(), mPlayableAreaLabel(), mCoordinatesLabel(),
      mFullMapLabel(), mHelpLabel(), mCursor(), mOverrideSettings(), mRegenerating(false),
      mUpdatePending(false), mZoomPending(false), mPad(0), mPointerX(0.0f), mPointerY(0.0f),
      mPointerWidth(0.0f), mPointerHeight(0.0f), mMapWidth(0.0f), mMapHeight(0.0f),
      mNavigatorWidth(0.0f), mNavigatorHeight(0.0f), mCriticalSection()
{
}

UIScene_BiomePreviewMenu::~UIScene_BiomePreviewMenu()
{
}

void UIScene_BiomePreviewMenu::setNewPointerPos(float x, float y)
{
    mPointerX = x;
    mPointerY = y;
    mPointerWidth = (float)mNavigatorMap.mImage.getWidth();
    mPointerHeight = (float)mNavigatorMap.mImage.getHeight();
    Biome* biome = mOverrideSettings.GetBiomeAt((x - mPointerX) / mPointerWidth,
                                                (y - mPointerY) / mPointerHeight);
    if (biome != 0)
    {
        std::vector<HtmlString> lines;
        lines.push_back(HtmlString(biome->getName(false), (eMinecraftColour)0, false, false, -1));
        UIString label(HtmlString::Compose(&lines));
        mCursor.setLabel(label, false, true);
        mPlayableAreaLabel.setLabel(UIString(biome->getDescription()), false, false);
    }
    BlockPos cursor = mOverrideSettings.GetCursorBlockPos((x - mPointerX) / mPointerWidth,
                                                           (y - mPointerY) / mPointerHeight);
    wchar_t text[64];
    swprintf(text, 64, L"%d, %d", cursor.x, cursor.z);
    mCoordinatesLabel.setLabel(UIString(text), false, false);
}

void UIScene_BiomePreviewMenu::setNewPointerPos(const _UIVec2D& position)
{
    setNewPointerPos(position.x, position.y);
}

void UIScene_BiomePreviewMenu::UpdatePixelCaches(int sectionX, int sectionZ, bool atVerticalEdge,
                                                  bool atHorizontalEdge)
{
    mUpdatePending = false;
    if (sectionX != -1 && sectionZ != -1)
    {
        EnterCriticalSection(&mCriticalSection);
        if (mNavigatorMap.mTextureData == 0)
        {
            LeaveCriticalSection(&mCriticalSection);
            int imageWidth = mOverrideSettings.GetImageWidth();
            mNavigatorMap.mTextureDataSize = imageWidth * imageWidth * 4;
            mNavigatorMap.mTextureData = new unsigned char[mNavigatorMap.mTextureDataSize];
            EnterCriticalSection(&mCriticalSection);
            memset(mNavigatorMap.mTextureData, 0, mNavigatorMap.mTextureDataSize);
        }

        unsigned int width = mOverrideSettings.GetBiomeSectionWidth();
        unsigned int height = mOverrideSettings.GetBiomeSectionWidth();
        if (atVerticalEdge)
            width /= 2;
        if (atHorizontalEdge)
            height /= 2;
        mOverrideSettings.UpdateBiomeMapPixelsInRegion(
            (unsigned int*)mNavigatorMap.mTextureData, sectionX, sectionZ, width, height,
            mUpdatePending, true);
        LeaveCriticalSection(&mCriticalSection);
    }

    if (mOverrideSettings.isZoomed())
    {
        EnterCriticalSection(&mCriticalSection);
        delete[] mBiomeMap.mTextureData;
        mBiomeMap.mTextureData = 0;
        mBiomeMap.mTextureDataSize = 0;
        int zoomLevel = mOverrideSettings.GetZoomLevel();
        arrayWithLength<unsigned char> image = mOverrideSettings.CreateZoomedImage(
            (unsigned int*)mNavigatorMap.mTextureData, zoomLevel);
        mBiomeMap.mTextureData = image.data;
        mBiomeMap.mTextureDataSize = image.length;
        LeaveCriticalSection(&mCriticalSection);
    }

    mZoomPending = false;
    mRegenerating = true;
}

unsigned int UIScene_BiomePreviewMenu::UpdateRegenBiomes(void* data)
{
    UIScene_BiomePreviewMenu* scene = static_cast<UIScene_BiomePreviewMenu*>(data);
    IntCache::CreateNewThreadStorage();
    while (scene != 0 && !scene->mOverrideSettings.FullyGenerated() && !scene->mZoomPending)
    {
        int x = -1;
        int z = -1;
        bool vertical = false;
        bool horizontal = false;
        if (scene->mOverrideSettings.RegenerateNearestBiomeSection(x, z, vertical, horizontal))
            break;
        scene->UpdatePixelCaches(x, z, vertical, horizontal);
    }
    IntCache::ReleaseThreadStorage();
    return 0;
}

UIScene_BiomePreviewMenu::CustomImage* UIScene_BiomePreviewMenu::getImage(bool wholeMap)
{
    return wholeMap ? &mNavigatorMap : &mBiomeMap;
}

unsigned int UIScene_BiomePreviewMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    mMainPanel.setupControl(this, root, "MainPanel");
    mBiomeMap.mImage.setupControl(this, mMainPanel.getFuiRenderNode(), "BitmapBiomeMap");
    mNavigatorMap.mImage.setupControl(this, mMainPanel.getFuiRenderNode(), "NavigatorBiomeMap");
    mPlayableAreaLabel.setupControl(this, root, "PlayableAreaLabel");
    mFullMapLabel.setupControl(this, root, "FullMapLabel");
    mCoordinatesLabel.setupControl(this, root, "CoordinatesLabel");
    mTimer.setupControl(this, root, "Timer");
    mHelpLabel.setupControl(this, root, "Html_Help");
    mCursor.setupControl(this, root, "cursor");
    return 1;
}

void UIScene_BiomePreviewMenu::updateTooltips()
{
    int scale = (int)mOverrideSettings.GetGenerationScale();
    int zoom = (int)mOverrideSettings.GetZoomLevel();
    gConsoleUIController.SetTooltips(0, 0xd8a6b805, 0xe21d8023, 0xd30e0a41, -1,
                                     zoom == 0 ? -1 : zoom == 1 ? -2 : -1,
                                     scale == 2 ? 0x5b2efac2 : -1,
                                     scale == 1 ? -2 : scale == 2 ? 0x7bc900dd : -1,
                                     0x7bc900dd, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_BiomePreviewMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, (EUIScene)0x42, true);
    m_layer->showComponent(m_playerId, (EUIScene)0x43, false);
}

std::wstring UIScene_BiomePreviewMenu::getMoviePath()
{
    return L"BiomeSelectMenu";
}

void UIScene_BiomePreviewMenu::customDraw(const char*, fuiRect*)
{
}

void UIScene_BiomePreviewMenu::dmouse(float& x, float& y, float& width, float& height)
{
    x = mPointerX;
    y = mPointerY;
    width = mPointerWidth;
    height = mPointerHeight;
}

void UIScene_BiomePreviewMenu::componentExtents(UIControl& control, float& left, float& top,
                                                 float& right, float& bottom)
{
    left = (float)control.getXPos();
    top = (float)control.getYPos();
    right = left + (float)control.getWidth();
    bottom = top + (float)control.getHeight();
}

bool UIScene_BiomePreviewMenu::isOverComponent(UIControl& control, float x, float y)
{
    float left, top, right, bottom;
    componentExtents(control, left, top, right, bottom);
    return x >= left && x <= right && y >= top && y <= bottom;
}

void UIScene_BiomePreviewMenu::onMouseTick()
{
    float dx, dy, dirX, dirY;
    dmouse(dx, dy, dirX, dirY);
    if ((dirX != 0.0f || dirY != 0.0f) &&
        isOverComponent(mBiomeMap.mImage, mNavigatorWidth, mNavigatorHeight))
    {
        float scale = 1.0f;
        float moveX = dx * dx * dirX * 0.6f;
        float moveY = dy * dy * dirY * 0.6f;
        _UIVec2D pos;
        pos.x = mNavigatorWidth + mPointerX;
        pos.y = mNavigatorHeight + mPointerY;
        if (!gRenderer->IsWidescreen() || g_pApp->GetLocalPlayerCount() > 1)
            scale = 0.6f;
        moveX *= scale;
        moveY *= scale;
        pos.x += moveX;
        pos.y -= moveY;
        if (pos.x < mPointerWidth)
            pos.x = mPointerWidth;
        if (pos.x > mPointerHeight)
            pos.x = mPointerHeight;
        if (pos.y < mMapWidth)
            pos.y = mMapWidth;
        if (pos.y > mMapHeight)
            pos.y = mMapHeight;
        setNewPointerPos(pos);
    }
}

void UIScene_BiomePreviewMenu::UpdatePreviewImage(const arrayWithLength<unsigned char>& pixels,
                                                   bool wholeMap)
{
    if (pixels.data == 0 || pixels.length == 0)
        return;
    CustomImage* image = getImage(wholeMap);
    if (!image->mTextureLoaded)
    {
        image->mTextureLoaded = true;
        gConsoleUIController.registerSubstitutionTexture(wholeMap ? L"NavigatorPreview" : L"BiomePreview",
                                                         image->mTexture);
        image->mImage.setTextureName(wholeMap ? L"NavigatorPreview" : L"BiomePreview", true);
    }
    else
    {
        image->mImage.tick();
    }
    if (!wholeMap)
        setNewPointerPos(mPointerX, mPointerY);
}

void UIScene_BiomePreviewMenu::tick()
{
    UIScene::tick();
    onMouseTick();
    if (!mRegenerating)
        mUpdatePending = true;
}

void UIScene_BiomePreviewMenu::updateMapNavigatorFromOverrideSettings()
{
    double scale = pow(2.0, (double)mOverrideSettings.GetZoomLevel());
    mNavigatorWidth = (float)(mOverrideSettings.GetZoomedCentreXPercent() / scale);
    mNavigatorHeight = (float)(mOverrideSettings.GetZoomedCentreZPercent() / scale);
}

void UIScene_BiomePreviewMenu::handleScale(bool increase)
{
    bool changed = increase ? mOverrideSettings.IncreaseScale() : mOverrideSettings.DecreaseScale();
    if (changed)
    {
        updateTooltips();
        EnterCriticalSection(&mCriticalSection);
        updateMapNavigatorFromOverrideSettings();
        mUpdatePending = true;
        mRegenerating = true;
        LeaveCriticalSection(&mCriticalSection);
    }
}

void UIScene_BiomePreviewMenu::handleZoom(bool zoomIn)
{
    EnterCriticalSection(&mCriticalSection);
    bool changed = zoomIn ? mOverrideSettings.ZoomIn() : mOverrideSettings.ZoomOut();
    LeaveCriticalSection(&mCriticalSection);
    if (changed)
    {
        updateMapNavigatorFromOverrideSettings();
        mZoomPending = true;
    }
}

void UIScene_BiomePreviewMenu::handleNavigate(ENavDir direction)
{
    arrayWithLength<unsigned char> pixels;
    bool changed = false;
    EnterCriticalSection(&mCriticalSection);
    switch (direction)
    {
    case NAV_UP: changed = mOverrideSettings.ScrollUp(pixels); break;
    case NAV_DOWN: changed = mOverrideSettings.ScrollDown(pixels); break;
    case NAV_LEFT: changed = mOverrideSettings.ScrollLeft(pixels); break;
    case NAV_RIGHT: changed = mOverrideSettings.ScrollRight(pixels); break;
    }
    LeaveCriticalSection(&mCriticalSection);
    if (changed)
        UpdatePreviewImage(pixels, false);
}

void UIScene_BiomePreviewMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                            bool pressed, bool& handled)
{
    if (pressed)
    {
        if (action == 0x42) handleScale(true);
        else if (action == 0x43) handleScale(false);
        else if (action == 0x44) handleZoom(true);
        else if (action == 0x45) handleZoom(false);
        else if (action == 0x46) handleNavigate(NAV_UP);
        else if (action == 0x47) handleNavigate(NAV_DOWN);
        else if (action == 0x48) handleNavigate(NAV_LEFT);
        else if (action == 0x49) handleNavigate(NAV_RIGHT);
        else if (action == 0x4a) navigateBack();
        else return;
        handled = true;
    }
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, released, pressed);
}

UIControl* UIScene_BiomePreviewMenu::GetMainPanel()
{
    return &mMainPanel;
}

int UIScene_BiomePreviewMenu::getSceneType()
{
    return 0x40;
}
