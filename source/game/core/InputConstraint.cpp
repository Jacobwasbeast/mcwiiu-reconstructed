#include "game/core/InputConstraint.hpp"

#include "game/core/CInput.hpp"

InputConstraint::~InputConstraint()
{
}

int InputConstraint::getType()
{
    return 0;
}

bool InputConstraint::isMappingConstrained(int playerId, int mapping)
{
    if (constrainedMapping == mapping || mapping <= 22)
    {
        return true;
    }

    unsigned int constrainedButtons =
        g_pInput->GetGameJoypadMaps(playerId, (unsigned char)constrainedMapping);
    unsigned int requestedButtons = g_pInput->GetGameJoypadMaps(playerId, (unsigned char)mapping);

    return (constrainedButtons & requestedButtons) != 0;
}
