#include "game/core/Gui.hpp"

#include <algorithm>

#include "game/core/BossHealthOverlay.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

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
      mVignetteBrightness(0.5f)
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
    GlStateManager::depthMask(true);
    GlStateManager::enableDepthTest();
    GlStateManager::enableAlphaTest();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void Gui::renderThermalOverlay(ThermalAreaRuleDefinition* rule, float alpha, int width, int height,
                               float partialTick)
{
    if (rule == 0) {
        return;
    }
    GlStateManager::disableAlphaTest();
    GlStateManager::disableDepthTest();
    GlStateManager::depthMask(false);
    GlStateManager::blendFunc(4, 5);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, alpha * 0.5f);
    GlStateManager::depthMask(true);
    GlStateManager::enableDepthTest();
    GlStateManager::enableAlphaTest();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void Gui::render(float partialTick, bool hideGui, int width, int height)
{
    if (hideGui) {
        return;
    }
    if (Minecraft::useFancyGraphics()) {
        mMinecraft->mPlayer->getBrightness();
        renderVignette(partialTick, width, height);
    }
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
    if (playerIndex == -1) {
        int i;
        for (i = 0; i < 4; ++i) {
            addMessage(message, i, overlay, filtered, append, force);
        }
        return;
    }
    if (playerIndex < 0 || playerIndex >= 4) {
        return;
    }
    if (GameSettings::GetGameSettings(playerIndex, static_cast<eGameSetting>(overlay ? 0x1a : 0x26)) != 0 ||
        force) {
        mMessages[playerIndex].push_back(GuiMessage(message));
        if (mMessages[playerIndex].size() > 50) {
            mMessages[playerIndex].erase(mMessages[playerIndex].begin());
        }
    }
    if (overlay && filtered) {
        if (append) {
            mNowPlaying[playerIndex] += message;
        } else {
            mNowPlaying[playerIndex] = message;
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

uint64_t Gui::setSingleMessage(const std::wstring& message, int playerIndex, int duration)
{
    if (playerIndex >= 0 && playerIndex < 4) {
        mSingleMessages[playerIndex] = message;
        mSingleMessageTime[playerIndex] = duration < 1 ? 0xb4 : duration;
        mSingleMessageVisible = true;
    }
    return static_cast<uint64_t>((unsigned long)this) << 32;
}

uint64_t Gui::setNowPlaying(const std::wstring& message)
{
    int i;
    const wchar_t* prefix = L10N::GetString(0x91249e7e);
    std::wstring text = prefix == 0 ? std::wstring() : std::wstring(prefix);
    text += message;
    for (i = 0; i < 4; i += 2) {
        mNowPlaying[i] = text;
        mNowPlaying[i + 1] = mNowPlaying[i];
        mSingleMessageTime[i] = 0x3c;
        mSingleMessageTime[i + 1] = 0x3c;
    }
    mSingleMessageVisible = true;
    return static_cast<uint64_t>((unsigned long)this) << 32;
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
    float opacity = minimum;
    if (playerIndex >= 0 && playerIndex < 4 && messageIndex < mMessages[playerIndex].size() &&
        mMessages[playerIndex][messageIndex].age < 200) {
        float value = (1.0f - static_cast<float>(mMessages[playerIndex][messageIndex].age) / 200.0f) * 10.0f;
        opacity = value < minimum ? minimum : (value > 1.0f ? 1.0f : value);
    }
    return static_cast<double>(opacity * opacity);
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
