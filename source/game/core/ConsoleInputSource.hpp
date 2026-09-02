#ifndef GAME_CORE_CONSOLEINPUTSOURCE_HPP
#define GAME_CORE_CONSOLEINPUTSOURCE_HPP

#include <string>

class ConsoleInputSource
{
public:
    virtual ~ConsoleInputSource();
    virtual const std::wstring& getName() const = 0;
    virtual void sendMessage(const std::wstring& message) = 0;
    virtual bool canUseCommand(int permissionLevel, const std::wstring& commandName) const = 0;
};

#endif
