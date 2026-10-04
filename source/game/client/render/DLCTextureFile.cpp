#include "game/client/render/DLCTextureFile.hpp"

#include <vector>

template<typename T> std::wstring _toString(T value);
template<typename T> T _fromString(const std::wstring& text);
std::vector<std::wstring> stringSplit(const std::wstring& text, wchar_t separator);

DLCTextureFile::DLCTextureFile(const std::wstring& path)
    : DLCFile(DLCManager::eDLCType_Texture, path.substr(0, path.length() - 4))
    , m_parameters()
{
    m_dataSize = 0;
    m_pData = NULL;
    m_disableMipMap = false;
    m_hasAnimation = false;
}

DLCTextureFile::~DLCTextureFile()
{
}

void DLCTextureFile::addData(unsigned char* pData, unsigned int size)
{
    m_pData = pData;
    m_dataSize = size;
}

unsigned char* DLCTextureFile::getData(unsigned int& size)
{
    size = m_dataSize;
    return m_pData;
}

void DLCTextureFile::addParameter(DLCManager::EDLCParameterType parameter, const std::wstring& value)
{
    if (parameter == DLCManager::eDLCParameterType_Anim)
    {
        std::wstring animation = value;
        if (animation.empty())
        {
            animation = L",";
        }

        if (animation[0] == L'%')
        {
            std::vector<std::wstring> parts = stringSplit(animation.substr(1), L'%');
            m_parameters[parts[0]] = parts[1];
        }
        else
        {
            m_parameters[L"True"] = animation;
        }
        m_hasAnimation = true;
        return;
    }

    if (parameter == DLCManager::eDLCParameterType_DisableMipMap)
    {
        m_disableMipMap = _fromString<bool>(value);
        if (!m_disableMipMap && value.compare(0, value.length(), L"True", 4) == 0)
        {
            m_disableMipMap = true;
        }
    }
}

std::wstring DLCTextureFile::getParameterAsString(DLCManager::EDLCParameterType parameter, const std::wstring& name)
{
    if (parameter == DLCManager::eDLCParameterType_Anim)
    {
        if (!m_hasAnimation)
        {
            return L",";
        }

        boost::unordered_map<std::wstring, std::wstring>::const_iterator it = m_parameters.find(name);
        if (it == m_parameters.end())
        {
            return m_parameters.begin()->second;
        }
        return it->second;
    }

    if (parameter == DLCManager::eDLCParameterType_DisableMipMap)
    {
        return _toString<bool>(m_disableMipMap);
    }

    return L"";
}

bool DLCTextureFile::getParameterAsBool(DLCManager::EDLCParameterType parameter)
{
    if (parameter == DLCManager::eDLCParameterType_Anim)
    {
        return m_hasAnimation;
    }
    if (parameter != DLCManager::eDLCParameterType_DisableMipMap)
    {
        return false;
    }
    return m_disableMipMap;
}
