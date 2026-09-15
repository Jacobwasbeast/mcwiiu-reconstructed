#include "game/ui/FJ_Hud.hpp"

#include <algorithm>

#include "game/ui/FJ_BitmapIcon.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_Helpers.hpp"
#include "game/ui/FJ_MobEffect_Tile.hpp"
#include "game/ui/FJ_MobEffect_Tile_Normal.hpp"
#include "game/ui/FJ_MobEffect_Tile_Small.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_ProgressBar.hpp"
#include "game/util/Math.hpp"

#include <cstdio>
#include <cstring>
#include <cwchar>

#include "game/ui/fuiRect.hpp"

#include "game/ui/FJ_ExperienceBar.hpp"
#include "game/ui/FJ_HorseJumpBar.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_Label_HUD_Splash.hpp"
#include "game/ui/FJ_Label_HUD_SplashBig.hpp"
#include "game/ui/FJ_LabelOutline.hpp"

namespace
{
template<typename T>
T& hudField(FJ_Hud* self, size_t offset)
{
    return *reinterpret_cast<T*>(reinterpret_cast<char*>(self) + offset);
}

FJ_FuiNode*& hudNode(FJ_Hud* self, size_t offset)
{
    return hudField<FJ_FuiNode*>(self, offset);
}

int& hudInt(FJ_Hud* self, size_t offset)
{
    return hudField<int>(self, offset);
}

float& hudFloat(FJ_Hud* self, size_t offset)
{
    return hudField<float>(self, offset);
}

unsigned char& hudByte(FJ_Hud* self, size_t offset)
{
    return hudField<unsigned char>(self, offset);
}

bool& hudBool(FJ_Hud* self, size_t offset)
{
    return hudField<bool>(self, offset);
}

float loadFloatBits(int bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

struct HudGlobalNames
{
    std::string crosshair[2];
    std::string offhandSlot[2];
    std::string hudRoot[2];
    std::string hudXpLevel[2];
    std::string scale[2];
    std::string airBubbleFull;
    std::string airBubblePop;
    std::string airBubbleEmpty;
    std::string hudSizeLabel[2];
};

HudGlobalNames& hudGlobalNames()
{
    static HudGlobalNames names;
    static bool initialized = false;
    if (!initialized)
    {
        names.crosshair[0] = "CrosshairSmall";
        names.crosshair[1] = "CrosshairNormal";
        names.offhandSlot[0] = "OffhandSlot_Small";
        names.offhandSlot[1] = "OffhandSlot_Normal";
        names.hudRoot[0] = "HudSmall";
        names.hudRoot[1] = "HudNormal";
        names.hudXpLevel[0] = "HUD_XP_Level_Small";
        names.hudXpLevel[1] = "HUD_XP_Level_Normal";
        names.scale[0] = "ScaleSmall";
        names.scale[1] = "ScaleNormal";
        names.airBubbleFull = "AirBubble_Full";
        names.airBubblePop = "AirBubble_Pop";
        names.airBubbleEmpty = "AirBubble_Empty";
        names.hudSizeLabel[0] = "Small";
        names.hudSizeLabel[1] = "Normal";
        initialized = true;
    }
    return names;
}

const std::string& hudVariantName(const std::string* table, int hudSize)
{
    if (hudSize < 0)
        hudSize = 0;
    if (hudSize > 1)
        hudSize = 1;
    return table[hudSize];
}

void positionGameModeChildX(FJ_FuiNode* node, float x)
{
    if (node != NULL)
        node->setX(x);
}

void positionGameModeChildY(FJ_FuiNode* node, float y)
{
    if (node != NULL)
        node->setY(y);
}

float alignedBelow(float anchorY, float anchorHeight, float selfHeight, float factor)
{
    return static_cast<float>(Math::round((anchorHeight - selfHeight) * factor)) + anchorY;
}

float alignedBeside(float anchorX, float anchorWidth, float offset, float factor)
{
    return static_cast<float>(Math::round(-(anchorWidth * factor - (anchorX - offset))));
}

std::wstring& hudSplashText(FJ_Hud* self)
{
    return hudField<std::wstring>(self, 0x4b8);
}

void setHudNodeVisible(FJ_Hud* self, size_t offset, bool visible)
{
    FJ_FuiNode* node = hudNode(self, offset);
    if (node != NULL)
        node->setVisible(visible);
}

void setHudWideLabel(FJ_FuiNode* node, const std::wstring& label)
{
    if (node == NULL)
        return;
    if (node->isFJ_Label_HUD_SplashBig())
    {
        node->asFJ_Label_HUD_SplashBig()->SetLabel(label);
        return;
    }
    if (node->isFJ_Label_HUD_Splash())
    {
        node->asFJ_Label_HUD_Splash()->SetLabel(label);
        return;
    }
    FJ_Base* base = node->asFJ_Base();
    if (base != NULL)
        base->SetLabel(label);
    else
    {
        FJ_Label* text = node->asFJ_Label();
        if (text != NULL)
            text->SetLabel(label);
    }
}

FJ_FuiNode* splashTextPanelChild(FJ_FuiNode* splashClip)
{
    if (splashClip == NULL)
        return NULL;
    FJ_FuiNode* panel = splashClip->getChildByName("TextPanel");
    return panel != NULL ? panel->getChildAt(0) : NULL;
}
} // namespace


FJ_Hud::FJ_Hud(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode)
    , m_gameMode(0)
{
    std::fill(m_layout, m_layout + sizeof(m_layout), 0);
    std::fill(m_statusEffectTiles, m_statusEffectTiles + 27, static_cast<FJ_MobEffect_Tile*>(NULL));
    std::fill(m_statusEffectAmbient, m_statusEffectAmbient + 27, false);
    std::fill(m_damageIndicators, m_damageIndicators + 4, static_cast<FJ_FuiNode*>(NULL));
    std::fill(m_damageIndicatorAlpha, m_damageIndicatorAlpha + 4, 0.0f);
    m_iggySplash = NULL;
    m_offhandSlot = NULL;
    m_hotbarAnchor = NULL;
    m_splashX = -1.0f;
    m_splashY = -1.0f;
    m_offhandEnabled = false;
    m_rightHanded = false;

    hudField<short>(this, 0x46c) = 0;
    hudInt(this, 0x30) = 0;
    hudInt(this, 0x484) = 7;
    hudInt(this, 0x4d4) = 7;
    hudInt(this, 0x480) = 0;
    hudField<short>(this, 0x4bc) = 0;
    for (int flag = 0x40; flag <= 0x4b; ++flag)
        hudByte(this, static_cast<size_t>(flag)) = 1;
    hudInt(this, 0x4d0) = 0;
    hudInt(this, 0x34) = 0;
    hudInt(this, 0x38) = 0;
    hudInt(this, 0x3c) = 0;

    float stageHeight = getStage()->getStageHeight();
    bool standardResolution = (stageHeight == 1080.0f) || (stageHeight == 540.0f);
    hudByte(this, 0x20) = standardResolution;
    if (standardResolution)
    {
        hudInt(this, 0x44c) = -1;
        hudInt(this, 0x450) = -1;
        hudInt(this, 0x24) = 10;
        hudInt(this, 0x28) = 0x10;
        hudByte(this, 0x384) = 1;
        hudInt(this, 0x49c) = 0;
        hudInt(this, 0x2c) = 0x10;
        addEventListener(eFJEventType_InitHud, this, &_InitHud, false);
        addEventListener(eFJEventType_Init, this, &_init, false);
    }
    else
    {
        hudInt(this, 0x24) = 8;
        hudInt(this, 0x44c) = -1;
        hudInt(this, 0x450) = -1;
        hudInt(this, 0x28) = 8;
        hudInt(this, 0x2c) = 0;
        hudByte(this, 0x384) = 1;
        hudInt(this, 0x49c) = 0;
        addEventListener(eFJEventType_InitHud, this, &_InitHud, false);
        addEventListener(eFJEventType_Init, this, &_init, false);
    }

    hudInt(this, 0x458) = static_cast<int>(getChildByName("Jukebox")->getHeight());
    hudInt(this, 0x45c) = static_cast<int>(getChildByName("SelectedLabel")->getHeight());
    hudInt(this, 0x488) = static_cast<int>(getChildByName("Label1")->getY());
    hudInt(this, 0x48c) = static_cast<int>(getChildByName("SelectedLabel")->getWidth());
    hudInt(this, 0x490) = static_cast<int>(getChildByName("Jukebox")->getWidth());
    hudInt(this, 0x494) = static_cast<int>(getChildByName("AnimatedSplashText")->getWidth());
    hudInt(this, 0x498) = static_cast<int>(getChildByName("WrongWayLabel")->getWidth());

    hudByte(this, 0x4a0) = 0;
    hudFloat(this, 0x4a4) = 1.0f;
    hudByte(this, 0x4d9) = 1;
    hudByte(this, 0x4da) = 0;
    hudByte(this, 0x4ec) = 0;
    setVisible(false);
    hudByte(this, 0x4b0) = 1;
    hudByte(this, 0x4d8) = 0;

    hudNode(this, 0x24c) = NULL;
    float safeZoneHeight = getStage()->getStageHeight();
    if (safeZoneHeight == 720.0f || safeZoneHeight == 360.0f)
        hudInt(this, 0x4ac) = 0x23;
    else if (safeZoneHeight == 1080.0f || safeZoneHeight == 540.0f)
        hudInt(this, 0x4ac) = 0x32;
    else if (safeZoneHeight == 480.0f || safeZoneHeight == 408.0f)
        hudInt(this, 0x4ac) = 0x20;
    else
        hudInt(this, 0x4ac) = 0x1e;

    FJ_FuiNode* gamertagNode = getChildByName("Gamertag");
    if (gamertagNode != NULL)
    {
        FJ_Label* gamertag = gamertagNode->asFJ_Label();
        hudNode(this, 0x24c) = gamertag;
        gamertag->SetLabel(std::wstring());
    }

    hudNode(this, 0x2a0) = getChildByName("WrongWayLabel")->asFJ_Label();
    hudNode(this, 0x250) = getChildByName("GameModeSplash")->asFJ_Label();
    hudNode(this, 0x258) = getChildByName("AnimatedSplash")->asMovieClip();
    hudInt(this, 0x25c) = static_cast<int>(hudNode(this, 0x258)->getHeight());
    hudNode(this, 0x258)->setVisible(false);
    hudNode(this, 0x254) = getChildByName("AnimatedSplashText")->asFJ_Label();
    hudByte(this, 0x4b1) = 0;
    hudNode(this, 0x2a4) = getChildByName("RoundNumber")->asFJ_Label();
    hudNode(this, 0x2a8) = getChildByName("TimeLabel")->asFJ_Label();
    hudNode(this, 0x2ac) = getChildByName("TimeIcon")->asMovieClip();
    hudNode(this, 0x2b0) = getChildByName("SpeedLabel")->asFJ_Label();
    hudNode(this, 0x2b4) = getChildByName("SpeedIcon")->asMovieClip();
    hudNode(this, 0x2b8) = getChildByName("ScoreLabel")->asFJ_Label();
    hudNode(this, 0x2bc) = getChildByName("ScoreIcon")->asMovieClip();
    hudNode(this, 0x2c0) = getChildByName("RaceProgress")->asMovieClip();
    hudNode(this, 0x2c4) = hudNode(this, 0x2c0)->getChildByName("RaceBar")->asMovieClip();
    hudNode(this, 0x2c8) = hudNode(this, 0x2c4)->getChildByName("GlideHealthBar")->asMovieClip();
    hudNode(this, 0x2d8) = getChildByName("Pickup")->asMovieClip();

    hudInt(this, 0x31c) = static_cast<int>(hudNode(this, 0x2bc)->getX());
    hudInt(this, 0x320) = static_cast<int>(hudNode(this, 0x2a8)->getWidth());
    hudInt(this, 0x324) = static_cast<int>(hudNode(this, 0x2b0)->getWidth());
    hudInt(this, 0x4dc) = static_cast<int>(hudNode(this, 0x2c4)->getChildByName("Bar")->getHeight());
    hudInt(this, 0x4e0) = static_cast<int>(hudNode(this, 0x2c4)->getChildByName("Bar")->getWidth());
    hudInt(this, 0x4e4) = 0;
    hudInt(this, 0x4e8) = -1;

    hudNode(this, 0x328) = getChildByName("Gold")->asMovieClip();
    hudNode(this, 0x32c) = getChildByName("Silver")->asMovieClip();
    hudNode(this, 0x330) = getChildByName("Bronze")->asMovieClip();
    hudNode(this, 0x334) = getChildByName("PositionOne")->asFJ_Label();
    hudNode(this, 0x338) = getChildByName("PositionTwo")->asFJ_Label();
    hudNode(this, 0x33c) = getChildByName("PositionThree")->asFJ_Label();

    static_cast<FJ_Label*>(hudNode(this, 0x250))->SetLabel(std::wstring());

    FJ_FuiNode* wrongWay = hudNode(this, 0x2a0);
    if (wrongWay->isFJ_Label_HUD_SplashBig())
        wrongWay->asFJ_Label_HUD_SplashBig()->SetLabel(std::wstring());
    else if (wrongWay->isFJ_Label_HUD_Splash())
        wrongWay->asFJ_Label_HUD_Splash()->SetLabel(std::wstring());

    static_cast<FJ_Label*>(hudNode(this, 0x254))->SetLabel(std::wstring());
    getChildByName("SelectedLabel.Label")->asFJ_Label()->SetLabel(std::wstring());
    getChildByName("Jukebox")->asFJ_Label()->SetLabel(std::wstring());

    static const char* const kLabelNames[10] =
    {
        "Label1", "Label2", "Label3", "Label4", "Label5",
        "Label6", "Label7", "Label8", "Label9", "Label10"
    };
    for (int i = 0; i < 10; ++i)
        getChildByName(kLabelNames[i])->asFJ_Label()->SetLabel(std::wstring());

    init(NULL);
}

FJ_Hud::~FJ_Hud()
{
    m_checkpointNodes.clear();
}

FJ_Hud* FJ_Hud::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Hud(pRenderNode);
}

unsigned int FJ_Hud::getType() { return 89; }

void FJ_Hud::SetPlayerLevel(int level)
{
    FJ_LabelOutline* xpLabel = static_cast<FJ_LabelOutline*>(hudNode(this, 0x240));

    bool visible;
    if (level == 0)
    {
        xpLabel->SetLabel(std::wstring(L"X"));
        visible = false;
        hudInt(this, 0x39c) = 0;
    }
    else
    {
        wchar_t buffer[16];
        swprintf(buffer, 16, L"%d", level);
        xpLabel->SetLabel(std::wstring(buffer));
        visible = hudNode(this, 0x1b4)->isVisible();
        hudInt(this, 0x39c) = level;
    }
    xpLabel->setVisible(visible);

    fuiRect rect = getRect(xpLabel);
    int labelSpan = static_cast<int>(rect.xMin + rect.xMin + xpLabel->getWidth());
    float x = static_cast<float>(Math::round(
        static_cast<float>(hudInt(this, 0x4f0)) * 0.5f + hudNode(this, 0x58)->getX() -
        static_cast<float>(labelSpan) * 0.5f));
    xpLabel->setX(x);

    float y = static_cast<float>(Math::round(
        static_cast<float>(hudInt(this, 0x360)) * hudNode(this, 0x58)->getScaleY() * 0.5f +
        hudNode(this, 0x104)->getY() * hudNode(this, 0x58)->getScaleY() +
        hudNode(this, 0x58)->getY() - xpLabel->getHeight() * 0.5f - 1.0f));
    xpLabel->setY(y);
}
void FJ_Hud::SetActiveSlot(int slot)
{
    char slotName[64];
    sprintf(slotName, "iggy_slot_%d", slot);
    FJ_FuiNode* clip = hudNode(this, 0x244)->getChildByName(slotName)->asMovieClip();
    if (clip != NULL)
    {
        hudNode(this, 0x248)->setX(clip->getX() - 12.0f);
        hudInt(this, 0x370) = slot;
    }
}
void FJ_Hud::SetHealth(int health, bool highlight, bool hardcore, bool hide)
{
    static const int kNormalFrame[4] = { 1, 4, 3, 2 };
    static const int kHideFrame[4] = { 1, 8, 7, 2 };
    static const int kHardcoreFrame[4] = { 1, 0xc, 0xb, 2 };

    FJ_FuiNode** healthIcons = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x108);

    for (int i = 0; i < 10; ++i)
    {
        int threshold = 2 * (i + 1);
        int state;
        if (health < threshold - 1)
            state = 0;
        else if (health < threshold)
            state = 1;
        else
            state = 2;

        int index = highlight ? 3 + state : state;
        if (index > 3)
            index = 3;

        int frame;
        if (!hardcore)
            frame = kNormalFrame[index];
        else if (!hide)
            frame = kHideFrame[index];
        else
            frame = kHardcoreFrame[index];

        healthIcons[i]->gotoAndStop(frame);
    }

    if (!hardcore)
    {
        hudByte(this, 0x3a0) = 0;
        hudInt(this, 0x350) = health;
        hudByte(this, 0x3a1) = hide;
    }
    else
    {
        hudByte(this, 0x3a0) = hardcore;
        hudByte(this, 0x3a1) = hide;
        hudInt(this, 0x350) = health;
    }
}
void FJ_Hud::SetFood(int food, bool highlight)
{
    static const int kFoodFrame[4] = { 1, 1, 3, 5 };
    FJ_FuiNode** foodIcons = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x134);

    for (int i = 0; i < 10; ++i)
    {
        int threshold = 2 * (i + 1);
        int state;
        if (food < threshold - 1)
            state = 0;
        else if (food < threshold)
            state = 1;
        else
            state = 2;

        int index = state * 2 + (highlight ? 1 : 0);
        if (index > 3)
            index = 3;

        foodIcons[i]->gotoAndStop(kFoodFrame[index]);
    }

    hudInt(this, 0x354) = food;
    hudByte(this, 0x3a0) = highlight;
}
void FJ_Hud::SetAir(int air)
{
    HudGlobalNames& names = hudGlobalNames();
    FJ_FuiNode** bubble = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x160);
    for (int i = 0; i < 10; ++i)
    {
        if (i < air)
            bubble[i]->gotoAndStop(names.airBubbleFull);
        else if (air < 10 && i == air)
            bubble[i]->gotoAndPlay(names.airBubblePop);
        else
            bubble[i]->gotoAndStop(names.airBubbleEmpty);
    }
    hudInt(this, 0x35c) = air;
}
void FJ_Hud::SetArmour(int armour)
{
    FJ_FuiNode** icons = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x18c);
    for (int i = 0; i < 10; ++i)
    {
        int threshold = 2 * (i + 1);
        int frame;
        if (armour < threshold - 1)
            frame = 1;
        else if (armour < threshold)
            frame = 2;
        else
            frame = 3;
        icons[i]->gotoAndStop(frame);
    }
    hudInt(this, 0x358) = armour;
}
void FJ_Hud::SetRegenerationEffect(bool enabled) { ShowHealth(enabled); }
void FJ_Hud::SetJukeBoxLabelPos()
{
    FJ_FuiNode* jukebox = getChildByName("Jukebox");

    if (hudInt(this, 0x49c) == 0)
    {
        FJ_FuiNode* selectedLabel = getChildByName("SelectedLabel");
        jukebox->setY(static_cast<float>(Math::round(
            selectedLabel->getY() - selectedLabel->getHeight() * 0.5f -
            static_cast<float>(hudInt(this, 0x458)) * 0.5f)));
    }
    else if (hudInt(this, 0x460) == -1 || hudInt(this, 0x464) == -1)
    {
        jukebox->setY(static_cast<float>(hudInt(this, 0x30)));
    }
    else
    {
        jukebox->setY(static_cast<float>(hudInt(this, 0x30)) +
            (getStage()->getStageHeight() - static_cast<float>(hudInt(this, 0x464))));
        if (hudNode(this, 0x24c) != NULL)
            getChildByName("Jukebox")->incY(hudNode(this, 0x24c)->getHeight());
    }
}
void FJ_Hud::SetSelectedLabel(const std::wstring& label, bool visible)
{
    if (visible)
    {
        getChildByName("SelectedLabel.Label")->asFJ_Label()->SetLabel(label);
        getChildByName("SelectedLabel")->gotoAndStop(std::string("On"));

        if (hudInt(this, 0x460) != -1)
        {
            FJ_FuiNode* selectedLabel = getChildByName("SelectedLabel");
            float x = static_cast<float>(Math::round(
                static_cast<float>(hudInt(this, 0x460)) * 0.5f -
                static_cast<float>(hudInt(this, 0x48c)) * 0.5f));
            selectedLabel->setX(x);
        }
    }

    FJ_FuiNode* selectedLabel = getChildByName("SelectedLabel");
    float y = static_cast<float>(Math::round(
        hudNode(this, 0x58)->getY() - selectedLabel->getHeight() * 0.5f -
        static_cast<float>(hudInt(this, 0x45c))));
    selectedLabel->setY(y);

    if (!hudNode(this, 0x88)->isVisible() && !hudNode(this, 0x104)->isVisible() &&
        !hudNode(this, 0x130)->isVisible())
    {
        float delta = static_cast<float>(Math::round(
            static_cast<float>(hudInt(this, 0x360)) * hudNode(this, 0x58)->getScaleY() * 2.0f));
        getChildByName("SelectedLabel")->incY(delta);
    }
    else if (hudNode(this, 0x188)->isVisible() && hudNode(this, 0x5c)->isVisible())
    {
        getChildByName("SelectedLabel")->decY(
            hudNode(this, 0x5c)->getHeight() * hudNode(this, 0x58)->getScaleY());
    }

    SetJukeBoxLabelPos();
}
void FJ_Hud::SetHealthAbsorb(bool visible, int health)
{
    hudNode(this, 0x5c)->setVisible(visible);
    hudByte(this, 0x43) = visible;

    FJ_FuiNode** absorbIcons = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x60);
    for (int i = 0; i < 10; ++i)
    {
        FJ_FuiNode* icon = absorbIcons[i];
        icon->setVisible(2 * i < health);

        int frame;
        if (health >= 2 * i + 2)
            frame = 1;
        else if (health >= 2 * i + 1)
            frame = 2;
        else
            frame = 0;
        icon->gotoAndStop(frame);
    }

    if (!visible || health < 1)
        hudNode(this, 0x188)->setY(0.0f);
    else
        hudNode(this, 0x188)->setY(-hudNode(this, 0x104)->getY());

    hudInt(this, 0x364) = health;
    hudByte(this, 0x368) = visible;

    SetSelectedLabel(std::wstring(), false);
}
void FJ_Hud::SetFoodSaturationLevel(int level) { m_iTabIndex = level; }
void FJ_Hud::ShowExpBar(bool visible) { FJ_FuiNode* node = getChildByName("expBar"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::SetRidingHorse(bool riding, bool showJump, int health)
{
    if (riding)
    {
        FJ_FuiNode** horseIcons = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x8c);
        for (int n = 0; n < 30; ++n)
        {
            FJ_FuiNode* icon = horseIcons[n];
            icon->setVisible(health > 2 * n);
            if (icon->isVisible())
            {
                int frame;
                if (health < 2 * n + 1)
                    frame = 1;
                else if (health < 2 * n + 2)
                    frame = 4;
                else
                    frame = 3;
                icon->gotoAndStop(frame);
            }
        }
    }

    FJ_FuiNode* foodBar = hudNode(this, 0x130);
    bool showFoodBar = !riding && hudNode(this, 0x104)->isVisible();
    foodBar->setVisible(showFoodBar);
    hudByte(this, 0x44) = foodBar->isVisible();

    bool showExpBar = (!riding || !showJump) &&
        hudNode(this, 0x104)->isVisible() && !hudNode(this, 0x1bc)->isVisible();
    ShowExpBar(showExpBar);

    hudNode(this, 0x1b8)->setVisible(riding && showJump);

    hudByte(this, 0x386) = showJump;
    hudInt(this, 0x388) = health;
    hudByte(this, 0x385) = riding;
}
void FJ_Hud::SetHorseHealth(int health, bool highlight)
{
    static const int kFrame[4] = { 1, 4, 3, 2 };

    if (hudByte(this, 0x385) != 0)
    {
        FJ_FuiNode** horseIcons = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x8c);
        int highlightOffset = highlight ? 3 : 0;
        bool grow = hudInt(this, 0x388) < health;

        for (int n = 0; n < 30; ++n)
        {
            FJ_FuiNode* icon = horseIcons[n];
            bool visible;
            if (grow)
            {
                visible = (2 * n < health);
                icon->setVisible(visible);
            }
            else
            {
                visible = icon->isVisible();
            }

            if (visible)
            {
                int threshold = 2 * n + 2;
                int state;
                if (health < threshold - 1)
                    state = 0;
                else if (health < threshold)
                    state = 1;
                else
                    state = 2;

                int index = highlightOffset + state;
                if (index > 3)
                    index = 3;

                icon->gotoAndStop(kFrame[index]);
            }
        }

        if (hudInt(this, 0x388) < health)
            hudInt(this, 0x388) = health;
        hudInt(this, 0x38c) = health;
    }
}
void FJ_Hud::SetExpBarProgress(float progress) { FJ_FuiNode* node = getChildByName("expBar"); if (node != NULL) node->setAlpha(progress); }
void FJ_Hud::SetHorseJumpBarProgress(float progress) { FJ_FuiNode* node = getChildByName("horseJumpBar"); if (node != NULL) node->setAlpha(progress); }
void FJ_Hud::ShowQuickSelect(bool visible)
{
    hudNode(this, 0x244)->setVisible(visible);
    getChildByName("SelectedLabel")->setVisible(visible);
    hudNode(this, 0x54)->setVisible(hudByte(this, 0x4da) != 0 && visible);

    if (hudNode(this, 0x1bc)->isVisible())
    {
        if (visible)
        {
            hudNode(this, 0x1bc)->setY(hudNode(this, 0x1b4)->getY());
        }
        else
        {
            float hotBarY = hudNode(this, 0x244)->getY();
            float hotBarHeight = hudNode(this, 0x244)->getHeight();
            float rootScaleY1 = hudNode(this, 0x58)->getScaleY();
            float aliveHeight = hudNode(this, 0x1bc)->getHeight();
            float rootScaleY2 = hudNode(this, 0x58)->getScaleY();
            float value = (hotBarY + hotBarHeight / rootScaleY1) - aliveHeight / rootScaleY2;
            hudNode(this, 0x1bc)->setY(-static_cast<float>(Math::round(value)));
        }
    }

    hudByte(this, 0x4a) = visible;
}
void FJ_Hud::ShowCrosshair(bool visible) { FJ_FuiNode* node = getChildByName("crosshair"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::UpdateBossBarPositions()
{
    int count = hudInt(this, 0x3a4);
    if (count <= 0)
        return;

    FJ_FuiNode** bossBars = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x3a8);
    for (int i = 0; i < count; ++i)
    {
        FJ_FuiNode* bar = bossBars[i];
        if (bar == NULL)
            continue;

        if (hudInt(this, 0x460) == -1 || hudInt(this, 0x464) == -1)
        {
            float x = static_cast<float>(Math::round(
                getStage()->getStageWidth() * 0.5f - bar->getWidth() * 0.5f));
            bar->setX(x);
            float y = static_cast<float>(Math::round(
                bar->getHeight() * static_cast<float>(i) + static_cast<float>(hudInt(this, 0x30))));
            bar->setY(y);
        }
        else
        {
            float x = static_cast<float>(Math::round(
                static_cast<float>(hudInt(this, 0x460) / 2) - bar->getWidth() * 0.5f));
            bar->setX(x);
            float y = static_cast<float>(Math::round(
                bar->getHeight() * static_cast<float>(i) +
                (static_cast<float>(hudInt(this, 0x30)) +
                 (getStage()->getStageHeight() - static_cast<float>(hudInt(this, 0x464))))));
            bar->setY(y);
        }
    }
}
void FJ_Hud::SetGamertag(const std::wstring& gamertag)
{
    FJ_FuiNode* node = getChildByName("Gamertag");
    if (node != NULL && node->asFJ_Base() != NULL)
        node->asFJ_Base()->SetTextOnly(gamertag);
    UpdateBossBarPositions();
}
void FJ_Hud::PositionGameModeComponents()
{
    if (m_gameMode != 2)
        return;

    SetJukeBoxLabelPos();

    int stageWidth = hudInt(this, 0x460);
    float layoutY = loadFloatBits(hudInt(this, 0x34));
    if (m_offhandEnabled)
        layoutY += loadFloatBits(hudInt(this, 0x4ac));

    if (stageWidth == -1)
    {
        FJ_FuiNodeStage* stage = getStage();
        if (stage != NULL)
            stageWidth = static_cast<int>(stage->getStageWidth());
    }

    FJ_FuiNodeStage* stage = getStage();
    if (stage == NULL)
        return;

    const float refScale = 5.0f;
    float stackY = static_cast<float>(Math::round(
        stage->getStageHeight() / refScale + loadFloatBits(hudInt(this, 0x30))));

    if (hudInt(this, 0x460) != -1 && hudInt(this, 0x464) != -1)
    {
        float customY = loadFloatBits(hudInt(this, 0x464));
        stackY = static_cast<float>(Math::round(
            stage->getStageHeight() - customY + customY / refScale + loadFloatBits(hudInt(this, 0x30))));
    }

    FJ_FuiNode* anchor = hudNode(this, 0x2c0);
    if (anchor != NULL)
    {
        FJ_FuiNode* scaleNode = hudNode(this, 0x2c4);
        float scaleX = scaleNode != NULL ? scaleNode->getScaleX() : 1.0f;
        float yInset = static_cast<float>(Math::round(loadFloatBits(hudInt(this, 0x4dc)) * scaleX * 1.5f));
        anchor->setY(stage->getStageHeight() - layoutY - yInset);
    }

    float rightX = static_cast<float>(stageWidth - hudInt(this, 0x3c));
    FJ_FuiNode* node = hudNode(this, 0x2d8);
    if (node != NULL)
        node->setX(rightX - node->getWidth());
    if (node != NULL)
        node->setY(stage->getStageHeight() - layoutY - node->getHeight());

    node = hudNode(this, 0x2ac);
    if (node != NULL)
        node->setX(rightX - node->getWidth());
    node = hudNode(this, 0x2b4);
    if (node != NULL)
        node->setX(rightX - node->getWidth());
    node = hudNode(this, 0x2bc);
    if (node != NULL)
        node->setX(rightX - node->getWidth());

    if (hudNode(this, 0x2ac) != NULL)
        hudNode(this, 0x2ac)->setY(stackY);

    const float rowGap = 1.25f;
    node = hudNode(this, 0x2b4);
    if (node != NULL && hudNode(this, 0x2ac) != NULL)
    {
        FJ_FuiNode* ref = hudNode(this, 0x2ac);
        positionGameModeChildY(node,
            static_cast<float>(Math::round(ref->getHeight() * rowGap + ref->getY())));
    }
    node = hudNode(this, 0x2bc);
    if (node != NULL && hudNode(this, 0x2b4) != NULL)
    {
        FJ_FuiNode* ref = hudNode(this, 0x2b4);
        positionGameModeChildY(node,
            static_cast<float>(Math::round(ref->getHeight() * rowGap + ref->getY())));
    }

    const float sideFactor = 0.25f;
    node = hudNode(this, 0x2a8);
    if (node != NULL && hudNode(this, 0x2ac) != NULL)
    {
        FJ_FuiNode* ref = hudNode(this, 0x2ac);
        float x = alignedBeside(ref->getX(), ref->getWidth(), loadFloatBits(hudInt(this, 0x320)), sideFactor);
        node->setX(x);
    }
    node = hudNode(this, 0x2b0);
    if (node != NULL && hudNode(this, 0x2ac) != NULL)
    {
        FJ_FuiNode* ref = hudNode(this, 0x2ac);
        float x = alignedBeside(ref->getX(), ref->getWidth(), loadFloatBits(hudInt(this, 0x324)), sideFactor);
        node->setX(x);
    }
    node = hudNode(this, 0x2b8);
    if (node != NULL && hudNode(this, 0x2ac) != NULL)
    {
        FJ_FuiNode* ref = hudNode(this, 0x2ac);
        float x = alignedBeside(ref->getX(), ref->getWidth(), loadFloatBits(hudInt(this, 0x324)), sideFactor);
        node->setX(x);
    }

    const float centerFactor = 0.5f;
    node = hudNode(this, 0x2a8);
    if (node != NULL && hudNode(this, 0x2ac) != NULL)
    {
        FJ_FuiNode* ref = hudNode(this, 0x2ac);
        float y = alignedBelow(ref->getY(), ref->getHeight(), node->getHeight(), centerFactor);
        node->setY(y);
    }
    node = hudNode(this, 0x2b0);
    if (node != NULL && hudNode(this, 0x2b4) != NULL)
    {
        FJ_FuiNode* ref = hudNode(this, 0x2b4);
        float y = alignedBelow(ref->getY(), ref->getHeight(), node->getHeight(), centerFactor);
        node->setY(y);
    }
    node = hudNode(this, 0x2b8);
    if (node != NULL && hudNode(this, 0x2bc) != NULL)
    {
        FJ_FuiNode* ref = hudNode(this, 0x2bc);
        float y = alignedBelow(ref->getY(), ref->getHeight(), node->getHeight(), centerFactor);
        node->setY(y);
    }

    float labelX = loadFloatBits(hudInt(this, 0x38));
    if (hudNode(this, 0x328) != NULL)
        hudNode(this, 0x328)->setX(labelX);
    if (hudNode(this, 0x32c) != NULL)
        hudNode(this, 0x32c)->setX(labelX);
    if (hudNode(this, 0x330) != NULL)
        hudNode(this, 0x330)->setX(labelX);
    if (hudNode(this, 0x328) != NULL)
        hudNode(this, 0x328)->setY(stackY);

    node = hudNode(this, 0x32c);
    if (node != NULL && hudNode(this, 0x328) != NULL)
    {
        FJ_FuiNode* ref = hudNode(this, 0x328);
        positionGameModeChildY(node,
            static_cast<float>(Math::round(ref->getHeight() * rowGap + ref->getY())));
    }
    node = hudNode(this, 0x330);
    if (node != NULL && hudNode(this, 0x32c) != NULL)
    {
        FJ_FuiNode* ref = hudNode(this, 0x32c);
        positionGameModeChildY(node,
            static_cast<float>(Math::round(ref->getHeight() * rowGap + ref->getY())));
    }

    node = hudNode(this, 0x334);
    if (node != NULL && hudNode(this, 0x328) != NULL)
        node->setX(labelX + hudNode(this, 0x328)->getWidth());
    node = hudNode(this, 0x338);
    if (node != NULL && hudNode(this, 0x32c) != NULL)
        node->setX(labelX + hudNode(this, 0x32c)->getWidth());
    node = hudNode(this, 0x33c);
    if (node != NULL && hudNode(this, 0x330) != NULL)
        node->setX(labelX + hudNode(this, 0x330)->getWidth());

    node = hudNode(this, 0x334);
    if (node != NULL && hudNode(this, 0x328) != NULL)
    {
        FJ_FuiNode* ref = hudNode(this, 0x328);
        float y = alignedBelow(ref->getY(), ref->getHeight(), node->getHeight(), centerFactor);
        node->setY(y);
    }
    node = hudNode(this, 0x338);
    if (node != NULL && hudNode(this, 0x32c) != NULL)
    {
        FJ_FuiNode* ref = hudNode(this, 0x32c);
        float y = alignedBelow(ref->getY(), ref->getHeight(), node->getHeight(), centerFactor);
        node->setY(y);
    }
    node = hudNode(this, 0x33c);
    if (node != NULL && hudNode(this, 0x330) != NULL)
    {
        FJ_FuiNode* ref = hudNode(this, 0x330);
        float y = alignedBelow(ref->getY(), ref->getHeight(), node->getHeight(), centerFactor);
        node->setY(y);
    }

    float jukeboxX = static_cast<float>(Math::round(
        (stageWidth - hudInt(this, 0x498)) * sideFactor));
    FJ_FuiNode* jukebox = getChildByName("Jukebox");
    float jukeboxY = jukebox != NULL ? jukebox->getY() : 0.0f;
    float jukeboxYOffset = loadFloatBits(hudInt(this, 0x458));

    node = hudNode(this, 0x2a0);
    if (node != NULL)
    {
        node->setX(jukeboxX);
        node->setY(jukeboxY + jukeboxYOffset);
    }

    node = hudNode(this, 0x2a4);
    if (node != NULL && anchor != NULL)
    {
        FJ_FuiNode* scaleNode = hudNode(this, 0x2c4);
        float scaleX = scaleNode != NULL ? scaleNode->getScaleX() : 1.0f;
        float inset = static_cast<float>(Math::round(loadFloatBits(hudInt(this, 0x4dc)) * scaleX * sideFactor));
        node->setX(anchor->getX() + anchor->getWidth() + inset);
        float scaleY = scaleNode != NULL ? scaleNode->getScaleY() : 1.0f;
        float yAdjust = static_cast<float>(Math::round(
            loadFloatBits(hudInt(this, 0x4dc)) * scaleY * sideFactor - node->getHeight() * sideFactor - 1.0f));
        node->setY(anchor->getY() + yAdjust);
    }
}
void FJ_Hud::PositionIggySplash()
{
    if (m_iggySplash == NULL)
        m_iggySplash = getChildByName("IggySplash");
    if (m_iggySplash == NULL)
        return;
    FJ_FuiNodeStage* stage = getStage();
    if (stage == NULL)
        return;
    float x = m_splashX;
    float y = m_splashY;
    if (x == -1.0f || y == -1.0f)
    {
        x = (stage->getStageWidth() - m_iggySplash->getWidth() * 0.5f) * 0.5f;
        y = stage->getStageHeight() * 0.25f;
    }
    m_iggySplash->setX(static_cast<float>(Math::round(x)));
    m_iggySplash->setY(static_cast<float>(Math::round(y)));
}
void FJ_Hud::RepositionHud(int width, int height)
{
    hudNode(this, 0x58)->setX(static_cast<float>(Math::round(
        static_cast<float>(width) * 0.5f - static_cast<float>(hudInt(this, 0x4f0)) * 0.5f)));

    hudNode(this, 0x2c0)->setX(static_cast<float>(Math::round(
        static_cast<float>(width) * 0.5f - hudNode(this, 0x2c0)->getWidth() * 0.5f)));

    hudInt(this, 0x460) = width;
    hudInt(this, 0x464) = height;

    getChildByName("Jukebox")->setX(static_cast<float>(Math::round(
        static_cast<float>(width - hudInt(this, 0x490)) * 0.5f)));

    getChildByName("SelectedLabel")->setX(static_cast<float>(Math::round(
        static_cast<float>(width) * 0.5f - static_cast<float>(hudInt(this, 0x48c)) * 0.5f)));

    FJ_FuiNode* crosshairContainer = hudNode(this, 0x50);
    FJ_FuiNode* crosshair = crosshairContainer->getChildAt(0)->getChildByName("Crosshair")->asMovieClip();
    crosshairContainer->setX(static_cast<float>(Math::round(
        static_cast<float>(width / 2) - crosshairContainer->getWidth() * 0.5f)));
    crosshairContainer->setY(static_cast<float>(Math::round(
        getStage()->getStageHeight() - crosshair->getHeight() * 0.5f - static_cast<float>(height) * 0.5f)));

    FJ_FuiNode* armourIcons = hudNode(this, 0x4c);
    if (hudInt(this, 0x38) < 1)
        armourIcons->setX(armourIcons->getWidth());
    else
        armourIcons->setX(static_cast<float>(hudInt(this, 0x38)));
    armourIcons->setY(static_cast<float>(Math::round(
        getStage()->getStageHeight() - armourIcons->getHeight() * 0.5f - static_cast<float>(height) * 0.5f)));

    PositionGameModeComponents();
    PositionIggySplash();
    UpdateBossBarPositions();
}
void FJ_Hud::SetAlpha(float alpha)
{
    hudNode(this, 0x58)->setAlpha(alpha);
    getChildByName(std::string("SelectedLabel"))->setAlpha(alpha);
    hudNode(this, 0x50)->setAlpha(alpha);
    hudNode(this, 0x4c)->setAlpha(alpha);
    if (hudNode(this, 0x24c) != NULL)
        hudNode(this, 0x24c)->setAlpha(alpha);

    int tileCount = hudInt(this, 0x3a4);
    if (tileCount > 0)
    {
        FJ_FuiNode** tiles = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x3a8);
        for (int i = 0; i < tileCount; ++i)
        {
            if (tiles[i] != NULL)
                tiles[i]->setAlpha(alpha);
        }
    }

    int indicatorCount = hudInt(this, 0x3bc);
    if (indicatorCount > 0)
    {
        FJ_FuiNode** indicators = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x3c0);
        for (int i = 0; i < indicatorCount; ++i)
        {
            if (indicators[i] != NULL)
                indicators[i]->setAlpha(alpha);
        }
    }

    hudNode(this, 0x240)->setAlpha(alpha);
    hudNode(this, 0x4c)->setAlpha(alpha);
    hudNode(this, 0x2c0)->setAlpha(alpha);
    hudNode(this, 0x328)->setAlpha(alpha);
    hudNode(this, 0x32c)->setAlpha(alpha);
    hudNode(this, 0x330)->setAlpha(alpha);
    hudNode(this, 0x334)->setAlpha(alpha);
    hudNode(this, 0x338)->setAlpha(alpha);
    hudNode(this, 0x33c)->setAlpha(alpha);
    hudNode(this, 0x2a4)->setAlpha(alpha);
    hudNode(this, 0x2a8)->setAlpha(alpha);
    hudNode(this, 0x2ac)->setAlpha(alpha);
    hudNode(this, 0x2b0)->setAlpha(alpha);
    hudNode(this, 0x2b4)->setAlpha(alpha);
    hudNode(this, 0x2b8)->setAlpha(alpha);
    hudNode(this, 0x2bc)->setAlpha(alpha);
    hudNode(this, 0x2d8)->setAlpha(alpha);
    hudNode(this, 0x2a0)->setAlpha(alpha);
    hudNode(this, 0x54)->setAlpha(alpha);
    hudFloat(this, 0x4a4) = alpha;
    hudByte(this, 0x4a0) = 1;
}
void FJ_Hud::InitDamageIndicators()
{
    FJ_FuiNode* root = getChildAt(0);
    FJ_FuiNode* indicators = root == NULL ? NULL : root->getChildByName("DamageIndicators");
    if (indicators == NULL)
        return;
    for (int i = 0; i < 4; ++i)
    {
        std::string name = std::string("Damage_") + static_cast<char>('0' + i);
        m_damageIndicators[i] = indicators->getChildByName(name);
        m_damageIndicatorAlpha[i] = 0.0f;
        if (m_damageIndicators[i] != NULL)
            m_damageIndicators[i]->setAlpha(0.0f);
    }
}
int FJ_Hud::GetHotBarX()
{
    if (hudNode(this, 0x58) == NULL || hudNode(this, 0x244) == NULL)
        return 0;
    return static_cast<int>(hudNode(this, 0x244)->getX() * hudNode(this, 0x58)->getScaleX() + hudNode(this, 0x58)->getX());
}
int FJ_Hud::GetHotBarWidth()
{
    if (hudNode(this, 0x58) == NULL || hudNode(this, 0x244) == NULL)
        return 0;
    return static_cast<int>(hudNode(this, 0x244)->getWidth() * hudNode(this, 0x58)->getScaleX());
}
int FJ_Hud::GetHotBarY()
{
    if (hudNode(this, 0x58) == NULL || hudNode(this, 0x244) == NULL)
        return 0;
    return static_cast<int>(hudNode(this, 0x244)->getY() * hudNode(this, 0x58)->getScaleY() + hudNode(this, 0x58)->getY());
}
void FJ_Hud::SetupOffhandSlot()
{
    if (m_offhandSlot == NULL)
        m_offhandSlot = getChildByName("offhandSlot");
    if (m_hotbarAnchor == NULL)
        m_hotbarAnchor = getChildByName("hotbar");
    if (m_offhandSlot == NULL)
        return;
    bool visible = m_offhandEnabled && (m_hotbarAnchor == NULL || m_hotbarAnchor->isVisible());
    m_offhandSlot->setVisible(visible);
    float width = static_cast<float>(Math::round(m_offhandSlot->getWidth() * 0.25f));
    float x = m_rightHanded ? GetHotBarX() - m_offhandSlot->getWidth() - width
                            : GetHotBarX() + GetHotBarWidth() + width;
    m_offhandSlot->setX(x);
    m_offhandSlot->setY(static_cast<float>(GetHotBarY()));
}
void FJ_Hud::ShowPulser(bool visible) { FJ_FuiNode* node = getChildByName("pulser"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowAnimatedLogoText(FJ_Event*)
{
    int factor = 6;
    hudByte(this, 0x4d8) = 1;
    if (hudInt(this, 0x460) != -1 && hudInt(this, 0x464) != -1)
    {
        if (hudInt(this, 0x460) == 0x280 || hudInt(this, 0x464) == 0x168)
            factor = 7;
        else
            factor = 8;
    }

    FJ_FuiNode* splashClip = hudNode(this, 0x258);
    FJ_FuiNode* splashLabel = hudNode(this, 0x254);

    float splashClipY = splashClip->getY();
    float textPanelY = splashClip->getChildByName(std::string("TextPanel"))->getY();
    float scaleY1 = splashClip->getScaleY();
    float textPanelHeight = splashClip->getChildByName(std::string("TextPanel"))->getHeight();
    float scaleY2 = splashClip->getScaleY();
    float labelHeight = splashLabel->getHeight();
    float scaleY3 = splashClip->getScaleY();

    splashLabel->setY(static_cast<float>(factor) * scaleY3
        - (labelHeight * 0.5f - -(textPanelHeight * scaleY2 * 0.5f - (textPanelY * scaleY1 + splashClipY))));

    removeEventListener(eFJEventType_ShowAnimatedLogoText, this, &_ShowAnimatedLogoText);
}
void FJ_Hud::_ShowAnimatedLogoText(FJ_FuiNode*, FJ_Event*) { }
void FJ_Hud::ShowAnimatedSplash(const std::wstring& label, bool visible)
{
    FJ_FuiNode* splashClip = hudNode(this, 0x258);
    FJ_FuiNode* splashLabel = hudNode(this, 0x254);
    if (splashClip != NULL)
        splashClip->setVisible(visible);
    if (splashLabel != NULL)
        splashLabel->setVisible(visible);
    hudByte(this, 0x4b1) = visible ? 1 : 0;
    hudSplashText(this) = label;
    if (!visible)
    {
        if (splashClip != NULL)
            splashClip->gotoAndStop(1);
        setHudWideLabel(splashLabel, label);
        return;
    }

    hudInt(this, 0x4b4) = 5;
    ShowPulser(false);
    addEventListener(eFJEventType_ShowAnimatedLogoText, this, &_ShowAnimatedLogoText, false);
    if (splashClip != NULL)
        splashClip->gotoAndStop(1);

    FJ_FuiNode* textChild = splashTextPanelChild(splashClip);
    if (textChild != NULL)
    {
        textChild->setScaleX(1.0f);
        textChild->setScaleY(1.0f);
    }

    const float halfScale = 0.5f;
    FJ_FuiNode* jukebox = getChildByName("Jukebox");
    float jukeboxY = jukebox != NULL ? jukebox->getY() : 0.0f;
    float jukeboxHeight = jukebox != NULL ? jukebox->getHeight() : 0.0f;
    const float splashYOffset =
        loadFloatBits(hudInt(this, 0x25c)) * halfScale + jukeboxY + jukeboxHeight;

    if (hudInt(this, 0x460) != -1 && hudInt(this, 0x464) != -1)
    {
        float splashX = static_cast<float>(Math::round(static_cast<float>(hudInt(this, 0x460)) * halfScale));
        if (splashClip != NULL)
            splashClip->setX(splashX);
        if (splashClip != NULL)
            splashClip->setY(static_cast<float>(Math::round(splashYOffset)));
        if (splashLabel != NULL)
        {
            float labelX = static_cast<float>(Math::round(
                static_cast<float>(hudInt(this, 0x460) - hudInt(this, 0x494)) * halfScale));
            splashLabel->setX(labelX);
        }
        FJ_FuiNode* panelChild = splashTextPanelChild(splashClip);
        if (panelChild != NULL && textChild != NULL)
        {
            float panelHeight = textChild->getHeight();
            float textHeight = static_cast<float>(Math::round(panelHeight * 1.2f));
            panelChild->setHeight(textHeight);
        }
        setHudWideLabel(splashLabel, label);
        if (splashLabel != NULL)
            splashLabel->setAlpha(0.0f);
    }
    else
    {
        FJ_FuiNodeStage* stage = getStage();
        if (stage != NULL && splashClip != NULL)
        {
            float splashX = static_cast<float>(Math::round(stage->getStageWidth() * halfScale));
            splashClip->setX(splashX);
            splashClip->setY(static_cast<float>(Math::round(splashYOffset)));
        }
        setHudWideLabel(splashLabel, label);
        if (splashLabel != NULL)
            splashLabel->setAlpha(0.0f);
    }

    FJ_FuiNode* widthChild = splashTextPanelChild(splashClip);
    if (widthChild != NULL && splashClip != NULL && splashLabel != NULL)
    {
        float panelWidth = widthChild->getWidth();
        float labelWidth = splashLabel->getWidth();
        float splashScaleX = splashClip->getScaleX();
        if (splashScaleX != 0.0f)
        {
            float scaledLabelWidth = static_cast<float>(Math::round(labelWidth / splashScaleX));
            widthChild->setWidth(panelWidth * 0.25f + scaledLabelWidth);
        }
    }
    if (splashClip != NULL)
        splashClip->gotoAndPlay(2);
}
void FJ_Hud::UpdateOffsets(FJ_Event*)
{
    int highlightIndex = -1;
    if (hudByte(this, 0x378) != 0)
        highlightIndex = hudInt(this, 0x380) % 25;
    hudInt(this, 0x374) = highlightIndex;

    FJ_FuiNode** healthIcons = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x108);
    FJ_FuiNode** absorbIcons = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x60);
    FJ_FuiNode** foodIcons = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x134);

    for (int i = 0; i < 10; ++i)
    {
        float healthY = 0.0f;
        if (hudInt(this, 0x350) < 5)
        {
            int shake = FJ_Helpers::RetInstance()->nextInt(2);
            healthY = static_cast<float>(shake * (hudInt(this, 0x37c) + 1));
        }

        if (highlightIndex == i)
        {
            healthY = static_cast<float>(hudInt(this, 0x37c) * -2 - 2);
            if (healthY != healthIcons[i]->getY())
                healthIcons[i]->setY(healthY);
        }
        else
        {
            if (hudByte(this, 0x368) != 0)
            {
                float absorbY = 0.0f;
                if (highlightIndex == i + 10)
                    absorbY = static_cast<float>(hudInt(this, 0x37c) * -2 - 2);
                if (absorbIcons[i]->isVisible() && absorbY != absorbIcons[i]->getY())
                    absorbIcons[i]->setY(absorbY);
            }
            if (healthY != healthIcons[i]->getY())
                healthIcons[i]->setY(healthY);
        }

        float foodY = 0.0f;
        if (hudInt(this, 0x36c) < 1)
        {
            int period = hudInt(this, 0x354) * 3 + 1;
            if (hudInt(this, 0x380) % period == 0)
            {
                int roll = FJ_Helpers::RetInstance()->nextInt(3);
                foodY = static_cast<float>(roll - 1) * static_cast<float>(hudInt(this, 0x37c) + 1);
            }
        }
        if (foodY != foodIcons[i]->getY())
            foodIcons[i]->setY(foodY);
    }

    hudInt(this, 0x380) = (hudInt(this, 0x380) < 0x7fffffff) ? hudInt(this, 0x380) + 1 : 0;

    for (int i = 0; i < 4; ++i)
    {
        if (m_damageIndicatorAlpha[i] > 0.0f)
        {
            if (m_damageIndicators[i] != NULL)
                m_damageIndicators[i]->setAlpha(m_damageIndicatorAlpha[i] / 30.0f);
            m_damageIndicatorAlpha[i] -= 1.0f;
        }
    }

    if (hudByte(this, 0x4d8) != 0)
    {
        hudNode(this, 0x254)->incAlpha(0.1f);
        if (1.0f <= hudNode(this, 0x254)->getAlpha())
        {
            hudNode(this, 0x254)->setAlpha(1.0f);
            hudByte(this, 0x4d8) = 0;
        }
    }

    if (hudByte(this, 0x4b1) == 0)
    {
        if (hudNode(this, 0x258)->currentFrame() != 1)
            ShowAnimatedSplash(std::wstring(), hudByte(this, 0x4b1) != 0);
        if (hudByte(this, 0x4b1) == 0)
            return;
    }

    if (hudNode(this, 0x258)->currentFrame() == 1)
    {
        if (hudInt(this, 0x4b4) < 1)
        {
            ShowAnimatedSplash(hudSplashText(this), hudByte(this, 0x4b1) != 0);
            return;
        }
        hudInt(this, 0x4b4) = hudInt(this, 0x4b4) - 1;
    }
}
void FJ_Hud::_UpdateOffsets(FJ_FuiNode*, FJ_Event*) { }
void FJ_Hud::InitHud(FJ_Event* event)
{
    (void)event;
    HudGlobalNames& names = hudGlobalNames();
    int hudSize = hudInt(this, 0x44c);
    hudInt(this, 0x450) = hudSize;
    setVisible(true);

    if (hudByte(this, 0x384) != 0)
    {
        hudByte(this, 0x378) = 0;
        hudByte(this, 0x385) = 0;
        hudByte(this, 0x386) = 1;
        hudByte(this, 0x3a0) = 0;
        hudByte(this, 0x3a1) = 0;
        hudByte(this, 0x368) = 0;
        hudInt(this, 0x380) = 0;
        hudInt(this, 0x36c) = 0;
        hudInt(this, 0x374) = -1;
        hudInt(this, 0x370) = 0;
        hudInt(this, 0x37c) = 1;
        hudInt(this, 0x388) = 0;
        hudInt(this, 0x39c) = 0;
        hudInt(this, 0x3a4) = 0;
        hudInt(this, 0x3bc) = 0;
        hudInt(this, 0x3a8) = 0;
        hudInt(this, 0x3ac) = 0;
        hudInt(this, 0x3b0) = 0;
        hudInt(this, 0x3b4) = 0;
        hudInt(this, 0x394) = 0;
        hudInt(this, 0x3b8) = 0;
        hudInt(this, 0x398) = 0;
        int* cursor = reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x39c);
        for (int block = 0; block < 3; ++block)
        {
            cursor[9] = 0;
            cursor[15] = 0;
            cursor[14] = 0;
            cursor[17] = 0;
            cursor[16] = 0;
            cursor[11] = 0;
            cursor[10] = 0;
            cursor[13] = 0;
            cursor[12] = 0;
            cursor += 9;
        }
    }

    FJ_FuiNode* armourIcons = getChildByName("ArmourBarIcons");
    hudNode(this, 0x4c) = armourIcons != NULL ? armourIcons->asMovieClip() : NULL;

    FJ_FuiNode* crosshair = getChildByName(hudVariantName(names.crosshair, hudSize));
    hudNode(this, 0x50) = crosshair != NULL ? crosshair->asMovieClip() : NULL;

    FJ_FuiNode* hudRoot = getChildByName(hudVariantName(names.hudRoot, hudSize));
    hudRoot = hudRoot != NULL ? hudRoot->asMovieClip() : NULL;
    hudNode(this, 0x58) = hudRoot;
    if (hudRoot != NULL)
        hudRoot->setX(static_cast<float>(Math::round(hudRoot->getX())));

    FJ_FuiNode* bars = hudRoot;
    FJ_FuiNode* child = bars != NULL ? bars->getChildByName("FJ_HealthBar") : NULL;
    hudNode(this, 0x104) = child != NULL ? child->asMovieClip() : NULL;
    child = bars != NULL ? bars->getChildByName("FJ_HealthAbsorbBar") : NULL;
    hudNode(this, 0x5c) = child != NULL ? child->asMovieClip() : NULL;
    child = bars != NULL ? bars->getChildByName("FJ_HorseHealthBar") : NULL;
    hudNode(this, 0x88) = child != NULL ? child->asMovieClip() : NULL;
    child = bars != NULL ? bars->getChildByName("FJ_FoodBar") : NULL;
    hudNode(this, 0x130) = child != NULL ? child->asMovieClip() : NULL;
    child = bars != NULL ? bars->getChildByName("FJ_AirBar") : NULL;
    hudNode(this, 0x15c) = child != NULL ? child->asMovieClip() : NULL;
    child = bars != NULL ? bars->getChildByName("FJ_ArmourBar") : NULL;
    hudNode(this, 0x188) = child != NULL ? child->asMovieClip() : NULL;

    child = bars != NULL ? bars->getChildByName("ExpBar") : NULL;
    child = child != NULL ? child->asMovieClip() : NULL;
    hudNode(this, 0x1b4) = child != NULL ? child->asFJ_ExperienceBar() : NULL;

    child = bars != NULL ? bars->getChildByName("HorseJumpBar") : NULL;
    child = child != NULL ? child->asMovieClip() : NULL;
    hudNode(this, 0x1b8) = child != NULL ? child->asFJ_HorseJumpBar() : NULL;

    child = bars != NULL ? bars->getChildByName("AliveDeadBar") : NULL;
    hudNode(this, 0x1bc) = child != NULL ? child->asMovieClip() : NULL;

    child = getChildByName(hudVariantName(names.hudXpLevel, hudSize));
    child = child != NULL ? child->asMovieClip() : NULL;
    hudNode(this, 0x240) = child != NULL ? child->asFJ_LabelOutline() : NULL;

    child = bars != NULL ? bars->getChildByName("HotBar") : NULL;
    hudNode(this, 0x244) = child != NULL ? child->asMovieClip() : NULL;
    child = hudNode(this, 0x244) != NULL ? hudNode(this, 0x244)->getChildByName("HotbarSelector") : NULL;
    hudNode(this, 0x248) = child != NULL ? child->asMovieClip() : NULL;

    child = getChildByName(hudVariantName(names.offhandSlot, hudSize));
    hudNode(this, 0x54) = child != NULL ? child->asMovieClip() : NULL;

    if (hudNode(this, 0x104) != NULL)
        hudInt(this, 0x360) = static_cast<int>(hudNode(this, 0x104)->getHeight());
    if (bars != NULL)
        hudInt(this, 0x4f0) = static_cast<int>(bars->getWidth());

    char buffer[64];
    FJ_FuiNode** iconCursor = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x60);
    for (int slot = 10; slot > 0; --slot)
    {
        std::snprintf(buffer, sizeof(buffer), "FJ_Health_0%d", slot);
        FJ_FuiNode* healthBar = hudNode(this, 0x104);
        FJ_FuiNode* icon = healthBar != NULL ? healthBar->getChildByName(buffer) : NULL;
        iconCursor[42] = icon;
        if (icon != NULL)
            icon->gotoAndStop(3);

        std::snprintf(buffer, sizeof(buffer), "FJ_FoodBar_0%d", slot);
        FJ_FuiNode* foodBar = hudNode(this, 0x130);
        icon = foodBar != NULL ? foodBar->getChildByName(buffer) : NULL;
        iconCursor[53] = icon;
        if (icon != NULL)
            icon->gotoAndStop(2);

        std::snprintf(buffer, sizeof(buffer), "FJ_AirBubble_0%d", slot);
        FJ_FuiNode* airBar = hudNode(this, 0x15c);
        icon = airBar != NULL ? airBar->getChildByName(buffer) : NULL;
        iconCursor[64] = icon;
        if (icon != NULL)
            icon->gotoAndStop(names.airBubbleFull);

        std::snprintf(buffer, sizeof(buffer), "FJ_Armour_0%d", slot);
        FJ_FuiNode* armourBar = hudNode(this, 0x188);
        icon = armourBar != NULL ? armourBar->getChildByName(buffer) : NULL;
        iconCursor[75] = icon;
        if (icon != NULL)
            icon->gotoAndStop(1);

        std::snprintf(buffer, sizeof(buffer), "FJ_Health_0%d", slot);
        FJ_FuiNode* absorbBar = hudNode(this, 0x5c);
        icon = absorbBar != NULL ? absorbBar->getChildByName(buffer) : NULL;
        iconCursor[0] = icon;
        if (icon != NULL)
            icon->gotoAndStop(1);

        iconCursor += 1;
    }

    FJ_FuiNode* aliveBar = hudNode(this, 0x1bc);
    if (aliveBar != NULL)
    {
        float width = aliveBar->getWidth() / aliveBar->getScaleX();
        hudInt(this, 0x22c) = static_cast<int>(width);
        FJ_FuiNode* aliveTemplate = aliveBar->getChildByName("Alive00");
        hudInt(this, 0x238) = aliveTemplate != NULL ? static_cast<int>(aliveTemplate->getWidth()) : 0;
        int aliveCount = hudInt(this, 0x24);
        int spacing = hudInt(this, 0x238) * aliveCount;
        hudInt(this, 0x230) = static_cast<int>(Math::round(
            static_cast<float>(hudInt(this, 0x22c) - spacing) / (static_cast<float>(aliveCount) - 1.0f)));

        FJ_FuiNode** aliveSlot = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x1c0);
        for (int i = 0; i < aliveCount; ++i)
        {
            std::snprintf(buffer, sizeof(buffer), "Alive0%d", i);
            FJ_FuiNode* icon = aliveBar->getChildByName(buffer);
            aliveSlot[i] = icon;
            int frame = hudByte(this, 0x20) != 0 ? 0x12 : 10;
            if (icon != NULL)
            {
                icon->gotoAndStop(frame);
                icon->setVisible(false);
            }
        }

        if (hudByte(this, 0x20) != 0)
        {
            FJ_FuiNode* smallAlive = aliveBar->getChildByName("SmallAlive00");
            hudInt(this, 0x23c) = smallAlive != NULL ? static_cast<int>(smallAlive->getWidth()) : 0;
            int smallCount = hudInt(this, 0x2c);
            int smallSpacing = hudInt(this, 0x23c) * smallCount;
            hudInt(this, 0x234) = static_cast<int>(Math::round(
                static_cast<float>(hudInt(this, 0x22c) - smallSpacing) / (static_cast<float>(smallCount) - 1.0f)));
            FJ_FuiNode** smallSlot = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x1e8);
            for (int i = 0; i < smallCount; ++i)
            {
                std::snprintf(buffer, sizeof(buffer), "SmallAlive%02d", i);
                FJ_FuiNode* icon = aliveBar->getChildByName(buffer);
                smallSlot[i] = icon;
                if (icon != NULL)
                {
                    icon->gotoAndStop(0x12);
                    icon->setVisible(false);
                }
            }
        }
    }

    int playerCount = hudInt(this, 0x28);
    if (playerCount > 0)
    {
        FJ_FuiNode** playerStore = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x2dc);
        FJ_FuiNode* playerRoot = hudNode(this, 0x2c4);
        for (int i = 0; i < playerCount; ++i)
        {
            std::snprintf(buffer, sizeof(buffer), "Player_%d", i);
            FJ_FuiNode* player = playerRoot != NULL ? playerRoot->getChildByName(buffer) : NULL;
            playerStore[i] = player;
            int frame = hudByte(this, 0x20) != 0 ? 0x12 : 10;
            if (player != NULL)
            {
                player->gotoAndStop(frame);
                player->setVisible(false);
            }
            if (i > 0 && player != NULL)
            {
                FJ_FuiNode* outline = player->getChildByName("CircleOutline");
                if (outline != NULL)
                    outline->setVisible(false);
            }
        }
    }

    FJ_FuiNode** horseSlot = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x8c);
    FJ_FuiNode* horseBar = hudNode(this, 0x88);
    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 6; ++j)
        {
            std::snprintf(buffer, sizeof(buffer), "FJ_Health_%02d", i * 6 + j);
            FJ_FuiNode* icon = horseBar != NULL ? horseBar->getChildByName(buffer) : NULL;
            horseSlot[j] = icon;
            if (icon != NULL)
                icon->gotoAndStop(3);
        }
        horseSlot += 1;
    }

    int anchorY = 0;
    int stageWidth = hudInt(this, 0x460);
    if (hudByte(this, 0x384) == 0)
    {
        if (hudNode(this, 0x58) != NULL)
            hudNode(this, 0x58)->setVisible(hudByte(this, 0x40) != 0);
        if (hudNode(this, 0x104) != NULL)
            hudNode(this, 0x104)->setVisible(hudByte(this, 0x41) != 0);
        if (hudNode(this, 0x88) != NULL)
            hudNode(this, 0x88)->setVisible(hudByte(this, 0x42) != 0);
        if (hudNode(this, 0x5c) != NULL)
            hudNode(this, 0x5c)->setVisible(hudByte(this, 0x43) != 0);
        if (hudNode(this, 0x130) != NULL)
            hudNode(this, 0x130)->setVisible(hudByte(this, 0x44) != 0);
        if (hudNode(this, 0x15c) != NULL)
            hudNode(this, 0x15c)->setVisible(hudByte(this, 0x45) != 0);
        if (hudNode(this, 0x188) != NULL)
            hudNode(this, 0x188)->setVisible(hudByte(this, 0x46) != 0);
        if (hudNode(this, 0x1b4) != NULL)
            hudNode(this, 0x1b4)->setVisible(hudByte(this, 0x47) != 0);
        if (hudNode(this, 0x1bc) != NULL)
            hudNode(this, 0x1bc)->setVisible(hudByte(this, 0x48) != 0);
        if (hudNode(this, 0x240) != NULL)
            hudNode(this, 0x240)->setVisible(hudByte(this, 0x47) != 0);
        if (hudNode(this, 0x4c) != NULL)
            hudNode(this, 0x4c)->setVisible(hudByte(this, 0x49) != 0);
        SetPlayerLevel(hudInt(this, 0x39c));
        SetActiveSlot(hudInt(this, 0x370));
        SetHealth(hudInt(this, 0x350), hudByte(this, 0x3a0) != 0, hudByte(this, 0x3a0) != 0, hudByte(this, 0x3a1) != 0);
        SetFood(hudInt(this, 0x354), hudByte(this, 0x3a0) != 0);
        SetAir(hudInt(this, 0x35c));
        SetArmour(hudInt(this, 0x358));
        SetRegenerationEffect(hudByte(this, 0x378) != 0);
        SetHealthAbsorb(hudByte(this, 0x368) != 0, hudInt(this, 0x364));
        SetFoodSaturationLevel(hudInt(this, 0x36c));
        SetRidingHorse(hudByte(this, 0x385) != 0, hudByte(this, 0x386) != 0, hudInt(this, 0x388));
        SetHorseHealth(hudInt(this, 0x38c), false);
        SetExpBarProgress(hudFloat(this, 0x394));
        SetHorseJumpBarProgress(hudFloat(this, 0x398));
        ShowQuickSelect(hudByte(this, 0x4a) != 0);
        ShowCrosshair(hudByte(this, 0x4b) != 0);
        if (hudInt(this, 0x460) != -1 && hudInt(this, 0x464) != -1)
            RepositionHud(hudInt(this, 0x460), hudInt(this, 0x464));
        if (hudNode(this, 0x58) != NULL)
            anchorY = static_cast<int>(hudNode(this, 0x58)->getY());
        hudInt(this, 0x454) = anchorY;
    }
    else
    {
        SetPlayerLevel(0);
        SetActiveSlot(0);
        if (hudNode(this, 0x1bc) != NULL)
            hudNode(this, 0x1bc)->setVisible(false);
        if (hudNode(this, 0x4c) != NULL)
        {
            hudByte(this, 0x48) = 0;
            hudNode(this, 0x4c)->setVisible(false);
        }
        if (hudNode(this, 0x15c) != NULL)
        {
            hudByte(this, 0x49) = 0;
            hudNode(this, 0x15c)->setVisible(false);
        }
        hudByte(this, 0x45) = 0;
        FJ_FuiNode* selected = getChildByName("SelectedLabel.Label");
        FJ_Label* label = selected != NULL ? selected->asFJ_Label() : NULL;
        if (label != NULL)
            label->setUseHtmlText(true);
        selected = getChildByName("SelectedLabel.Label");
        label = selected != NULL ? selected->asFJ_Label() : NULL;
        if (label != NULL)
            label->SetLabel(std::wstring());
        hudByte(this, 0x384) = 0;
        if (hudNode(this, 0x58) != NULL)
            anchorY = static_cast<int>(hudNode(this, 0x58)->getY());
        hudInt(this, 0x454) = anchorY;
        stageWidth = hudInt(this, 0x460);
    }

    if (stageWidth != -1 && hudInt(this, 0x464) != -1)
    {
        FJ_FuiNode* jukebox = getChildByName("Jukebox");
        if (jukebox != NULL)
        {
            FJ_FuiNodeStage* stage = getStage();
            if (stage != NULL)
            {
                float y = loadFloatBits(hudInt(this, 0x30)) +
                    stage->getStageHeight() - loadFloatBits(hudInt(this, 0x464));
                jukebox->setY(y);
            }
        }
        anchorY = hudInt(this, 0x454);
    }

    float rootY = loadFloatBits(anchorY - hudInt(this, 0x4a8));
    if (hudNode(this, 0x58) != NULL)
        hudNode(this, 0x58)->setY(rootY);

    SetPlayerLevel(hudInt(this, 0x39c));
    SetSelectedLabel(std::wstring(), false);
    if (hudByte(this, 0x4a0) != 0)
        SetAlpha(hudFloat(this, 0x4a4));
    InitDamageIndicators();
    SetupOffhandSlot();
    FJ_Label* gamertag = hudNode(this, 0x250) != NULL ? hudNode(this, 0x250)->asFJ_Label() : NULL;
    if (gamertag != NULL)
        gamertag->SetLabel(std::wstring());
    addEventListener(eFJEventType_EnterFrame, this, &_UpdateOffsets, false);
    hudByte(this, 0x4ec) = 0;
}

void FJ_Hud::_InitHud(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Hud*>(node)->InitHud(event);
}
void FJ_Hud::LoadHud(int hudSize)
{
    if (hudInt(this, 0x44c) != hudSize)
    {
        removeEventListener(eFJEventType_EnterFrame, this, &_UpdateOffsets);
        gotoAndStop(hudVariantName(hudGlobalNames().hudSizeLabel, hudSize));
        hudByte(this, 0x4ec) = 1;
        hudInt(this, 0x44c) = hudSize;
        setVisible(false);
        if (hudInt(this, 0x450) == hudInt(this, 0x44c))
        {
            setVisible(true);
            addEventListener(eFJEventType_EnterFrame, this, &_UpdateOffsets, false);
            hudByte(this, 0x4ec) = 0;
        }
    }
}
void FJ_Hud::init(FJ_Event* event)
{
    hudInt(this, 0x464) = -1;
    hudInt(this, 0x460) = -1;
    hudInt(this, 0x354) = 0x14;
    hudInt(this, 0x35c) = 10;
    hudInt(this, 0x358) = 0;
    hudInt(this, 0x350) = 0x14;
    hudField<std::wstring>(this, 0x468) = L"";
    LoadHud(1);
}
void FJ_Hud::_init(FJ_FuiNode*, FJ_Event*) { }
int FJ_Hud::GetHotBarHeight()
{
    if (hudNode(this, 0x58) == NULL || hudNode(this, 0x244) == NULL)
        return 0;
    return static_cast<int>(hudNode(this, 0x244)->getHeight() * hudNode(this, 0x58)->getScaleY());
}
unsigned char FJ_Hud::GetHudLoading()
{
    return *reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(this) + 0x4ec);
}
void FJ_Hud::SetSafeZone(int left, int top, int right, int bottom)
{
    hudInt(this, 0x30) = left;
    hudInt(this, 0x34) = top;
    hudInt(this, 0x38) = right;
    hudInt(this, 0x3c) = bottom;

    if (hudByte(this, 0x4b0) == 0)
        top -= hudInt(this, 0x4ac);
    hudInt(this, 0x4a8) = top;

    if (hudByte(this, 0x4ec) == 0)
    {
        if (hudInt(this, 0x460) != -1 && hudInt(this, 0x464) != -1)
        {
            getChildByName("Jukebox")->setY(static_cast<float>(left) +
                (getStage()->getStageHeight() - static_cast<float>(hudInt(this, 0x464))));
            top = hudInt(this, 0x4a8);
        }

        hudNode(this, 0x58)->setY(static_cast<float>(hudInt(this, 0x454) - top));
        SetupOffhandSlot();
        right = hudInt(this, 0x38);
    }

    FJ_FuiNode* armourIcons = hudNode(this, 0x4c);
    if (right < 1)
        armourIcons->setX(armourIcons->getWidth());
    else
        armourIcons->setX(static_cast<float>(right));

    float stageHeight = getStage()->getStageHeight();
    float armourHeight = armourIcons->getHeight();
    if (hudInt(this, 0x464) == -1)
        armourIcons->setY(static_cast<float>(Math::round((stageHeight - armourHeight) * 0.5f)));
    else
        armourIcons->setY(static_cast<float>(Math::round(
            stageHeight - armourHeight * 0.5f - static_cast<float>(hudInt(this, 0x464)) * 0.5f)));

    getChildByName("Jukebox")->setY(static_cast<float>(hudInt(this, 0x488) - hudInt(this, 0x4a8)));

    static const char* const kLabelNames[10] =
    {
        "Label1", "Label2", "Label3", "Label4", "Label5",
        "Label6", "Label7", "Label8", "Label9", "Label10"
    };
    static const char* const kLabelBackgroundNames[10] =
    {
        "Label1Background", "Label2Background", "Label3Background", "Label4Background", "Label5Background",
        "Label6Background", "Label7Background", "Label8Background", "Label9Background", "Label10Background"
    };

    FJ_FuiNode* label = getChildByName(kLabelNames[0]);
    getChildByName(kLabelBackgroundNames[0])->setY(label->getY());
    for (int i = 1; i < 10; ++i)
    {
        FJ_FuiNode* previousLabel = getChildByName(kLabelNames[i - 1]);
        FJ_FuiNode* previousBackground = getChildByName(kLabelBackgroundNames[i - 1]);
        FJ_FuiNode* currentLabel = getChildByName(kLabelNames[i]);
        currentLabel->setY(previousLabel->getY() - previousBackground->getHeight());
        getChildByName(kLabelBackgroundNames[i])->setY(currentLabel->getY());
    }

    PositionGameModeComponents();
    PositionIggySplash();
}
void FJ_Hud::SetAnimatedSplashTexture(const std::wstring& texture)
{
    FJ_FuiNode* node = getChildByName("AnimatedLogo.AnimatedLogoTexture");
    if (node != NULL)
    {
        FJ_BitmapIcon* icon = node->asFJ_BitmapIcon();
        if (icon != NULL)
        {
            icon->SetTextureName(texture);
        }
    }
}
void FJ_Hud::SetPulserText(const std::wstring& text)
{
    FJ_FuiNode* node = getChildByName("pulser");
    if (node != NULL && node->asFJ_Base() != NULL)
        node->asFJ_Base()->SetTextOnly(text);
    PositionIggySplash();
}
void FJ_Hud::SetTooltipsEnabled(bool enabled)
{
    FJ_FuiNode* node = getChildByName("tooltips");
    if (node != NULL)
        node->setVisible(enabled);
    SetupOffhandSlot();
    RepositionHud(static_cast<int>(getWidth()), static_cast<int>(getHeight()));
}
void FJ_Hud::ShowOffhandSlot(bool visible) { FJ_FuiNode* node = getChildByName("offhandSlot"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::SetIsRightHanded(bool rightHanded)
{
    m_rightHanded = rightHanded;
    SetupOffhandSlot();
}
void FJ_Hud::ShowDirectionalDamage(int direction, int, float alpha)
{
    float timer = alpha * 30.0f;
    if (direction == 8)
    {
        for (int i = 0; i < 8; ++i)
        {
            hudNode(this, 0x260 + static_cast<size_t>(i) * 4)->setAlpha(1.0f);
            hudFloat(this, 0x280 + static_cast<size_t>(i) * 4) = timer;
        }
    }
    else
    {
        hudNode(this, 0x260 + static_cast<size_t>(direction) * 4)->setAlpha(1.0f);
        hudFloat(this, 0x280 + static_cast<size_t>(direction) * 4) = timer;
    }
}
void FJ_Hud::SetHudGameMode(int gameMode)
{
    m_gameMode = gameMode;
    SetJukeBoxLabelPos();

    setHudNodeVisible(this, 0x58, gameMode != 2);
    if (gameMode != 2)
    {
        hudByte(this, 0x40) = 1;
        setHudNodeVisible(this, 0x2a0, false);
        setHudNodeVisible(this, 0x2a4, false);
        setHudNodeVisible(this, 0x2a8, false);
        setHudNodeVisible(this, 0x2ac, false);
        setHudNodeVisible(this, 0x2b0, false);
        setHudNodeVisible(this, 0x2b4, false);
        setHudNodeVisible(this, 0x2b8, false);
        setHudNodeVisible(this, 0x2bc, false);
        setHudNodeVisible(this, 0x2c0, false);
        setHudNodeVisible(this, 0x2d8, false);
        setHudNodeVisible(this, 0x334, false);
        setHudNodeVisible(this, 0x338, false);
        setHudNodeVisible(this, 0x33c, false);
        setHudNodeVisible(this, 0x328, false);
        setHudNodeVisible(this, 0x32c, false);
        setHudNodeVisible(this, 0x330, false);
        return;
    }

    hudByte(this, 0x40) = 0;
    setHudNodeVisible(this, 0x2c0, true);
    setHudNodeVisible(this, 0x2a0, true);
    setHudNodeVisible(this, 0x2a4, true);
    setHudNodeVisible(this, 0x2a8, true);
    setHudNodeVisible(this, 0x2ac, true);
    setHudNodeVisible(this, 0x2b0, true);
    setHudNodeVisible(this, 0x2b4, true);
    setHudNodeVisible(this, 0x2b8, false);
    setHudNodeVisible(this, 0x2bc, false);
    setHudNodeVisible(this, 0x2d8, false);
    setHudNodeVisible(this, 0x334, false);
    setHudNodeVisible(this, 0x338, false);
    setHudNodeVisible(this, 0x33c, false);
    setHudNodeVisible(this, 0x328, false);
    setHudNodeVisible(this, 0x32c, false);
    setHudNodeVisible(this, 0x330, false);

    const std::wstring emptyLabel;
    FJ_FuiNode* splashNode = hudNode(this, 0x2a0);
    if (splashNode != NULL && splashNode->isFJ_Label_HUD_SplashBig())
        splashNode->asFJ_Label_HUD_SplashBig()->SetLabel(emptyLabel);
    else if (splashNode != NULL && splashNode->isFJ_Label_HUD_Splash())
        splashNode->asFJ_Label_HUD_Splash()->SetLabel(emptyLabel);

    setHudWideLabel(hudNode(this, 0x2a4), emptyLabel);
    setHudWideLabel(hudNode(this, 0x2a8), emptyLabel);
    setHudWideLabel(hudNode(this, 0x2b0), emptyLabel);
    setHudWideLabel(hudNode(this, 0x2b8), emptyLabel);
    setHudWideLabel(hudNode(this, 0x334), emptyLabel);
    setHudWideLabel(hudNode(this, 0x338), emptyLabel);
    setHudWideLabel(hudNode(this, 0x33c), emptyLabel);
    PositionGameModeComponents();

    FJ_FuiNode* healthRoot = hudNode(this, 0x2c8);
    if (healthRoot == NULL)
        return;

    char buffer[64];
    for (int slot = 0; slot < 3; ++slot)
    {
        std::snprintf(buffer, sizeof(buffer), "FJ_Health_0%d", slot);
        FJ_FuiNode* icon = healthRoot->getChildByName(buffer);
        hudNode(this, 0x2cc + static_cast<size_t>(slot) * 4) = icon;
        if (icon != NULL)
            icon->gotoAndStop(3);
    }
}
void FJ_Hud::SetPosition(int position, const std::wstring& label, bool visible)
{
    FJ_FuiNode* labelNode;
    FJ_FuiNode* iconNode;
    if (position == 0)
    {
        labelNode = hudNode(this, 0x334);
        iconNode = hudNode(this, 0x328);
    }
    else if (position == 1)
    {
        labelNode = hudNode(this, 0x338);
        iconNode = hudNode(this, 0x32c);
    }
    else if (position == 2)
    {
        labelNode = hudNode(this, 0x33c);
        iconNode = hudNode(this, 0x330);
    }
    else
    {
        return;
    }

    labelNode->asFJ_Label()->SetLabel(label);
    labelNode->setVisible(visible);
    iconNode->setVisible(visible);
}
void FJ_Hud::SetSpeedometer(int value)
{
    if (value < 1)
    {
        hudNode(this, 0x2b4)->getChildAt(0)->gotoAndStop(1);
    }
    else
    {
        if (value > 0xc)
            value = 0xc;
        hudNode(this, 0x2b4)->getChildAt(0)->gotoAndStop(value);
    }
}
void FJ_Hud::SetTimerIcon(int value, int time)
{
    FJ_FuiNode* clip = hudNode(this, 0x2ac)->getChildAt(0)->asMovieClip();
    FJ_FuiNode* minutesIcon = clip->getChildByName("minutes")->asMovieClip();
    FJ_FuiNode* secondsIcon = clip->getChildByName("seconds")->asMovieClip();
    float minutesFrame = (176.0f - static_cast<float>(static_cast<unsigned int>(value))) / 60.0f * 12.0f;
    float secondsFrame = (176.0f - static_cast<float>(static_cast<unsigned int>(time))) / 60.0f * 12.0f;
    minutesIcon->gotoAndStop(static_cast<int>(minutesFrame) + 1);
    secondsIcon->gotoAndStop(static_cast<int>(secondsFrame) + 1);
}
void FJ_Hud::SetScoreIcon(int value)
{
    FJ_FuiNode* root = hudNode(this, 0x2bc)->getChildAt(0);
    FJ_FuiNode* background = root->getChildByName(std::string("background"))->asMovieClip();
    FJ_FuiNode* animation = root->getChildByName(std::string("animation"))->asMovieClip();
    background->gotoAndStop(value + 1);

    int previous = hudInt(this, 0x4e8);
    hudInt(this, 0x4e4) = value;
    if (value != previous)
    {
        if (previous == 0)
            animation->gotoAndPlay(std::string("PlayGrey"));
        else if (previous == 1)
            animation->gotoAndPlay(std::string("PlayGreen"));
        else if (previous < 3)
            animation->gotoAndPlay(std::string("PlayGold"));
        else if (previous != 3)
        {
            hudInt(this, 0x4e8) = value;
            return;
        }
        else
            animation->gotoAndPlay(std::string("PlayDiamond"));

        hudInt(this, 0x4e8) = hudInt(this, 0x4e4);
    }
}
void FJ_Hud::ShowScore(bool visible)
{
    hudNode(this, 0x2b8)->setVisible(visible);
    hudNode(this, 0x2bc)->setVisible(visible);
}
void FJ_Hud::ShowArmorBar(bool visible) { FJ_FuiNode* node = getChildByName("armorBar"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowArmorIcons(int armour, bool visible)
{
    if (hudInt(this, 0x49c) != 1)
        return;

    const char* childName;
    if (armour == 0)
        childName = "Armor_Head";
    else if (armour == 1)
        childName = "Armor_Chest";
    else if (armour < 3)
        childName = "Armor_Legs";
    else if (armour == 3)
        childName = "Armor_Boots";
    else
        return;

    FJ_FuiNode* icons = hudNode(this, 0x4c)->getChildAt(0)->getChildByName("Armor_Icons");
    FJ_FuiNode* piece = icons->getChildByName(childName);
    piece->setVisible(visible);
}
void FJ_Hud::SetControlAlpha(const std::string& control, float alpha) { FJ_FuiNode* node = getChildByName(control); if (node != NULL) node->setAlpha(alpha); }
void FJ_Hud::SetWrongWayLabel(const std::wstring& label)
{
    FJ_FuiNode* node = getChildByName("WrongWay");
    if (node != NULL && node->asFJ_Base() != NULL)
        node->asFJ_Base()->SetTextOnly(label);
}
unsigned int FJ_Hud::GetStatusEffectsYOffset() { return hudInt(this, 0x42c); }
void FJ_Hud::HideSelectedLabel() { getChildByName("SelectedLabel")->gotoAndPlay(std::string("Fade")); }
void FJ_Hud::UpdateStatusEffectTile(int activeCount, int slot, int effect, bool ambient, float alpha)
{
    static const int kEffectRemap[4] = { -1, 12, 9, 2 };

    if (activeCount < 27)
    {
        for (int i = activeCount; i < 27; ++i)
        {
            if (m_statusEffectTiles[i] != NULL)
            {
                removeChild(m_statusEffectTiles[i], true);
                m_statusEffectTiles[i] = NULL;
            }
        }
    }

    hudInt(this, 0x3bc) = activeCount;
    hudInt(this, 0x42c) = 0;
    if (activeCount == 0)
        return;

    int remappedEffect = kEffectRemap[effect];
    FJ_MobEffect_Tile* tile;
    if (slot < activeCount && m_statusEffectTiles[slot] != NULL)
    {
        tile = m_statusEffectTiles[slot];
        m_statusEffectAmbient[slot] = ambient;
    }
    else
    {
        float stageHeight = getStage()->getStageHeight();
        FJ_MobEffect_Tile* created;
        if (stageHeight == 1080.0f || stageHeight == 720.0f)
        {
            if (hudInt(this, 0x460) != -1 && hudInt(this, 0x464) != -1)
                created = FJ_MobEffect_Tile_Small::Create(m_pRenderNode);
            else
                created = FJ_MobEffect_Tile_Normal::Create(m_pRenderNode);
        }
        else
        {
            created = FJ_MobEffect_Tile_Small::Create(m_pRenderNode);
        }
        created->Init();
        m_statusEffectTiles[slot] = created;
        m_statusEffectAmbient[slot] = ambient;
        addChild(created);
        tile = created;
    }

    if (tile != NULL)
    {
        tile->SetupEffectType(remappedEffect + 2);
        tile->SetEffectAlpha(alpha);
    }

    int nonAmbientCount = 0;
    int ambientCount = 0;
    int count = hudInt(this, 0x3bc);
    for (int i = 0; i < count; ++i)
    {
        FJ_MobEffect_Tile* current = m_statusEffectTiles[i];
        if (current == NULL)
            continue;

        float heightContribution = 0.0f;
        if (hudNode(this, 0x24c) != NULL && current->isVisible() && hudInt(this, 0x480) != 0)
        {
            float gamertagStageHeight = getStage()->getStageHeight();
            if (gamertagStageHeight == 1080.0f)
                heightContribution = getHeight() * 1.25f;
            else
                heightContribution = hudNode(this, 0x24c)->getHeight() * 1.5f;
        }

        float widthOffset;
        float heightForY = heightContribution;
        if (!m_statusEffectAmbient[i])
        {
            widthOffset = static_cast<float>(nonAmbientCount) * current->getWidth();
            heightForY = heightContribution + current->getHeight();
            ++nonAmbientCount;
        }
        else
        {
            widthOffset = static_cast<float>(ambientCount) * current->getWidth();
            ++ambientCount;
        }

        if (hudInt(this, 0x460) == -1 || hudInt(this, 0x464) == -1)
        {
            float stageWidth = getStage()->getStageWidth();
            float x = static_cast<float>(Math::round(
                (stageWidth - static_cast<float>(hudInt(this, 0x3c))) - current->getWidth() - widthOffset));
            current->setX(x);
            float y = static_cast<float>(Math::round(heightForY + static_cast<float>(hudInt(this, 0x30))));
            current->setY(y);
            int bottom = static_cast<int>(y) + static_cast<int>(current->getHeight());
            if (hudInt(this, 0x42c) < bottom)
                hudInt(this, 0x42c) = bottom;
        }
        else
        {
            float x = static_cast<float>(hudInt(this, 0x460) - hudInt(this, 0x3c)) -
                current->getWidth() - widthOffset;
            current->setX(x);
            float stageHeight2 = getStage()->getStageHeight();
            float y = (heightForY + static_cast<float>(hudInt(this, 0x30))) +
                (stageHeight2 - static_cast<float>(hudInt(this, 0x464)));
            current->setY(y);
        }
    }
}
void FJ_Hud::SetNumBossBars(int count)
{
    hudInt(this, 0x3a4) = count;
    if (count < 5)
    {
        FJ_FuiNode** bossBars = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x3a8);
        for (int i = count; i < 5; ++i)
        {
            if (bossBars[i] != NULL)
            {
                removeChild(bossBars[i], true);
                bossBars[i] = NULL;
            }
        }
    }
}
void FJ_Hud::SetBossBar(int index, float progress, const std::wstring& name)
{
    FJ_FuiNode** bossBarSlot = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x3a8);
    if (bossBarSlot[index] == NULL)
    {
        FJ_FuiNode* created = NULL;
        FJ_FuiNodeStage* stage = getStage();
        float stageHeight = stage->getStageHeight();
        bool handled = false;
        if (stageHeight != 1080.0f)
        {
            stage = getStage();
            stageHeight = stage->getStageHeight();
            if (stageHeight != 720.0f)
            {
                stage = getStage();
                stageHeight = stage->getStageHeight();
                if (stageHeight == 480.0f)
                {
                    created = FJ_Factory::createObject("fourj.ProgressBars.FJ_DragonHealth_02", m_pRenderNode, true, NULL);
                }
                else
                {
                    stage = getStage();
                    stageHeight = stage->getStageHeight();
                    if (stageHeight == 408.0f)
                        created = FJ_Factory::createObject("fourj.ProgressBars.FJ_DragonHealth_02", m_pRenderNode, true, NULL);
                }
                handled = true;
            }
        }

        if (!handled)
        {
            if (hudInt(this, 0x460) == -1 || hudInt(this, 0x464) == -1)
                created = FJ_Factory::createObject("fourj.ProgressBars.FJ_DragonHealth_03", m_pRenderNode, true, NULL);
            else
                created = FJ_Factory::createObject("fourj.ProgressBars.FJ_DragonHealth_01", m_pRenderNode, true, NULL);
        }

        bossBarSlot[index] = created;
        addChild(created);
    }

    bossBarSlot[index]->asFJ_ProgressBar()->setProgress(progress);
    bossBarSlot[index]->asFJ_Base()->SetLabel(name);
    UpdateBossBarPositions();
}
void FJ_Hud::ShowGameModePlayers(bool visible)
{
    hudNode(this, 0x1bc)->setVisible(visible);
    hudByte(this, 0x48) = visible;
}
void FJ_Hud::SetGameModeNumPlayers(int count, bool visible)
{
    if (hudByte(this, 0x20) == 0)
    {
        if (count > 8)
            count = 8;

        if (m_gameMode == 2)
        {
            int playerCount = hudInt(this, 0x28);
            if (playerCount > 0)
            {
                FJ_FuiNode** playerSlot = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x2dc);
                for (int i = 0; i < playerCount; ++i)
                    playerSlot[i]->setVisible(i < count);
                hudInt(this, 0x228) = count;
                return;
            }
        }
        else
        {
            int spacing = Math::round((hudInt(this, 0x22c) - static_cast<float>(hudInt(this, 0x238)) * 8.0f) / 7.0f);
            int lastIndex = count - 1;
            int aliveCount = hudInt(this, 0x24);
            int leftover = hudInt(this, 0x22c) - (hudInt(this, 0x238) * count + lastIndex * spacing);
            hudInt(this, 0x230) = spacing;
            int centerOffset = leftover / 2;

            if (aliveCount > 0)
            {
                FJ_FuiNode** aliveSlot = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x1c0);
                int spacingSum = hudInt(this, 0x238) + hudInt(this, 0x230);
                float parkedX = static_cast<float>(centerOffset);
                for (int i = 0; i < aliveCount; ++i)
                {
                    if (visible)
                    {
                        if (i <= lastIndex)
                        {
                            aliveSlot[i]->setX(static_cast<float>(spacingSum * i));
                            aliveSlot[i]->setVisible(true);
                        }
                        else
                        {
                            aliveSlot[i]->setX(parkedX);
                            aliveSlot[i]->setVisible(false);
                        }
                    }
                    else
                    {
                        aliveSlot[i]->setX(i <= lastIndex
                            ? parkedX + static_cast<float>(spacingSum * i)
                            : parkedX);
                        aliveSlot[i]->setVisible(i <= lastIndex);
                    }
                }
                hudInt(this, 0x228) = count;
                return;
            }
        }
    }
    else
    {
        if (m_gameMode == 2)
        {
            int playerCount = hudInt(this, 0x28);
            if (playerCount > 0)
            {
                FJ_FuiNode** playerSlot = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x2dc);
                for (int i = 0; i < playerCount; ++i)
                    playerSlot[i]->setVisible(i < count);
                hudInt(this, 0x228) = count;
                return;
            }
        }
        else
        {
            int lastIndex = count - 1;
            int width = hudInt(this, 0x22c);
            int aliveCount = hudInt(this, 0x24);
            int centerOffset;
            int smallSpacing = 0;

            if (count < 11)
            {
                int spacing;
                if (count < 9)
                    spacing = Math::round((width - static_cast<float>(hudInt(this, 0x238)) * 8.0f) / 7.0f);
                else
                    spacing = Math::round((width - hudInt(this, 0x238) * count) / static_cast<float>(lastIndex));

                int leftover = width - (hudInt(this, 0x238) * count + lastIndex * spacing);
                hudInt(this, 0x230) = spacing;
                centerOffset = leftover / 2;
            }
            else
            {
                int smallTotal = hudInt(this, 0x23c) * count;
                smallSpacing = (width - smallTotal) / lastIndex;
                int leftover = width - (smallTotal + lastIndex * smallSpacing);
                centerOffset = leftover / 2;
            }

            float parkedX = static_cast<float>(centerOffset);

            if (aliveCount > 0)
            {
                FJ_FuiNode** aliveSlot = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x1c0);
                for (int i = 0; i < aliveCount; ++i)
                {
                    if (count < 11)
                    {
                        if (i > lastIndex)
                        {
                            aliveSlot[i]->setVisible(false);
                        }
                        else
                        {
                            int spacingSum = hudInt(this, 0x238) + hudInt(this, 0x230);
                            if (!visible)
                                aliveSlot[i]->setX(parkedX + static_cast<float>(spacingSum * i));
                            else
                                aliveSlot[i]->setX(static_cast<float>(spacingSum) * static_cast<float>(i));
                            aliveSlot[i]->setVisible(true);
                        }
                    }
                    else
                    {
                        aliveSlot[i]->setVisible(false);
                    }
                }
            }

            int smallCount = hudInt(this, 0x2c);
            if (smallCount > 0)
            {
                FJ_FuiNode** smallSlot = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x1e8);
                for (int i = 0; i < smallCount; ++i)
                {
                    if (count < 11)
                    {
                        smallSlot[i]->setVisible(false);
                    }
                    else if (!visible)
                    {
                        smallSlot[i]->setX(parkedX + static_cast<float>((hudInt(this, 0x23c) + smallSpacing) * i));
                        smallSlot[i]->setVisible(i <= lastIndex);
                    }
                    else
                    {
                        smallSlot[i]->setX(static_cast<float>(hudInt(this, 0x23c) + hudInt(this, 0x234)) * static_cast<float>(i));
                        smallSlot[i]->setVisible(i <= lastIndex);
                    }
                }
                hudInt(this, 0x228) = count;
                return;
            }
        }
    }

    hudInt(this, 0x228) = count;
}

void FJ_Hud::SetGameModePlayerState(int player, int state, bool visible)
{
    if (hudByte(this, 0x20) == 0)
    {
        if (m_gameMode == 2)
        {
            if (player < hudInt(this, 0x28))
            {
                FJ_FuiNode** playerSlot = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x2dc);
                playerSlot[player]->gotoAndStop(state + 1);
                playerSlot[player]->setAlpha(visible ? 1.0f : 0.5f);
            }
        }
        else if (hudInt(this, 0x228) < 9 && player < hudInt(this, 0x24))
        {
            FJ_FuiNode** aliveSlot = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x1c0);
            aliveSlot[player]->gotoAndStop(state + 1);
            aliveSlot[player]->setAlpha(visible ? 1.0f : 0.5f);
        }
    }
    else if (m_gameMode == 2)
    {
        if (player < hudInt(this, 0x28))
        {
            FJ_FuiNode** playerSlot = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x2dc);
            playerSlot[player]->gotoAndStop(state + 1);
            playerSlot[player]->setAlpha(visible ? 1.0f : 0.5f);
        }
    }
    else if (hudInt(this, 0x24) < hudInt(this, 0x228))
    {
        if (player < hudInt(this, 0x2c))
        {
            FJ_FuiNode** smallSlot = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x1e8);
            smallSlot[player]->gotoAndStop(state + 1);
            smallSlot[player]->setAlpha(visible ? 1.0f : 0.5f);
        }
    }
    else if (player < hudInt(this, 0x24))
    {
        FJ_FuiNode** aliveSlot = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x1c0);
        aliveSlot[player]->gotoAndStop(state + 1);
        aliveSlot[player]->setAlpha(visible ? 1.0f : 0.5f);
    }
}
void FJ_Hud::SetPercentageMapTraversed(int percentage, float progress)
{
    FJ_FuiNode** playerSlot = reinterpret_cast<FJ_FuiNode**>(reinterpret_cast<char*>(this) + 0x2dc);
    FJ_FuiNode* node = playerSlot[percentage];

    float targetWidth = static_cast<float>(static_cast<unsigned int>(hudInt(this, 0x4e0)));
    node->setX(static_cast<float>(Math::round((progress / 100.0f) * (targetWidth - node->getWidth()))));

    float targetHeight = static_cast<float>(static_cast<unsigned int>(hudInt(this, 0x4dc)));
    node->setY((targetHeight - node->getHeight()) * 0.5f);
}
void FJ_Hud::AddCheckpointAtPercentage(float percentage)
{
    FJ_FuiNode* node = getChildByName("Checkpoint");
    if (node == NULL)
        return;
    node->setX(static_cast<float>(Math::round(getWidth() * percentage)));
    addChild(node);
    m_checkpointNodes.push_back(node);
}
void FJ_Hud::ClearCheckpointPositions()
{
    for (std::vector<FJ_FuiNode*>::size_type i = 0; i < m_checkpointNodes.size(); ++i)
        hudNode(this, 0x2c0)->removeChild(m_checkpointNodes[i], true);
    m_checkpointNodes.clear();
}
void FJ_Hud::AnimateCheckpoint(int index) { if (index >= 0 && index < (int)m_checkpointNodes.size() && m_checkpointNodes[index] != NULL) m_checkpointNodes[index]->gotoAndPlay(2); }
void FJ_Hud::SetGlideHealth(int health)
{
    static const int kGlideHealthFrame[4] = { 1, 4, 3, 2 };
    char buffer[64];

    for (int slot = 0; slot < 3; ++slot)
    {
        std::snprintf(buffer, sizeof(buffer), "FJ_Health_0%d", slot);
        FJ_FuiNode* icon = hudNode(this, 0x2c8)->getChildByName(buffer);
        hudNode(this, 0x2cc + static_cast<size_t>(slot) * 4) = icon;
        int state = (slot < health) ? 2 : 0;
        icon->gotoAndStop(kGlideHealthFrame[state]);
    }
}
void FJ_Hud::ShowHealth(bool visible) { FJ_FuiNode* node = getChildByName("health"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowHorseHealth(bool visible) { FJ_FuiNode* node = getChildByName("horseHealth"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowFood(bool visible) { FJ_FuiNode* node = getChildByName("food"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowAir(bool visible) { FJ_FuiNode* node = getChildByName("air"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowArmour(bool visible) { FJ_FuiNode* node = getChildByName("armour"); if (node != NULL) node->setVisible(visible); }
