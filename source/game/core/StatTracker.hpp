#ifndef GAME_CORE_STATTRACKER_HPP
#define GAME_CORE_STATTRACKER_HPP

#include "game/core/StatsUID.hpp"
#include "game/core/UntypedStatTracker.hpp"

template <class T>
class StatTracker : public UntypedStatTracker
{
public:
    StatTracker(const StatsUID& statsUID, T initialValue);
    virtual ~StatTracker();

    T getValue() const { return mValue; }

protected:
    T mValue;
};

template <class T>
StatTracker<T>::StatTracker(const StatsUID& statsUID, T initialValue)
    : UntypedStatTracker(statsUID)
    , mValue(initialValue)
{
}

template <class T>
StatTracker<T>::~StatTracker()
{
}

#endif
