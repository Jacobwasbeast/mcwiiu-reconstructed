#include "game/core/Difficulty.hpp"

const Difficulty* Difficulty::PEACEFUL = new Difficulty(0, 0x4E52B99A);
const Difficulty* Difficulty::EASY = new Difficulty(1, 0xB558EFD9);
const Difficulty* Difficulty::NORMAL = new Difficulty(2, 0x5D131124);
const Difficulty* Difficulty::HARD = new Difficulty(3, 0xF70F3270);

const Difficulty* Difficulty::BY_ID[4] = { PEACEFUL, EASY, NORMAL, HARD };

Difficulty::Difficulty(int id, int key)
    : id(id),
      key(key)
{
}

const Difficulty* Difficulty::byId(int id)
{
    return BY_ID[id % 4];
}

int Difficulty::getId() const
{
    return id;
}

int Difficulty::getKey() const
{
    return key;
}
