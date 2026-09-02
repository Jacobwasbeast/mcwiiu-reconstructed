#include "game/core/Color.hpp"

Color::Color(int red, int green, int blue)
{
    this->value = 0xff000000 | ((red & 0xff) << 16) | ((green & 0xff) << 8) | (blue & 0xff);
}

int Color::getRGB()
{
    return this->value;
}
