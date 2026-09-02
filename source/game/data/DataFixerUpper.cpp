#include "game/data/DataFixerUpper.hpp"

#include <string>

#include "game/core/DataWalker.hpp"
#include "game/data/DataFix.hpp"

static const std::wstring DATA_VERSION_TAG = L"DataVersion";

DataFixerUpper::DataFixerUpper(int version)
    : mWalkers(11)
    , mFixes(11)
{
    mVersion = version;
}

DataFixerUpper::~DataFixerUpper()
{
    for (FixMap::iterator fixEntry = mFixes.begin(); fixEntry != mFixes.end(); ++fixEntry)
    {
        std::vector<DataFix*>* fixes = fixEntry->second;
        if (!fixes->empty())
        {
            for (std::vector<DataFix*>::iterator fix = fixes->begin(); fix != fixes->end(); ++fix)
            {
                delete *fix;
            }
            delete fixes;
        }
    }

    for (WalkerMap::iterator walkerEntry = mWalkers.begin(); walkerEntry != mWalkers.end(); ++walkerEntry)
    {
        std::vector<DataWalker*>* walkers = walkerEntry->second;
        if (!walkers->empty())
        {
            for (std::vector<DataWalker*>::iterator walker = walkers->begin(); walker != walkers->end(); ++walker)
            {
                delete *walker;
            }
            delete walkers;
        }
    }
}

CompoundTag* DataFixerUpper::updateData(DataFixTypes::EDataFixTypes type, CompoundTag* tag)
{
    int version = -1;
    if (tag->contains(DATA_VERSION_TAG, 99))
    {
        version = tag->getInt(DATA_VERSION_TAG);
        if (version > 921)
        {
            return tag;
        }
    }

    return update(type, tag, version);
}

CompoundTag* DataFixerUpper::update(DataFixTypes::EDataFixTypes type, CompoundTag* tag, int version)
{
    if (version < mVersion)
    {
        tag = applyWalkers(type, applyFixes(type, tag, version), version);
    }

    return tag;
}

CompoundTag* DataFixerUpper::applyFixes(DataFixTypes::EDataFixTypes type, CompoundTag* tag, int version)
{
    if (mFixes.find(type) != mFixes.end())
    {
        std::vector<DataFix*>* fixes = mFixes[type];
        if (!fixes->empty())
        {
            for (std::vector<DataFix*>::iterator fix = fixes->begin(); fix != fixes->end(); ++fix)
            {
                if (version < (*fix)->getVersion())
                {
                    tag = (*fix)->fix(tag);
                }
            }
        }
    }

    return tag;
}

CompoundTag* DataFixerUpper::applyWalkers(DataFixTypes::EDataFixTypes type, CompoundTag* tag, int version)
{
    WalkerMap::iterator walkerEntry = mWalkers.find(type);
    if (walkerEntry != mWalkers.end())
    {
        std::vector<DataWalker*>* walkers = walkerEntry->second;
        if (!walkers->empty())
        {
            for (std::vector<DataWalker*>::iterator walker = walkers->begin(); walker != walkers->end(); ++walker)
            {
                tag = (*walker)->walk(this, tag, version);
            }
        }
    }

    return tag;
}

std::vector<DataWalker*>* DataFixerUpper::getOrCreateWalker(DataFixTypes::EDataFixTypes type)
{
    std::vector<DataWalker*>* walkers = mWalkers[type];
    if (walkers != NULL && !walkers->empty())
    {
        return walkers;
    }

    walkers = new std::vector<DataWalker*>();
    mWalkers[type] = walkers;

    return walkers;
}

void DataFixerUpper::addWalker(DataFixTypes::EDataFixTypes type, DataWalker* walker)
{
    getOrCreateWalker(type)->push_back(walker);
}

std::vector<DataFix*>* DataFixerUpper::getOrCreateFix(DataFixTypes::EDataFixTypes type)
{
    std::vector<DataFix*>* fixes = mFixes[type];
    if (fixes != NULL && !fixes->empty())
    {
        return fixes;
    }

    fixes = new std::vector<DataFix*>();
    mFixes[type] = fixes;

    return fixes;
}

void DataFixerUpper::addFixer(DataFixTypes::EDataFixTypes type, DataFix* fix)
{
    std::vector<DataFix*>* fixes = getOrCreateFix(type);

    int version = fix->getVersion();
    if (version > mVersion)
    {
        return;
    }

    if (fixes->empty() || fixes->back()->getVersion() <= version)
    {
        fixes->push_back(fix);
        return;
    }

    for (std::vector<DataFix*>::iterator existing = fixes->begin(); existing != fixes->end(); ++existing)
    {
        if (version < (*existing)->getVersion())
        {
            fixes->push_back(fix);
            return;
        }
    }
}
