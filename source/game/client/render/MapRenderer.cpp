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
#include "game/world/block/MaterialColor.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Options.hpp"
#include "game/core/StringFormat.hpp"

static unsigned int gMapColours[208];
static bool gMapColoursLoaded = true;

bool sortByTexture(const MapDecoration& left, const MapDecoration& right)
{
    bool result;
    if (!left.getType()->isPlayer())
    {
        if (!right.getType()->isPlayer())
        {
            int leftIcon = left.getImg() < 16;
            int rightIcon = right.getImg() < 16;
            result = false;
            if (leftIcon && !rightIcon)
                result = true;
            return result;
        }
    }

    result = false;
    if (!left.getType()->isPlayer() && right.getType()->isPlayer())
        result = true;
    return result;
}

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
        int material = i / 4;
        if (material == 0)
        {
            gMapColours[i] = 16 + ((i + i / 128) * 8 & 8);
            continue;
        }
        unsigned int colour = colours->getColour(MaterialColor::MATERIAL_COLORS[material]->col);
        int brightness = 220;
        if (i % 4 == 2)
        {
            gMapColours[i] = (((((colour >> 16) & 255) * 255) / 255) << 24) |
                             (((((colour >> 8) & 255) * 255) / 255) << 16) |
                             ((((colour & 255) * 255) / 255) << 8) | 255;
        }
        else
        {
            if (i % 4 == 0)
                brightness = 180;
            gMapColours[i] = (((((colour >> 16) & 255) * brightness) / 255) << 24) |
                             (((((colour >> 8) & 255) * brightness) / 255) << 16) |
                             ((((colour & 255) * brightness) / 255) << 8) | 255;
        }
    }
    gMapColoursLoaded = false;
}

MapRenderer::MapInstance* MapRenderer::getMapInstance(
    const boost::shared_ptr<MapItemSavedData>& data)
{
    boost::unordered_map<std::wstring, MapInstance*>::iterator found = mMapInstances.find(data->id);
    if (found == mMapInstances.end())
    {
        mMapInstances[data->id] = new MapInstance(this, data);
        found = mMapInstances.find(data->id);
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
        mTextures->releaseTexture(it->second->mTexture);
        if (it->second != NULL)
            delete it->second;
    }
    mMapInstances.clear();
}

boost::shared_ptr<MapItemSavedData> MapRenderer::getData(MapInstance* instance)
{
    if (instance != NULL)
        return instance->mData;
    return boost::shared_ptr<MapItemSavedData>();
}

MapRenderer::MapInstance::MapInstance(MapRenderer* renderer,
                                      const boost::shared_ptr<MapItemSavedData>& data)
    : mRenderer(renderer), mData(data), mTexture(-1), mTextureDirty(false)
{
    mPixels = arrayWithLength<int>(16384, true);
    arrayWithLength<int>::zero(mPixels.data, 16384);
    if (gMapColoursLoaded)
        MapRenderer::reloadColours();
}

void MapRenderer::MapInstance::fjUpdateWithRenderContext()
{
    if (mTexture == -1)
    {
        BufferedImage image(128, 128, 0);
        mTexture = mRenderer->mTextures->getTexture(&image, C4JRender::eTextureFormat_RGBA8, false);
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
    Player* viewer = player.get();
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
    unsigned int index = 0;
    int texture = -1;
    float depth = -0.04f;
    {
        boost::unordered_map<int, int> counts;
        std::vector<MapDecoration>::const_iterator it;
        for (it = decorations.begin(); it != decorations.end(); ++it)
        {
            if ((viewer || it->renderOnFrame()) && it->visible &&
                (!viewer || it->getType() != &MapDecoration::Type::TYPES[7]))
            {
                unsigned int img = it->getImg();
                int image = img < 16 ? 14 : 120;
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
                    GlStateManager::rotatef((float)(it->getRot() * 360) / 16.0f, 0.0f, 0.0f, 1.0f);
                    GlStateManager::scalef(4.0f, 4.0f, 3.0f);
                    GlStateManager::translatef(-0.125f, 0.125f, 0.0f);
                    int column = (img & 15) % 4;
                    int row = (int)(img & 15) / 4;
                    float minU = (float)column / 4.0f;
                    float maxU = (float)(column + 1) / 4.0f;
                    float minV = (float)row / 4.0f;
                    float maxV = (float)(row + 1) / 4.0f;
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
    }

    GlStateManager::pushMatrix();
    GlStateManager::translatef(0.0f, 0.0f, -0.06f);
    GlStateManager::scalef(1.0f, 1.0f, 1.0f);
    if (player)
    {
        std::wstring text = formatwstr(L"X: %d, Y: %d, Z: %d", (int)std::floor(player->mX),
                                       (int)std::floor(player->mY + player->getEyeHeight()),
                                       (int)std::floor(player->mZ));
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
