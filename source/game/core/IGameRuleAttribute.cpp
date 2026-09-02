#include "game/core/IGameRuleAttribute.hpp"

static const wchar_t* sAttributeNames[] =
{
    L"descriptionName",
    L"promptName",
    L"dataTag",
    L"enchantmentId",
    L"enchantmentLevel",
    L"itemId",
    L"quantity",
    L"auxValue",
    L"slot",
    L"name",
    L"food",
    L"health",
    L"blockId",
    L"useCoords",
    L"seed",
    L"flatworld",
    L"filename",
    L"rot",
    L"data",
    L"block",
    L"entity",
    L"facing",
    L"edgeBlock",
    L"fillBlock",
    L"skipAir",
    L"x",
    L"x0",
    L"x1",
    L"y",
    L"y0",
    L"y1",
    L"z",
    L"z0",
    L"z1",
    L"chunkX",
    L"chunkZ",
    L"yRot",
    L"xRot",
    L"spawnX",
    L"spawnY",
    L"spawnZ",
    L"orientation",
    L"dimension",
    L"topblockId",
    L"biomeId",
    L"feature",
    L"minCount",
    L"maxCount",
    L"weight",
    L"id",
    L"probability",
    L"method",
    L"hasBeenInCreative",
    L"cloudHeight",
    L"fogDistance",
    L"dayTime",
    L"target",
    L"speed",
    L"dir",
    L"type",
    L"pass",
    L"for",
    L"random",
    L"blockAux",
    L"size",
    L"scale",
    L"freq",
    L"func",
    L"upper",
    L"lower",
    L"dY",
    L"thickness",
    L"points",
    L"holeSize",
    L"variant",
    L"startHeight",
    L"pattern",
    L"colour",
    L"primary",
    L"laps",
    L"liftForceModifier",
    L"staticLift",
    L"targetHeight",
    L"speedBoost",
    L"boostDirection",
    L"condition_type",
    L"condition_value_0",
    L"condition_value_1",
    L"beam_length",
    L"beam_direction",
    L"string_IDS"
};

static const int sAttributeNameCount = sizeof(sAttributeNames) / sizeof(sAttributeNames[0]);

IGameRuleAttribute::IGameRuleAttribute()
    : mOptional(false),
      mHasValue(false)
{
}

IGameRuleAttribute::~IGameRuleAttribute()
{
}

int IGameRuleAttribute::attributeIdFromName(const std::wstring& name)
{
    for (int id = 0; id < sAttributeNameCount; ++id)
    {
        if (name == sAttributeNames[id])
        {
            return id;
        }
    }

    return -1;
}

const wchar_t* IGameRuleAttribute::attributeName(ConsoleGameRules::EGameRuleAttr attribute)
{
    return sAttributeNames[attribute];
}

void IGameRuleAttribute::setOptional(bool optional)
{
    mOptional = optional;
}

bool IGameRuleAttribute::isOptional()
{
    return mOptional;
}

bool IGameRuleAttribute::hasValue()
{
    return mHasValue;
}
