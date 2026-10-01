#ifndef GAME_CLIENT_RENDER_VILLAGERZOMBIEMODEL_HPP
#define GAME_CLIENT_RENDER_VILLAGERZOMBIEMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidModel.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelDefinition;

class VillagerZombieModel : public HumanoidModel
{
public:
    VillagerZombieModel(const ModelDefinition& definition);
    VillagerZombieModel(float scale, float yOffset, bool armourModel, const ModelDefinition& definition);


    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"zombie.villager"; }

    void _init(float scale, float yOffset, bool armourModel, const ModelDefinition& definition);

    int version();
};

#endif
