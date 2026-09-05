#ifndef GAME_WORLD_LEVELRULES_HPP
#define GAME_WORLD_LEVELRULES_HPP

#include <string>
#include <vector>

class LevelRuleset;
class LevelGenerationOptions;

class LevelRules
{
public:
    enum ELevelRuleType
    {
        eLevelRuleType_Unknown = 0
    };

    class LevelRulesetGroup
    {
    public:
        std::vector<LevelRuleset*> rulesets;

        static bool SortMapRules(const LevelRulesetGroup* first, const LevelRulesetGroup* second);
    };

    LevelRules();
    void addLevelRule(const std::wstring& name, LevelRuleset* ruleset);
    void removeLevelRule(LevelRuleset* ruleset);
    void getRulesByType(ELevelRuleType type, std::vector<LevelRuleset*>* rules, bool requirePurchase);
    void populateRulesets();
    void getRulesByType(ELevelRuleType type, std::vector<LevelRulesetGroup*>* groups, bool requirePurchase);
    LevelGenerationOptions* getLevelGenFromUniqueId(ELevelRuleType type, int uniqueId) const;
    std::vector<LevelRuleset*>* getLevelRules();
    void getRulesExcludingType(ELevelRuleType type, std::vector<LevelRuleset*>* rules, bool requirePurchase);

private:
    std::vector<LevelRuleset*> m_levelRules;
    std::vector<LevelRulesetGroup*> m_ruleGroups;
};

#endif
