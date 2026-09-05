#include "game/client/render/MapRenderer.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

#include "game/client/render/Font.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/MapDecoration.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/core/Options.hpp"

static unsigned int gMapColours[208];
static bool gMapColoursLoaded = true;

static const eMinecraftColour gMapColourSources[52] =
{
    eMinecraftColour_Material_None,
    eMinecraftColour_Material_Grass,
    eMinecraftColour_Material_Sand,
    eMinecraftColour_Material_Cloth,
    eMinecraftColour_Material_Fire,
    eMinecraftColour_Material_Ice,
    eMinecraftColour_Material_Metal,
    eMinecraftColour_Material_Plant,
    eMinecraftColour_Material_Snow,
    eMinecraftColour_Material_Clay,
    eMinecraftColour_Material_Dirt,
    eMinecraftColour_Material_Stone,
    eMinecraftColour_Material_Water,
    eMinecraftColour_Material_Wood,
    eMinecraftColour_Material_Emerald,
    eMinecraftColour_Material_Quartz,
    eMinecraftColour_Material_Gold,
    eMinecraftColour_Material_Diamond,
    eMinecraftColour_Material_Lapis,
    eMinecraftColour_Material_Podzol,
    eMinecraftColour_Material_Nether,
    eMinecraftColour_Material_Color_Orange,
    eMinecraftColour_Material_Color_Magenta,
    eMinecraftColour_Material_Color_Light_Blue,
    eMinecraftColour_Material_Color_Yellow,
    eMinecraftColour_Material_Color_Light_Green,
    eMinecraftColour_Material_Color_Pink,
    eMinecraftColour_Material_Color_Grey,
    eMinecraftColour_Material_Color_Silver,
    eMinecraftColour_Material_Color_Cyan,
    eMinecraftColour_Material_Color_Purple,
    eMinecraftColour_Material_Color_Blue,
    eMinecraftColour_Material_Color_Brown,
    eMinecraftColour_Material_Color_Green,
    eMinecraftColour_Material_Color_Red,
    eMinecraftColour_Material_Color_Black,
    eMinecraftColour_Material_Terracotta_White,
    eMinecraftColour_Material_Terracotta_Orange,
    eMinecraftColour_Material_Terracotta_Magenta,
    eMinecraftColour_Material_Terracotta_Light_Blue,
    eMinecraftColour_Material_Terracotta_Yellow,
    eMinecraftColour_Material_Terracotta_Light_Green,
    eMinecraftColour_Material_Terracotta_Pink,
    eMinecraftColour_Material_Terracotta_Grey,
    eMinecraftColour_Material_Terracotta_Silver,
    eMinecraftColour_Material_Terracotta_Cyan,
    eMinecraftColour_Material_Terracotta_Purple,
    eMinecraftColour_Material_Terracotta_Blue,
    eMinecraftColour_Material_Terracotta_Brown,
    eMinecraftColour_Material_Terracotta_Green,
    eMinecraftColour_Material_Terracotta_Red,
    eMinecraftColour_Material_Terracotta_Black
};

bool sortByTexture(const MapDecoration& left, const MapDecoration& right)
{
    return left.getImg() < right.getImg();
}

std::wstring formatwstr(const wchar_t* format, ...);

MapRenderer::MapRenderer(Font* font, Options* options, Textures* textures)
    : mTextures(textures), mMapInstances(), mOptions(options), mFont(font)
{
}

void MapRenderer::reloadColours()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    ColourTable* colours = minecraft->getColourTable();
    int i;
    for (i = 0; i < 208; ++i)
    {
        if (i < 4)
        {
            gMapColours[i] = 16 + ((i * 8) & 8);
            continue;
        }
        unsigned int colour = colours->getColour(gMapColourSources[i / 4]);
        unsigned int red = (colour >> 16) & 255;
        unsigned int green = (colour >> 8) & 255;
        unsigned int blue = colour & 255;
        unsigned int brightness = i % 4 == 2 ? 255 : (i % 4 == 0 ? 180 : 220);
        gMapColours[i] = (((red * brightness) / 255) << 24) |
                         (((green * brightness) / 255) << 16) |
                         (((blue * brightness) / 255) << 8) | 255;
    }
    gMapColoursLoaded = false;
}

MapRenderer::MapInstance* MapRenderer::getMapInstance(
    const boost::shared_ptr<MapItemSavedData>& data)
{
    const std::wstring& id = data->id;
    boost::unordered_map<std::wstring, MapInstance*>::iterator found = mMapInstances.find(id);
    if (found == mMapInstances.end())
    {
        MapInstance* instance = new MapInstance(this, data);
        mMapInstances[id] = instance;
        found = mMapInstances.find(id);
    }
    return found->second;
}

void MapRenderer::render(const boost::shared_ptr<MapItemSavedData>& data,
                         const boost::shared_ptr<Player>& player)
{
    getMapInstance(data)->draw(player);
}

void MapRenderer::update(const boost::shared_ptr<MapItemSavedData>& data)
{
    getMapInstance(data)->updateTexture();
}

MapRenderer::MapInstance* MapRenderer::getMapInstanceIfExists(const std::wstring& id)
{
    boost::unordered_map<std::wstring, MapInstance*>::iterator found = mMapInstances.find(id);
    if (found != mMapInstances.end())
        return found->second;
    return NULL;
}

void MapRenderer::resetData()
{
    boost::unordered_map<std::wstring, MapInstance*>::iterator it;
    for (it = mMapInstances.begin(); it != mMapInstances.end(); ++it)
    {
        if (it->second != NULL)
        {
            mTextures->releaseTexture(it->second->mTexture);
            delete it->second;
        }
    }
    mMapInstances.clear();
}

boost::shared_ptr<MapItemSavedData> MapRenderer::getData(MapInstance* instance)
{
    if (instance == NULL)
        return boost::shared_ptr<MapItemSavedData>();
    return instance->mData;
}

MapRenderer::MapInstance::MapInstance(MapRenderer* renderer,
                                      const boost::shared_ptr<MapItemSavedData>& data)
    : mRenderer(renderer), mData(data), mTexture(-1), mPixels(16384, true), mTextureDirty(false)
{
    if (gMapColoursLoaded)
        MapRenderer::reloadColours();
}

void MapRenderer::MapInstance::fjUpdateWithRenderContext()
{
    if (mTexture == -1)
    {
        BufferedImage image(0, 128, 128);
        mTexture = mRenderer->mTextures->getTexture(&image, (C4JRender::eTextureFormat)0, false);
        mTextureDirty = true;
    }
    if (mTextureDirty)
    {
        mRenderer->mTextures->replaceTextureDirect(mPixels, 128, 128, mTexture);
    }
}

void MapRenderer::MapInstance::draw(const boost::shared_ptr<Player>& player)
{
    fjUpdateWithRenderContext();
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    GlStateManager::bindTexture(mTexture);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(1, 5);
    GlStateManager::disableAlphaTest();
    builder->begin();
    builder->vertexUV(0.0f, 128.0f, -0.02f, 0.0f, 1.0f);
    builder->vertexUV(128.0f, 128.0f, -0.02f, 1.0f, 1.0f);
    builder->vertexUV(128.0f, 0.0f, -0.02f, 1.0f, 0.0f);
    builder->vertexUV(0.0f, 0.0f, -0.02f, 0.0f, 0.0f);
    tesselator->end();
    GlStateManager::enableAlphaTest();
    GlStateManager::disableBlend();

    std::vector<MapDecoration> decorations = mData->fjGetAllDecorations();
    std::sort(decorations.begin(), decorations.end(), sortByTexture);
    int index = 0;
    int texture = -1;
    float depth = -0.04f;
    boost::unordered_map<int, int> counts;
    std::vector<MapDecoration>::const_iterator it;
    for (it = decorations.begin(); it != decorations.end(); ++it)
    {
        if ((player || it->renderOnFrame()) && it->visible &&
            (!player || it->getType() != &MapDecoration::Type::TYPES[7]))
        {
            int image = it->getImg() < 16 ? 14 : 120;
            int hash = it->hashCode();
            if (counts.find(hash) == counts.end() || counts.at(hash) < 3)
            {
                if (texture != image)
                {
                    texture = image;
                    mRenderer->mTextures->bind(mRenderer->mTextures->loadTexture(image));
                }
                GlStateManager::pushMatrix();
                GlStateManager::translatef((float)it->getX() / 2.0f + 64.0f,
                                            (float)it->getY() / 2.0f + 64.0f, depth);
                GlStateManager::rotatef((float)it->getRot() * 360.0f / 16.0f, 0.0f, 0.0f, 1.0f);
                GlStateManager::scalef(4.0f, 4.0f, 3.0f);
                GlStateManager::translatef(-0.125f, 0.125f, 0.0f);
                float minU = (float)(it->getImg() & 3) / 4.0f;
                float maxU = (float)((it->getImg() & 3) + 1) / 4.0f;
                float minV = (float)((it->getImg() & 15) / 4) / 4.0f;
                float maxV = (float)(((it->getImg() & 15) / 4) + 1) / 4.0f;
                float iconDepth = (float)index * -0.001f;
                builder->begin();
                builder->vertexUV(-1.0f, 1.0f, iconDepth, minU, minV);
                builder->vertexUV(1.0f, 1.0f, iconDepth, maxU, minV);
                builder->vertexUV(1.0f, -1.0f, iconDepth, maxU, maxV);
                builder->vertexUV(-1.0f, -1.0f, iconDepth, minU, maxV);
                tesselator->end();
                GlStateManager::popMatrix();
                ++index;
                ++counts[hash];
                depth -= 0.01f;
            }
        }
    }

    GlStateManager::pushMatrix();
    GlStateManager::translatef(0.0f, 0.0f, -0.06f);
    GlStateManager::scalef(0.5f, 0.5f, 0.5f);
    if (player)
    {
        Entity* entity = (Entity*)player.get();
        std::wstring text = formatwstr(L"X: %d, Y: %d, Z: %d", (int)std::floor(entity->mX),
                                       (int)std::floor(entity->mY + entity->getEyeHeight()),
                                       (int)std::floor(entity->mZ));
        mRenderer->mFont->draw(text, 0, 0,
            (int)Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Map_Text));
    }
    GlStateManager::popMatrix();
}

void MapRenderer::MapInstance::updateTexture()
{
    unsigned int i;
    for (i = 0; i < mData->mColors.length; ++i)
        mPixels.data[i] = gMapColours[mData->mColors.data[i]];
    mTextureDirty = true;
}
