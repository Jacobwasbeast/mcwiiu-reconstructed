#include "game/core/OverwritableWalker.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixer.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/util/ResourceLocation.hpp"

OverwritableWalker::OverwritableWalker(unsigned long long type, const std::wstring& name)
    : mType(type)
    , mName(name)
{
}

OverwritableWalker::~OverwritableWalker()
{
}

CompoundTag* OverwritableWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    const std::wstring idName(L"id");
    const std::wstring oldId(tag->getString(idName));
    ResourceLocation id(oldId);
    ResourceLocation key(EntityIO::getKey(mType));

    if (key.equals(&id))
    {
        tag->putString(idName, mName);
        fixer->update(DataFixTypes::eDataFixTypes_BlockEntity, tag, version);
        tag->putString(idName, oldId);
    }

    return tag;
}
