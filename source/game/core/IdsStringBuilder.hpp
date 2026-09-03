#ifndef GAME_CORE_IDSSTRINGBUILDER_HPP
#define GAME_CORE_IDSSTRINGBUILDER_HPP

#include <string>

class IdsStringBuilder
{
public:
    explicit IdsStringBuilder(int id);
    virtual ~IdsStringBuilder();
    virtual std::wstring getString();

private:
    int id;
};

#endif
