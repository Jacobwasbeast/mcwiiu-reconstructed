#include "game/item/Effect.hpp"

#include "game/core/GlStateManager.hpp"
#include "game/core/ProgramManager.hpp"
#include "game/core/Uniform.hpp"

void Effect::destroy()
{
    ProgramManager* manager = (ProgramManager*)ProgramManager::getInstance();
    manager->releaseProgram(this);
}

void Effect::clear()
{
    unsigned int texture = 0;
    for (boost::unordered_map<std::wstring, RenderTarget*>::iterator it = mSamplers.begin();
         it != mSamplers.end(); ++it, ++texture)
    {
        if (it->second != 0)
        {
            GlStateManager::activeTexture(static_cast<int>(texture));
            GlStateManager::bindTexture(0);
        }
    }
}

void Effect::apply()
{
    static_cast<void>(this);
}

void Effect::markDirty()
{
    mDirty = true;
}

Uniform* Effect::getUniform(const std::wstring& name)
{
    boost::unordered_map<std::wstring, Uniform*>::iterator it = mUniforms.find(name);
    if (it != mUniforms.end())
        return it->second;
    return 0;
}

Uniform* Effect::safeGetUniform(const std::wstring& name)
{
    Uniform* uniform = getUniform(name);
    if (uniform != 0)
        return uniform;
    return Uniform::safeFallback();
}

void Effect::setSampler(std::wstring name, RenderTarget* target)
{
    boost::unordered_map<std::wstring, RenderTarget*>::iterator it = mSamplers.find(name);
    if (it != mSamplers.end())
        mSamplers.erase(it);
    mSamplers[name] = target;
    markDirty();
}
