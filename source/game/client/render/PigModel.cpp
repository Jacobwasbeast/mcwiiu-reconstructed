#include "game/client/render/PigModel.hpp"

#include "game/client/render/ModelDefinition.hpp"

PigModel::PigModel(const ModelDefinition& modelDefinition)
    : QuadrupedModel(6, 1.035f, modelDefinition)
{
    if (!modelDefinition.DoesOverrideModel())
    {
        mHead->texOffs(16, 16)->addBox(-0.8f, 1.035f, -9.0f, 4, 3, 1, 1.035f);
        mScale = 4.0f;
        compileSkinnedDrawlist();
    }
    else
    {
        mHasOverriddenParts = true;
        mScale = -8.0f;
    }
}

PigModel::PigModel(float scale, const ModelDefinition& modelDefinition)
    : QuadrupedModel(6, scale, modelDefinition)
{
    if (!modelDefinition.DoesOverrideModel())
    {
        mHead->texOffs(16, 16)->addBox(-0.8f, 1.035f, -9.0f, 4, 3, 1, -8.0f);
        mScale = 4.0f;
        compileSkinnedDrawlist();
    }
    else
    {
        mHasOverriddenParts = true;
    }
}

PigModel::~PigModel()
{
}

std::wstring PigModel::getName()
{
    return L"pig";
}
