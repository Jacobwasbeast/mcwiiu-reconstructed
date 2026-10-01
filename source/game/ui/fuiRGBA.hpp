#ifndef GAME_UI_FUIRGBA_HPP
#define GAME_UI_FUIRGBA_HPP

struct fuiRGBA
{
    union
    {
        unsigned int rgba;
        struct
        {
            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;
        };
    };

    void EndianSwap();
};

#endif
