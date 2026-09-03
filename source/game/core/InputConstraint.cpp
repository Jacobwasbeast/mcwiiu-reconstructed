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

    bool constrainedButton =
        g_pInput->ButtonDown(playerId, (unsigned char)constrainedMapping);
    bool requestedButton = g_pInput->ButtonDown(playerId, (unsigned char)mapping);

    return constrainedButton && requestedButton;
}
