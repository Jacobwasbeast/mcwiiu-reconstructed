#include "game/core/PostPass.hpp"
#include "game/item/Effect.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Uniform.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/RenderTarget.hpp"

template<typename T> std::wstring _toString(T value);



void PostPass::destroy()
{
    mEffect->destroy();
}

void PostPass::prepareState()
{
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableBlend();
    GlStateManager::disableDepthTest();
    GlStateManager::disableAlphaTest();
    GlStateManager::disableFog();
    GlStateManager::disableLighting();
    GlStateManager::disableColorMaterial();
    GlStateManager::enableTexture();
    GlStateManager::bindTexture(0);
}



void PostPass::addAuxAsset(std::wstring name, RenderTarget* target, int width, int height)
{
    mAuxAssetNames.push_back(name);
    mAuxAssets.push_back(target);
    mAuxWidths.push_back(width);
    mAuxHeights.push_back(height);
}

Effect* PostPass::getEffect()
{
    return mEffect;
}

void PostPass::process(float time)
{
    const int outputWidth = mOutput->renderWidth;
    const int outputHeight = mOutput->renderHeight;
    const int inputWidth = mInput->renderWidth;
    const int inputHeight = mInput->renderHeight;

    prepareState();
    GlStateManager::viewport(0, 0, outputWidth, outputHeight);
    mEffect->setSampler(L"DiffuseSampler", mInput);
    for (unsigned int i = 0; i < mAuxAssetNames.size(); ++i)
    {
        mEffect->setSampler(mAuxAssetNames.at(i), mAuxAssets.at(i));
        Uniform* auxSize = mEffect->safeGetUniform(L"AuxSize" + _toString(i));
        auxSize->set(static_cast<float>(mAuxWidths.at(i)), static_cast<float>(mAuxHeights.at(i)));
    }
    mEffect->safeGetUniform(L"ProjMat")->set(mOrthoMatrix);
    mEffect->safeGetUniform(L"InSize")->set(static_cast<float>(inputWidth), static_cast<float>(inputHeight));
    mEffect->safeGetUniform(L"OutSize")->set(static_cast<float>(outputWidth), static_cast<float>(outputHeight));
    mEffect->safeGetUniform(L"Time")->set(time);
    Minecraft* minecraft = Minecraft::GetInstance();
    mEffect->safeGetUniform(L"ScreenSize")->set(static_cast<float>(minecraft->mWidth), static_cast<float>(minecraft->mHeight));
    mEffect->apply();
    GlStateManager::depthMask(false);
    GlStateManager::colorMask(true, true, true, true);
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin();
    builder->color(255, 255, 255, 255);
    builder->vertex(0.0f, static_cast<float>(outputHeight), 500.0f);
    builder->vertex(static_cast<float>(outputWidth), static_cast<float>(outputHeight), 500.0f);
    builder->vertex(static_cast<float>(outputWidth), 0.0f, 500.0f);
    builder->vertex(0.0f, 0.0f, 500.0f);
    builder->end();
    GlStateManager::depthMask(true);
    GlStateManager::colorMask(true, true, true, true);
    mEffect->clear();
}
