#ifndef GAME_CORE_BOXED_HPP
#define GAME_CORE_BOXED_HPP

#include <string>
#include <typeinfo>

class Boxed
{
public:
    Boxed();
    Boxed(const std::type_info* valueClass);
    virtual ~Boxed();

    virtual bool isA(const std::type_info* valueClass) const;
    virtual bool isA(const std::type_info& valueClass) const;
    virtual unsigned int hashOf() const = 0;
    virtual bool operator==(const Boxed* other) const = 0;
    virtual bool operator==(Boxed* other) const;
    virtual bool operator!=(const Boxed* other) const;
    virtual bool operator!=(Boxed* other) const;
    virtual void setValue(const Boxed* other) = 0;
    virtual std::wstring toString() const = 0;

protected:
    const std::type_info* mValueClass;
};

#endif
