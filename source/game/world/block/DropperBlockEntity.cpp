#include "game/world/block/DropperBlockEntity.hpp"

#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/item/ItemListDataWalker.hpp"

DropperBlockEntity::DropperBlockEntity()
{
}

DropperBlockEntity::~DropperBlockEntity()
{
}

BlockEntity* DropperBlockEntity::create()
{
    return new DropperBlockEntity();
}

int DropperBlockEntity::getTypeId(bool)
{
    return 10;
}

void DropperBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                     (DataWalker*)new ItemListDataWalker(0x3c00201ULL, L"Items"));
}

boost::shared_ptr<BlockEntity> DropperBlockEntity::clone()
{
    boost::shared_ptr<DropperBlockEntity> result(new DropperBlockEntity());
    BlockEntity::clone(result);
    if (mCustomName)
    {
        result->mCustomName->set(mCustomName->raw(), std::wstring(), false, false);
    }
    return result;
}

unsigned long long DropperBlockEntity::GetType()
{
    return 0x003c002010000000ULL;
}
