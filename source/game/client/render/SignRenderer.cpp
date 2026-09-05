#include "game/client/render/SignRenderer.hpp"

#include <wchar.h>
#include "game/client/render/SignModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/L10N.hpp"
#include "game/world/block/SignBlockEntity.hpp"

extern unsigned int XGetLanguage();

SignRenderer::SignRenderer() : mTextCacheCount(0), mModel(NULL), mCensor(false), mLanguage(0)
{
    ModelDefinition definition;
    mModel = new SignModel(definition);
    int i, j;
    for (i = 0; i < 20; ++i)
        for (j = 0; j < 10; ++j)
        {
            mTextCache[i].ids[j] = -1;
            mTextCache[i].ages[j] = 0;
        }
    mLanguage = XGetLanguage();
}

SignRenderer::~SignRenderer()
{
    delete mModel;
}

uint64_t SignRenderer::CensorString(std::wstring& text)
{
    const wchar_t* blank = L" ";
    if (text.compare(0, text.size(), blank, wcslen(blank)) != 0)
    {
        unsigned int language = XGetLanguage();
        if (language == 2 || (language > 6 && (language < 9 || language == 0x11)))
        {
            const wchar_t* censored = L"Censored";
            text.assign(censored, wcslen(censored));
        }
        else
        {
            const wchar_t* censored = L10N::GetString(0x62950b66);
            text.assign(censored, wcslen(censored));
        }
    }
    return 0;
}

void SignRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z,
                          float a, int breakProgress, bool bTranslucent, float alpha,
                          bool bUseDisplayList)
{
    boost::shared_ptr<SignBlockEntity> sign = boost::static_pointer_cast<SignBlockEntity>(blockEntity);
    int orientation = sign->getData();
    const float half = 0.5f;
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x + half, (float)y + half, (float)z + half);
    if (orientation == 0)
        GlStateManager::rotatef(-(float)sign->getData() * 2.0454545f, 0.0f, 1.0f, 0.0f);
    else
    {
        float angle = orientation == 2 ? 180.0f : orientation == 4 ? 90.0f : orientation == 5 ? -90.0f : 0.0f;
        GlStateManager::rotatef(-angle, 0.0f, 1.0f, 0.0f);
        GlStateManager::translatef(0.0f, -0.3125f, -0.4375f);
    }
    if (breakProgress >= 0)
        bindTexture(&BREAKING_LOCATIONS[breakProgress]);
    GlStateManager::pushMatrix();
    GlStateManager::scalef(0.6666667f, -0.6666667f, -0.6666667f);
    mModel->render(true);
    GlStateManager::popMatrix();
    Font* font = getFont();
    GlStateManager::translatef(0.0f, 0.33333334f, 0.046666667f);
    GlStateManager::scalef(0.010416667f, -0.010416667f, 0.010416667f);
    GlStateManager::normal3f(0.0f, 0.0f, 1.0f);
    GlStateManager::depthMask(false);
    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Sign_Text) | 0xff000000;
    if (breakProgress < 0)
    {
        GlStateManager::disableLighting();
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        builder->begin();
        builder->color((int)colour);
        int line;
        for (line = 0; line < 4; ++line)
        {
            std::wstring text = sign->getCreatedPlayerUUID();
            if (!mCensor && sign->wasCreatedByRestrictedPlayer())
            {
                if (!sign->IsVerified())
                    text = L"Awaiting Approval";
                else if (!sign->IsCensored())
                    CensorString(text);
            }
            font->drawNoBeginEndBuilder(text, -font->width(text) / 2, line * 10, (int)colour);
        }
        tesselator->end();
        sign->setTextCacheIndex(0);
        mTextCache[0].ids[0] = sign->getUniqueId();
    }
    GlStateManager::enableLighting();
    GlStateManager::depthMask(true);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::popMatrix();
}

void SignRenderer::tick()
{
    bool censor = g_pApp->IsConsoleChatCensored();
    int language = XGetLanguage();
    int i, j;
    if (censor == mCensor && language == mLanguage)
    {
        for (i = 0; i < 20; ++i)
            for (j = 0; j < 10; ++j)
                if (mTextCache[i].ids[j] != -1 && ++mTextCache[i].ages[j] > 20)
                {
                    mTextCache[i].ids[j] = -1;
                    --mTextCacheCount;
                }
        return;
    }
    mCensor = censor;
    mLanguage = language;
    for (i = 0; i < 20; ++i)
        for (j = 0; j < 10; ++j)
        {
            mTextCache[i].ids[j] = -1;
            mTextCache[i].ages[j] = 0;
        }
    mTextCacheCount = 0;
}
