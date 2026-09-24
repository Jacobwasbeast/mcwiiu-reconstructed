#include "game/client/render/PigModel.hpp"

#include "game/client/render/ModelDefinition.hpp"

PigModel::PigModel(const ModelDefinition& modelDefinition)
    : QuadrupedModel(6, 0.0f, modelDefinition)
{
    if (!modelDefinition.DoesOverrideModel())
    {
        mHead->texOffs(16, 16)->addBox(-0.8f, 0.0f, -9.0f, 4, 3, 1, 0.0f);
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
        mHead->texOffs(16, 16)->addBox(-0.8f, 0.0f, -9.0f, 4, 3, 1, scale);
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
