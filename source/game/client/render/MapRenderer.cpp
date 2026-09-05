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
        unsigned int colour = colours->getColour((eMinecraftColour)i);
        unsigned int red = (colour >> 16) & 255;
        unsigned int green = (colour >> 8) & 255;
        unsigned int blue = colour & 255;
        gMapColours[i] = (red << 24) | (green << 16) | (blue << 8) | 255;
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
        mTextureDirty = false;
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
    builder->vertexUV(0.0f, 128.0f, 0.0f, 0.0f, 1.0f);
    builder->vertexUV(128.0f, 128.0f, 0.0f, 1.0f, 1.0f);
    builder->vertexUV(128.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    builder->vertexUV(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    tesselator->end();
    GlStateManager::enableAlphaTest();
    GlStateManager::disableBlend();

    std::vector<MapDecoration> decorations = mData->fjGetAllDecorations();
    std::sort(decorations.begin(), decorations.end(), sortByTexture);
    GlStateManager::pushMatrix();
    GlStateManager::translatef(64.0f, 64.0f, -0.02f);
    GlStateManager::scalef(1.0f, 1.0f, 1.0f);
    int index = 0;
    int texture = -1;
    boost::unordered_map<int, int> counts;
    std::vector<MapDecoration>::const_iterator it;
    for (it = decorations.begin(); it != decorations.end(); ++it)
    {
        if ((!player || it->renderOnFrame()) && it->visible)
        {
            int image = it->getImg() < 16 ? 14 : 120;
            if (counts.find(it->hashCode()) == counts.end() || counts[it->hashCode()] < 3)
            {
                if (texture != image)
                {
                    texture = image;
                    mRenderer->mTextures->bind(mRenderer->mTextures->loadTexture(image));
                }
                GlStateManager::pushMatrix();
                GlStateManager::translatef((float)it->getX() / 2.0f + 64.0f,
                                            (float)it->getY() / 2.0f + 64.0f, -0.03f);
                GlStateManager::rotatef((float)it->getRot() * 360.0f / 16.0f, 0.0f, 0.0f, 1.0f);
                GlStateManager::scalef(4.0f, 4.0f, 3.0f);
                builder->begin();
                builder->vertexUV(-1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
                builder->vertexUV(1.0f, 1.0f, 0.0f, 1.0f, 0.0f);
                builder->vertexUV(1.0f, -1.0f, 0.0f, 1.0f, 1.0f);
                builder->vertexUV(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f);
                tesselator->end();
                GlStateManager::popMatrix();
                ++index;
                ++counts[it->hashCode()];
            }
        }
    }
    GlStateManager::popMatrix();

    GlStateManager::pushMatrix();
    GlStateManager::translatef(0.0f, 0.0f, -0.04f);
    GlStateManager::scalef(0.5f, 0.5f, 0.5f);
    if (player)
    {
        Entity* entity = (Entity*)player.get();
        std::wstring text = formatwstr(L"X: %d, Y: %d, Z: %d", (int)std::floor(entity->mX),
                                       (int)std::floor(entity->mY), (int)std::floor(entity->mZ));
        mRenderer->mFont->draw(text, 0, 0,
            (int)Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Map_Text));
    }
    GlStateManager::popMatrix();
}

void MapRenderer::MapInstance::updateTexture()
{
    mTextureDirty = true;
}
