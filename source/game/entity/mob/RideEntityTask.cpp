#include "game/entity/mob/RideEntityTask.hpp"

#include "game/entity/mob/Entity.hpp"

RideEntityTask::~RideEntityTask()
{
}

bool RideEntityTask::isCompleted()
{
    return completed;
}

void RideEntityTask::onRideEntity(boost::shared_ptr<Entity> entity)
{
    unsigned long long type = entity->GetType();

    if ((entityType & 0x3F) != 0)
    {
        if (type != (unsigned long long)entityType)
        {
            return;
        }
    }
    else if ((type & entityType) != (unsigned long long)entityType)
    {
        return;
    }

    completed = true;
}
