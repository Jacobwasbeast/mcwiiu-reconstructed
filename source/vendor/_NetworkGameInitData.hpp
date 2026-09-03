#ifndef VENDOR__NETWORKGAMEINITDATA_HPP
#define VENDOR__NETWORKGAMEINITDATA_HPP

#include <vector>

namespace LevelRules
{
class LevelRulesetGroup;
}

class _NetworkGameInitData
{
public:
    _NetworkGameInitData();

    int m_value0;
    int m_value1;
    int m_value2;
    int m_value3;
    int m_value4;
    int m_value5;
    int m_value6;
    unsigned char m_flag0;
    int m_value7;
    unsigned char m_mode;
    int m_value8;
    int m_value9;
    int m_value10;
    std::vector<LevelRules::LevelRulesetGroup*> m_ruleGroups;
    unsigned char m_flag1;
    int m_value11;
    int m_value12;
    int m_value13;
    unsigned char m_flag2;
};

#endif
