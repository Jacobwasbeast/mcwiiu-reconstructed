#include "game/data/SavedDataStorage.hpp"

#include <algorithm>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/NbtIo.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/ConsoleSaveFileInputStream.hpp"
#include "game/data/ConsoleSaveFileOutputStream.hpp"
#include "game/data/ConsoleSavePath.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/data/ShortTag.hpp"
#include "game/data/Tag.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/world/generation/StructureFeatureSavedData.hpp"
#include "game/world/generation/Villages.hpp"

void __debugbreak();

SavedDataStorage::SavedDataStorage(LevelStorage* levelStorage)
{
    mLevelStorage = levelStorage;
    loadAuxValues();
}

void SavedDataStorage::loadAuxValues()
{
    mAuxValues.clear();
    if (mLevelStorage != NULL)
    {
        ConsoleSavePath dataFile = mLevelStorage->getDataFile(L"idcounts");
        if (dataFile.getPath().length() != 0 && mLevelStorage->getSaveFile()->doesFileExist(dataFile))
        {
            ConsoleSaveFileInputStream saveFileStream(mLevelStorage->getSaveFile(), dataFile);
            DataInputStream stream(&saveFileStream);
            CompoundTag* tag = NbtIo::read(&stream);
            stream.close();
            boost::unordered_map<std::wstring, Tag*>* tags = tag->getAllTagsMap();
            for (boost::unordered_map<std::wstring, Tag*>::iterator it = tags->begin(); it != tags->end(); ++it)
            {
                ShortTag* shortTag = dynamic_cast<ShortTag*>(it->second);
                if (shortTag != NULL)
                {
                    mAuxValues.insert(std::make_pair(it->first, shortTag->mValue));
                }
            }
        }
    }
}

boost::shared_ptr<SavedData> SavedDataStorage::get(const std::type_info& type, const std::wstring& id)
{
    boost::unordered_map<std::wstring, boost::shared_ptr<SavedData> >::iterator it = mSavedData.find(id);
    if (it != mSavedData.end())
    {
        return it->second;
    }
    boost::shared_ptr<SavedData> savedData;
    if (mLevelStorage != NULL)
    {
        ConsoleSavePath dataFile = mLevelStorage->getDataFile(id);
        if (dataFile.getPath().length() != 0 && mLevelStorage->getSaveFile()->doesFileExist(dataFile))
        {
            if (type == typeid(MapItemSavedData))
            {
                boost::shared_ptr<MapItemSavedData> mapData(new MapItemSavedData(id));
                savedData = boost::dynamic_pointer_cast<SavedData>(mapData);
            }
            else if (type == typeid(Villages))
            {
                boost::shared_ptr<Villages> villages(new Villages(id));
                savedData = boost::dynamic_pointer_cast<SavedData>(villages);
            }
            else if (type == typeid(StructureFeatureSavedData))
            {
                boost::shared_ptr<StructureFeatureSavedData> featureData(new StructureFeatureSavedData(id));
                savedData = boost::dynamic_pointer_cast<SavedData>(featureData);
            }
            else
            {
                __debugbreak();
            }
            ConsoleSaveFileInputStream stream(mLevelStorage->getSaveFile(), dataFile);
            CompoundTag* tag = NbtIo::readCompressed(&stream);
            stream.close();
            savedData->load(tag->getCompound(L"data"));
            delete tag;
        }
    }
    if (savedData)
    {
        mSavedData.insert(std::pair<const std::wstring, boost::shared_ptr<SavedData> >(id, savedData));
        mAllSavedData.push_back(savedData);
    }
    return savedData;
}

void SavedDataStorage::set(const std::wstring& id, boost::shared_ptr<SavedData> data)
{
    if (data)
    {
        boost::unordered_map<std::wstring, boost::shared_ptr<SavedData> >::iterator it = mSavedData.find(id);
        if (it != mSavedData.end())
        {
            std::vector<boost::shared_ptr<SavedData> >::iterator existing =
                std::find(mAllSavedData.begin(), mAllSavedData.end(), it->second);
            if (existing != mAllSavedData.end())
            {
                mAllSavedData.erase(existing);
            }
            mSavedData.erase(it);
        }
        mSavedData.insert(std::pair<const std::wstring, boost::shared_ptr<SavedData> >(id, data));
        mAllSavedData.push_back(data);
    }
}

void SavedDataStorage::save(boost::shared_ptr<SavedData> data)
{
    if (mLevelStorage != NULL)
    {
        ConsoleSavePath dataFile = mLevelStorage->getDataFile(data->id);
        if (dataFile.getPath().length() != 0)
        {
            CompoundTag* tag = new CompoundTag();
            tag->put(L"data", data->save(new CompoundTag()));
            ConsoleSaveFileOutputStream stream(mLevelStorage->getSaveFile(), dataFile);
            NbtIo::writeCompressed(tag, &stream);
            stream.close();
            delete tag;
        }
    }
}

void SavedDataStorage::save()
{
    for (std::vector<boost::shared_ptr<SavedData> >::iterator it = mAllSavedData.begin(); it != mAllSavedData.end(); ++it)
    {
        boost::shared_ptr<SavedData> data = *it;
        if (data->isDirty())
        {
            save(data);
            data->setDirty(false);
        }
    }
}

int SavedDataStorage::getFreeAuxValueFor(const std::wstring& id)
{
    short auxValue = 0;
    boost::unordered_map<std::wstring, short>::iterator it = mAuxValues.find(id);
    if (it != mAuxValues.end())
    {
        auxValue = (short)(it->second + 1);
    }
    mAuxValues[id] = auxValue;
    if (mLevelStorage != NULL)
    {
        ConsoleSavePath dataFile = mLevelStorage->getDataFile(L"idcounts");
        bool hasDataFile = dataFile.getPath().length() != 0;
        if (hasDataFile)
        {
            CompoundTag* tag = new CompoundTag();
            for (boost::unordered_map<std::wstring, short>::iterator counts = mAuxValues.begin(); counts != mAuxValues.end(); ++counts)
            {
                short count = counts->second;
                tag->putShort(counts->first.c_str(), count);
            }
            ConsoleSaveFileOutputStream saveFileStream(mLevelStorage->getSaveFile(), dataFile);
            DataOutputStream stream(&saveFileStream);
            NbtIo::write(tag, &stream);
            stream.close();
        }
    }
    return auxValue;
}

int SavedDataStorage::getAuxValueForMap(PlayerUID uid, int dimension, int x, int z, int scale)
{
    if (mLevelStorage != NULL)
    {
        return mLevelStorage->getAuxValueForMap(uid, dimension, x, z, scale);
    }
    if (dimension == 1)
    {
        return 253;
    }
    if (dimension == -1)
    {
        return 254;
    }
    return 255;
}
