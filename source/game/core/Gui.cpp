#include "game/core/Gui.hpp"
#include "game/core/StringIds.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

#include <boost/assert.hpp>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/BossHealthOverlay.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/Item.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/ThermalAreaRuleDefinition.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/util/AABB.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/core/XLanguage.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/MemSect.hpp"

float gGuiScreenScale = 1.0f;

namespace
{
ResourceLocation guiPumpkinOverlayTexture(static_cast<_TEXTURE_NAME>(2));
ResourceLocation guiThermalOverlayTexture(static_cast<_TEXTURE_NAME>(171));
ResourceLocation guiThermalLiftOverlayTexture(static_cast<_TEXTURE_NAME>(172));
}

Gui::Gui(Minecraft* minecraft)
    : mItemRenderer(0),
      mRandom(new Random()),
      mMinecraft(minecraft),
      mUnknownD4(0),
      mUnknownE8(0),
      mUnknownEC(7),
      mTickCount(0),
      mSingleMessageVisible(false),
      mJukeboxTime(0.0f),
      mUnknown19C(2.55f),
      mUnknown1D0(0),
      mVignetteBrightness(1.0f)
{
    int i;
    for (i = 0; i < 4; ++i) {
        mMessages[i].clear();
        mNowPlaying[i].clear();
        mSingleMessages[i].clear();
        mSingleMessageTime[i] = 0;
        mBossOverlays[i] = new BossHealthOverlay(minecraft);
    }
    for (i = 0; i < 4; ++i) {
        mCharacterDisplayTime[i] = 0;
        mThermalOverlayRules[i] = 0;
        mThermalOverlayFade[i] = 0;
    }
    if (minecraft != 0) {
        mItemRenderer = minecraft->getItemRenderer();
    }
}

BossHealthOverlay* Gui::getBossOverlay(int index)
{
    return mBossOverlays[index];
}

void Gui::renderVignette(float partialTick, int width, int height)
{
    float target = 1.0f - partialTick;
    if (target < 0.5f) {
        mVignetteBrightness += (0.5f - mVignetteBrightness) * 0.01f;
    } else {
        if (target > 1.0f) {
            target = 1.0f;
        }
        mVignetteBrightness += (target - mVignetteBrightness) * 0.01f;
    }
}

void Gui::renderPumpkin(int width, int height)
{
    GlStateManager::disableDepthTest();
    GlStateManager::depthMask(false);
    GlStateManager::blendFunc(4, 5);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableAlphaTest();
    MemSect(0x1f);
    mMinecraft->mTextures->bindTexture(&guiPumpkinOverlayTexture);
    MemSect(0);
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    builder->begin();
    builder->vertexUV(0.0f, static_cast<float>(height), -90.0f, 0.0f, 1.0f);
    builder->vertexUV(static_cast<float>(width), static_cast<float>(height), -90.0f, 1.0f, 1.0f);
    builder->vertexUV(static_cast<float>(width), 0.0f, -90.0f, 1.0f, 0.0f);
    builder->vertexUV(0.0f, 0.0f, -90.0f, 0.0f, 0.0f);
    builder->end();
    GlStateManager::depthMask(true);
    GlStateManager::enableDepthTest();
    GlStateManager::enableAlphaTest();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void Gui::renderPortalOverlay(float portalTime, int width, int height)
{
    if (portalTime < 1.0f) {
        portalTime = portalTime * portalTime * portalTime * portalTime * 0.8f + 0.2f;
    }
    GlStateManager::disableAlphaTest();
    GlStateManager::disableDepthTest();
    GlStateManager::depthMask(false);
    GlStateManager::blendFunc(4, 5);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, portalTime);
    MemSect(0x1f);
    mMinecraft->mTextures->bindTexture(&BLOCKS_LOCATION);
    MemSect(0);
    TextureAtlasSprite* sprite = Blocks::portal->getTexture(Direction::UP);
    const double minU = sprite->getU0(false);
    const double minV = sprite->getV0(false);
    const double maxU = sprite->getU1(false);
    const double maxV = sprite->getV1(false);
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    builder->begin();
    builder->vertexUV(0.0f, static_cast<float>(height), -90.0f, static_cast<float>(minU), static_cast<float>(maxV));
    builder->vertexUV(static_cast<float>(width), static_cast<float>(height), -90.0f, static_cast<float>(maxU), static_cast<float>(maxV));
    builder->vertexUV(static_cast<float>(width), 0.0f, -90.0f, static_cast<float>(maxU), static_cast<float>(minV));
    builder->vertexUV(0.0f, 0.0f, -90.0f, static_cast<float>(minU), static_cast<float>(minV));
    builder->end();
    GlStateManager::depthMask(true);
    GlStateManager::enableDepthTest();
    GlStateManager::enableAlphaTest();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void Gui::renderThermalOverlay(ThermalAreaRuleDefinition* rule, float alpha, int width, int height,
                               float partialTick)
{
    if (rule->isEnabled() && rule->getElytraSpeedBoost() > 0.0) {
        return;
    }
    GlStateManager::disableAlphaTest();
    GlStateManager::disableDepthTest();
    GlStateManager::depthMask(false);
    GlStateManager::blendFunc(4, 5);
    MemSect(0x1f);
    ResourceLocation* texture = &guiThermalOverlayTexture;
    float textureAlpha = 1.0f;
    if (rule->isEnabled()) {
        if (rule->getLiftForceModifier() <= 1.0) {
            if (rule->getMaxHeight() > 0.0 || rule->getLiftSpeed() != 0.0)
                texture = &guiThermalLiftOverlayTexture;
        } else {
            textureAlpha = 0.5f;
        }
    }
    mMinecraft->mTextures->bindTexture(texture);
    MemSect(0);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, textureAlpha);
    const float time = (static_cast<float>(mMinecraft->mGameRenderer->getTickCount()) + partialTick) *
                       -0.011111111f;
    const float topV = time + 1.0f;
    const float bottomV = time + 0.25f;
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    builder->begin();
    builder->color(1.0f, 1.0f, 1.0f, alpha * 0.5f);
    builder->vertexUV(0.0f, static_cast<float>(height), -90.0f, 0.0f, topV);
    builder->vertexUV(static_cast<float>(width), static_cast<float>(height), -90.0f, 6.0f, topV);
    builder->color(1.0f, 1.0f, 1.0f, 0.0f);
    builder->vertexUV(static_cast<float>(width), static_cast<float>(height) * 0.25f, -90.0f, 6.0f, bottomV);
    builder->vertexUV(0.0f, static_cast<float>(height) * 0.25f, -90.0f, 0.0f, bottomV);
    builder->end();
    GlStateManager::depthMask(true);
    GlStateManager::enableDepthTest();
    GlStateManager::enableAlphaTest();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void Gui::render(float partialTick, bool hideGui, int width, int height)
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = mMinecraft->mPlayer;
    const int playerIndex = player->mUserIndex;
    const int viewport = player->mViewport;
    const int setting = static_cast<int>(GameSettings::GetGameSettings(
        playerIndex, viewport == 0 ? eGameSetting_HUDSize : eGameSetting_HUDSizeSplitscreen));
    const int guiScale = setting + 2;
    ScreenSizeCalculator size(mMinecraft->mOptions, guiScale);
    const int screenWidth = static_cast<int>(size.getWidth());
    const int screenHeight = static_cast<int>(size.getHeight());
    gGuiScreenScale = static_cast<float>(guiScale);

    float xScale = 0.5f;
    float yScale = 0.5f;
    float scaleOne = 1.0f;
    int x = 0;
    int y = 0;
    bool joinedLayout = false;
    switch (viewport) {
    case 0:
        x = screenWidth / 20;
        y = screenHeight / 20;
        xScale = scaleOne;
        yScale = scaleOne;
        break;
    case 1:
        gGuiScreenScale *= 0.5f;
        x = screenWidth / 10;
        y = screenHeight / 10;
        yScale = scaleOne;
        joinedLayout = true;
        break;
    case 2:
        gGuiScreenScale *= 0.5f;
        x = screenWidth / 10;
        y = 0;
        yScale = scaleOne;
        joinedLayout = true;
        break;
    case 3:
        gGuiScreenScale *= 0.5f;
        x = screenWidth / 10;
        y = screenHeight / 10;
        xScale = scaleOne;
        joinedLayout = true;
        break;
    case 4:
        gGuiScreenScale *= 0.5f;
        x = 0;
        y = (guiScale == 3 ? 0 : (guiScale == 4 ? -5 : 10)) + screenHeight / 10;
        xScale = scaleOne;
        joinedLayout = true;
        break;
    case 5:
        gGuiScreenScale *= 0.5f;
        x = screenWidth / 10;
        y = screenHeight / 10;
        xScale = scaleOne;
        yScale = scaleOne;
        break;
    case 6:
        gGuiScreenScale *= 0.5f;
        x = 0;
        y = screenHeight / 10;
        xScale = scaleOne;
        yScale = scaleOne;
        break;
    case 7:
        gGuiScreenScale *= 0.5f;
        x = screenWidth / 10;
        y = 0;
        xScale = scaleOne;
        yScale = scaleOne;
        break;
    case 8:
        gGuiScreenScale *= 0.5f;
        xScale = scaleOne;
        yScale = scaleOne;
        break;
    default:
        xScale = scaleOne;
        yScale = scaleOne;
        break;
    }

    bool joinTransform = false;
    if (!gConsoleUIController.GetMenuDisplayed(playerIndex))
        joinTransform = gApp.GetXuiAction(playerIndex) != eXuiAction_AutoSave;
    if (GameSettings::GetGameSettings(playerIndex, eGameSetting_Tooltips) == 0 && joinTransform)
        BOOST_ASSERT(mMinecraft->mPlayer);
    if (GameSettings::GetGameSettings(playerIndex, eGameSetting_DisplayHUD) == 0)
        joinTransform = false;

    mMinecraft->mGameRenderer->setupGuiScreen(guiScale);
    mMinecraft->mGameRenderer->renderItemActivationAnimation(screenWidth, screenHeight, partialTick);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);

    if (Minecraft::useFancyGraphics())
        renderVignette(player->getBrightness(), screenWidth, screenHeight);

    not_null_ptr<ItemInstance> helmet = player->mInventory->getArmor(3);
    if (!player->GetThirdPersonView(false) &&
        helmet->getItem() == Item::byBlock(Blocks::carved_pumpkin) && !player->isSpectator())
        renderPumpkin(screenWidth, screenHeight);

    if (!player->hasEffect(MobEffects::CONFUSION)) {
        const float portalIntensity = player->mPortalEffectIntensityO +
                                      (player->mPortalEffectIntensity - player->mPortalEffectIntensityO) *
                                          partialTick;
        if (portalIntensity > 0.0f)
            renderPortalOverlay(portalIntensity, screenWidth, screenHeight);
    }

    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (Minecraft::InMiniGame(eMiniGameId_Glide, true)) {
        if (options && options->getRequiredGameRules()) {
            std::vector<ThermalAreaRuleDefinition*> areas;
            options->getRequiredGameRules()->getThermalAreas(&areas);
            for (std::vector<ThermalAreaRuleDefinition*>::const_iterator it = areas.begin();
                 it != areas.end(); ++it) {
                ThermalAreaRuleDefinition* rule = *it;
                if (!rule->isEnabled() || player->getFallFlyingTicks() <= 4)
                    continue;
                boost::shared_ptr<Entity> entity = boost::static_pointer_cast<Entity>(player);
                if (!rule->getConditionsMet(entity))
                    continue;
                AABB* area = rule->getArea();
                AABB* sweptVolume = player->getSweptVolume();
                if (sweptVolume->intersects(area->mMinX, area->mMinY, area->mMinZ,
                                            area->mMaxX, area->mMaxY, area->mMaxZ)) {
                    mThermalOverlayRules[playerIndex] = rule;
                    mThermalOverlayFade[playerIndex] = 30;
                    break;
                }
            }
            ThermalAreaRuleDefinition* rule = mThermalOverlayRules[playerIndex];
            int& fade = mThermalOverlayFade[playerIndex];
            if (rule && fade >= 0) {
                renderThermalOverlay(rule, static_cast<float>(fade) / 30.0f,
                                     screenWidth, screenHeight, partialTick);
                --fade;
            } else {
                mThermalOverlayRules[playerIndex] = 0;
                fade = 0;
            }
        }
    }

    if (!mMinecraft->mGameMode->isCutScene()) {
        if (joinTransform && joinedLayout) {
            GlStateManager::pushMatrix();
            GlStateManager::scalef(xScale, yScale, xScale);
        }
        if (GameSettings::GetGameSettings(playerIndex, eGameSetting_AnimatedCharacter) != 0 && joinTransform) {
            bool renderPreview = true;
            if (player->isSneaking() || player->isSprinting()) {
                mCharacterDisplayTime[playerIndex] = 30;
            } else if (player->mAbilities.mFlying || player->isFallFlying() || player->isFallFlying()) {
                mCharacterDisplayTime[playerIndex] = 5;
            } else {
                if (mCharacterDisplayTime[playerIndex] < 1) {
                    renderPreview = false;
                } else {
                    --mCharacterDisplayTime[playerIndex];
                    renderPreview = mCharacterDisplayTime[playerIndex] >= 1;
                }
            }

            if (renderPreview) {
                boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();
                boost::shared_ptr<Entity> renderedPlayer = boost::static_pointer_cast<Entity>(player);
                EntityRenderDispatcher* dispatcher = mMinecraft->getEntityRenderDispatcher();
                dispatcher->prepare(mMinecraft->mLevel, mMinecraft->mFont, camera, mMinecraft->mSpectateEntity,
                                    mMinecraft->mOptions, partialTick);

                const float savedYRot = player->mYRot;
                const float savedXRot = player->mXRot;
                const float savedYRotO = player->mYRotO;
                const int savedFireTicks = player->mRemainingFireTicks;
                const bool savedSharedFlag = player->getSharedFlag(Entity::FLAG_ONFIRE);

                GlStateManager::enableRescaleNormal();
                GlStateManager::enableColorMaterial();
                GlStateManager::pushMatrix();
                GlStateManager::translatef(static_cast<float>(x + 10), static_cast<float>(y + 17), 50.0f);
                GlStateManager::scalef(-12.0f, 12.0f, 12.0f);
                GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);
                GlStateManager::rotatef(135.0f, 0.0f, 1.0f, 0.0f);
                Lighting::turnOn();
                GlStateManager::rotatef(-135.0f, 0.0f, 1.0f, 0.0f);
                const float headPitch = std::atan(0.25f);
                GlStateManager::rotatef(-headPitch * 20.0f, 1.0f, 0.0f, 0.0f);
                const float interpolatedBodyYaw = player->mYBodyRotO +
                                                  (player->mYBodyRot - player->mYBodyRotO);
                GlStateManager::rotatef(interpolatedBodyYaw + 15.707963943481445f,
                                        0.0f, 1.0f, 0.0f);
                GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);

                if (!player->isFallFlying()) {
                    player->mYRot = interpolatedBodyYaw;
                    player->mYRotO = interpolatedBodyYaw;
                    player->mXRot = -std::atan(0.25f) * 20.0f;
                }
                player->clearFire();
                player->setSharedFlag(Entity::FLAG_ONFIRE, false);
                if (!player->isFallFlying()) {
                    GlStateManager::translatef(0.0f, -1.62f, 0.0f);
                } else {
                    const double pitch = Mth::clamp(static_cast<double>(player->mXRot), -90.0, 90.0);
                    GlStateManager::translatef(0.0f, 1.62f * static_cast<float>(pitch / 90.0), 0.0f);
                }
                dispatcher->mCameraYRot = 180.0f;
                dispatcher->mRenderName = true;
                dispatcher->render(renderedPlayer, 0.0, 0.0, 0.0, 0.0f,
                                   1.0f, false, false, true, false);
                dispatcher->mRenderName = false;

                player->mYRot = savedYRot;
                player->mXRot = savedXRot;
                player->mYRotO = savedYRotO;
                player->mRemainingFireTicks = savedFireTicks;
                player->setSharedFlag(Entity::FLAG_ONFIRE, savedSharedFlag);
                GlStateManager::popMatrix();
                Lighting::turnOff();
                GlStateManager::disableRescaleNormal();
            }
        }
    }

    GlStateManager::disableBlend();
    GlStateManager::blendFunc(4, 5);
    const int sleepTimer = static_cast<int>(player->getSleepTimer());
    if (sleepTimer > 0) {
        GlStateManager::disableDepthTest();
        GlStateManager::disableAlphaTest();
        float alpha = static_cast<float>(sleepTimer) / 100.0f;
        if (alpha > 1.0f)
            alpha = 1.0f - static_cast<float>(sleepTimer - 100) / 10.0f;
        fill(0, 0, static_cast<int>(static_cast<float>(screenWidth) / xScale),
             static_cast<int>(static_cast<float>(screenHeight) / yScale),
             (static_cast<int>(220.0f * alpha) << 24) | 0x101020);
        GlStateManager::enableAlphaTest();
        GlStateManager::enableDepthTest();
    }
    if (!player->isAlive()) {
        GlStateManager::disableDepthTest();
        GlStateManager::disableAlphaTest();
        const float alpha = static_cast<float>(player->getDeathFadeTimer()) / 21.0f;
        fill(0, 0, static_cast<int>(static_cast<float>(screenWidth) / xScale),
             static_cast<int>(static_cast<float>(screenHeight) / yScale),
             (static_cast<int>(220.0f * alpha) << 24) | 0x200000);
        GlStateManager::enableAlphaTest();
        GlStateManager::enableDepthTest();
    }

    mJukeboxTime = partialTick;
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::disableAlphaTest();
    if (joinTransform && joinedLayout)
        GlStateManager::popMatrix();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableBlend();
    GlStateManager::enableAlphaTest();
}

void Gui::renderSlot(int playerIndex, int x, int y, float partialTick)
{
    not_null_ptr<ItemInstance> item =
        mMinecraft->mPlayer->mInventory->getItem(static_cast<unsigned int>(playerIndex));
    if (item->isEmpty()) {
        return;
    }

    const int popTime = item->getPopTime();
    const float popRemaining = static_cast<float>(popTime) - partialTick;
    if (0.0f < popRemaining) {
        const float popScale = popRemaining / 5.0f + 1.0f;
        GlStateManager::pushMatrix();
        GlStateManager::translatef(static_cast<float>(x + 8), static_cast<float>(y + 12), 0.0f);
        GlStateManager::scalef(1.0f / popScale, (popScale + 1.0f) * 0.5f, 1.0f);
        GlStateManager::translatef(static_cast<float>(-(x + 8)), static_cast<float>(-(y + 12)), 0.0f);
    }

    mItemRenderer->renderAndDecorateItem(item, x, y);

    if (0.0f < popRemaining) {
        GlStateManager::popMatrix();
    }

    mItemRenderer->renderGuiItemDecorations(mMinecraft->mFont, item, x, y, 1.0f);
}

void Gui::tick()
{
    int i;
    ++mTickCount;
    for (i = 0; i < 4; ++i) {
        if (mSingleMessageTime[i] > 0) {
            --mSingleMessageTime[i];
        }
        if (mSingleMessageTime[i] == 0) {
            mSingleMessages[i].clear();
        }
        std::vector<GuiMessage>::iterator it;
        for (it = mMessages[i].begin(); it != mMessages[i].end(); ++it) {
            ++it->age;
        }
    }
}

void Gui::clearMessages(int playerIndex)
{
    if (playerIndex == -1) {
        int i;
        for (i = 0; i < 4; ++i) {
            mMessages[i].clear();
        }
    } else if (playerIndex >= 0 && playerIndex < 4) {
        mMessages[playerIndex].clear();
    }
}

void Gui::addMessage(const std::wstring& message, int playerIndex, bool overlay, bool filtered,
                     bool append, bool force)
{
    std::wstring text = message;

    boost::shared_ptr<MultiplayerLocalPlayer> player = mMinecraft->mPlayer;
    BOOST_ASSERT(player.get() != 0);
    const unsigned int language = XGetLanguage();
    const bool narrowGlyphs = language != XC_LANGUAGE_JAPANESE && language != XC_LANGUAGE_KOREAN &&
                              language != XC_LANGUAGE_TCHINESE;
    unsigned int maxLength;
    if (static_cast<unsigned int>(player->mViewport) < 3) {
        if (narrowGlyphs) {
            maxLength = gRenderer->IsHiDef() ? 0x69 : 0x37;
        } else {
            maxLength = gRenderer->IsHiDef() ? 0x46 : 0x23;
        }
    } else {
        maxLength = narrowGlyphs ? 0x37 : 0x23;
    }

    bool wrapped = false;
    while (text.length() > maxLength) {
        const unsigned int searchEnd = std::min<unsigned int>(text.length(), maxLength);
        unsigned int split = static_cast<unsigned int>(text.find_last_of(L" ", searchEnd));
        if (!narrowGlyphs) {
            split = maxLength;
        }
        if (split != searchEnd) {
            ++split;
        }
        addMessage(text.substr(0, split), playerIndex, overlay, filtered, true, false);
        text = text.substr(split);
        wrapped = true;
    }

    if (playerIndex == -1) {
        int i;
        for (i = 0; i < 4; ++i) {
            if (mMinecraft->mLocalPlayers[i] == NULL) {
                continue;
            }
            if (GameSettings::GetGameSettings(i, overlay ? eGameSetting_DeathMessages : eGameSetting_DisplayGameMessages) != 0 ||
                force) {
                mMessages[i].insert(mMessages[i].begin(), GuiMessage(text));
                while (mMessages[i].size() > 50) {
                    mMessages[i].pop_back();
                }
            }
        }
        return;
    }

    if (GameSettings::GetGameSettings(playerIndex, overlay ? eGameSetting_DeathMessages : eGameSetting_DisplayGameMessages) != 0 ||
        force) {
        mMessages[playerIndex].insert(mMessages[playerIndex].begin(), GuiMessage(text));
        while (mMessages[playerIndex].size() > 50) {
            mMessages[playerIndex].pop_back();
        }
    }
    if (overlay && filtered) {
        if (append) {
            mNowPlaying[playerIndex] = text + L"\n";
        } else if (wrapped) {
            mNowPlaying[playerIndex] += text;
        } else {
            mNowPlaying[playerIndex] = text;
        }
    }
}

void Gui::clearSingleMessages()
{
    int i;
    for (i = 0; i < 4; ++i) {
        clearSingleMessages(i);
    }
    mSingleMessageVisible = false;
}

void Gui::clearSingleMessages(int playerIndex)
{
    if (playerIndex >= 0 && playerIndex < 4) {
        mSingleMessages[playerIndex].clear();
        mSingleMessageTime[playerIndex] = 0;
    }
    mSingleMessageVisible = false;
}

void Gui::setSingleMessage(const std::wstring& message, int playerIndex, int duration)
{
    if (playerIndex >= 0 && playerIndex < 4) {
        mSingleMessages[playerIndex] = message;
        mSingleMessageTime[playerIndex] = duration < 1 ? 0xb4 : duration;
        mSingleMessageVisible = true;
    }
}

void Gui::setNowPlaying(const std::wstring& message)
{
    int i;
    const wchar_t* prefix = L10N::GetString(IDS_NOWPLAYING);
    std::wstring text = prefix == 0 ? std::wstring() : std::wstring(prefix);
    text += message;
    for (i = 0; i < 4; i += 2) {
        mNowPlaying[i] = text;
        mNowPlaying[i + 1] = mNowPlaying[i];
        mSingleMessageTime[i] = 0x3c;
        mSingleMessageTime[i + 1] = 0x3c;
    }
    mSingleMessageVisible = true;
}

void Gui::displayClientMessage(int stringId, int playerIndex)
{
    const wchar_t* text = L10N::GetString(stringId);
    addMessage(text == 0 ? std::wstring() : std::wstring(text), playerIndex, false, false, false, false);
}

void Gui::onDisconnected()
{
    int i;
    for (i = 0; i < 4; ++i) {
        mBossOverlays[i]->reset();
    }
}

double Gui::getOpacity(int playerIndex, unsigned int messageIndex)
{
    const float minimum = 0.5f;
    if (playerIndex >= 0 && playerIndex < 4 && messageIndex < mMessages[playerIndex].size() &&
        mMessages[playerIndex][messageIndex].age < 200) {
        float value = (1.0f - static_cast<float>(mMessages[playerIndex][messageIndex].age) / 200.0f) * 10.0f;
        float opacity = value < minimum ? minimum : (value > 1.0f ? 1.0f : value);
        return static_cast<double>(opacity * opacity);
    }
    return 0.0;
}

double Gui::getJukeboxOpacity(int playerIndex)
{
    float value = 0.0f;
    if (playerIndex >= 0 && playerIndex < 4) {
        value = (static_cast<float>(mSingleMessageTime[playerIndex]) - mJukeboxTime) * 256.0f / 20.0f;
    }
    if (value > 255.0f) {
        value = 255.0f;
    }
    if (value < 0.0f) {
        value = 0.0f;
    }
    return static_cast<double>(value / 255.0f);
}

void Gui::ShowCharacterDisplay(int playerIndex)
{
    if (playerIndex >= 0 && playerIndex < 4) {
        mCharacterDisplayTime[playerIndex] = 90;
    }
}
