#ifndef GAME_UI_FJ_MOBEFFECT_TILE_SMALL_HPP
#define GAME_UI_FJ_MOBEFFECT_TILE_SMALL_HPP

class FJ_MobEffect_Tile;
class fuiRenderNode;

class FJ_MobEffect_Tile_Small : public FJ_MobEffect_Tile
{
public:
    FJ_MobEffect_Tile_Small(fuiRenderNode* pRenderNode);

    virtual ~FJ_MobEffect_Tile_Small();

    static FJ_MobEffect_Tile_Small* Create(fuiRenderNode* pRenderNode);

    virtual unsigned int getType();
};

#endif
