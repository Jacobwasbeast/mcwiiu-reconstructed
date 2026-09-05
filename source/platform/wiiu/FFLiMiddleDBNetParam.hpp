#ifndef PLATFORM_WIIU_FFLIMIDDLEDBNETPARAM_HPP
#define PLATFORM_WIIU_FFLIMIDDLEDBNETPARAM_HPP

struct FFLiMiddleDBNetParam
{
    void Init();
    void Set(unsigned short index);

private:
    unsigned short m_index;
};

#endif
