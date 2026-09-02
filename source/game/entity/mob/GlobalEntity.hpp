#ifndef GAME_ENTITY_MOB_GLOBALENTITY_HPP
#define GAME_ENTITY_MOB_GLOBALENTITY_HPP

class Entity;
class Level;

class GlobalEntity : public Entity
{
public:
    GlobalEntity(Level* level)
        : Entity(level, true)
    {
    }

    virtual ~GlobalEntity();
};

#endif
