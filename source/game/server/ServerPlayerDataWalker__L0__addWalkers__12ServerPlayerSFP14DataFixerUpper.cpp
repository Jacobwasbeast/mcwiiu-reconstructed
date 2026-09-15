#include "game/server/ServerPlayerDataWalker__L0__addWalkers__12ServerPlayerSFP14DataFixerUpper.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixer.hpp"
#include "game/data/DataFixTypes.hpp"

ServerPlayerDataWalker__L0__addWalkers__12ServerPlayerSFP14DataFixerUpper::~ServerPlayerDataWalker__L0__addWalkers__12ServerPlayerSFP14DataFixerUpper()
{
}

CompoundTag* ServerPlayerDataWalker__L0__addWalkers__12ServerPlayerSFP14DataFixerUpper::walk(
    DataFixer* fixer, CompoundTag* tag, int version)
{
    const std::wstring rootVehicleName(L"RootVehicle");
    if (tag->contains(rootVehicleName, 10))
    {
        CompoundTag* rootVehicle = tag->getCompound(rootVehicleName);
        const std::wstring entityName(L"Entity");
        if (rootVehicle->contains(entityName, 10))
        {
            rootVehicle->put(entityName,
                fixer->update(DataFixTypes::eDataFixTypes_Entity,
                    rootVehicle->getCompound(entityName), version));
        }
    }
    return tag;
}
