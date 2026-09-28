#ifndef GAME_CORE_INTEGERCONDITION_HPP
#define GAME_CORE_INTEGERCONDITION_HPP

#include <string>
#include <utility>
#include <vector>

#include "game/core/GameCondition.hpp"

class MasterGameMode;
class XmlObject;

enum EOperator
{
    EOperator_LessThan,
    EOperator_LessThanOrEquals,
    EOperator_GreaterThan,
    EOperator_GreaterThanOrEquals
};

class IntegerCondition : public GameCondition
{
public:
    IntegerCondition(const XmlObject& object);
    virtual ~IntegerCondition() {}

    void _init(const XmlObject& object, EOperator op);
    static std::pair<bool, EOperator> parseOperator(std::wstring name);

    virtual bool IsSatisfied(MasterGameMode* gameMode) const;
    virtual unsigned char GetMessage() const;
    virtual void AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const;
    virtual unsigned int GetThreshold(MasterGameMode* gameMode) const;
    virtual int GetValue(MasterGameMode* gameMode) const = 0;

protected:
    EOperator m_operator;
    int m_threshold;
    int m_option;
};

#endif
