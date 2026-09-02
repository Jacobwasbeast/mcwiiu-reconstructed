#ifndef GAME_WORLD_LEVELCONFLICTEXCEPTION_HPP
#define GAME_WORLD_LEVELCONFLICTEXCEPTION_HPP

#include <exception>
#include <string>

class LevelConflictException : public std::exception
{
public:
    LevelConflictException(const std::wstring& message);
    ~LevelConflictException();
};

#endif
