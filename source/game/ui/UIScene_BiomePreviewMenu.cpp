#include "game/ui/UIScene_BiomePreviewMenu.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>

#include "game/core/ConsoleUIController.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/L10N.hpp"
#include "game/core/GameSettings.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

C4JThreadImpl* UIScene_BiomePreviewMenu::s_regenThread = 0;

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
      mFullMapLabel(), mHelpLabel(), mCursor(), mOverrideSettings(), mPreviewZoomLevel(0),
      mTargetZoomLevel(0), mShouldStopRegen(false), mRegenerating(false),
      mUpdatePending(false), mZoomPending(false), mPad(0), mPointerX(0.0f), mPointerY(0.0f),
      mPointerWidth(0.0f), mPointerHeight(0.0f), mMapWidth(0.0f), mMapHeight(0.0f),
      mNavigatorWidth(0.0f), mNavigatorHeight(0.0f), mCriticalSection()
{
    initialiseMovie();
    mHelpLabel.init(UIString(L""));
    mFullMapLabel.init(UIString(L10N::GetString(0xb0507c4a)));
    mPlayableAreaLabel.init(UIString(L""));
    mCoordinatesLabel.init(UIString(L"0, 0"));
    mTimer.setVisible(false);
    mOverrideSettings.SetWidthInChunksToDisplay(0x36, 0);
    mPreviewZoomLevel = mTargetZoomLevel = (int)mOverrideSettings.GetZoomLevel();
    InitializeCriticalSection(&mCriticalSection);
    s_regenThread = new C4JThreadImpl(reinterpret_cast<int (*)(void*)>(&UIScene_BiomePreviewMenu::UpdateRegenBiomes),
                                       this, "Regenerating Section", 0);
    s_regenThread->SetProcessor(0);
    s_regenThread->Run();
}

UIScene_BiomePreviewMenu::~UIScene_BiomePreviewMenu()
{
    mShouldStopRegen = true;
    if (s_regenThread != 0)
    {
        s_regenThread->WaitForCompletion(-1);
        delete s_regenThread;
        s_regenThread = 0;
    }
    DeleteCriticalSection(&mCriticalSection);
    wchar_t biomeName[15];
    swprintf(biomeName, 15, L"BiomePreview_%d", mPreviewZoomLevel);
    gConsoleUIController.unregisterSubstitutionTexture(biomeName, true);
    gConsoleUIController.unregisterSubstitutionTexture(L"NavigatorPreview", true);
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
    m_controls.push_back(&mMainPanel);

    fuiRenderNode* panel = mMainPanel.getFuiRenderNode();
    mBiomeMap.mImage.setupControl(this, panel, "BitmapBiomeMap");
    m_controls.push_back(&mBiomeMap.mImage);
    mNavigatorMap.mImage.setupControl(this, panel, "NavigatorBiomeMap");
    m_controls.push_back(&mNavigatorMap.mImage);
    mPlayableAreaLabel.setupControl(this, panel, "PlayableAreaLabel");
    m_controls.push_back(&mPlayableAreaLabel);
    mFullMapLabel.setupControl(this, panel, "FullMapLabel");
    m_controls.push_back(&mFullMapLabel);
    mCoordinatesLabel.setupControl(this, panel, "CoordinatesLabel");
    m_controls.push_back(&mCoordinatesLabel);
    mTimer.setupControl(this, panel, "Timer");
    m_controls.push_back(&mTimer);
    mHelpLabel.setupControl(this, panel, "Html_Help");
    m_controls.push_back(&mHelpLabel);
    mCursor.setupControl(this, panel, "cursor");
    m_controls.push_back(&mCursor);
    mScrollUp.setupControl(this, panel, "ScrollUp");
    m_controls.push_back(&mScrollUp);
    mScrollDown.setupControl(this, panel, "ScrollDown");
    m_controls.push_back(&mScrollDown);
    mScrollLeft.setupControl(this, panel, "ScrollLeft");
    m_controls.push_back(&mScrollLeft);
    mScrollRight.setupControl(this, panel, "ScrollRight");
    m_controls.push_back(&mScrollRight);
    return 1;
}

void UIScene_BiomePreviewMenu::updateTooltips()
{
    int zoom = (int)mOverrideSettings.GetZoomLevel();
    int zoomTooltip = zoom == 1 ? -2 : zoom == 2 ? 0x5b2efac2 : -1;
    int scaleTooltip = zoom < 2 ? 0x5b2efac2 : -1;
    int scale = (int)mOverrideSettings.GetGenerationScale();
    int scaleSlot6 = scale == 1 ? -2 : scale == 2 ? 0x7bc900dd : -1;
    int scaleSlot7 = (scale == 0 || scale == 1) ? 0x7bc900dd : -1;

    gConsoleUIController.SetTooltips(0, 0xd8a6b805, 0xe21d8023, 0xd30e0a41, -1,
                                     zoomTooltip, scaleTooltip, scaleSlot6, scaleSlot7,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
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
    double sensitivity = (double)GameSettings::GetGameSettings(m_playerId, (eGameSetting)5) / 100.0;
    x = (float)(g_pInput->GetJoypadStick_Menu_RX(m_playerId) * sensitivity);
    y = (float)(g_pInput->GetJoypadStick_Menu_RY(m_playerId) * sensitivity);
    if (fabsf(x) < 0.01f)
        width = 0.0f;
    else
        width = x / fabsf(x);
    height = 0.0f;
    if (fabsf(y) >= 0.01f)
        height = y / fabsf(y);
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
    int imageWidth = mOverrideSettings.GetImageWidth();
    std::wstring textureName = L"NavigatorPreview";
    if (!wholeMap)
    {
        wchar_t nameBuf[15];
        int zoomLevel = mTargetZoomLevel;
        if (mTargetZoomLevel != mPreviewZoomLevel)
        {
            if (!mBiomeMap.mTextureLoaded)
            {
                mPreviewZoomLevel = zoomLevel;
            }
            else
            {
                wchar_t oldNameBuf[15];
                swprintf(oldNameBuf, 15, L"BiomePreview_%d", mPreviewZoomLevel);
                gConsoleUIController.unregisterSubstitutionTexture(oldNameBuf, true);
                mBiomeMap.mTextureLoaded = false;
                mBiomeMap.mTexture = GlStateManager::genTexture();
                mPreviewZoomLevel = zoomLevel;
            }
        }
        double scale = pow(2.0, (double)zoomLevel);
        swprintf(nameBuf, 15, L"BiomePreview_%d", zoomLevel);
        imageWidth = (int)((double)imageWidth / scale);
        textureName = nameBuf;
    }
    CustomImage* image = getImage(wholeMap);
    if (!image->mTextureLoaded)
    {
        image->mTextureLoaded = true;
        gConsoleUIController.registerSubstitutionTexture(textureName.c_str(), image->mTexture);
        image->mImage.setTextureName(textureName.c_str(), true);
    }
    if (!wholeMap)
        setNewPointerPos(mPointerX, mPointerY);
}

void UIScene_BiomePreviewMenu::tick()
{
    UIScene::tick();
    onMouseTick();
    int width = 0;
    int height = 0;
    m_layer->getRenderDimensions(width, height);
    if (width == 0x500 && height == 0x1e0)
        width = 0x280;
    gFui->dispatchMouseMoveEvent(m_movie, mPointerX * (float)width / (float)m_stageWidth,
                                 mPointerY * (float)height / (float)m_stageHeight);
    if (mRegenerating && TryEnterCriticalSection(&mCriticalSection))
    {
        arrayWithLength<unsigned char> navigatorPixels;
        navigatorPixels.data = mNavigatorMap.mTextureData;
        navigatorPixels.length = mNavigatorMap.mTextureDataSize;
        UpdatePreviewImage(navigatorPixels, true);
        if (!mOverrideSettings.isZoomed() || mTargetZoomLevel == 0)
        {
            mTargetZoomLevel = 0;
            UpdatePreviewImage(navigatorPixels, false);
        }
        else
        {
            arrayWithLength<unsigned char> biomePixels;
            biomePixels.data = mBiomeMap.mTextureData;
            biomePixels.length = mBiomeMap.mTextureDataSize;
            UpdatePreviewImage(biomePixels, false);
        }
        mRegenerating = false;
        LeaveCriticalSection(&mCriticalSection);
    }
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
        memset(mNavigatorMap.mTextureData, 0, mNavigatorMap.mTextureDataSize);
        mUpdatePending = true;
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
        updateTooltips();
        updateMapNavigatorFromOverrideSettings();
        mZoomPending = true;
    }
}

void UIScene_BiomePreviewMenu::handleNavigate(ENavDir direction)
{
    arrayWithLength<unsigned char> pixels;
    pixels.data = mNavigatorMap.mTextureData;
    pixels.length = mNavigatorMap.mTextureDataSize;
    if (!mOverrideSettings.isZoomed())
    {
        EnterCriticalSection(&mCriticalSection);
        switch (direction)
        {
        case NAV_LEFT: mOverrideSettings.RegenerateLeft(pixels); break;
        case NAV_RIGHT: mOverrideSettings.RegenerateRight(pixels); break;
        case NAV_UP: mOverrideSettings.RegenerateUp(pixels); break;
        case NAV_DOWN: mOverrideSettings.RegenerateDown(pixels); break;
        }
        LeaveCriticalSection(&mCriticalSection);
        mUpdatePending = true;
    }
    else
    {
        bool changed = false;
        EnterCriticalSection(&mCriticalSection);
        switch (direction)
        {
        case NAV_LEFT: changed = mOverrideSettings.ScrollLeft(pixels); break;
        case NAV_RIGHT: changed = mOverrideSettings.ScrollRight(pixels); break;
        case NAV_UP: changed = mOverrideSettings.ScrollUp(pixels); break;
        case NAV_DOWN: changed = mOverrideSettings.ScrollDown(pixels); break;
        }
        LeaveCriticalSection(&mCriticalSection);
        mUpdatePending = true;
        if (changed)
        {
            mZoomPending = true;
            updateMapNavigatorFromOverrideSettings();
        }
    }
}

void UIScene_BiomePreviewMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                            bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, released, pressed);
    switch (action)
    {
    case 2:
        if (released)
        {
            EnterCriticalSection(&mCriticalSection);
            mOverrideSettings.GenerateNewSeed();
            wchar_t seedText[64];
            swprintf(seedText, 64, L"%lld", mOverrideSettings.GetSeed());
            mPlayableAreaLabel.setLabel(UIString(seedText), false, false);
            updateTooltips();
            updateMapNavigatorFromOverrideSettings();
            memset(mNavigatorMap.mTextureData, 0, mNavigatorMap.mTextureDataSize);
            mUpdatePending = true;
            LeaveCriticalSection(&mCriticalSection);
            handled = true;
        }
        break;
    case 4:
        if (released)
        {
            handleNavigate(NAV_UP);
            handled = true;
        }
        break;
    case 5:
        if (released)
        {
            handleNavigate(NAV_DOWN);
            handled = true;
        }
        break;
    case 6:
        if (released)
            handleNavigate(NAV_RIGHT);
        handled = true;
        break;
    case 7:
        if (released)
        {
            handleNavigate(NAV_LEFT);
            handled = true;
        }
        break;
    case 8:
        if (!released)
            return;
        if (!repeat)
        {
            handleZoom(true);
            handled = true;
            return;
        }
        handled = true;
        break;
    case 9:
        if (!released)
            return;
        if (!repeat)
        {
            handleZoom(false);
            handled = true;
            return;
        }
        handled = true;
        break;
    case 10:
        if (!released)
            return;
        if (!repeat)
        {
            handleScale(true);
            handled = true;
            return;
        }
        handled = true;
        break;
    case 11:
        if (!released)
            return;
        if (!repeat)
        {
            handleScale(false);
            handled = true;
            return;
        }
        handled = true;
        break;
    case 0x14:
        if (released)
        {
            navigateBack();
            handled = true;
        }
        break;
    case 0x15:
        if (released)
        {
            navigateBack();
            handled = true;
        }
        break;
    default:
        break;
    }
}

UIControl* UIScene_BiomePreviewMenu::GetMainPanel()
{
    return &mMainPanel;
}

int UIScene_BiomePreviewMenu::getSceneType()
{
    return 0x40;
}
