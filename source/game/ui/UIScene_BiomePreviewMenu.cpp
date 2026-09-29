#include "game/ui/UIScene_BiomePreviewMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>

#include "game/core/ConsoleUIController.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/L10N.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/StringConversion.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"
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
      mFullMapLabel(), mHelpLabel(), mCursor(), mInitData(0), mOverrideSettings(), mPreviewZoomLevel(0),
      mTargetZoomLevel(0), mImageReady(false), mSectionsChanged(false), mRegenerate(false),
      mPointerOffsetX(0.0f), mPointerOffsetY(0.0f), mMapLeft(0.0f), mMapRight(0.0f), mMapTop(0.0f),
      mMapBottom(0.0f), mPointer(), mStopRegen(false), mCriticalSection()
{
    initialiseMovie();
    mHelpLabel.init(UIString(L""));
    mFullMapLabel.init(UIString(L10N::GetString(0xb0507c4a)));
    mPlayableAreaLabel.init(UIString(L""));
    mCoordinatesLabel.init(UIString(L"0, 0"));
    mTimer.setVisible(false);

    mInitData = static_cast<LaunchMoreOptionsMenuInitData*>(data);
    mOverrideSettings.SetWidthInChunksToDisplay(0x36, 0);
    mOverrideSettings.SetSeed(mInitData->mSeed);
    mPlayableAreaLabel.setLabel(UIString(_toString<long long>(mOverrideSettings.GetSeed())), false, false);
    mOverrideSettings.SetGenerationScale(static_cast<BiomeSource::eBiomeGenerationScale>(mInitData->mBiomeScale));
    mOverrideSettings.SetCentrePosition(mInitData->mBiomeCentreXChunk, mInitData->mBiomeCentreZChunk);
    mPreviewZoomLevel = mTargetZoomLevel = (int)mOverrideSettings.GetZoomLevel();

    float width = (float)mBiomeMap.mImage.getWidth();
    float height = (float)mBiomeMap.mImage.getHeight();
    float left = (float)mBiomeMap.mImage.getXPos();
    float top = (float)mBiomeMap.mImage.getYPos();
    mMapTop = top;
    mMapLeft = left;
    mMapBottom = top + height;
    mPointerOffsetY = 0.0f;
    mPointerOffsetX = 0.0f;
    mMapRight = left + width;
    mPointer.y = (mMapBottom - top) * 0.5f + top;
    mPointer.x = (mMapRight - left) * 0.5f + left;
    setNewPointerPos(mPointer);

    InitializeCriticalSection(&mCriticalSection);
    mStopRegen = false;
    mImageReady = false;
    mRegenerate = true;
    mSectionsChanged = true;
    s_regenThread = new C4JThreadImpl(reinterpret_cast<int (*)(void*)>(&UIScene_BiomePreviewMenu::UpdateRegenBiomes),
                                       this, "Regenerating Section", 0);
    s_regenThread->SetProcessor(0);
    s_regenThread->Run();
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_BiomePreviewMenu::~UIScene_BiomePreviewMenu()
{
    mStopRegen = true;
    s_regenThread->WaitForCompletion(-1);
    delete s_regenThread;
    DeleteCriticalSection(&mCriticalSection);
    wchar_t biomeName[15];
    swprintf(biomeName, 15, L"BiomePreview%d", mPreviewZoomLevel);
    gConsoleUIController.unregisterSubstitutionTexture(biomeName, true);
    gConsoleUIController.unregisterSubstitutionTexture(L"NavigatorPreview", true);
}

void UIScene_BiomePreviewMenu::setNewPointerPos(float x, float y)
{
    mPointer.x = x;
    mPointer.y = y;
    float mapX = (x - mMapLeft) / (float)mBiomeMap.mImage.getWidth();
    float mapY = (mPointer.y - mMapTop) / (float)mBiomeMap.mImage.getHeight();
    Biome* biome = mOverrideSettings.GetBiomeAt(mapX, mapY);
    if (biome == 0)
    {
        mCursor.setLabel(UIString(L""), false, false);
        mHelpLabel.setLabel(UIString(L""), false, false);
    }
    else
    {
        std::vector<HtmlString> lines;
        std::wstring name = biome->getName(false);
        if (name.empty())
            name = biome->getName(true) + L" NEEDS LOCALISED";
        lines.push_back(HtmlString(name, eMinecraftColour_NOTSET, false, false, 0));
        mCursor.setLabel(UIString(HtmlString::Compose(&lines)), false, true);
        mHelpLabel.setLabel(UIString(biome->getDescription()), false, false);
    }
    BlockPos cursor = mOverrideSettings.GetCursorBlockPos(mapX, mapY);
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
    mSectionsChanged = false;
    if (sectionX != -1 && sectionZ != -1)
    {
        EnterCriticalSection(&mCriticalSection);
        if (mNavigatorMap.mTextureData == 0)
        {
            LeaveCriticalSection(&mCriticalSection);
            int size = mOverrideSettings.GetImageWidth() * mOverrideSettings.GetImageWidth() * 4;
            mNavigatorMap.mTextureData = new unsigned char[size];
            mNavigatorMap.mTextureDataSize = size;
            EnterCriticalSection(&mCriticalSection);
            memset(mNavigatorMap.mTextureData, 0, mNavigatorMap.mTextureDataSize);
        }

        int width = mOverrideSettings.GetBiomeSectionWidth();
        int height = mOverrideSettings.GetBiomeSectionWidth();
        if (atVerticalEdge)
            width /= 2;
        if (atHorizontalEdge)
            height /= 2;
        mOverrideSettings.UpdateBiomeMapPixelsInRegion((unsigned int*)mNavigatorMap.mTextureData, sectionX,
                                                       sectionZ, width, height, mSectionsChanged, true);
        LeaveCriticalSection(&mCriticalSection);
    }

    if (mOverrideSettings.isZoomed())
    {
        EnterCriticalSection(&mCriticalSection);
        if (mBiomeMap.mTextureData != 0)
        {
            delete[] mBiomeMap.mTextureData;
            mBiomeMap.mTextureDataSize = 0;
        }
        arrayWithLength<unsigned char> image =
            mOverrideSettings.CreateZoomedImage((unsigned int*)mNavigatorMap.mTextureData, mTargetZoomLevel);
        mBiomeMap.mTextureData = image.data;
        mBiomeMap.mTextureDataSize = image.length;
        LeaveCriticalSection(&mCriticalSection);
    }

    mRegenerate = false;
    mImageReady = true;
}

unsigned int UIScene_BiomePreviewMenu::UpdateRegenBiomes(void* data)
{
    UIScene_BiomePreviewMenu* scene = static_cast<UIScene_BiomePreviewMenu*>(data);
    IntCache::CreateNewThreadStorage();
    while (!scene->mStopRegen)
    {
        int sectionX;
        int sectionZ;
        bool atVerticalEdge;
        bool atHorizontalEdge;
        for (;;)
        {
            atHorizontalEdge = false;
            sectionX = -1;
            sectionZ = -1;
            atVerticalEdge = false;
            if ((!scene->mOverrideSettings.FullyGenerated() &&
                 scene->mOverrideSettings.RegenerateNearestBiomeSection(sectionX, sectionZ, atVerticalEdge,
                                                                        atHorizontalEdge)) ||
                scene->mRegenerate)
                break;
            if (scene->mStopRegen)
            {
                IntCache::ReleaseThreadStorage();
                return 0;
            }
        }
        scene->UpdatePixelCaches(sectionX, sectionZ, atVerticalEdge, atHorizontalEdge);
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
    int zoomTooltip = zoom == 1 ? -2 : zoom == 2 ? IDS_TOOLTIPS_ZOOM : -1;
    int scaleTooltip = zoom < 2 ? IDS_TOOLTIPS_ZOOM : -1;
    int scale = (int)mOverrideSettings.GetGenerationScale();
    int scaleSlot6 = scale == 1 ? -2 : scale == 2 ? IDS_TOOLTIPS_SCALE : -1;
    int scaleSlot7 = (scale == 0 || scale == 1) ? IDS_TOOLTIPS_SCALE : -1;

    gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_ACCEPT, IDS_TOOLTIPS_CANCEL, 0xd30e0a41, -1,
                                     zoomTooltip, scaleTooltip, scaleSlot6, scaleSlot7,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_BiomePreviewMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
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
    float sensitivity = (float)GameSettings::GetGameSettings(m_playerId, eGameSetting_InterfaceSensitivity) / 100.0f;
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

void UIScene_BiomePreviewMenu::componentExtents(UIControl& control, float& left, float& right, float& top,
                                                 float& bottom)
{
    left = (float)control.getXPos();
    top = (float)control.getYPos();
    right = left + (float)control.getWidth();
    bottom = top + (float)control.getHeight();
}

bool UIScene_BiomePreviewMenu::isOverComponent(UIControl& control, float x, float y)
{
    float left = 0.0f;
    float right = 0.0f;
    float top = 0.0f;
    float bottom = 0.0f;
    componentExtents(control, left, right, top, bottom);
    return left <= x && x <= right && top <= y && y <= bottom;
}

void UIScene_BiomePreviewMenu::onMouseTick()
{
    float dx = 0.0f;
    float dy = 0.0f;
    float dirX = 0.0f;
    float dirY = 0.0f;
    dmouse(dx, dy, dirX, dirY);
    if ((dirX != 0.0f || dirY != 0.0f) && isOverComponent(mBiomeMap.mImage, mPointer.x, mPointer.y))
    {
        float moveX = dx * dx * dirX * 13.0f;
        float moveY = dy * dy * dirY * 13.0f;
        _UIVec2D pos;
        pos.x = mPointer.x + mPointerOffsetX;
        pos.y = mPointer.y + mPointerOffsetY;
        float scale = 1.0f;
        if (!gRenderer->IsHiDef() || g_pApp->GetLocalPlayerCount() > 1)
            scale = 0.6f;
        moveX *= scale;
        moveY *= scale;
        pos.x += moveX;
        pos.y -= moveY;
        if (pos.x <= mMapLeft)
            pos.x = mMapLeft;
        if (mMapRight - pos.x < 0.0f)
            pos.x = mMapRight;
        if (pos.y <= mMapTop)
            pos.y = mMapTop;
        if (mMapBottom - pos.y < 0.0f)
            pos.y = mMapBottom;
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
        if (mTargetZoomLevel != mPreviewZoomLevel)
        {
            if (mBiomeMap.mTextureLoaded)
            {
                wchar_t oldNameBuf[15];
                swprintf(oldNameBuf, 15, L"BiomePreview%d", mPreviewZoomLevel);
                gConsoleUIController.unregisterSubstitutionTexture(oldNameBuf, true);
                mBiomeMap.mTextureLoaded = false;
                mBiomeMap.mTexture = GlStateManager::genTexture();
            }
            mPreviewZoomLevel = mTargetZoomLevel;
        }
        double scale = pow(2.0, (double)mPreviewZoomLevel);
        swprintf(nameBuf, 15, L"BiomePreview%d", mPreviewZoomLevel);
        imageWidth /= (int)scale;
        textureName = nameBuf;
    }
    CustomImage* image = getImage(wholeMap);
    gRenderer->TextureBind(image->mTexture);
    if (!image->mTextureLoaded)
    {
        gRenderer->TextureData(imageWidth, imageWidth, pixels.data, 0, C4JRender::eTextureFormat_RGBA8);
        image->mTextureLoaded = true;
        gConsoleUIController.registerSubstitutionTexture(textureName.c_str(), image->mTexture);
        image->mImage.setTextureName(textureName.c_str(), true);
    }
    else
    {
        gRenderer->TextureDataUpdate(0, 0, imageWidth, imageWidth, pixels.data, 0);
    }
    if (!wholeMap)
        setNewPointerPos(mPointer);
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
    gFui->dispatchMouseMoveEvent(m_movie, (float)(int)(mPointer.x * ((float)width / (float)m_stageWidth)),
                                 (float)(int)(mPointer.y * ((float)height / (float)m_stageHeight)));
    if (mImageReady && TryEnterCriticalSection(&mCriticalSection))
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
        mImageReady = false;
        LeaveCriticalSection(&mCriticalSection);
    }
}

void UIScene_BiomePreviewMenu::SetMapNavigator(int, int, int)
{
}

void UIScene_BiomePreviewMenu::updateMapNavigatorFromOverrideSettings()
{
    int scale = (int)pow(2.0, (double)mOverrideSettings.GetZoomLevel());
    SetMapNavigator(mOverrideSettings.GetZoomedCentreXPercent(), mOverrideSettings.GetZoomedCentreZPercent(),
                    scale);
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
        mSectionsChanged = true;
        LeaveCriticalSection(&mCriticalSection);
    }
}

void UIScene_BiomePreviewMenu::handleZoom(bool zoomOut)
{
    EnterCriticalSection(&mCriticalSection);
    bool changed = zoomOut ? mOverrideSettings.ZoomOut() : mOverrideSettings.ZoomIn();
    LeaveCriticalSection(&mCriticalSection);
    if (changed)
    {
        updateTooltips();
        updateMapNavigatorFromOverrideSettings();
        mRegenerate = true;
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
        mSectionsChanged = true;
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
        mSectionsChanged = true;
        if (changed)
        {
            mRegenerate = true;
            updateMapNavigatorFromOverrideSettings();
        }
    }
}

void UIScene_BiomePreviewMenu::handleInput(int playerId, int action, bool repeat, bool pressed,
                                            bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuX:
        if (pressed)
        {
            EnterCriticalSection(&mCriticalSection);
            mOverrideSettings.GenerateNewSeed();
            mPlayableAreaLabel.setLabel(UIString(_toString<long long>(mOverrideSettings.GetSeed())), false, false);
            updateTooltips();
            updateMapNavigatorFromOverrideSettings();
            memset(mNavigatorMap.mTextureData, 0, mNavigatorMap.mTextureDataSize);
            mSectionsChanged = true;
            LeaveCriticalSection(&mCriticalSection);
            handled = true;
        }
        break;
    case EControllerActions_MenuUp:
        if (pressed)
        {
            handleNavigate(NAV_UP);
            handled = true;
        }
        break;
    case EControllerActions_MenuDown:
        if (pressed)
        {
            handleNavigate(NAV_DOWN);
            handled = true;
        }
        break;
    case EControllerActions_MenuRight:
        if (!pressed)
            return;
        handleNavigate(NAV_RIGHT);
        handled = true;
        break;
    case EControllerActions_MenuLeft:
        if (pressed)
        {
            handleNavigate(NAV_LEFT);
            handled = true;
        }
        break;
    case EControllerActions_MenuPageUp:
        if (!pressed)
            return;
        if (!repeat)
            handleZoom(true);
        handled = true;
        break;
    case EControllerActions_MenuPageDown:
        if (!pressed)
            return;
        if (!repeat)
            handleZoom(false);
        handled = true;
        break;
    case EControllerActions_MenuRightScroll:
        if (!pressed)
            return;
        if (!repeat)
            handleScale(true);
        handled = true;
        break;
    case EControllerActions_MenuLeftScroll:
        if (!pressed)
            return;
        if (!repeat)
            handleScale(false);
        handled = true;
        break;
    case EControllerActions_MenuOk:
        if (pressed)
        {
            mInitData->mSeed = _toString<long long>(mOverrideSettings.GetSeed());
            mInitData->mBiomeScale = mOverrideSettings.GetBiomeScale();
            mInitData->mBiomeCentreXChunk = mOverrideSettings.GetCentreXChunk();
            mInitData->mBiomeCentreZChunk = mOverrideSettings.GetCentreZChunk();
            navigateBack();
            handled = true;
        }
        break;
    case EControllerActions_MenuCancel:
        if (pressed)
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
    return EUIScene_BiomePreviewMenu;
}
