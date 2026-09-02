#ifndef GAME_CLIENT_RENDER_MOBSPAWNERRENDERER_HPP
#define GAME_CLIENT_RENDER_MOBSPAWNERRENDERER_HPP

#include "game/client/render/BlockEntityRenderer.hpp"

#include <boost/shared_ptr.hpp>

class BaseMobSpawner;
class BlockEntity;

class MobSpawnerRenderer : public BlockEntityRenderer
{
public:
    virtual ~MobSpawnerRenderer();

    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a, int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList);

    static void render(BaseMobSpawner* pSpawner, double x, double y, double z, float a);
};

#endif
