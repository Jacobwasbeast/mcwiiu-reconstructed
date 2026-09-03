#include "game/entity/mob/EntityElderGuardianSplitFix.hpp"

EntityElderGuardianSplitFix::~EntityElderGuardianSplitFix()
{
}

int EntityElderGuardianSplitFix::getVersion()
{
    return 700;
}

CompoundTag* EntityElderGuardianSplitFix::fix(CompoundTag* tag)
{
    if (CheckID(tag, L"Guardian"))
    {
        if (tag->getBoolean(L"Elder"))
        {
            tag->putString(L"id", L"ElderGuardian");
        }

        Tag* elder = tag->get(L"Elder");

        if (elder != NULL)
        {
            delete elder;
        }

        tag->remove(L"Elder");
    }

    return tag;
}
