#include "game/client/render/TexturePackRepository.hpp"

#include <boost/unordered_map.hpp>
#include <new>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/DLCTexturePack.hpp"
#include "game/client/render/MapRenderer.hpp"

extern ConsoleUIController g_ConsoleUIController;

DefaultTexturePack* TexturePackRepository::defaultTexturePack = NULL;

TexturePackRepository::TexturePackRepository(File file, Minecraft* minecraft)
    : m_unknown0(0)
    , m_unknown1(0)
    , m_minecraft(minecraft)
    , m_allTexturePacks(NULL)
    , m_selected(NULL)
    , m_previousSelected(NULL)
    , m_skinReloaded(false)
{
    if (defaultTexturePack == NULL)
    {
        defaultTexturePack = new DefaultTexturePack();
    }

    m_allTexturePacks = new std::vector<TexturePack*>();
    m_allTexturePacks->push_back(defaultTexturePack);
    m_texturePacksById[defaultTexturePack->getId()] = defaultTexturePack;
    m_selected = defaultTexturePack;
    m_selected->loadUI();
    m_selected->loadCustomModels();
}

TexturePack* TexturePackRepository::getSelected()
{
    if (m_selected != NULL && m_selected->hasData() && m_selected->hasValidData())
    {
        return m_selected;
    }
    return defaultTexturePack;
}

void TexturePackRepository::clearInvalidTexturePacks()
{
    std::vector<TexturePack*>::iterator iterator = m_texturePacks.begin();
    while (iterator != m_texturePacks.end())
    {
        if (*iterator != NULL)
        {
            delete *iterator;
        }
        ++iterator;
    }
}

void TexturePackRepository::addDebugPacks()
{
}

bool TexturePackRepository::selectSkin(TexturePack* texturePack)
{
    if (texturePack == m_selected)
    {
        return false;
    }
    m_previousSelected = m_selected;
    m_selected = texturePack;
    m_skinReloaded = false;
    return true;
}

unsigned int TexturePackRepository::getSelectedId()
{
    if (m_selected != NULL)
    {
        return m_selected->getId();
    }
    return 0;
}

TexturePackRepository::TexturePackIdNames* TexturePackRepository::getTexturePackIdNames()
{
    TexturePackIdNames* names = new TexturePackIdNames();
    for (std::vector<TexturePack*>::iterator iterator = m_allTexturePacks->begin();
         iterator != m_allTexturePacks->end(); ++iterator)
    {
        TexturePack* texturePack = *iterator;
        names->push_back(TexturePackIdName(texturePack->getId(), texturePack->getName()));
    }
    return names;
}

bool TexturePackRepository::selectTexturePackById(unsigned int packId, bool forceReload)
{
    CMinecraftApp::sInstance.SetRequiredTexturePackID(packId);
    boost::unordered_map<unsigned int, TexturePack*>::iterator iterator = m_texturePacksById.find(packId);
    if (iterator == m_texturePacksById.end())
    {
        if (selectSkin(defaultTexturePack))
        {
            CMinecraftApp::sInstance.StartReloadingTexturePack();
        }
        return false;
    }

    TexturePack* texturePack = iterator->second;
    if (texturePack != m_selected)
    {
        selectSkin(texturePack);
        if (texturePack->hasData())
        {
            CMinecraftApp::sInstance.StartReloadingTexturePack();
            return true;
        }
        texturePack->loadData();
    }
    return true;
}

TexturePack* TexturePackRepository::getTexturePackById(unsigned int packId)
{
    boost::unordered_map<unsigned int, TexturePack*>::iterator iterator = m_texturePacksById.find(packId);
    if (iterator != m_texturePacksById.end())
    {
        return iterator->second;
    }
    return NULL;
}

TexturePack* TexturePackRepository::addTexturePackFromDLC(DLCPack* dlcPack, unsigned int packId)
{
    TexturePack* texturePack = NULL;
    packId &= 0x00ffffff;
    if (dlcPack != NULL)
    {
        texturePack = new DLCTexturePack(packId, dlcPack, defaultTexturePack);
        m_allTexturePacks->push_back(texturePack);
        m_texturePacksById[packId] = texturePack;
    }
    return texturePack;
}

void TexturePackRepository::updateUI()
{
    if (m_previousSelected != NULL && m_previousSelected != m_selected)
    {
        m_previousSelected->unloadUI();
        m_selected->loadUI();
        MapRenderer::reloadColours();
        g_ConsoleUIController.StartReloadSkinThread();
        m_previousSelected = NULL;
    }
}

bool TexturePackRepository::needsUIUpdate()
{
    return m_previousSelected != NULL && m_previousSelected != m_selected;
}

int TexturePackRepository::getTexturePackCount()
{
    return static_cast<int>(m_allTexturePacks->size());
}

TexturePack* TexturePackRepository::getTexturePackByIndex(unsigned int index)
{
    if (index < m_allTexturePacks->size())
    {
        return m_allTexturePacks->at(index);
    }
    return NULL;
}

unsigned int TexturePackRepository::getTexturePackIndexByPackId(unsigned int packId)
{
    unsigned int index = 0;
    for (std::vector<TexturePack*>::iterator iterator = m_allTexturePacks->begin();
         iterator != m_allTexturePacks->end(); ++iterator, ++index)
    {
        if ((*iterator)->getId() == packId)
        {
            return index;
        }
    }
    return 0;
}
