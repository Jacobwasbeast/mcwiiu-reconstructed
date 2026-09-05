#ifndef GAME_CLIENT_RENDER_SIGNRENDERER_HPP
#define GAME_CLIENT_RENDER_SIGNRENDERER_HPP

#include <stdint.h>
#include <string>
#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockEntityRenderer.hpp"

class BlockEntity;
class SignModel;

class SignRenderer : public BlockEntityRenderer
{
private:
    struct TextCacheEntry
    {
        int ids[10];
        int ages[10];
    };
    TextCacheEntry mTextCache[20];
    int mTextCacheCount;
    SignModel* mModel;
    bool mCensor;
    unsigned char mPadding[3];
    int mLanguage;

public:
    SignRenderer();
    virtual ~SignRenderer();
    uint64_t CensorString(std::wstring& text);
    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z,
                        float a, int breakProgress, bool bTranslucent, float alpha,
                        bool bUseDisplayList);
    virtual void tick();
};

#endif
