#include "game/client/render/DefaultTexturePack.hpp"

#include <string>

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/File.hpp"
#include "game/core/FileInputStream.hpp"
#include "game/core/InputStream.hpp"
#include "game/core/L10N.hpp"

DefaultTexturePack::DefaultTexturePack()
    : AbstractTexturePack(0, NULL, L10N::GetString(0x3a2a406), NULL)
{
    loadIcon();
    loadName();
    loadDescription();
    AbstractTexturePack::loadColourTable();
    AbstractTexturePack::loadMaterialTable();
    loadCustomModels();
}

DefaultTexturePack::~DefaultTexturePack()
{
}

void DefaultTexturePack::loadIcon()
{
    std::wstring fileName = L"Graphics\\TexturePackIcon.png";

    if (CMinecraftApp::sInstance.hasArchiveFile(fileName, false))
    {
        arrayWithLength<unsigned char> iconData = CMinecraftApp::sInstance.getArchiveFile(fileName, false);
        m_pIconData = iconData.data;
        m_iconSize = iconData.length;
    }
}

void DefaultTexturePack::loadName()
{
    m_name = L10N::GetString(0x3a2a406);
}

void DefaultTexturePack::loadDescription()
{
    m_desc1 = L"LOCALISE ME: The default look of";
}

void DefaultTexturePack::loadCustomModels()
{
    m_customModels.clear();

    File file(getPath(true, NULL) + L"res/models.bin");

    if (!file.exists())
    {
        theApp->FatalLoadError(L"<AbstractTexturePack@%d> Failed ", 191);
    }
    else
    {
        unsigned int length = (unsigned int)file.length();
        arrayWithLength<unsigned char> data(length, true);
        {
            FileInputStream fileStream(file);
            fileStream.read(data, 0, length);
            fileStream.close();

            ByteArrayInputStream byteStream(data);
            DataInputStream input(&byteStream);
            int version = input.readInt();
            int count = input.readInt();

            for (int index = 0; index < count; index++)
            {
                std::wstring modelName = input.readUTF();
                m_customModels[modelName].readModel(&input, version);
            }
        }

    }
}

bool DefaultTexturePack::hasFile(const std::wstring& fileName)
{
    return true;
}

bool DefaultTexturePack::isTerrainUpdateCompatible()
{
    return true;
}

std::wstring DefaultTexturePack::getName()
{
    return L10N::GetString(0x3a2a406);
}

std::wstring DefaultTexturePack::getDesc1()
{
    return L10N::GetString(0x8b1d2c06);
}

InputStream* DefaultTexturePack::getResourceImplementation(const std::wstring& fileName)
{
    std::wstring resourcePath = L"Common\\res\\TitleUpdate\\res";
    resourcePath += fileName;
    return InputStream::getResourceAsStream(resourcePath);
}

void DefaultTexturePack::loadUI()
{
    AbstractTexturePack::loadDefaultUI();
    AbstractTexturePack::loadUI();
}

void DefaultTexturePack::unloadUI()
{
    AbstractTexturePack::unloadUI();
}

DLCPack* DefaultTexturePack::getDLCPack()
{
    return NULL;
}

bool DefaultTexturePack::hasData()
{
    return true;
}

bool DefaultTexturePack::hasValidData()
{
    return true;
}

bool DefaultTexturePack::hasAudio()
{
    return false;
}

bool DefaultTexturePack::isLoadingData()
{
    return false;
}
