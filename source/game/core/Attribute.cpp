#include "game/core/Attribute.hpp"

static const unsigned int ATTRIBUTE_NAMES[eATTRIBUTE_ID_Count] =
{
    0xBADA09B9,
    0x46BDC66F,
    0x102AF61B,
    0xAA6BDC35,
    0xF25163D2,
    0x400FEB0C,
    0xAD135001,
    0x1EA0E2FE,
    0x6CA19582,
    0xAAE7CF20,
    0xC9BC92C8,
    0
};

Attribute::~Attribute()
{
}

int Attribute::getName(eATTRIBUTE_ID id)
{
    return ATTRIBUTE_NAMES[id];
}
